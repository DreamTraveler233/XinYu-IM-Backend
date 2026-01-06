#ifndef __IM_MODEL_MEDIA_SESSION_HPP__
#define __IM_MODEL_MEDIA_SESSION_HPP__

#include <cstdint>
#include <string>

namespace IM::model {

struct UploadSession {
    std::string upload_id;
    uint64_t user_id = 0;
    std::string file_name;
    uint64_t file_size = 0;
    uint32_t shard_size = 0;
    uint32_t shard_num = 0;
    uint32_t uploaded_count = 0;
    uint8_t status = 0;
    std::string temp_path;
    std::string created_at;
};

}  // namespace IM::model

#endif  // __IM_MODEL_MEDIA_SESSION_HPP__