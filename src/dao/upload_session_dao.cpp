#include "dao/upload_session_dao.hpp"

#include "db/mysql.hpp"

namespace IM::dao {

static constexpr const char* kDBName = "default";

bool UploadSessionDAO::Create(const UploadSession& s, std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql =
        "INSERT INTO im_upload_session (upload_id, user_id, file_name, file_size, shard_size, "
        "shard_num, uploaded_count, status, temp_path, created_at, updated_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, NOW(), NOW())";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindString(1, s.upload_id);
    stmt->bindUint64(2, s.user_id);
    stmt->bindString(3, s.file_name);
    stmt->bindUint64(4, s.file_size);
    stmt->bindUint32(5, s.shard_size);
    stmt->bindUint32(6, s.shard_num);
    stmt->bindUint32(7, s.uploaded_count);
    stmt->bindUint8(8, s.status);
    if (!s.temp_path.empty())
        stmt->bindString(9, s.temp_path);
    else
        stmt->bindNull(9);

    if (stmt->execute() != 0) {
        if (err) *err = stmt->getErrStr();
        return false;
    }
    return true;
}

bool UploadSessionDAO::Get(const std::string& upload_id, UploadSession& out, std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql =
        "SELECT upload_id, user_id, file_name, file_size, shard_size, shard_num, uploaded_count, "
        "status, temp_path, created_at FROM im_upload_session WHERE upload_id = ? LIMIT 1";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindString(1, upload_id);
    auto res = stmt->query();
    if (!res) {
        if (err) *err = "query failed";
        return false;
    }

    if (!res->next()) {
        if (err) *err = "no record found";
        return false;
    }

    out.upload_id = res->getString(0);
    out.user_id = res->getUint64(1);
    out.file_name = res->getString(2);
    out.file_size = res->getUint64(3);
    out.shard_size = res->getUint32(4);
    out.shard_num = res->getUint32(5);
    out.uploaded_count = res->getUint32(6);
    out.status = res->getUint8(7);
    out.temp_path = res->isNull(8) ? "" : res->getString(8);
    out.created_at = res->getTime(9);

    return true;
}

bool UploadSessionDAO::UpdateUploadedCount(const std::string& upload_id, uint32_t count,
                                           std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql =
        "UPDATE im_upload_session SET uploaded_count = ?, updated_at = NOW() WHERE upload_id = ?";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindUint32(1, count);
    stmt->bindString(2, upload_id);
    if (stmt->execute() != 0) {
        if (err) *err = stmt->getErrStr();
        return false;
    }
    return true;
}

bool UploadSessionDAO::UpdateStatus(const std::string& upload_id, uint8_t status,
                                    std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql =
        "UPDATE im_upload_session SET status = ?, updated_at = NOW() WHERE upload_id = ?";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindUint8(1, status);
    stmt->bindString(2, upload_id);
    if (stmt->execute() != 0) {
        if (err) *err = stmt->getErrStr();
        return false;
    }
    return true;
}

}  // namespace IM::dao
