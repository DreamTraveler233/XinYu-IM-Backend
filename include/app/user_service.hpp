#ifndef __APP_SETTING_SERVICE_HPP__
#define __APP_SETTING_SERVICE_HPP__

#include <cstdint>
#include <optional>
#include <string>

#include "result.hpp"

namespace CIM::app {

class UserService {
   public:
    // 加载用户信息
    static UserResult LoadUserInfo(const uint64_t uid);

    //  更新用户密码
    static VoidResult UpdatePassword(const uint64_t uid, const std::string& old_password,
                                     const std::string& new_password);

    // 更新用户信息
    static VoidResult UpdateUserInfo(const uint64_t uid, const std::string& nickname,
                                     const std::string& avatar, const std::string& motto,
                                     const uint32_t gender, const std::string& birthday);

    // 更新手机号
    static VoidResult UpdateMobile(const uint64_t uid, const std::string& password,
                                   const std::string& new_mobile);

    // 判断手机号是否已注册
    static UserResult GetUserByMobile(const std::string& mobile, const std::string& channel);

    // 用户下线
    static VoidResult Offline(const uint64_t id);

    // 获取用户在线状态
    static StatusResult GetUserOnlineStatus(const uint64_t id);

    // 保存用户设置
    static VoidResult SaveConfigInfo(const uint64_t user_id, const std::string& theme_mode,
                                     const std::string& theme_bag_img,
                                     const std::string& theme_color,
                                     const std::string& notify_cue_tone,
                                     const std::string& keyboard_event_notify);

    // 加载用户设置
    static ConfigInfoResult LoadConfigInfo(const uint64_t user_id);

    // 加载用户信息
    static UserInfoResult LoadUserInfoSimple(const uint64_t uid);
};

}  // namespace CIM::app

#endif  // __SETTING_SERVICE_HPP__
