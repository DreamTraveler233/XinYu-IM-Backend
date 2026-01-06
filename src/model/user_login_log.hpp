#ifndef __IM_MODEL_USER_LOGIN_LOG_HPP__
#define __IM_MODEL_USER_LOGIN_LOG_HPP__

#include <cstdint>
#include <ctime>
#include <string>

namespace IM::model {

/**
 * @brief 用户登录日志实体类 (对应数据库 tb_user_login_log 表)
 */
struct UserLoginLog {
    uint64_t id = 0;              // 主键ID
    uint64_t user_id = 0;         // 用户ID
    std::string mobile = "";      // 尝试登录的手机号，可空
    std::string platform = "";    // 登录平台
    std::string ip = "";          // 登录IP
    std::string address = "";     // IP归属地，可空
    std::string user_agent = "";  // UA/设备指纹，可空
    int8_t success = 0;           // 是否成功：1=成功 0=失败
    std::string reason = "";      // 失败原因，可空
    std::time_t created_at = 0;   // 记录时间
};

}  // namespace IM::model

#endif  // __IM_MODEL_USER_LOGIN_LOG_HPP__