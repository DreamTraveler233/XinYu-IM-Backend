#include "dao/user_dao.hpp"

#include "db/mysql.hpp"
#include "base/macro.hpp"

namespace IM::dao {

static constexpr const char* kDBName = "default";
static auto g_logger = IM_LOG_ROOT();

bool UserDAO::Create(const std::shared_ptr<IM::MySQL>& db, const User& u, uint64_t& out_id,
                     std::string* err) {
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql =
        "INSERT INTO im_user (mobile, email, nickname, avatar, avatar_media_id, motto, birthday, gender, "
        "online_status, last_online_at, is_qiye, is_robot, is_disabled, created_at, updated_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NOW(), NOW())";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindString(1, u.mobile);
    if (!u.email.empty())
        stmt->bindString(2, u.email);
    else
        stmt->bindNull(2);
    stmt->bindString(3, u.nickname);
    if (!u.avatar.empty())
        stmt->bindString(4, u.avatar);
    else
        stmt->bindNull(4);
    if (!u.avatar_media_id.empty())
        stmt->bindString(5, u.avatar_media_id);
    else
        stmt->bindNull(5);
    if (!u.motto.empty())
        stmt->bindString(6, u.motto);
    else
        stmt->bindNull(6);
    if (!u.birthday.empty())
        stmt->bindString(7, u.birthday);
    else
        stmt->bindNull(7);
    stmt->bindUint8(8, u.gender);
    stmt->bindString(9, u.online_status);
    if (u.last_online_at != 0)
        stmt->bindTime(10, u.last_online_at);
    else
        stmt->bindNull(10);
    stmt->bindUint8(11, u.is_qiye);
    stmt->bindUint8(12, u.is_robot);
    stmt->bindUint8(13, u.is_disabled);

    if (stmt->execute() != 0) {
        if (err) *err = stmt->getErrStr();
        return false;
    }
    out_id = stmt->getLastInsertId();
    return true;
}

bool UserDAO::GetByMobile(const std::string& mobile, User& out, std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql =
        "SELECT id, mobile, email, nickname, avatar, avatar_media_id, motto, DATE_FORMAT(birthday, '%Y-%m-%d') as "
        "birthday, gender, online_status, last_online_at, is_qiye, is_robot, is_disabled, "
        "created_at, updated_at FROM im_user WHERE mobile = ? LIMIT 1";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindString(1, mobile);
    auto res = stmt->query();
    if (!res) {
        if (err) *err = "query failed";
        return false;
    }

    if (!res->next()) {
        if (err) *err = "no record found";
        return false;
    }

    out.id = res->getUint64(0);
    out.mobile = res->isNull(1) ? std::string() : res->getString(1);
    out.email = res->isNull(2) ? std::string() : res->getString(2);
    out.nickname = res->isNull(3) ? std::string() : res->getString(3);
    out.avatar = res->isNull(4) ? std::string() : res->getString(4);
    out.avatar_media_id = res->isNull(5) ? std::string() : res->getString(5);
    out.motto = res->isNull(6) ? std::string() : res->getString(6);
    out.birthday = res->isNull(7) ? std::string() : res->getString(7);
    out.gender = res->getUint8(8);
    out.online_status = res->isNull(9) ? "N" : res->getString(9);
    out.last_online_at = res->isNull(10) ? 0 : res->getTime(10);
    out.is_qiye = res->isNull(11) ? 0 : res->getUint8(11);
    out.is_robot = res->isNull(12) ? 0 : res->getUint8(12);
    out.is_disabled = res->isNull(13) ? 0 : res->getUint8(13);
    out.created_at = res->getTime(14);
    out.updated_at = res->getTime(15);

    return true;
}

bool UserDAO::GetByEmail(const std::string& email, User& out, std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql =
        "SELECT id, mobile, email, nickname, avatar, avatar_media_id, motto, DATE_FORMAT(birthday, '%Y-%m-%d') as "
        "birthday, gender, online_status, last_online_at, is_qiye, is_robot, is_disabled, "
        "created_at, updated_at FROM im_user WHERE email = ? LIMIT 1";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindString(1, email);
    auto res = stmt->query();
    if (!res) {
        if (err) *err = "query failed";
        return false;
    }

    if (!res->next()) {
        if (err) *err = "no record found";
        return false;
    }

    out.id = res->getUint64(0);
    out.mobile = res->isNull(1) ? std::string() : res->getString(1);
    out.email = res->isNull(2) ? std::string() : res->getString(2);
    out.nickname = res->isNull(3) ? std::string() : res->getString(3);
    out.avatar = res->isNull(4) ? std::string() : res->getString(4);
    out.avatar_media_id = res->isNull(5) ? std::string() : res->getString(5);
    out.motto = res->isNull(6) ? std::string() : res->getString(6);
    out.birthday = res->isNull(7) ? std::string() : res->getString(7);
    out.gender = res->getUint8(8);
    out.online_status = res->isNull(9) ? "N" : res->getString(9);
    out.last_online_at = res->isNull(10) ? 0 : res->getTime(10);
    out.is_qiye = res->isNull(11) ? 0 : res->getUint8(11);
    out.is_robot = res->isNull(12) ? 0 : res->getUint8(12);
    out.is_disabled = res->isNull(13) ? 0 : res->getUint8(13);
    out.created_at = res->getTime(14);
    out.updated_at = res->getTime(15);

    return true;
}

bool UserDAO::GetById(uint64_t id, User& out, std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql =
        "SELECT id, mobile, email, nickname, avatar, avatar_media_id, motto, DATE_FORMAT(birthday, "
        "'%Y-%m-%d') as birthday, gender, online_status, last_online_at, is_robot, is_qiye, "
        "is_disabled, created_at, updated_at FROM im_user WHERE id = ? LIMIT 1";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindUint64(1, id);
    auto res = stmt->query();
    if (!res) {
        if (err) *err = "query failed";
        return false;
    }

    if (!res->next()) {
        if (err) *err = "no record found";
        return false;
    }

    out.id = res->getUint64(0);
    out.mobile = res->isNull(1) ? std::string() : res->getString(1);
    out.email = res->isNull(2) ? std::string() : res->getString(2);
    out.nickname = res->isNull(3) ? std::string() : res->getString(3);
    out.avatar = res->isNull(4) ? std::string() : res->getString(4);
    out.avatar_media_id = res->isNull(5) ? std::string() : res->getString(5);
    out.motto = res->isNull(6) ? std::string() : res->getString(6);
    out.birthday = res->isNull(7) ? std::string() : res->getString(7);
    out.gender = res->getUint8(8);
    out.online_status = res->isNull(9) ? std::string("N") : res->getString(9);
    out.last_online_at = res->isNull(10) ? 0 : res->getTime(10);
    out.is_robot = res->isNull(11) ? 0 : res->getUint8(11);
    out.is_qiye = res->isNull(12) ? 0 : res->getUint8(12);
    out.is_disabled = res->isNull(13) ? 0 : res->getUint8(13);
    out.created_at = res->getTime(14);
    out.updated_at = res->getTime(15);

    return true;
}

bool UserDAO::GetById(const std::shared_ptr<IM::MySQL>& db, const uint64_t id, User& out,
                      std::string* err) {
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql =
        "SELECT id, mobile, email, nickname, avatar, avatar_media_id, motto, DATE_FORMAT(birthday, "
        "'%Y-%m-%d') as birthday, gender, online_status, last_online_at, is_robot, is_qiye, "
        "is_disabled, created_at, updated_at FROM im_user WHERE id = ? LIMIT 1";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindUint64(1, id);
    auto res = stmt->query();
    if (!res) {
        if (err) *err = "query failed";
        return false;
    }

    if (!res->next()) {
        if (err) *err = "no record found";
        return false;
    }

    out.id = res->getUint64(0);
    out.mobile = res->isNull(1) ? std::string() : res->getString(1);
    out.email = res->isNull(2) ? std::string() : res->getString(2);
    out.nickname = res->isNull(3) ? std::string() : res->getString(3);
    out.avatar = res->isNull(4) ? std::string() : res->getString(4);
    out.avatar_media_id = res->isNull(5) ? std::string() : res->getString(5);
    out.motto = res->isNull(6) ? std::string() : res->getString(6);
    out.birthday = res->isNull(7) ? std::string() : res->getString(7);
    out.gender = res->getUint8(8);
    out.online_status = res->isNull(9) ? std::string("N") : res->getString(9);
    out.last_online_at = res->isNull(10) ? 0 : res->getTime(10);
    out.is_robot = res->isNull(11) ? 0 : res->getUint8(11);
    out.is_qiye = res->isNull(12) ? 0 : res->getUint8(12);
    out.is_disabled = res->isNull(13) ? 0 : res->getUint8(13);
    out.created_at = res->getTime(14);
    out.updated_at = res->getTime(15);

    return true;
}
bool UserDAO::UpdateUserInfo(uint64_t id, const std::string& nickname, const std::string& avatar,
                             const std::string& motto, const uint8_t gender,
                             const std::string& birthday, std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }

