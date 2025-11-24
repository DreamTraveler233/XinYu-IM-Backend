#include "infra/repository/media_repository_impl.hpp"

namespace IM::infra::repository {

static constexpr const char* kDBName = "default";

MediaRepositoryImpl::MediaRepositoryImpl(std::shared_ptr<IM::MySQLManager> db_manager)
    : m_db_manager(std::move(db_manager)) {}

bool MediaRepositoryImpl::CreateMediaFile(const model::MediaFile& f, std::string* err) {
    auto db = m_db_manager->get(kDBName);
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

bool MediaRepositoryImpl::GetMediaFileByUploadId(const std::string& upload_id,
                                                 model::MediaFile& out, std::string* err) {
    auto db = m_db_manager->get(kDBName);
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

bool MediaRepositoryImpl::GetMediaFileById(const std::string& id, model::MediaFile& out,
                                           std::string* err) {
    auto db = m_db_manager->get(kDBName);
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

bool MediaRepositoryImpl::CreateMediaSession(const model::UploadSession& s, std::string* err) {
    auto db = m_db_manager->get(kDBName);
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

bool MediaRepositoryImpl::GetMediaSessionByUploadId(const std::string& upload_id,
                                                    model::UploadSession& out, std::string* err) {
    auto db = m_db_manager->get(kDBName);
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

bool MediaRepositoryImpl::UpdateUploadedCount(const std::string& upload_id, uint32_t count,
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

bool MediaRepositoryImpl::UpdateMediaSessionStatus(const std::string& upload_id, uint8_t status,
                                                   std::string* err) {
    auto db = m_db_manager->get(kDBName);
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

}  // namespace IM::infra::repository