#include "talk_service.hpp"

#include "macro.hpp"

namespace CIM::app {

static auto g_logger = CIM_LOG_NAME("root");

TalkSessionListResult TalkService::getSessionListByUserId(const uint64_t user_id) {
    TalkSessionListResult result;
    std::string err;

    if (!dao::TalkSessionDao::getSessionListByUserId(user_id, result.data, &err)) {
        if (!err.empty()) {
            CIM_LOG_ERROR(g_logger)
                << "TalkService::getSessionListByUserId failed, user_id=" << user_id
                << ", err=" << err;
            result.code = 500;
            result.err = "获取会话列表失败";
        }
    }

    result.ok = true;
    return result;
}

}  // namespace CIM::app