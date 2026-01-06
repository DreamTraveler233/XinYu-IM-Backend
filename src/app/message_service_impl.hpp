#ifndef __IM_APP_MESSAGE_SERVICE_IMPL_HPP__
#define __IM_APP_MESSAGE_SERVICE_IMPL_HPP__

#include "domain/repository/contact_repository.hpp"
#include "domain/repository/message_repository.hpp"
#include "domain/repository/talk_repository.hpp"
#include "domain/repository/user_repository.hpp"
#include "domain/service/message_service.hpp"

namespace IM::app {

class MessageServiceImpl : public IM::domain::service::IMessageService {
   public:
    explicit MessageServiceImpl(IM::domain::repository::IMessageRepository::Ptr message_repo,
                                IM::domain::repository::ITalkRepository::Ptr talk_repo,
                                IM::domain::repository::IUserRepository::Ptr user_repo,
                                IM::domain::repository::IContactRepository::Ptr contact_repo);

    Result<dto::MessagePage> LoadRecords(const uint64_t current_user_id, const uint8_t talk_mode,
                                         const uint64_t to_from_id, uint64_t cursor,
                                         uint32_t limit) override;
    Result<dto::MessagePage> LoadHistoryRecords(const uint64_t current_user_id,
                                                const uint8_t talk_mode, const uint64_t to_from_id,
                                                const uint16_t msg_type, uint64_t cursor,
                                                uint32_t limit) override;
    Result<std::vector<dto::MessageRecord>> LoadForwardRecords(
        const uint64_t current_user_id, const uint8_t talk_mode,
        const std::vector<std::string>& msg_ids) override;
    Result<void> DeleteMessages(const uint64_t current_user_id, const uint8_t talk_mode,
                                const uint64_t to_from_id,
                                const std::vector<std::string>& msg_ids) override;
    Result<void> DeleteAllMessagesInTalkForUser(const uint64_t current_user_id,
                                                const uint8_t talk_mode,
                                                const uint64_t to_from_id) override;
    Result<void> ClearTalkRecords(const uint64_t current_user_id, const uint8_t talk_mode,
                                  const uint64_t to_from_id) override;
    Result<void> RevokeMessage(const uint64_t current_user_id, const uint8_t talk_mode,
                               const uint64_t to_from_id, const std::string& msg_id) override;
    Result<dto::MessageRecord> SendMessage(
        const uint64_t current_user_id, const uint8_t talk_mode, const uint64_t to_from_id,
        const uint16_t msg_type, const std::string& content_text, const std::string& extra,
        const std::string& quote_msg_id, const std::string& msg_id,
        const std::vector<uint64_t>& mentioned_user_ids) override;
    Result<void> UpdateMessageStatus(const uint64_t current_user_id, const uint8_t talk_mode,
                                     const uint64_t to_from_id, const std::string& msg_id,
                                     uint8_t status) override;

   private:
    uint64_t resolveTalkId(const uint8_t talk_mode, const uint64_t to_from_id) override;
    bool buildRecord(const IM::model::Message& msg, IM::dto::MessageRecord& out,
                     std::string* err) override;
    bool GetTalkId(const uint64_t current_user_id, const uint8_t talk_mode,
                   const uint64_t to_from_id, uint64_t& talk_id, std::string& err) override;

   private:
    IM::domain::repository::IMessageRepository::Ptr m_message_repo;
    IM::domain::repository::ITalkRepository::Ptr m_talk_repo;
    IM::domain::repository::IUserRepository::Ptr m_user_repo;
    IM::domain::repository::IContactRepository::Ptr m_contact_repo;
};

}  // namespace IM::app

#endif  // __IM_APP_MESSAGE_SERVICE_IMPL_HPP__