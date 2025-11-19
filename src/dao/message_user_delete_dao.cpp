#include "dao/message_user_delete_dao.hpp"

#include "db/mysql.hpp"

namespace CIM::dao {

static constexpr const char* kDBName = "default";

bool MessageUserDeleteDao::MarkUserDelete(const std::shared_ptr<CIM::MySQL>& db,
                                          const std::string& msg_id, const uint64_t user_id,
                                          std::string* err) {
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql =
        "INSERT IGNORE INTO im_message_user_delete (msg_id,user_id,deleted_at) VALUES (?,?,NOW())";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindString(1, msg_id);
    stmt->bindUint64(2, user_id);
    if (stmt->execute() != 0) {
        if (err) *err = stmt->getErrStr();
        return false;
    }
    return true;
}

bool MessageUserDeleteDao::MarkAllMessagesDeletedByUserInTalk(const std::shared_ptr<CIM::MySQL>& db, const uint64_t talk_id,
                                                              const uint64_t user_id,
                                                              std::string* err) {
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }

    // 通过 INSERT ... SELECT 批量把 talk 中消息写入用户删除表；使用 INSERT IGNORE 避免重复
    const char* sql =
        "INSERT IGNORE INTO im_message_user_delete (msg_id, user_id, deleted_at) "
        "SELECT id, ?, NOW() FROM im_message WHERE talk_id = ?";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindUint64(1, user_id);
    stmt->bindUint64(2, talk_id);
    if (stmt->execute() != 0) {
        if (err) *err = stmt->getErrStr();
        return false;
    }
    return true;
}
}  // namespace CIM::dao
