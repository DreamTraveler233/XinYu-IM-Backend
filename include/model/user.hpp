#ifndef __IM_MODEL_USER_HPP__
#define __IM_MODEL_USER_HPP__

#include <cstdint>
#include <ctime>
#include <string>

namespace IM::model {

/**
 * @brief 用户实体类 (对应数据库 tb_user 表)
 */
struct User {
    uint64_t id = 0;                   // 用户ID（数据库自增ID）
    std::string mobile = "";           // 手机号（唯一，用于登录/找回）
    std::string email = "";            // 邮箱（唯一，可选，用于找回/通知）
    std::string nickname = "";         // 昵称
    std::string avatar = "";           // 头像URL，可选
    std::string avatar_media_id = "";  // 头像文件ID
    std::string motto = "";            // 个性签名，可选
    std::string birthday = "";         // 生日 YYYY-MM-DD，可选
    uint8_t gender = 0;                // 性别：0=未知 1=男 2=女
    std::string online_status = "N";   // 在线状态：N:离线 Y:在线
    std::time_t last_online_at = 0;    // 最后在线时间，可空
    uint8_t is_qiye = 0;               // 是否企业用户：0=否 1=是
    uint8_t is_robot = 0;              // 是否机器人账号：0=否 1=是
    uint8_t is_disabled = 0;           // 是否禁用：0=否 1=是
    std::time_t created_at = 0;        // 创建时间
    std::time_t updated_at = 0;        // 更新时间
};

}  // namespace IM::model

#endif  // __IM_MODEL_USER_HPP__
