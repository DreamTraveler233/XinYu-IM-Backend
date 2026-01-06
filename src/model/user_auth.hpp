#ifndef __IM_MODEL_USER_AUTH_HPP__
#define __IM_MODEL_USER_AUTH_HPP__

#include <cstdint>
#include <ctime>
#include <string>

namespace IM::model {

/**
 * @brief 用户认证实体类 (对应数据库 tb_user_auth 表)
 */
struct UserAuth {
    uint64_t user_id = 0;                              // 用户ID (im_user.id)
    std::string password_hash;                         // 密码哈希（bcrypt/argon2 等）
    std::string password_algo = "PBKDF2-HMAC-SHA256";  // 哈希算法标识，默认为 PBKDF2-HMAC-SHA256
    int16_t password_version = 1;                      // 密码版本
    std::time_t last_reset_at = 0;                     // 最近重置时间，可空
    std::time_t created_at = 0;                        // 创建时间 (timestamp)
    std::time_t updated_at = 0;                        // 更新时间 (timestamp)
};

}  // namespace IM::model

#endif  // __IM_MODEL_USER_AUTH_HPP__
