#ifndef __IM_UTIL_SECURITY_UTIL_HPP__
#define __IM_UTIL_SECURITY_UTIL_HPP__

#include <string>
#include "common/result.hpp"

namespace IM::util {

// 将密码解密成明文
Result<std::string> DecryptPassword(const std::string& encrypted_password, std::string& out_plaintext);

} // namespace IM::util

#endif
