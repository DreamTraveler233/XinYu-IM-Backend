#include "app/media_service_impl.hpp"

#include <dirent.h>
#include <sys/stat.h>

#include <cmath>
#include <fstream>
#include <sstream>

#include "base/macro.hpp"
#include "common/result.hpp"
#include "config/config.hpp"
#include "system/env.hpp"

namespace IM::app {

static auto g_logger = IM_LOG_NAME("root");

// 配置项，实际项目中应从配置文件读取
static auto g_upload_base_dir = IM::Config::Lookup<std::string>(
    "media.upload_base_dir", std::string("data/uploads"), "base dir for uploaded media files");
static auto g_temp_base_dir = IM::Config::Lookup<std::string>(
    "media.temp_base_dir", std::string("data/uploads/tmp"), "temp dir for multipart uploads");
static auto g_shard_size_default = IM::Config::Lookup<uint32_t>(
    "media.shard_size_default", (uint32_t)(5 * 1024 * 1024), "default shard size in bytes");
static auto g_temp_cleanup_interval = IM::Config::Lookup<uint32_t>(
    "media.temp_cleanup_interval", (uint32_t)3600, "temp dir cleanup interval seconds");
static auto g_temp_retention_secs = IM::Config::Lookup<uint32_t>(
    "media.temp_retention_secs", (uint32_t)(24 * 3600), "temp dir retention seconds");
static IM::Timer::ptr g_temp_cleanup_timer;

static std::string GetResolvedUploadBaseDir() {
    auto base = g_upload_base_dir->getValue();
    return IM::EnvMgr::GetInstance()->getAbsoluteWorkPath(base);
}

static std::string GetResolvedTempBaseDir() {
    auto base = g_temp_base_dir->getValue();
    return IM::EnvMgr::GetInstance()->getAbsoluteWorkPath(base);
}

MediaServiceImpl::MediaServiceImpl(IM::domain::repository::IMediaRepository::Ptr media_repo)
    : m_media_repo(std::move(media_repo)) {}

std::string MediaServiceImpl::GetStoragePath(const std::string& file_name) {
    time_t now = time(0);
    struct tm t;
    localtime_r(&now, &t);
    char buf[64];
    strftime(buf, sizeof(buf), "/%Y/%m/%d/", &t);
    std::string date_path = buf;

    std::string ext;
    size_t pos = file_name.rfind('.');
    if (pos != std::string::npos) {
        ext = file_name.substr(pos);
    }

    // 生成唯一文件名
    std::string id = IM::md5(IM::random_string(32) + std::to_string(now));
    return GetResolvedUploadBaseDir() + date_path + id + ext;
}

std::string MediaServiceImpl::GetTempPath(const std::string& upload_id) {
    return GetResolvedTempBaseDir() + "/" + upload_id;
}

Result<IM::model::UploadSession> MediaServiceImpl::InitMultipartUpload(const uint64_t user_id,
                                                                       const std::string& file_name,
                                                                       const uint64_t file_size) {
    Result<IM::model::UploadSession> r;
    std::string out_upload_id = IM::md5(IM::random_string(32) + std::to_string(time(0)));
    uint32_t out_shard_size = g_shard_size_default->getValue();

    model::UploadSession session;
    session.upload_id = out_upload_id;
    session.user_id = user_id;
    session.file_name = file_name;
    session.file_size = file_size;
    session.shard_size = out_shard_size;
    session.shard_num = (uint32_t)std::ceil((double)file_size / out_shard_size);
    session.uploaded_count = 0;
    session.status = 0;
    session.temp_path = GetTempPath(out_upload_id);

    // 创建临时目录
    if (!IM::FSUtil::Mkdir(session.temp_path)) {
        IM_LOG_ERROR(g_logger) << "create temp dir failed: " << session.temp_path;
        r.code = 500;
        r.err = "create temp dir failed";
        return r;
    }

    std::string repo_err;
    if (!m_media_repo->CreateMediaSession(session, &repo_err)) {
        IM_LOG_ERROR(g_logger) << "create upload session failed: " << repo_err;
        r.code = 500;
        r.err = repo_err;
        return r;
    }
    r.ok = true;
    r.data = session;
    return r;
}

// 初始化临时目录清理定时器
void MediaServiceImpl::InitTempCleanupTimer() {
    if (g_temp_cleanup_timer) {
        return;
    }
    uint32_t interval = g_temp_cleanup_interval->getValue();
    // 周期性执行
    g_temp_cleanup_timer = IM::IOManager::GetThis()->addTimer(
        interval * 1000,
        []() {
            try {
                std::string temp_base = GetResolvedTempBaseDir();
                DIR* dir = opendir(temp_base.c_str());
                if (!dir) return;
                struct dirent* dp = nullptr;
                time_t now = time(0);
                while ((dp = readdir(dir)) != nullptr) {
                    if (dp->d_type != DT_DIR) continue;
                    if (!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, "..")) continue;
                    std::string child = temp_base + "/" + dp->d_name;
                    struct stat st;
                    if (lstat(child.c_str(), &st) == 0) {
                        if ((now - st.st_mtime) > (time_t)g_temp_retention_secs->getValue()) {
                            IM::FSUtil::Rm(child);
                        }
                    }
                }
                closedir(dir);
            } catch (...) {
                IM_LOG_ERROR(g_logger) << "error when cleaning temp upload dir";
            }
        },
        true);
}

