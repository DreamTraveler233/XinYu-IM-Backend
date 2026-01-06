#ifndef __IM_MODEL_TALK_SESSION_HPP__
#define __IM_MODEL_TALK_SESSION_HPP__

#include <cstdint>
#include <ctime>
#include <optional>
#include <string>

namespace IM::model {

struct TalkSession {
    uint64_t user_id = 0;                        // 用户ID
    uint64_t talk_id = 0;                        // talk 主实体ID
    uint64_t to_from_id = 0;                     // 目标用户ID/群ID
    uint8_t talk_mode = 1;                       // 会话模式(1=单聊 2=群聊)
    uint8_t is_top = 2;                          // 是否置顶(1=是 2=否)
    uint8_t is_disturb = 2;                      // 是否免打扰(1=是 2=否)
    uint8_t is_robot = 2;                        // 是否机器人(1=是 2=否)
    uint64_t last_ack_seq = 0;                   // 最后已读序号
    uint32_t unread_num = 0;                     // 未读消息数
    std::optional<std::string> last_msg_id;      // 最后一条消息的ID（CHAR(32)）
    std::optional<uint16_t> last_msg_type;       // 最后一条消息的类型
    std::optional<uint64_t> last_sender_id;      // 最后一条消息的发送者ID
    std::optional<std::string> last_msg_digest;  // 最后一条消息的摘要
    std::optional<std::string> draft_text;       // 草稿文本
    std::optional<std::string> name;             // 会话对象名称
    std::optional<std::string> avatar;           // 会话对象头像
    std::optional<std::string> remark;           // 会话备注
    std::optional<std::time_t> deleted_at;       // 软删除时间
    std::time_t created_at = 0;                  // 创建时间
    std::time_t updated_at = 0;                  // 更新时间
};

}  // namespace IM::model

#endif  // __IM_MODEL_TALK_SESSION_HPP__