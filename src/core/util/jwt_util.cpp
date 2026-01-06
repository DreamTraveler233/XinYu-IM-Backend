#include "core/util/jwt_util.hpp"
#include <jwt-cpp/jwt.h>
#include "core/base/macro.hpp"
#include "common/config/config.hpp"
#include <chrono>

namespace IM::util {

static auto g_logger = IM_LOG_NAME("system");

// JWT签名密钥
static auto g_jwt_secret = IM::Config::Lookup<std::string>(
    "auth.jwt.secret", std::string("dev-secret"), "jwt hmac secret");
// JWT签发者
static auto g_jwt_issuer =
    IM::Config::Lookup<std::string>("auth.jwt.issuer", std::string("auth-service"), "jwt issuer");

TokenResult SignJwt(const std::string& uid, uint32_t expires_in) {
    TokenResult result;
    auto now = std::chrono::system_clock::now();
    auto exp = now + std::chrono::seconds(expires_in);
    try {
        result.data = jwt::create()
                          .set_type("JWS")
                          .set_issuer(g_jwt_issuer->getValue())
                          .set_issued_at(now)
                          .set_expires_at(exp)
                          .set_subject(uid)
                          .set_payload_claim("uid", jwt::claim(uid))
                          .sign(jwt::algorithm::hs256{g_jwt_secret->getValue()});
    } catch (const std::exception& e) {
        IM_LOG_ERROR(g_logger) << result.err;
        result.code = 500;
        result.err = "令牌签名失败！";
        return result;
    }

    result.ok = true;
    return result;
}

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
        IM_LOG_WARN(g_logger) << "jwt verify failed: " << e.what();
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
        IM_LOG_WARN(g_logger) << "jwt decode failed: " << e.what();
    }
    return false;
}

} // namespace IM::util
