#ifndef __IM_UTIL_JWT_UTIL_HPP__
#define __IM_UTIL_JWT_UTIL_HPP__

#include <string>
#include <cstdint>
#include "common/result.hpp"

namespace IM::util {

using TokenResult = IM::Result<std::string>;

// JWT 签发：返回签名后的 JWT 字符串
TokenResult SignJwt(const std::string& uid, uint32_t expires_in);

// JWT 校验：有效则返回 true 并输出 uid（字符串）
bool VerifyJwt(const std::string& token, std::string* out_uid = nullptr);

// JWT 是否过期
bool IsJwtExpired(const std::string& token);

} // namespace IM::util

#endif
