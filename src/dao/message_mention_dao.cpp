#include "dao/message_mention_dao.hpp"

#include "db/mysql.hpp"

namespace CIM::dao {

static constexpr const char* kDBName = "default";

bool MessageMentionDao::AddMentions(const std::shared_ptr<CIM::MySQL>& db,
                                    const std::string& msg_id,
                                    const std::vector<uint64_t>& mentioned_user_ids,
                                    std::string* err) {
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    if (mentioned_user_ids.empty()) return true;
    const char* sql =
        "INSERT IGNORE INTO im_message_mention (msg_id,mentioned_user_id) VALUES (?,?)";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    for (auto uid : mentioned_user_ids) {
        stmt->bindString(1, msg_id);
        stmt->bindUint64(2, uid);
        if (stmt->execute() != 0) {
            if (err) *err = stmt->getErrStr();
            return false;
        }
    }
    return true;
}

bool MessageMentionDao::GetMentions(const std::string& msg_id, std::vector<uint64_t>& out,
                                    std::string* err) {
    out.clear();
    auto db = CIM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql = "SELECT mentioned_user_id FROM im_message_mention WHERE msg_id=?";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindString(1, msg_id);
    auto res = stmt->query();
    if (!res) {
        if (err) *err = "query failed";
        return false;
    }
    while (res->next()) {
        out.push_back(res->getUint64(0));
    }
    return true;
}
}  // namespace CIM::dao
