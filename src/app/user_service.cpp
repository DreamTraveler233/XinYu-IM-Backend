#include "app/user_service.hpp"

#include "app/media_service.hpp"
#include "base/macro.hpp"
#include "common/common.hpp"
#include "dao/media_dao.hpp"
#include "dao/user_auth_dao.hpp"
#include "dao/user_dao.hpp"
#include "other/crypto_module.hpp"
#include "util/hash_util.hpp"
#include "util/password.hpp"

namespace IM::app {

static auto g_logger = IM_LOG_NAME("system");
UserResult UserService::LoadUserInfo(const uint64_t uid) {
    UserResult result;
    std::string err;

    if (!IM::dao::UserDAO::GetById(uid, result.data, &err)) {
        if (!err.empty()) {
            IM_LOG_ERROR(g_logger) << "LoadUserInfo GetById failed, uid=" << uid << ", err=" << err;
            result.code = 500;
            result.err = "加载用户信息失败";
            return result;
        }
    }

    if (!result.data.avatar.empty() && result.data.avatar.length() == 32) {
        bool is_id = true;
        for (char c : result.data.avatar) {
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                is_id = false;
                break;
            }
        }
        if (is_id) {
            IM::dao::MediaFile media;
            if (IM::app::MediaService::GetMediaFile(result.data.avatar, media)) {
                result.data.avatar = media.url;
            }
        }
    }

    result.ok = true;
    return result;
}

VoidResult UserService::UpdateUserInfo(const uint64_t uid, const std::string& nickname,
                                       const std::string& avatar, const std::string& motto,
                                       const uint32_t gender, const std::string& birthday) {
    VoidResult result;
    std::string err;

    if (!IM::dao::UserDAO::UpdateUserInfo(uid, nickname, avatar, motto, gender, birthday, &err)) {
        IM_LOG_ERROR(g_logger) << "UpdateUserInfo failed, uid=" << uid << ", err=" << err;
        result.code = 500;
        result.err = "更新用户信息失败";
        return result;
    }

    result.ok = true;
    return result;
}

VoidResult UserService::UpdateMobile(const uint64_t uid, const std::string& password,
                                     const std::string& new_mobile) {
    VoidResult result;
    std::string err;

    // 解密前端传入的登录密码
    std::string decrypted_password;
    auto dec_res = IM::DecryptPassword(password, decrypted_password);
    if (!dec_res.ok) {
        result.code = dec_res.code;
        result.err = dec_res.err;
        return result;
    }

    // 加载当前用户信息用于校验
    IM::dao::User user;
    if (!IM::dao::UserDAO::GetById(uid, user, &err)) {
        if (!err.empty()) {
            IM_LOG_ERROR(g_logger) << "UpdateMobile GetById failed, uid=" << uid << ", err=" << err;
            result.code = 404;
            result.err = "加载用户信息失败";
            return result;
        }
    }

    // 加载用户密码并验证
    IM::dao::UserAuth ua;
    if (!IM::dao::UserAuthDao::GetByUserId(uid, ua, &err)) {
        if (!err.empty()) {
            IM_LOG_ERROR(g_logger)
                << "UpdateMobile GetByUserId failed, uid=" << uid << ", err=" << err;
            result.code = 500;
            result.err = "加载用户认证信息失败";
            return result;
        }
    }

    if (!IM::util::Password::Verify(decrypted_password, ua.password_hash)) {
        result.code = 403;
        result.err = "密码错误";
        return result;
    }

    if (user.mobile == new_mobile) {
        result.code = 400;
        result.err = "新手机号不能与原手机号相同";
        return result;
    }

    // 检查新手机号是否已被其他账户使用
    IM::dao::User other_user;
    if (IM::dao::UserDAO::GetByMobile(new_mobile, other_user, &err)) {
        if (other_user.id != uid) {
            result.code = 400;
            result.err = "新手机号已被使用";
            return result;
        }
    }

    if (!IM::dao::UserDAO::UpdateMobile(uid, new_mobile, &err)) {
        if (!err.empty()) {
            IM_LOG_ERROR(g_logger)
                << "UpdateMobile UpdateMobile failed, uid=" << uid << ", err=" << err;
            result.code = 500;
            result.err = "更新手机号失败";
            return result;
        }
    }

    result.ok = true;
    return result;
}

