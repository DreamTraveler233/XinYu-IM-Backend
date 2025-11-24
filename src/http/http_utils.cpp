#include "http/http_utils.hpp"
#include "util/json_util.hpp"
#include "util/jwt_util.hpp"

namespace IM::http {

std::string Ok(const Json::Value& data) {
    return IM::JsonUtil::ToString(data);
}

std::string Error(const int code, const std::string& msg) {
    Json::Value root;
    root["code"] = code;
    root["message"] = msg;
    root["data"] = Json::nullValue;
    return IM::JsonUtil::ToString(root);
}

bool ParseBody(const std::string& body, Json::Value& out) {
    if (body.empty()) return false;
    if (!IM::JsonUtil::FromString(out, body)) return false;
    return out.isObject();
}

UidResult GetUidFromToken(IM::http::HttpRequest::ptr req, IM::http::HttpResponse::ptr res) {
    UidResult result;

    // 从请求头中提取 Token
    std::string header = req->getHeader("Authorization", "");
    std::string token;
    // 兼容 "Bearer <token>" 格式；严格校验前缀与长度，避免 substr 越界
    const std::string kBearer = "Bearer ";
    if (!header.empty() && header.rfind(kBearer, 0) == 0 && header.size() > kBearer.size()) {
        token = header.substr(kBearer.size());
    } else if (!header.empty() && header.find(' ') == std::string::npos) {
        // 兼容直接传裸 token 的情况（无前缀）
        token = header;
    }
    if (token.empty()) {
        result.code = 401;
        result.err = "未提供访问令牌！";
        return result;
    }

    // 验证 Token 的签名是否有效并提取用户 ID
    std::string uid_str;
    if (!IM::util::VerifyJwt(token, &uid_str)) {
        result.code = 401;
        result.err = "无效的访问令牌！";
        return result;
    }

    // 检查 Token 是否已过期
    if (IM::util::IsJwtExpired(token)) {
        result.code = 401;
        result.err = "访问令牌已过期！";
        return result;
    }

    result.data = std::stoull(uid_str);
    result.ok = true;
    return result;
}

IM::http::HttpStatus ToHttpStatus(const int code) {
    switch (code) {
        case 400:
            return IM::http::HttpStatus::BAD_REQUEST;  // 请求参数错误
        case 401:
            return IM::http::HttpStatus::UNAUTHORIZED;  // 未认证/Token无效
        case 403:
            return IM::http::HttpStatus::FORBIDDEN;  // 权限不足
        case 404:
            return IM::http::HttpStatus::NOT_FOUND;  // 资源不存在
        case 405:
            return IM::http::HttpStatus::METHOD_NOT_ALLOWED;  // 方法不允许
        case 406:
            return IM::http::HttpStatus::NOT_ACCEPTABLE;  // 不可接受
        case 408:
            return IM::http::HttpStatus::REQUEST_TIMEOUT;  // 请求超时
        case 409:
            return IM::http::HttpStatus::CONFLICT;  // 冲突（如重复）
        case 410:
            return IM::http::HttpStatus::GONE;  // 资源已删除
        case 413:
            return IM::http::HttpStatus::PAYLOAD_TOO_LARGE;  // 请求体过大
        case 415:
            return IM::http::HttpStatus::UNSUPPORTED_MEDIA_TYPE;  // 媒体类型不支持
        case 422:
            return IM::http::HttpStatus::UNPROCESSABLE_ENTITY;  // 语义错误
        case 429:
            return IM::http::HttpStatus::TOO_MANY_REQUESTS;  // 请求过多
        case 500:
            return IM::http::HttpStatus::INTERNAL_SERVER_ERROR;  // 服务端错误
        case 501:
            return IM::http::HttpStatus::NOT_IMPLEMENTED;  // 未实现
        case 502:
            return IM::http::HttpStatus::BAD_GATEWAY;  // 网关错误
        case 503:
            return IM::http::HttpStatus::SERVICE_UNAVAILABLE;  // 服务不可用
        case 504:
            return IM::http::HttpStatus::GATEWAY_TIMEOUT;  // 网关超时
        default:
            return IM::http::HttpStatus::INTERNAL_SERVER_ERROR;
    }
}

} // namespace IM::http
