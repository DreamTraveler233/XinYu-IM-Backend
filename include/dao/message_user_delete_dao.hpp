#ifndef __CIM_DAO_MESSAGE_USER_DELETE_DAO_HPP__
#define __CIM_DAO_MESSAGE_USER_DELETE_DAO_HPP__

#include <cstdint>
#include <memory>
#include <string>

#include "db/mysql.hpp"

namespace CIM::dao {
// im_message_user_delete 表 DAO：记录用户侧删除消息（仅影响本人视图）
class MessageUserDeleteDao {
   public:
    static bool MarkUserDelete(const std::shared_ptr<CIM::MySQL>& db, const std::string& msg_id,
                               const uint64_t user_id, std::string* err = nullptr);
                               
    // 标记某个会话内的所有消息为 user 已删除（对视图隐藏）
    static bool MarkAllMessagesDeletedByUserInTalk(const std::shared_ptr<CIM::MySQL>& db,
                                                   const uint64_t talk_id, const uint64_t user_id,
                                                   std::string* err = nullptr);
};
}  // namespace CIM::dao

#endif  // __CIM_DAO_MESSAGE_USER_DELETE_DAO_HPP__
