#include "app/user_service.hpp"

#include "dao/user_dao.hpp"
#include "macro.hpp"

namespace CIM::app {
static auto g_logger = CIM_LOG_NAME("root");
UserResult UserService::LoadUser(uint64_t uid) {
    UserResult result;
    if (!CIM::dao::UserDAO::GetById(uid, result.user)) {
        result.err = "用户不存在！";
        return result;
    }

    result.ok = true;
    return result;
}

bool UserService::UpdateUserInfo(const uint64_t uid, const std::string& nickname,
                                 const std::string& avatar, const std::string& motto,
                                 const uint32_t gender, const std::string& birthday) {
    std::string err;
    if (!CIM::dao::UserDAO::UpdateUserInfo(uid, nickname, avatar, motto, gender, birthday, &err)) {
        return false;
    }
    return true;
}
}  // namespace CIM::app