    bool is_id = false;
    if (avatar.length() == 32) {
        is_id = true;
        for (char c : avatar) {
            if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))) {
                is_id = false;
                break;
            }
        }
    }

    const char* sql =
        "UPDATE im_user SET nickname = COALESCE(NULLIF(?, ''), nickname), avatar = ?, avatar_media_id = ?, motto = NULLIF(?, ''), gender = ?, "
        "birthday = ?, "
        "updated_at = NOW() WHERE id = ?";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    // 始终绑定 nickname，但 SQL 会在为空时回退到现有值
    stmt->bindString(1, nickname);
    if (!avatar.empty())
        stmt->bindString(2, avatar);
    else
        stmt->bindNull(2);
    if (is_id)
        stmt->bindString(3, avatar);
    else
        stmt->bindNull(3);
    // 将 motto 绑定为字符串，SQL 的 NULLIF 会在空字符串时将其置为 NULL
    stmt->bindString(4, motto);
    stmt->bindUint8(5, gender);
    if (!birthday.empty())
        stmt->bindString(6, birthday);
    else
        stmt->bindNull(6);
    stmt->bindUint64(7, id);
    IM_LOG_DEBUG(g_logger) << "UserDAO::UpdateUserInfo bind values: id=" << id
                           << " nickname='" << nickname << "' avatar='" << avatar
                           << "' avatar_is_id=" << (is_id ? "1" : "0")
                           << " motto='" << motto << "' gender=" << (int)gender
                           << " birthday='" << birthday << "'";
    if (stmt->execute() != 0) {
        if (err) *err = stmt->getErrStr();
        return false;
    }
    return true;
}

