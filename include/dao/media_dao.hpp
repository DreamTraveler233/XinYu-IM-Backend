#ifndef __IM_DAO_MEDIA_DAO_HPP__
#define __IM_DAO_MEDIA_DAO_HPP__

#include <memory>
#include <string>
#include <vector>

#include "db/mysql.hpp"

namespace IM::dao {

struct MediaFile {
    std::string id;           // char(32)
    std::string upload_id;    // 可选
    uint64_t user_id = 0;
    std::string file_name;
    uint64_t file_size = 0;
    std::string mime;
    uint8_t storage_type = 1;
    std::string storage_path;
    std::string url;
    uint8_t status = 1;
    std::string created_at;
};

class MediaDAO {
   public:
    static bool Create(const MediaFile& f, std::string* err = nullptr);
    static bool GetByUploadId(const std::string& upload_id, MediaFile& out,
                              std::string* err = nullptr);
    static bool GetById(const std::string& id, MediaFile& out, std::string* err = nullptr);
};

}  // namespace IM::dao

#endif  // __IM_DAO_MEDIA_DAO_HPP__