VoidResult UserService::UpdateEmail(const uint64_t uid, const std::string& password,
                                    const std::string& new_email) {
    VoidResult result;
    std::string err;

    // 解密前端传入的登录密码
    std::string decrypted_password;
    auto dec_res = IM::DecryptPassword(password, decrypted_password);
    if (!dec_res.ok) {
        result.code = dec_res.code;
        result.err = dec_res.err;
        return result;
    }

    // 加载当前用户信息用于校验
    IM::dao::User user;
    if (!IM::dao::UserDAO::GetById(uid, user, &err)) {
        if (!err.empty()) {
            IM_LOG_ERROR(g_logger) << "UpdateEmail GetById failed, uid=" << uid << ", err=" << err;
            result.code = 404;
            result.err = "加载用户信息失败";
            return result;
        }
    }

    // 加载用户密码并验证
    IM::dao::UserAuth ua;
    if (!IM::dao::UserAuthDao::GetByUserId(uid, ua, &err)) {
        if (!err.empty()) {
            IM_LOG_ERROR(g_logger)
                << "UpdateEmail GetByUserId failed, uid=" << uid << ", err=" << err;
            result.code = 500;
            result.err = "加载用户认证信息失败";
            return result;
        }
    }

    if (!IM::util::Password::Verify(decrypted_password, ua.password_hash)) {
        result.code = 403;
        result.err = "密码错误";
        return result;
    }

    if (user.email == new_email) {
        result.code = 400;
        result.err = "新邮箱不能与原邮箱相同";
        return result;
    }

    // 检查新邮箱是否已被其他账户使用
    IM::dao::User other_user;
    if (IM::dao::UserDAO::GetByEmail(new_email, other_user, &err)) {
        if (other_user.id != uid) {
            result.code = 400;
            result.err = "新邮箱已被使用";
            return result;
        }
    }

    if (!IM::dao::UserDAO::UpdateEmail(uid, new_email, &err)) {
        if (!err.empty()) {
            IM_LOG_ERROR(g_logger)
                << "UpdateEmail UpdateEmail failed, uid=" << uid << ", err=" << err;
            result.code = 500;
            result.err = "更新邮箱失败";
            return result;
        }
    }

    result.ok = true;
    return result;
}

VoidResult UserService::UpdatePassword(const uint64_t uid, const std::string& old_password,
                                       const std::string& new_password) {
    VoidResult result;
    std::string err;

    // 1、密码解密
    std::string decrypted_old, decrypted_new;
    auto dec_old_res = IM::DecryptPassword(old_password, decrypted_old);
    if (!dec_old_res.ok) {
        result.code = dec_old_res.code;
        result.err = dec_old_res.err;
        return result;
    }
    auto dec_new_res = IM::DecryptPassword(new_password, decrypted_new);
    if (!dec_new_res.ok) {
        result.code = dec_new_res.code;
        result.err = dec_new_res.err;
        return result;
    }

    // 2、验证旧密码
    IM::dao::UserAuth ua;
    if (!IM::dao::UserAuthDao::GetByUserId(uid, ua, &err)) {
        if (!err.empty()) {
            IM_LOG_ERROR(g_logger)
                << "UpdatePassword GetByUserId failed, uid=" << uid << ", err=" << err;
            result.code = 500;
            result.err = "加载用户认证信息失败";
            return result;
        }
    }
    if (!IM::util::Password::Verify(decrypted_old, ua.password_hash)) {
        result.code = 403;
        result.err = "旧密码错误";
        return result;
    }

    // 3、生成新密码哈希
    auto new_password_hash = IM::util::Password::Hash(decrypted_new);
    if (new_password_hash.empty()) {
        if (!err.empty()) {
            IM_LOG_ERROR(g_logger) << "UpdatePasswordHash Hash failed, uid=" << uid;
            result.code = 500;
            result.err = "新密码哈希生成失败";
            return result;
        }
    }

    // 4、更新新密码
    if (!IM::dao::UserAuthDao::UpdatePasswordHash(uid, new_password_hash, &err)) {
        if (!err.empty()) {
            IM_LOG_ERROR(g_logger) << "UpdatePasswordHash failed, uid=" << uid << ", err=" << err;
            result.code = 500;
            result.err = "更新密码失败";
            return result;
        }
    }
    result.ok = true;
    return result;
}

UserResult UserService::GetUserByMobile(const std::string& mobile, const std::string& channel) {
    UserResult result;
    std::string err;

    if (channel == "register") {
        if (IM::dao::UserDAO::GetByMobile(mobile, result.data, &err)) {
            IM_LOG_ERROR(g_logger)
                << "GetUserByMobile failed, mobile=" << mobile << ", err=" << err;
            result.code = 400;
            result.err = "手机号已注册!";
            return result;
        }
    } else if (channel == "forget_account") {
        if (!IM::dao::UserDAO::GetByMobile(mobile, result.data, &err)) {
            IM_LOG_ERROR(g_logger)
                << "GetUserByMobile failed, mobile=" << mobile << ", err=" << err;
            result.code = 400;
            result.err = "手机号未注册!";
            return result;
        }
    }
    result.ok = true;
    return result;
}

