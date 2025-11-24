#ifndef __IM_DTO_MESSAGE_DTO_HPP__
#define __IM_DTO_MESSAGE_DTO_HPP__

#include <cstdint>
#include <string>
#include <vector>

namespace IM::dto {

struct MessageRecord {
    std::string msg_id;      // 消息ID字符串（原始为整数）
    uint64_t sequence = 0;   // 会话内序号
    uint16_t msg_type = 0;   // 消息类型
    uint64_t from_id = 0;    // 发送者用户ID
    std::string nickname;    // 发送者昵称
    std::string avatar;      // 发送者头像
    uint8_t is_revoked = 2;  // 撤回状态
    uint8_t status = 1;      // 发送状态：1成功 2发送中 3失败
    std::string send_time;   // 发送时间字符串
    std::string extra;       // 额外 JSON
    std::string quote;       // 引用消息 JSON
};

struct MessagePage {
    std::vector<MessageRecord> items;
    uint64_t cursor = 0;  // 下一次拉取的锚点（旧消息最小 sequence）
};

struct ForwardSrc {
    std::string src_msg_id;      // 原消息ID（CHAR(32)）
    uint64_t src_talk_id = 0;    // 原消息会话ID
    uint64_t src_sender_id = 0;  // 原发送者
};

}  // namespace IM::dto

#endif  // __IM_DTO_MESSAGE_DTO_HPP__