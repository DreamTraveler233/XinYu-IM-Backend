#ifndef __IM_DOMAIN_SERVICE_UPLOAD_SERVICE_HPP__
#define __IM_DOMAIN_SERVICE_UPLOAD_SERVICE_HPP__

#include <memory>
#include <vector>

#include "common/result.hpp"
#include "model/media_file.hpp"
#include "model/upload_session.hpp"

namespace IM::domain::service {

class IMediaService {
   public:
    using Ptr = std::shared_ptr<IMediaService>;
    virtual ~IMediaService() = default;

    // 初始化分片上传
    virtual Result<model::UploadSession> InitMultipartUpload(const uint64_t user_id,
                                                             const std::string& file_name,
                                                             const uint64_t file_size) = 0;

    // 上传分片，返回是否合并完成（true 表示已合并并生成文件）
    virtual Result<bool> UploadPart(const std::string& upload_id, const uint32_t split_index,
                                    const uint32_t split_num, const std::string& data) = 0;

    // 单文件上传
    virtual Result<model::MediaFile> UploadFile(const uint64_t user_id,
                                                const std::string& file_name,
                                                const std::string& data) = 0;

    // 获取媒体文件信息
    virtual Result<model::MediaFile> GetMediaFile(const std::string& media_id) = 0;
    // 通过 upload_id 获取媒体文件信息（有时 API 需要直接查找）
    virtual Result<model::MediaFile> GetMediaFileByUploadId(const std::string& upload_id) = 0;

    // 初始化临时分片目录清理定时器（可安全调用多次）
    virtual void InitTempCleanupTimer() = 0;

   private:
    // 获取存储路径
    virtual std::string GetStoragePath(const std::string& file_name) = 0;

    // 获取临时路径
    virtual std::string GetTempPath(const std::string& upload_id) = 0;

    // 合并分片
    virtual Result<model::MediaFile> MergeParts(const model::UploadSession& session) = 0;
};

}  // namespace IM::domain::service

#endif  // __IM_DOMAIN_SERVICE_UPLOAD_SERVICE_HPP__