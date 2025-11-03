#include "common/common.hpp"

#include <jwt-cpp/jwt.h>

#include "config.hpp"
#include "macro.hpp"
#include "net/tcp_server.hpp"
#include "util/json_util.hpp"

namespace CIM {

static auto g_logger = CIM_LOG_NAME("system");

// JWT签名密钥
static auto g_jwt_secret = CIM::Config::Lookup<std::string>(
    "auth.jwt.secret", std::string("dev-secret"), "jwt hmac secret");
// JWT签发者
static auto g_jwt_issuer =
    CIM::Config::Lookup<std::string>("auth.jwt.issuer", std::string("auth-service"), "jwt issuer");

std::string Ok(const Json::Value& data) {
    return CIM::JsonUtil::ToString(data);
}

std::string Error(int code, const std::string& msg) {
    Json::Value root;
    root["code"] = code;
    root["message"] = msg;
    return CIM::JsonUtil::ToString(root);
}

bool ParseBody(const std::string& body, Json::Value& out) {
    if (body.empty()) return false;
    if (!CIM::JsonUtil::FromString(out, body)) return false;
    return out.isObject();
}

std::string SignJwt(const std::string& uid, uint32_t expires_in) {
    auto now = std::chrono::system_clock::now();
    auto exp = now + std::chrono::seconds(expires_in);
    return jwt::create()
        .set_type("JWS")
        .set_issuer(g_jwt_issuer->getValue())
        .set_issued_at(now)
        .set_expires_at(exp)
        .set_subject(uid)
        .set_payload_claim("uid", jwt::claim(uid))
        .sign(jwt::algorithm::hs256{g_jwt_secret->getValue()});
}

/**
 * @brief 验证JWT令牌的有效性
 * @param token[in] 待验证的JWT令牌字符串
 * @param out_uid[out] 如果验证成功且该参数非空，则输出令牌中的用户ID
 * @return 验证成功返回true，否则返回false
 * 
 * 该函数使用HS256算法和预设的密钥来验证JWT令牌，
 * 同时检查签发者信息是否匹配。如果令牌有效且包含
 * uid声明，则将其写入out_uid参数中。
 */
bool VerifyJwt(const std::string& token, std::string* out_uid) {
    try {
        auto dec = jwt::decode(token);
        auto verifier = jwt::verify()
                            .allow_algorithm(jwt::algorithm::hs256{g_jwt_secret->getValue()})
                            .with_issuer(g_jwt_issuer->getValue());
        verifier.verify(dec);
        if (out_uid) {
            if (dec.has_payload_claim("uid")) {
                *out_uid = dec.get_payload_claim("uid").as_string();
            } else {
                *out_uid = "";
            }
        }
        return true;
    } catch (const std::exception& e) {
        CIM_LOG_WARN(g_logger) << "jwt verify failed: " << e.what();
        return false;
    }
}

bool IsJwtExpired(const std::string& token) {
    try {
        auto dec = jwt::decode(token);
        if (dec.has_expires_at()) {
            auto exp = dec.get_expires_at();
            return exp < std::chrono::system_clock::now();
        }
    } catch (const std::exception& e) {
        CIM_LOG_WARN(g_logger) << "jwt decode failed: " << e.what();
    }
    return false;
}

}  // namespace CIM