Result<bool> MediaServiceImpl::UploadPart(const std::string& upload_id, uint32_t split_index,
                                          uint32_t split_num, const std::string& data) {
    Result<bool> r;
    model::UploadSession session;
    std::string repo_err;
    if (!m_media_repo->GetMediaSessionByUploadId(upload_id, session, &repo_err)) {
        r.code = 404;
        r.err = repo_err.empty() ? "upload session not found" : repo_err;
        return r;
    }

    if (session.status != 0) {
        r.code = 400;
        r.err = "upload session is not active";
        return r;
    }

    std::string part_path = session.temp_path + "/part_" + std::to_string(split_index);
    struct stat st;
    bool existed = (lstat(part_path.c_str(), &st) == 0);
    if (!existed) {
        // write to temp file then rename for atomicity
        std::string tmp_path = part_path + ".tmp";
        std::ofstream ofs(tmp_path, std::ios::binary | std::ios::trunc);
        if (!ofs) {
            IM_LOG_ERROR(g_logger) << "open part tmp file failed: " << tmp_path;
            r.code = 500;
            r.err = "write part file failed";
            return r;
        }
        ofs.write(data.c_str(), data.size());
        ofs.close();
        // rename tmp to final
        if (rename(tmp_path.c_str(), part_path.c_str()) != 0) {
            IM_LOG_ERROR(g_logger)
                << "rename part tmp file failed: " << tmp_path << " -> " << part_path;
            r.code = 500;
            r.err = "write part file failed";
            return r;
        }
    } else {
        // Part already exists: ignore re-upload of same part
        IM_LOG_DEBUG(g_logger) << "part already exists, ignore write: " << part_path;
    }

    // 更新已上传数量
    // 注意：这里简单的+1可能存在并发问题，如果前端并发上传分片。
    // 严谨做法是检查该分片是否已存在，或者使用原子操作/锁。
    // 简化起见，假设前端串行或数据库update原子性（但uploaded_count是累加）。
    // 更好的方式是：不依赖uploaded_count判断完成，而是检查所有分片文件是否存在。
    // 这里我们先更新uploaded_count。
    // Recompute current part count and update uploaded_count atomically
    std::vector<std::string> files_after;
    IM::FSUtil::ListAllFile(files_after, session.temp_path, "");
    int part_count_after = 0;
    for (const auto& f : files_after) {
        if (f.find("part_") != std::string::npos) part_count_after++;
    }
    if (!m_media_repo->UpdateUploadedCount(upload_id, (uint32_t)part_count_after, &repo_err)) {
        r.code = 500;
        r.err = repo_err.empty() ? "update uploaded count failed" : repo_err;
        return r;
    }

    // 检查是否所有分片都已上传
    // 重新获取session以获得最新的uploaded_count
    if (!m_media_repo->GetMediaSessionByUploadId(upload_id, session, &repo_err)) {
        r.code = 500;
        r.err = repo_err;
        return r;
    }

    // 检查本地文件数量是否足够
    // 这里简化逻辑：如果 split_index == split_num - 1 (最后一个分片) 且 uploaded_count >= shard_num
    // 或者每次都检查文件数。
    // 为了稳健，我们检查目录下文件数。
    std::vector<std::string> files;
    IM::FSUtil::ListAllFile(files, session.temp_path, "");
    // 过滤出 part_ 开头的文件
    int part_count = 0;
    for (const auto& f : files) {
        if (f.find("part_") != std::string::npos) {
            part_count++;
        }
    }

    if (part_count == (int)session.shard_num) {
        auto merge_res = MergeParts(session);
        if (!merge_res.ok) {
            r.code = merge_res.code;
            r.err = merge_res.err;
            return r;
        }
        // merged and created file
        r.ok = true;
        r.data = true;
        return r;
    }

    r.ok = true;
    r.data = false;
    return r;
}

