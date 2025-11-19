#ifndef __CIM_DAO_MESSAGE_MENTION_DAO_HPP__
#define __CIM_DAO_MESSAGE_MENTION_DAO_HPP__

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "db/mysql.hpp"

namespace CIM::dao {
// im_message_mention 表 DAO：@提及映射
class MessageMentionDao {
   public:
    static bool AddMentions(const std::shared_ptr<CIM::MySQL>& db, const std::string& msg_id,
                            const std::vector<uint64_t>& mentioned_user_ids,
                            std::string* err = nullptr);
    // 获取被提及的用户 ID 列表（按 msg_id）
    static bool GetMentions(const std::string& msg_id, std::vector<uint64_t>& out,
                            std::string* err = nullptr);
};
}  // namespace CIM::dao

#endif  // __CIM_DAO_MESSAGE_MENTION_DAO_HPP__
