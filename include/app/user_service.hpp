#ifndef __APP_SETTING_SERVICE_HPP__
#define __APP_SETTING_SERVICE_HPP__

#include <cstdint>
#include <optional>
#include <string>

#include "dao/user_dao.hpp"

namespace CIM::app {

struct UserResult {
    bool ok = false;      // 是否成功
    std::string err;      // 错误描述
    CIM::dao::User user;  // 成功时的用户信息
};

class UserService {
   public:
    // 加载用户信息
    static UserResult LoadUser(uint64_t uid);

    // 更新用户信息
    static bool UpdateUserInfo(const uint64_t uid, const std::string& nickname,
                               const std::string& avatar, const std::string& motto,
                               const uint32_t gender, const std::string& birthday);
};

}  // namespace CIM::app

#endif  // __SETTING_SERVICE_HPP__
