#ifndef __IM_MODEL_MESSAGE_HPP__
#define __IM_MODEL_MESSAGE_HPP__

#include <cstdint>
#include <string>
#include <ctime>

namespace IM::model {

struct Message {
    std::string id;               // 消息ID（CHAR(32) UUID/HEX）
    uint64_t talk_id = 0;         // 会话ID
    uint64_t sequence = 0;        // 会话内序号
    uint8_t talk_mode = 0;        // 1=单聊 2=群聊
    uint16_t msg_type = 0;        // 消息类型
    uint64_t sender_id = 0;       // 发送者
    uint64_t receiver_id = 0;     // 单聊对端，群聊为空(0表示NULL)
    uint64_t group_id = 0;        // 群聊群ID，单聊为空(0表示NULL)
    std::string content_text;     // 文本/系统文本内容
    std::string extra;            // JSON 扩展（各类型专属字段）
    std::string quote_msg_id;     // 被引用消息ID（空字符串表示NULL）
    uint8_t is_revoked = 2;       // 1=已撤回 2=正常
    uint64_t revoke_by = 0;       // 撤回人(0表示NULL)
    std::time_t revoke_time = 0;  // 撤回时间(0表示NULL)
    uint8_t status = 1;           // 发送状态: 1=成功 2=发送中 3=失败
    std::time_t created_at = 0;   // 创建时间
    std::time_t updated_at = 0;   // 更新时间
};

}  // namespace IM::model

#endif  // __IM_MODEL_MESSAGE_HPP__