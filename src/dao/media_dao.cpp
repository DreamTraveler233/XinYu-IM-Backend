#include "dao/media_dao.hpp"

#include "db/mysql.hpp"

namespace IM::dao {

static constexpr const char* kDBName = "default";

bool MediaDAO::Create(const MediaFile& f, std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql =
        "INSERT INTO im_media_file (id, upload_id, user_id, file_name, file_size, mime, "
        "storage_type, storage_path, url, status, created_at, updated_at) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NOW(), NOW())";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindString(1, f.id);
    if (!f.upload_id.empty())
        stmt->bindString(2, f.upload_id);
    else
        stmt->bindNull(2);
    stmt->bindUint64(3, f.user_id);
    stmt->bindString(4, f.file_name);
    stmt->bindUint64(5, f.file_size);
    if (!f.mime.empty())
        stmt->bindString(6, f.mime);
    else
        stmt->bindNull(6);
    stmt->bindUint8(7, f.storage_type);
    stmt->bindString(8, f.storage_path);
    if (!f.url.empty())
        stmt->bindString(9, f.url);
    else
        stmt->bindNull(9);
    stmt->bindUint8(10, f.status);

    if (stmt->execute() != 0) {
        if (err) *err = stmt->getErrStr();
        return false;
    }
    return true;
}

bool MediaDAO::GetByUploadId(const std::string& upload_id, MediaFile& out, std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql =
        "SELECT id, upload_id, user_id, file_name, file_size, mime, storage_type, storage_path, "
        "url, status, created_at FROM im_media_file WHERE upload_id = ? LIMIT 1";
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

    out.id = res->getString(0);
    out.upload_id = res->isNull(1) ? "" : res->getString(1);
    out.user_id = res->getUint64(2);
    out.file_name = res->getString(3);
    out.file_size = res->getUint64(4);
    out.mime = res->isNull(5) ? "" : res->getString(5);
    out.storage_type = res->getUint8(6);
    out.storage_path = res->getString(7);
    out.url = res->isNull(8) ? "" : res->getString(8);
    out.status = res->getUint8(9);
    out.created_at = res->getTime(10);

    return true;
}

bool MediaDAO::GetById(const std::string& id, MediaFile& out, std::string* err) {
    auto db = IM::MySQLMgr::GetInstance()->get(kDBName);
    if (!db) {
        if (err) *err = "get mysql connection failed";
        return false;
    }
    const char* sql =
        "SELECT id, upload_id, user_id, file_name, file_size, mime, storage_type, storage_path, "
        "url, status, created_at FROM im_media_file WHERE id = ? LIMIT 1";
    auto stmt = db->prepare(sql);
    if (!stmt) {
        if (err) *err = "prepare sql failed";
        return false;
    }
    stmt->bindString(1, id);
    auto res = stmt->query();
    if (!res) {
        if (err) *err = "query failed";
        return false;
    }

    if (!res->next()) {
        if (err) *err = "no record found";
        return false;
    }

    out.id = res->getString(0);
    out.upload_id = res->isNull(1) ? "" : res->getString(1);
    out.user_id = res->getUint64(2);
    out.file_name = res->getString(3);
    out.file_size = res->getUint64(4);
    out.mime = res->isNull(5) ? "" : res->getString(5);
    out.storage_type = res->getUint8(6);
    out.storage_path = res->getString(7);
    out.url = res->isNull(8) ? "" : res->getString(8);
    out.status = res->getUint8(9);
    out.created_at = res->getTime(10);

    return true;
}

}  // namespace IM::dao
