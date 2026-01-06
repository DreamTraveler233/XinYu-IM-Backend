#ifndef __IM_INFRA_REPOSITORY_MESSAGE_REPOSITORY_IMPL_HPP__
#define __IM_INFRA_REPOSITORY_MESSAGE_REPOSITORY_IMPL_HPP__

#include "infra/db/mysql.hpp"
#include "domain/repository/message_repository.hpp"

namespace IM::infra::repository {

class MessageRepositoryImpl : public IM::domain::repository::IMessageRepository {
   public:
    explicit MessageRepositoryImpl(std::shared_ptr<IM::MySQLManager> db_manager);

    bool Create(const std::shared_ptr<IM::MySQL>& db, const model::Message& m,
                std::string* err = nullptr) override;
    bool GetById(const std::string& msg_id, model::Message& out,
                 std::string* err = nullptr) override;
    bool ListRecentDesc(const uint64_t talk_id, const uint64_t anchor_seq, const size_t limit,
                        std::vector<model::Message>& out, std::string* err = nullptr) override;
    bool ListRecentDescWithFilter(const uint64_t talk_id, const uint64_t anchor_seq,
                                  const size_t limit, const uint64_t user_id,
                                  const uint16_t msg_type, std::vector<model::Message>& out,
                                  std::string* err = nullptr) override;
    bool ListRecentDescWithFilter(const std::shared_ptr<IM::MySQL>& db, const uint64_t talk_id,
                                  const uint64_t anchor_seq, const size_t limit,
                                  const uint64_t user_id, const uint16_t msg_type,
                                  std::vector<model::Message>& out,
                                  std::string* err = nullptr) override;
    bool ListAfterAsc(const uint64_t talk_id, const uint64_t after_seq, const size_t limit,
                      std::vector<model::Message>& out, std::string* err = nullptr) override;
    bool GetByIds(const std::vector<std::string>& ids, std::vector<model::Message>& out,
                  std::string* err = nullptr) override;
    bool GetByIdsWithFilter(const std::vector<std::string>& ids, const uint64_t user_id,
                            std::vector<model::Message>& out, std::string* err = nullptr) override;
    bool Revoke(const std::shared_ptr<IM::MySQL>& db, const std::string& msg_id,
                const uint64_t user_id, std::string* err = nullptr) override;
    bool DeleteByTalkId(const std::shared_ptr<IM::MySQL>& db, const uint64_t talk_id,
                        std::string* err = nullptr) override;
    bool SetStatus(const std::shared_ptr<IM::MySQL>& db, const std::string& msg_id, uint8_t status,
                   std::string* err = nullptr) override;
    bool AddForwardMap(const std::shared_ptr<IM::MySQL>& db, const std::string& forward_msg_id,
                       const std::vector<dto::ForwardSrc>& sources,
                       std::string* err = nullptr) override;
    bool AddMentions(const std::shared_ptr<IM::MySQL>& db, const std::string& msg_id,
                     const std::vector<uint64_t>& mentioned_user_ids,
                     std::string* err = nullptr) override;
    bool GetMentions(const std::string& msg_id, std::vector<uint64_t>& out,
                     std::string* err = nullptr) override;
    bool MarkRead(const std::string& msg_id, const uint64_t user_id,
                  std::string* err = nullptr) override;
    bool MarkReadByTalk(const uint64_t talk_id, const uint64_t user_id,
                        std::string* err = nullptr) override;
    bool MarkUserDelete(const std::shared_ptr<IM::MySQL>& db, const std::string& msg_id,
                        const uint64_t user_id, std::string* err = nullptr) override;
    bool MarkAllMessagesDeletedByUserInTalk(const std::shared_ptr<IM::MySQL>& db,
                                            const uint64_t talk_id, const uint64_t user_id,
                                            std::string* err = nullptr) override;

   private:
    std::shared_ptr<IM::MySQLManager> m_db_manager;
};

}  // namespace IM::infra::repository

#endif  // __IM_INFRA_REPOSITORY_MESSAGE_REPOSITORY_IMPL_HPP__