#ifndef __IM_DOMAIN_SERVICE_USER_SERVICE_HPP__
#define __IM_DOMAIN_SERVICE_USER_SERVICE_HPP__

#include <memory>

#include "common/result.hpp"
#include "dto/user_dto.hpp"
#include "http/http_session.hpp"
#include "model/user.hpp"
#include "model/user_settings.hpp"

namespace IM::domain::service {

class IUserService {
   public:
    using Ptr = std::shared_ptr<IUserService>;
    virtual ~IUserService() = default;

    // 加载用户信息
    virtual Result<model::User> LoadUserInfo(const uint64_t uid) = 0;

    //  更新用户密码
    virtual Result<void> UpdatePassword(const uint64_t uid, const std::string& old_password,
                                        const std::string& new_password) = 0;

    // 更新用户信息
    virtual Result<void> UpdateUserInfo(const uint64_t uid, const std::string& nickname,
                                        const std::string& avatar, const std::string& motto,
                                        const uint32_t gender, const std::string& birthday) = 0;

    // 更新手机号
    virtual Result<void> UpdateMobile(const uint64_t uid, const std::string& password,
                                      const std::string& new_mobile,
                                      const std::string& sms_code) = 0;

    // 更新邮箱
    virtual Result<void> UpdateEmail(const uint64_t uid, const std::string& password,
                                     const std::string& new_email,
                                     const std::string& email_code) = 0;

    // 判断手机号是否已注册
    virtual Result<model::User> GetUserByMobile(const std::string& mobile,
                                                const std::string& channel) = 0;

    // 判断邮箱是否已注册
    virtual Result<model::User> GetUserByEmail(const std::string& email,
                                               const std::string& channel) = 0;

    // 用户下线
    virtual Result<void> Offline(const uint64_t id) = 0;

    // 获取用户在线状态
    virtual Result<std::string> GetUserOnlineStatus(const uint64_t id) = 0;

    // 保存用户设置
    virtual Result<void> SaveConfigInfo(const uint64_t user_id, const std::string& theme_mode,
                                        const std::string& theme_bag_img,
                                        const std::string& theme_color,
                                        const std::string& notify_cue_tone,
                                        const std::string& keyboard_event_notify) = 0;

    // 加载用户设置
    virtual Result<model::UserSettings> LoadConfigInfo(const uint64_t user_id) = 0;

    // 加载用户信息
    virtual Result<dto::UserInfo> LoadUserInfoSimple(const uint64_t uid) = 0;

    // 鉴权用户
    virtual Result<model::User> Authenticate(const std::string& mobile, const std::string& password,
                                             const std::string& platform) = 0;

    // 记录登录日志
    virtual Result<void> LogLogin(const Result<model::User>& result, const std::string& platform,
                                  IM::http::HttpSession::ptr session) = 0;

    // 用户上线
    virtual Result<void> GoOnline(const uint64_t id) = 0;

    // 注册新用户
    virtual Result<model::User> Register(const std::string& nickname, const std::string& mobile,
                                         const std::string& password, const std::string& sms_code,
                                         const std::string& platform) = 0;

    // 找回密码
    virtual Result<model::User> Forget(const std::string& mobile, const std::string& new_password,
                                       const std::string& sms_code) = 0;
};

}  // namespace IM::domain::service

#endif  // __IM_DOMAIN_SERVICE_USER_SERVICE_HPP__