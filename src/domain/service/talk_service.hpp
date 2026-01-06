#ifndef __IM_DOMAIN_SERVICE_TALK_SERVICE_HPP__
#define __IM_DOMAIN_SERVICE_TALK_SERVICE_HPP__

#include <vector>
#include <memory>

#include "dto/talk_dto.hpp"
#include "common/result.hpp"

namespace IM::domain::service {

class ITalkService {
   public:
    using Ptr = std::shared_ptr<ITalkService>;
    virtual ~ITalkService() = default;

    // 获取用户的会话列表
    virtual Result<std::vector<dto::TalkSessionItem>> getSessionListByUserId(
        const uint64_t user_id) = 0;

    // 设置会话置顶/取消置顶
    virtual Result<void> setSessionTop(const uint64_t user_id, const uint64_t to_from_id,
                                       const uint8_t talk_mode, const uint8_t action) = 0;

    // 设置会话免打扰/取消免打扰
    virtual Result<void> setSessionDisturb(const uint64_t user_id, const uint64_t to_from_id,
                                           const uint8_t talk_mode, const uint8_t action) = 0;

    // 创建会话
    virtual Result<dto::TalkSessionItem> createSession(const uint64_t user_id,
                                                       const uint64_t to_from_id,
                                                       const uint8_t talk_mode) = 0;

    // 删除会话
    virtual Result<void> deleteSession(const uint64_t user_id, const uint64_t to_from_id,
                                       const uint8_t talk_mode) = 0;

    // 清除会话未读消息数
    virtual Result<void> clearSessionUnreadNum(const uint64_t user_id, const uint64_t to_from_id,
                                               const uint8_t talk_mode) = 0;
};

}  // namespace IM::domain::service

#endif  // __IM_DOMAIN_SERVICE_TALK_SERVICE_HPP__