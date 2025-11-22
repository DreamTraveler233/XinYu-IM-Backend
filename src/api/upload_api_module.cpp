#include "api/upload_api_module.hpp"

#include <cctype>

#include "app/media_service.hpp"
#include "common/common.hpp"
#include "http/http_server.hpp"
#include "system/application.hpp"
#include "util/json_util.hpp"
#include "util/string_util.hpp"
#include <multipart_parser.h>
#include <algorithm>
#include <cstring>

namespace IM::api {

static auto g_logger = IM_LOG_NAME("root");

struct MultipartPart {
    std::string name;
    std::string filename;
    std::string content_type;
    std::string data;
};

// 使用 multipart-parser-c 库解析 multipart/form-data
// 该实现仍然一次性解析到内存（保持与之前行为一致），但使用成熟解析库提
// 升解析鲁棒性。长期推荐将解析改为流式处理。
struct MPParserContext {
    std::vector<MultipartPart> *parts;
    MultipartPart current;
    std::string last_header_field;
    std::string last_header_value;
};

static int mp_on_part_data_begin(multipart_parser *p) {
    MPParserContext *c = (MPParserContext *)multipart_parser_get_data(p);
    c->current = MultipartPart();
    c->last_header_field.clear();
    c->last_header_value.clear();
    return 0;
}

static int mp_on_header_field(multipart_parser *p, const char *at, size_t len) {
    MPParserContext *c = (MPParserContext *)multipart_parser_get_data(p);
    c->last_header_field.append(at, len);
    return 0;
}

static int mp_on_header_value(multipart_parser *p, const char *at, size_t len) {
    MPParserContext *c = (MPParserContext *)multipart_parser_get_data(p);
    c->last_header_value.append(at, len);

    std::string key = c->last_header_field;
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    std::string val = c->last_header_value;
    if (key.find("content-disposition") != std::string::npos) {
        size_t pos_name = val.find("name=");
        if (pos_name != std::string::npos) {
            size_t start = val.find('"', pos_name);
            if (start != std::string::npos) {
                size_t end = val.find('"', start + 1);
                if (end != std::string::npos) {
                    c->current.name = val.substr(start + 1, end - (start + 1));
                }
            } else {
                size_t eq = pos_name + 5;
                size_t end = val.find_first_of("; ", eq);
                if (end != std::string::npos) c->current.name = val.substr(eq, end - eq);
            }
        }
        size_t pos_file = val.find("filename=");
        if (pos_file != std::string::npos) {
            size_t start = val.find('"', pos_file);
            if (start != std::string::npos) {
                size_t end = val.find('"', start + 1);
                if (end != std::string::npos) {
                    c->current.filename = val.substr(start + 1, end - (start + 1));
                }
            } else {
                size_t eq = pos_file + 9;
                size_t end = val.find_first_of("; ", eq);
                if (end != std::string::npos) c->current.filename = val.substr(eq, end - eq);
            }
        }
    } else if (key.find("content-type") != std::string::npos) {
        c->current.content_type = val;
    }
    return 0;
}

static int mp_on_part_data(multipart_parser *p, const char *at, size_t len) {
    MPParserContext *c = (MPParserContext *)multipart_parser_get_data(p);
    c->current.data.append(at, len);
    return 0;
}

static int mp_on_part_data_end(multipart_parser *p) {
    MPParserContext *c = (MPParserContext *)multipart_parser_get_data(p);
    if (c->current.name.empty() && !c->current.filename.empty()) {
        c->current.name = "file";
    }
    c->parts->push_back(c->current);
    c->current = MultipartPart();
    c->last_header_field.clear();
    c->last_header_value.clear();
    return 0;
}

static int mp_on_body_end(multipart_parser *p) { return 0; }

static bool ParseMultipart(const std::string &body, const std::string &boundary,
                           std::vector<MultipartPart> &parts) {
    MPParserContext ctx;
    ctx.parts = &parts;

    multipart_parser_settings settings;
    memset(&settings, 0, sizeof(settings));
    settings.on_part_data_begin = mp_on_part_data_begin;
    settings.on_header_field = mp_on_header_field;
    settings.on_header_value = mp_on_header_value;
    settings.on_part_data = mp_on_part_data;
    settings.on_part_data_end = mp_on_part_data_end;
    settings.on_body_end = mp_on_body_end;

    std::string bound = boundary;  // the parser expects boundary string without leading '--'
    multipart_parser *parser = multipart_parser_init(bound.c_str(), &settings);
    if (!parser) return false;
    multipart_parser_set_data(parser, &ctx);
    size_t parsed = multipart_parser_execute(parser, body.c_str(), body.size());
    (void)parsed;
    multipart_parser_free(parser);
    return true;
}

// 从 Content-Type 中提取 boundary
static std::string GetBoundary(const std::string& content_type) {
    size_t pos = content_type.find("boundary=");
    if (pos == std::string::npos) return "";
    std::string val = content_type.substr(pos + 9);
    // 去除首尾空白
    while (!val.empty() && isspace((unsigned char)val.front())) val.erase(val.begin());
    while (!val.empty() && isspace((unsigned char)val.back())) val.pop_back();
    // 若存在则去除两端引号
    if (!val.empty() && val.front() == '"' && val.back() == '"') {
        val = val.substr(1, val.size() - 2);
    }
    return val;
}

UploadApiModule::UploadApiModule() : Module("api.upload", "0.1.0", "builtin") {}
bool UploadApiModule::onServerReady() {
    std::vector<IM::TcpServer::ptr> httpServers;
    if (!IM::Application::GetInstance()->getServer("http", httpServers)) {
        IM_LOG_WARN(g_logger) << "no http servers found when registering upload routes";
        return true;
    }

    // 初始化媒体服务的临时上传清理定时器
    IM::app::MediaService::InitTempCleanupTimer();

    for (auto& s : httpServers) {
        auto http = std::dynamic_pointer_cast<IM::http::HttpServer>(s);
        if (!http) continue;
        auto dispatch = http->getServletDispatch();

        dispatch->addServlet("/api/v1/upload/init-multipart", [](IM::http::HttpRequest::ptr req,
                                                                 IM::http::HttpResponse::ptr res,
                                                                 IM::http::HttpSession::ptr) {
            res->setHeader("Content-Type", "application/json");
            Json::Value body;
            std::string file_name;
            uint64_t file_size = 0;

            if (ParseBody(req->getBody(), body)) {
                file_name = IM::JsonUtil::GetString(body, "file_name");
                file_size = IM::JsonUtil::GetUint64(body, "file_size");
            }

            if (file_name.empty() || file_size == 0) {
                res->setStatus(ToHttpStatus(400));
                res->setBody(Error(400, "invalid params"));
                return 0;
            }

            auto uid_ret = GetUidFromToken(req, res);
            if (!uid_ret.ok) {
                res->setStatus(ToHttpStatus(uid_ret.code));
                res->setBody(Error(uid_ret.code, uid_ret.err));
                return 0;
            }

            std::string upload_id;
            uint32_t shard_size = 0;
            std::string err;
            if (!IM::app::MediaService::InitMultipartUpload(uid_ret.data, file_name, file_size,
                                                            upload_id, shard_size, &err)) {
                res->setStatus(ToHttpStatus(500));
                res->setBody(Error(500, err));
                return 0;
            }

            Json::Value data;
            data["upload_id"] = upload_id;
            data["shard_size"] = shard_size;
            res->setBody(Ok(data));
            return 0;
        });

        dispatch->addServlet("/api/v1/upload/multipart", [](IM::http::HttpRequest::ptr req,
                                                            IM::http::HttpResponse::ptr res,
                                                            IM::http::HttpSession::ptr) {
            res->setHeader("Content-Type", "application/json");

            // 解析 multipart/form-data
            std::string boundary = GetBoundary(req->getHeader("Content-Type", ""));
            if (boundary.empty()) {
                res->setStatus(ToHttpStatus(400));
                res->setBody(Error(400, "missing boundary"));
                return 0;
            }

            std::vector<MultipartPart> parts;
            if (!ParseMultipart(req->getBody(), boundary, parts)) {
                res->setStatus(ToHttpStatus(400));
                res->setBody(Error(400, "parse multipart failed"));
                return 0;
            }

            IM_LOG_INFO(g_logger) << "Parsed multipart parts count=" << parts.size();
            for (const auto& p : parts) {
                IM_LOG_DEBUG(g_logger)
                    << "part name=" << p.name << " filename=" << p.filename
                    << " content_type=" << p.content_type << " size=" << p.data.size();
            }

            std::string upload_id;
            uint32_t split_index = 0;
            uint32_t split_num = 0;
            std::string file_data;

            for (const auto& p : parts) {
                if (p.name == "upload_id")
                    upload_id = p.data;
                else if (p.name == "split_index")
                    split_index = std::stoul(p.data);
                else if (p.name == "split_num")
                    split_num = std::stoul(p.data);
                else if (p.name == "file")
                    file_data = p.data;
                // 如果 part 没有显式 name='file'，但具有 filename，则回退使用该 part
                else if (p.filename.size() > 0 && file_data.empty())
                    file_data = p.data;
            }

            if (upload_id.empty() || file_data.empty()) {
                IM_LOG_WARN(g_logger)
                    << "multipart upload missing param upload_id?=" << upload_id.empty()
                    << " file?=" << file_data.empty() << " parts_count=" << parts.size();
                res->setStatus(ToHttpStatus(400));
                res->setBody(Error(400, "missing params"));
                return 0;
            }

            // 鉴权
            auto uid_ret = GetUidFromToken(req, res);
            if (!uid_ret.ok) {
                res->setStatus(ToHttpStatus(uid_ret.code));
                res->setBody(Error(uid_ret.code, uid_ret.err));
                return 0;
            }

            std::string err;
            bool completed = IM::app::MediaService::UploadPart(upload_id, split_index, split_num,
                                                               file_data, &err);
            if (!err.empty()) {
                res->setStatus(ToHttpStatus(500));
                res->setBody(Error(500, err));
                return 0;
            }

            Json::Value data;
            data["is_completed"] = completed;
            if (completed) {
                IM::dao::MediaFile media;
                if (IM::app::MediaService::GetMediaFile(
                        IM::dao::MediaDAO::GetByUploadId(upload_id, media, nullptr) ? media.id : "",
                        media)) {
                    data["file_id"] = media.id;
                    data["url"] = media.url;
                }
            }
            res->setBody(Ok(data));
            return 0;
        });

        dispatch->addServlet("/api/v1/upload/media-file", [](IM::http::HttpRequest::ptr req,
                                                             IM::http::HttpResponse::ptr res,
                                                             IM::http::HttpSession::ptr) {
            res->setHeader("Content-Type", "application/json");

            std::string boundary = GetBoundary(req->getHeader("Content-Type", ""));
            if (boundary.empty()) {
                res->setStatus(ToHttpStatus(400));
                res->setBody(Error(400, "missing boundary"));
                return 0;
            }

            std::vector<MultipartPart> parts;
            if (!ParseMultipart(req->getBody(), boundary, parts)) {
                res->setStatus(ToHttpStatus(400));
                res->setBody(Error(400, "parse multipart failed"));
                return 0;
            }

            std::string file_data;
            std::string file_name;
            std::string file_content_type;

            for (const auto& p : parts) {
                if (p.name == "file") {
                    file_data = p.data;
                    file_name = p.filename;
                    file_content_type = p.content_type;
                    break;
                }
                // 回退：若没有显式 name='file'，则使用第一个看起来是文件的 part
                if (p.filename.size() > 0 && file_data.empty()) {
                    file_data = p.data;
                    file_name = p.filename;
                    file_content_type = p.content_type;
                    // 此处不提前 break，优先使用显式名为 'file' 的 part
                }
            }

            if (file_data.empty()) {
                IM_LOG_WARN(g_logger)
                    << "no file part found in multipart upload, parts_count=" << parts.size();
                res->setStatus(ToHttpStatus(400));
                res->setBody(Error(400, "missing file"));
                return 0;
            }
            if (file_name.empty()) file_name = "unknown";

            auto uid_ret = GetUidFromToken(req, res);
            if (!uid_ret.ok) {
                res->setStatus(ToHttpStatus(uid_ret.code));
                res->setBody(Error(uid_ret.code, uid_ret.err));
                return 0;
            }

            // 验证 MIME 类型（如头像上传）：仅允许图片
            std::vector<std::string> allowed_mimes = {"image/png", "image/jpg", "image/jpeg",
                                                      "image/webp", "image/gif"};
            bool ok_mime = false;
            if (!file_content_type.empty()) {
                // 在任何 ';' 参数之前提取基础 MIME 类型（例如 'image/jpeg; charset=UTF-8'）
                size_t semicolon = file_content_type.find(';');
                std::string base_mime = file_content_type;
                if (semicolon != std::string::npos)
                    base_mime = file_content_type.substr(0, semicolon);
                // 去除首尾空白
                while (!base_mime.empty() && base_mime.front() == ' ')
                    base_mime.erase(base_mime.begin());
                while (!base_mime.empty() && base_mime.back() == ' ') base_mime.pop_back();
                for (const auto& m : allowed_mimes) {
                    if (base_mime == m) {
                        ok_mime = true;
                        break;
                    }
                }
            }
            // 回退到扩展名检查
            if (!ok_mime) {
                std::string lower = file_name;
                for (auto& c : lower) c = std::tolower(c);
                if (lower.size() > 4) {
                    if (lower.find(".png") != std::string::npos ||
                        lower.find(".jpg") != std::string::npos ||
                        lower.find(".jpeg") != std::string::npos ||
                        lower.find(".webp") != std::string::npos ||
                        lower.find(".gif") != std::string::npos) {
                        ok_mime = true;
                    }
                }
            }

            if (!ok_mime) {
                res->setStatus(ToHttpStatus(400));
                res->setBody(Error(400, "invalid file type, only images allowed"));
                return 0;
            }

            IM::dao::MediaFile media;
            std::string err;
            if (!IM::app::MediaService::UploadFile(uid_ret.data, file_name, file_data, media,
                                                   &err)) {
                res->setStatus(ToHttpStatus(500));
                res->setBody(Error(500, err));
                return 0;
            }

            Json::Value data;
            data["id"] = media.id;
            data["src"] = media.url;
            res->setBody(Ok(data));
            return 0;
        });
    }
    return true;
}

}  // namespace IM::api
