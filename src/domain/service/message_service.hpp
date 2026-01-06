#ifndef __IM_DOMAIN_SERVICE_MESSAGE_SERVICE_HPP__
#define __IM_DOMAIN_SERVICE_MESSAGE_SERVICE_HPP__

#include <memory>
#include <vector>

#include "common/result.hpp"
#include "dto/message_dto.hpp"
#include "model/message.hpp"

namespace IM::domain::service {

class IMessageService {
   public:
    using Ptr = std::shared_ptr<IMessageService>;
    virtual ~IMessageService() = default;

    // 获取当前会话消息（按 sequence DESC 返回最新 -> 较旧）
    virtual Result<dto::MessagePage> LoadRecords(const uint64_t current_user_id,
                                                 const uint8_t talk_mode, const uint64_t to_from_id,
                                                 uint64_t cursor, uint32_t limit) = 0;

    // 获取当前会话历史消息（支持按 msg_type 过滤，0=全部）
    virtual Result<dto::MessagePage> LoadHistoryRecords(const uint64_t current_user_id,
                                                        const uint8_t talk_mode,
                                                        const uint64_t to_from_id,
                                                        const uint16_t msg_type, uint64_t cursor,
                                                        uint32_t limit) = 0;

    // 获取转发消息记录（传入一组消息ID，返回消息详情；不分页）
    virtual Result<std::vector<dto::MessageRecord>> LoadForwardRecords(
        const uint64_t current_user_id, const uint8_t talk_mode,
        const std::vector<std::string>& msg_ids) = 0;

    // 删除聊天记录（仅对本人视图生效）。
    virtual Result<void> DeleteMessages(const uint64_t current_user_id, const uint8_t talk_mode,
                                        const uint64_t to_from_id,
                                        const std::vector<std::string>& msg_ids) = 0;

    // 删除会话中该用户的所有消息（仅影响该用户视图）并删除会话视图
    virtual Result<void> DeleteAllMessagesInTalkForUser(const uint64_t current_user_id,
                                                        const uint8_t talk_mode,
                                                        const uint64_t to_from_id) = 0;

    // 清空聊天记录（软删除，将消息插入删除表中）
    virtual Result<void> ClearTalkRecords(const uint64_t current_user_id, const uint8_t talk_mode,
                                          const uint64_t to_from_id) = 0;

    // 撤回消息（仅发送者可撤回，后续可扩展管理员权限）。
    virtual Result<void> RevokeMessage(const uint64_t current_user_id, const uint8_t talk_mode,
                                       const uint64_t to_from_id, const std::string& msg_id) = 0;

    // 发送消息：返回刚插入的消息记录（含补充字段）。
    virtual Result<dto::MessageRecord> SendMessage(
        const uint64_t current_user_id, const uint8_t talk_mode, const uint64_t to_from_id,
        const uint16_t msg_type, const std::string& content_text, const std::string& extra,
        const std::string& quote_msg_id, const std::string& msg_id,
        const std::vector<uint64_t>& mentioned_user_ids) = 0;

    // 更新消息状态（通常由发送方请求，或服务器端发生发送失败时标记）
    virtual Result<void> UpdateMessageStatus(const uint64_t current_user_id,
                                             const uint8_t talk_mode, const uint64_t to_from_id,
                                             const std::string& msg_id, uint8_t status) = 0;

   private:
    // 根据 talk_mode 与对象ID 获取会话 talk_id（不存在返回 0）。
    virtual uint64_t resolveTalkId(const uint8_t talk_mode, const uint64_t to_from_id) = 0;

    // 将 DAO Message 转换为前端需要的记录结构（补充用户昵称头像、引用）。
    virtual bool buildRecord(const IM::model::Message& msg, IM::dto::MessageRecord& out,
                             std::string* err) = 0;

    // 统一获取 talk_id 的辅助函数（含权限校验等）
    virtual bool GetTalkId(const uint64_t current_user_id, const uint8_t talk_mode,
                           const uint64_t to_from_id, uint64_t& talk_id, std::string& err) = 0;
};

}  // namespace IM::domain::service

#endif  // __IM_DOMAIN_SERVICE_MESSAGE_SERVICE_HPP__