bool UserDAO::UpdateMobile(const uint64_t id, const std::string& new_mobile, std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql = "UPDATE im_user SET mobile = ?, updated_at = NOW() WHERE id = ?";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindString(1, new_mobile);
    stmt->bindUint64(2, id);
    if (stmt->execute() != 0) {
        if (err) *err = stmt->getErrStr();
        return false;
    }
    return true;
}

bool UserDAO::UpdateEmail(const uint64_t id, const std::string& new_email, std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql = "UPDATE im_user SET email = ?, updated_at = NOW() WHERE id = ?";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindString(1, new_email);
    stmt->bindUint64(2, id);
    if (stmt->execute() != 0) {
        if (err) *err = stmt->getErrStr();
        return false;
    }
    return true;
}

bool UserDAO::UpdateOnlineStatus(const uint64_t id, std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql = "UPDATE im_user SET online_status = ? WHERE id = ?";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindString(1, "Y");
    stmt->bindUint64(2, id);
    if (stmt->execute() != 0) {
        if (err) *err = stmt->getErrStr();
        return false;
    }
    return true;
}

bool UserDAO::UpdateOfflineStatus(const uint64_t id, std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql = "UPDATE im_user SET online_status = ?, last_online_at = NOW() WHERE id = ?";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = stmt->getErrStr();
        return false;
    }
    stmt->bindString(1, "N");
    stmt->bindUint64(2, id);
    if (stmt->execute() != 0) {
        if (err) *err = stmt->getErrStr();
        return false;
    }
    return true;
}

bool UserDAO::GetOnlineStatus(const uint64_t id, std::string& out_status, std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql = "SELECT online_status FROM im_user WHERE id = ? LIMIT 1";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = stmt->getErrStr();
        return false;
    }
    stmt->bindUint64(1, id);
    auto res = stmt->query();
    if (!res) {
        if (err) *err = "query failed";
        return false;
    }

    if (!res->next()) {
        if (err) *err = "no record found";
        return false;
    }

    out_status = res->isNull(0) ? std::string("N") : res->getString(0);

    return true;
}

bool UserDAO::GetUserInfoSimple(const uint64_t uid, UserInfo& out, std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql =
        "SELECT id, nickname, avatar, motto, gender, is_qiye, mobile, email "
        "FROM im_user WHERE id = ? LIMIT 1";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindUint64(1, uid);
    auto res = stmt->query();
    if (!res) {
        if (err) *err = "query failed";
        return false;
    }

    if (!res->next()) {
        if (err) *err = "no record found";
        return false;
    }

    out.uid = res->isNull(0) ? 0 : res->getUint64(0);
    out.nickname = res->isNull(1) ? std::string() : res->getString(1);
    out.avatar = res->isNull(2) ? std::string() : res->getString(2);
    out.motto = res->isNull(3) ? std::string() : res->getString(3);
    out.gender = res->isNull(4) ? 0 : res->getUint8(4);
    out.is_qiye = res->isNull(5) ? false : (res->getUint8(5) != 0);
    out.mobile = res->isNull(6) ? std::string() : res->getString(6);
    out.email = res->isNull(7) ? std::string() : res->getString(7);

    return true;
}
}  // namespace IM::dao
