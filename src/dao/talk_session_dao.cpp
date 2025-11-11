#include "talk_session_dao.hpp"

#include "db/mysql.hpp"

namespace CIM::dao {
static const char* kDBName = "default";

bool TalkSessionDao::getSessionListByUserId(const uint64_t user_id,
                                            std::vector<TalkSessionItem>& out, std::string* err) {
    auto db = CIM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql =
        "SELECT t.id, t.talk_mode, ts.to_from_id, ts.is_top, ts.is_disturb, ts.is_robot, "
        "ts.name, ts.avatar, ts.remark, ts.unread_num, ts.last_msg_digest, ts.updated_at FROM "
        "im_talk_session ts LEFT JOIN im_talk t ON ts.talk_id = t.id "
        "WHERE ts.user_id = ? ";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindUint64(1, user_id);
    auto res = stmt->query();
    if (!res) {
        if (err) *err = "query failed";
        return false;
    }
    while (res->next()) {
        TalkSessionItem item;
        item.id = res->getUint64(0);
        item.talk_mode = res->getUint8(1);
        item.to_from_id = res->getUint64(2);
        item.is_top = res->getUint8(3);
        item.is_disturb = res->getUint8(4);
        item.is_robot = res->getUint8(5);
        item.name = res->getString(6);
        item.avatar = res->getString(7);
        item.remark = res->getString(8);
        item.unread_num = res->getUint32(9);
        item.msg_text = res->getString(10);
        item.updated_at = res->getTime(11);
        out.push_back(item);
    }
    return true;
}

}  // namespace CIM::dao