Result<IM::model::MediaFile> MediaServiceImpl::MergeParts(const model::UploadSession& session) {
    Result<IM::model::MediaFile> r;
    std::string final_path = GetStoragePath(session.file_name);
    std::string dir = IM::FSUtil::Dirname(final_path);
    if (!IM::FSUtil::Mkdir(dir)) {
        r.code = 500;
        r.err = "create storage dir failed";
        return r;
    }

    std::ofstream ofs(final_path, std::ios::binary | std::ios::trunc);
    if (!ofs) {
        r.code = 500;
        r.err = "create final file failed";
        return r;
    }

    for (uint32_t i = 0; i < session.shard_num; ++i) {
        std::string part_path = session.temp_path + "/part_" + std::to_string(i);
        std::ifstream ifs(part_path, std::ios::binary);
        if (!ifs) {
            r.code = 500;
            r.err = "read part file failed: " + std::to_string(i);
            return r;
        }
        ofs << ifs.rdbuf();
        ifs.close();
    }
    ofs.close();

    // 创建 MediaFile 记录
    model::MediaFile media;
    media.id = IM::md5(IM::random_string(32));  // 生成新的 media id
    media.upload_id = session.upload_id;
    media.user_id = session.user_id;
    media.file_name = session.file_name;
    media.file_size = session.file_size;
    // mime 推断略，默认为空或根据后缀
    media.storage_type = 1;  // Local
    media.storage_path = final_path;
    // url 生成：假设有一个静态文件服务映射 /media/ -> data/uploads/
    // 这里需要根据实际部署配置 URL 前缀
    // 假设 Nginx 配置了 /media/ 指向 data/uploads/
    // final_path 是 data/uploads/2023/11/21/xxx.png
    // url 应该是 /media/2023/11/21/xxx.png
    // 简单替换 data/uploads 为 /media
    std::string url_path = final_path;
    const std::string base_dir = GetResolvedUploadBaseDir();
    if (url_path.find(base_dir) == 0) {
        url_path.replace(0, base_dir.length(), "/media");
    }
    media.url = url_path;
    media.status = 1;

    std::string repo_err;
    if (!m_media_repo->CreateMediaFile(media, &repo_err)) {
        r.code = 500;
        r.err = repo_err;
        return r;
    }

    // 更新 session 状态
    m_media_repo->UpdateMediaSessionStatus(session.upload_id, 1, nullptr);

    // 清理临时文件
    IM::FSUtil::Rm(session.temp_path);
    r.ok = true;
    r.data = media;
    return r;
}

Result<IM::model::MediaFile> MediaServiceImpl::UploadFile(uint64_t user_id,
                                                          const std::string& file_name,
                                                          const std::string& data) {
    Result<IM::model::MediaFile> r;
    std::string final_path = GetStoragePath(file_name);
    std::string dir = IM::FSUtil::Dirname(final_path);
    if (!IM::FSUtil::Mkdir(dir)) {
        r.code = 500;
        r.err = "create storage dir failed";
        return r;
    }

    std::ofstream ofs(final_path, std::ios::binary | std::ios::trunc);
    if (!ofs) {
        r.code = 500;
        r.err = "write file failed";
        return r;
    }
    ofs.write(data.c_str(), data.size());
    ofs.close();

    IM::model::MediaFile out_media;
    out_media.id = IM::md5(IM::random_string(32));
    out_media.user_id = user_id;
    out_media.file_name = file_name;
    out_media.file_size = data.size();
    out_media.storage_type = 1;
    out_media.storage_path = final_path;

    std::string url_path = final_path;
    const std::string base_dir2 = GetResolvedUploadBaseDir();
    if (url_path.find(base_dir2) == 0) {
        url_path.replace(0, base_dir2.length(), "/media");
    }
    out_media.url = url_path;
    out_media.status = 1;

    std::string repo_err;
    if (!m_media_repo->CreateMediaFile(out_media, &repo_err)) {
        r.code = 500;
        r.err = repo_err;
        return r;
    }
    r.ok = true;
    r.data = out_media;
    return r;
}

Result<IM::model::MediaFile> MediaServiceImpl::GetMediaFile(const std::string& media_id) {
    Result<IM::model::MediaFile> r;
    IM::model::MediaFile media;
    std::string repo_err;
    if (!m_media_repo->GetMediaFileById(media_id, media, &repo_err)) {
        r.code = 404;
        r.err = repo_err.empty() ? "media not found" : repo_err;
        return r;
    }
    r.ok = true;
    r.data = media;
    return r;
}

Result<IM::model::MediaFile> MediaServiceImpl::GetMediaFileByUploadId(
    const std::string& upload_id) {
    Result<IM::model::MediaFile> r;
    IM::model::MediaFile media;
    std::string repo_err;
    if (!m_media_repo->GetMediaFileByUploadId(upload_id, media, &repo_err)) {
        r.code = 404;
        r.err = repo_err.empty() ? "media not found by upload id" : repo_err;
        return r;
    }
    r.ok = true;
    r.data = media;
    return r;
}

}  // namespace IM::app