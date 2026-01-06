#include "core/util/security_util.hpp"
#include "infra/module/crypto_module.hpp"
#include "core/util/hash_util.hpp"

namespace IM::util {

Result<std::string> DecryptPassword(const std::string& encrypted_password, std::string& out_plaintext) {
    Result<std::string> result;

    // Base64 解码
    std::string cipher_bin = IM::base64decode(encrypted_password);
    if (cipher_bin.empty()) {
        result.code = 400;
        result.err = "密码解码失败！";
        return result;
    }
    // 私钥解密
    auto cm = IM::CryptoModule::Get();
    if (!cm || !cm->isReady()) {
        result.code = 500;
        result.err = "密钥模块未加载！";
        return result;
    }
    if (!cm->PrivateDecrypt(cipher_bin, out_plaintext)) {
        result.code = 400;
        result.err = "密码解密失败！";
        return result;
    }
    result.ok = true;
    return result;
}

} // namespace IM::util
