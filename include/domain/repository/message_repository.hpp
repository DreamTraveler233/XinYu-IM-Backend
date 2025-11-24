#ifndef __IM_DOMAIN_REPOSITORY_MESSAGE_REPOSITORY_HPP__
#define __IM_DOMAIN_REPOSITORY_MESSAGE_REPOSITORY_HPP__

#include <memory>
#include <optional>
#include <vector>

#include "db/mysql.hpp"
#include "dto/contact_dto.hpp"
#include "dto/message_dto.hpp"
#include "dto/user_dto.hpp"
#include "model/message.hpp"
#include "model/talk_session.hpp"

namespace IM::domain::repository {

class IMessageRepository {
   public:
    using Ptr = std::shared_ptr<IMessageRepository>;
    virtual ~IMessageRepository() = default;

    // 创建消息（写入 im_message）；不包含转发/已读/提及等附表逻辑。
    virtual bool Create(const std::shared_ptr<IM::MySQL>& db, const model::Message& m,
                        std::string* err = nullptr) = 0;

    // 根据消息ID查询。
    virtual bool GetById(const std::string& msg_id, model::Message& out,
                         std::string* err = nullptr) = 0;

    // 会话内按序号倒序分页（anchor_seq=0 时取最新）。返回 sequence 递减排序结果。
    virtual bool ListRecentDesc(const uint64_t talk_id, const uint64_t anchor_seq,
                                const size_t limit, std::vector<model::Message>& out,
                                std::string* err = nullptr) = 0;

    // 会话内按序号倒序分页（anchor_seq=0 时取最新），可排除某用户已删除的消息。
    virtual bool ListRecentDescWithFilter(const uint64_t talk_id, const uint64_t anchor_seq,
                                          const size_t limit, const uint64_t user_id,
                                          const uint16_t msg_type, std::vector<model::Message>& out,
                                          std::string* err = nullptr) = 0;

    // 与 ListRecentDesc 功能类似
    virtual bool ListRecentDescWithFilter(const std::shared_ptr<IM::MySQL>& db,
                                          const uint64_t talk_id, const uint64_t anchor_seq,
                                          const size_t limit, const uint64_t user_id,
                                          const uint16_t msg_type, std::vector<model::Message>& out,
                                          std::string* err = nullptr) = 0;

    // 会话内获取大于某序号的消息（升序）。
    virtual bool ListAfterAsc(const uint64_t talk_id, const uint64_t after_seq, const size_t limit,
                              std::vector<model::Message>& out, std::string* err = nullptr) = 0;

    // 批量根据 ids 获取消息（用于批量加载被引用的消息，避免 N+1 查询）
    virtual bool GetByIds(const std::vector<std::string>& ids, std::vector<model::Message>& out,
                          std::string* err = nullptr) = 0;

    // 批量根据 ids 获取消息，并可排除某个用户已删除的消息
    virtual bool GetByIdsWithFilter(const std::vector<std::string>& ids, const uint64_t user_id,
                                    std::vector<model::Message>& out,
                                    std::string* err = nullptr) = 0;

    // 撤回消息（状态置 1），仅当当前状态为正常(2)。
    virtual bool Revoke(const std::shared_ptr<IM::MySQL>& db, const std::string& msg_id,
                        const uint64_t user_id, std::string* err = nullptr) = 0;

    // 硬删除会话下的所有消息
    virtual bool DeleteByTalkId(const std::shared_ptr<IM::MySQL>& db, const uint64_t talk_id,
                                std::string* err = nullptr) = 0;

    // 更新消息的发送状态（用于标记失败/成功等）
    virtual bool SetStatus(const std::shared_ptr<IM::MySQL>& db, const std::string& msg_id,
                           uint8_t status, std::string* err = nullptr) = 0;

    // 添加转发消息的来源映射记录
    virtual bool AddForwardMap(const std::shared_ptr<IM::MySQL>& db,
                               const std::string& forward_msg_id,
                               const std::vector<dto::ForwardSrc>& sources,
                               std::string* err = nullptr) = 0;
    // 获取转发消息的来源映射列表
    virtual bool AddMentions(const std::shared_ptr<IM::MySQL>& db, const std::string& msg_id,
                             const std::vector<uint64_t>& mentioned_user_ids,
                             std::string* err = nullptr) = 0;

    // 获取被提及的用户 ID 列表（按 msg_id）
    virtual bool GetMentions(const std::string& msg_id, std::vector<uint64_t>& out,
                             std::string* err = nullptr) = 0;

    // 记录用户对消息的阅读（幂等插入）
    virtual bool MarkRead(const std::string& msg_id, const uint64_t user_id,
                          std::string* err = nullptr) = 0;

    // 根据会话ID和用户ID批量标记已读
    virtual bool MarkReadByTalk(const uint64_t talk_id, const uint64_t user_id,
                                std::string* err = nullptr) = 0;

    // 记录用户侧删除消息（仅影响本人视图）
    virtual bool MarkUserDelete(const std::shared_ptr<IM::MySQL>& db, const std::string& msg_id,
                                const uint64_t user_id, std::string* err = nullptr) = 0;

    // 标记某个会话内的所有消息为 user 已删除（对视图隐藏）
    virtual bool MarkAllMessagesDeletedByUserInTalk(const std::shared_ptr<IM::MySQL>& db,
                                                    const uint64_t talk_id, const uint64_t user_id,
                                                    std::string* err = nullptr) = 0;
};

}  // namespace IM::domain::repository

#endif  // __IM_DOMAIN_REPOSITORY_MESSAGE_REPOSITORY_HPP__