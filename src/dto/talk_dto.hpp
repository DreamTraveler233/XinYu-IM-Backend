#ifndef __IM_DTO_TALK_DTO_HPP__
#define __IM_DTO_TALK_DTO_HPP__

#include <cstdint>
#include <string>

namespace IM::dto {

struct TalkSessionItem {
    uint64_t id = 0;              // 会话ID
    uint8_t talk_mode = 1;        // 会话模式（1=单聊 2=群聊）
    uint64_t to_from_id = 0;      // 目标用户ID
    uint8_t is_top = 2;           // 是否置顶
    uint8_t is_disturb = 2;       // 是否免打扰
    uint8_t is_robot = 2;         // 是否机器人
    std::string name = "";        // 会话对象名称（用户名/群名称）
    std::string avatar = "";      // 会话对象头像URL
    std::string remark = "";      // 会话备注
    uint32_t unread_num = 0;      // 未读消息数
    std::string msg_text = "";    // 最后一条消息预览文本
    std::string updated_at = "";  // 最后更新时间
};

}  // namespace IM::dto

#endif  // __IM_DTO_TALK_DTO_HPP__