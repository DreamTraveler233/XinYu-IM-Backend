#ifndef __IM_DAO_UPLOAD_SESSION_DAO_HPP__
#define __IM_DAO_UPLOAD_SESSION_DAO_HPP__

#include <memory>
#include <string>
#include <vector>

#include "db/mysql.hpp"

namespace IM::dao {

struct UploadSession {
    std::string upload_id;
    uint64_t user_id = 0;
    std::string file_name;
    uint64_t file_size = 0;
    uint32_t shard_size = 0;
    uint32_t shard_num = 0;
    uint32_t uploaded_count = 0;
    uint8_t status = 0;  // 0: uploading, 1: completed, 2: failed
    std::string temp_path;
    std::string created_at;
};

class UploadSessionDAO {
   public:
    static bool Create(const UploadSession& s, std::string* err = nullptr);
    static bool Get(const std::string& upload_id, UploadSession& out, std::string* err = nullptr);
    static bool UpdateUploadedCount(const std::string& upload_id, uint32_t count,
                                    std::string* err = nullptr);
    static bool UpdateStatus(const std::string& upload_id, uint8_t status,
                             std::string* err = nullptr);
};

}  // namespace IM::dao

#endif  // __IM_DAO_UPLOAD_SESSION_DAO_HPP__