UserResult UserService::GetUserByEmail(const std::string& email, const std::string& channel) {
    UserResult result;
    std::string err;
    if (channel == "register" || channel == "update_email") {
        if (IM::dao::UserDAO::GetByEmail(email, result.data, &err)) {
            if(!err.empty()) {
                IM_LOG_ERROR(g_logger)
                    << "GetUserByEmail failed, email=" << email << ", err=" << err;
                result.code = 500;
                result.err = "查询邮箱失败!";
                return result;
            }
        }
    } else if (channel == "forget_account") {
        if (!IM::dao::UserDAO::GetByEmail(email, result.data, &err)) {
            IM_LOG_ERROR(g_logger) << "GetUserByEmail failed, email=" << email << ", err=" << err;
            result.code = 400;
            result.err = "邮箱未注册!";
            return result;
        }
    }
    result.ok = true;
    return result;
}

VoidResult UserService::Offline(const uint64_t id) {
    VoidResult result;
    std::string err;

    if (!IM::dao::UserDAO::UpdateOfflineStatus(id, &err)) {
        if (!err.empty()) {
            IM_LOG_ERROR(g_logger)
                << "UpdateOfflineStatus failed, user_id=" << id << ", err=" << err;
            result.code = 500;
            result.err = "更新在线状态失败";
            return result;
        }
    }

    result.ok = true;
    return result;
}

StatusResult UserService::GetUserOnlineStatus(const uint64_t id) {
    StatusResult result;
    std::string err;

    if (!IM::dao::UserDAO::GetOnlineStatus(id, result.data, &err)) {
        if (!err.empty()) {
            IM_LOG_ERROR(g_logger)
                << "GetUserOnlineStatus failed, user_id=" << id << ", err=" << err;
            result.code = 500;
            result.err = "获取用户在线状态失败";
            return result;
        }
    }

    result.ok = true;
    return result;
}

VoidResult UserService::SaveConfigInfo(const uint64_t user_id, const std::string& theme_mode,
                                       const std::string& theme_bag_img,
                                       const std::string& theme_color,
                                       const std::string& notify_cue_tone,
                                       const std::string& keyboard_event_notify) {
    VoidResult result;
    std::string err;

    IM::dao::UserSettings new_settings;
    new_settings.user_id = user_id;
    new_settings.theme_mode = theme_mode;
    new_settings.theme_bag_img = theme_bag_img;
    new_settings.theme_color = theme_color;
    new_settings.notify_cue_tone = notify_cue_tone;
    new_settings.keyboard_event_notify = keyboard_event_notify;
    if (!IM::dao::UserSettingsDAO::Upsert(new_settings, &err)) {
        if (!err.empty()) {
            IM_LOG_ERROR(g_logger)
                << "Upsert new UserSettings failed, user_id=" << user_id << ", err=" << err;
            result.code = 500;
            result.err = "保存用户设置失败";
            return result;
        }
    }
    result.ok = true;
    return result;
}

ConfigInfoResult UserService::LoadConfigInfo(const uint64_t user_id) {
    ConfigInfoResult result;
    std::string err;

    if (!IM::dao::UserSettingsDAO::GetConfigInfo(user_id, result.data, &err)) {
        if (!err.empty()) {
            IM_LOG_ERROR(g_logger)
                << "LoadConfigInfo failed, user_id=" << user_id << ", err=" << err;
            result.code = 500;
            result.err = "加载用户设置失败";
            return result;
        }
    }

    result.ok = true;
    return result;
}

UserInfoResult UserService::LoadUserInfoSimple(const uint64_t uid) {
    UserInfoResult result;
    std::string err;

    if (!IM::dao::UserDAO::GetUserInfoSimple(uid, result.data, &err)) {
        if (!err.empty()) {
            IM_LOG_ERROR(g_logger) << "LoadUserInfoSimple failed, uid=" << uid << ", err=" << err;
            result.code = 404;
            result.err = "加载用户信息失败";
            return result;
        }
    }

    if (!result.data.avatar.empty() && result.data.avatar.length() == 32) {
        bool is_id = true;
        for (char c : result.data.avatar) {
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                is_id = false;
                break;
            }
        }
        if (is_id) {
            IM::dao::MediaFile media;
            if (IM::app::MediaService::GetMediaFile(result.data.avatar, media)) {
                result.data.avatar = media.url;
            }
        }
    }

    result.ok = true;
    return result;
}

}  // namespace IM::app