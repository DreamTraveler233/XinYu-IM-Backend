#ifndef __IM_APP_MEDIA_SERVICE_HPP__
#define __IM_APP_MEDIA_SERVICE_HPP__

#include <string>
#include <vector>

#include "dao/media_dao.hpp"
#include "dao/upload_session_dao.hpp"

namespace IM::app {

class MediaService {
   public:
    // 初始化分片上传
    static bool InitMultipartUpload(uint64_t user_id, const std::string& file_name,
                                    uint64_t file_size, std::string& out_upload_id,
                                    uint32_t& out_shard_size, std::string* err = nullptr);

    // 上传分片
    static bool UploadPart(const std::string& upload_id, uint32_t split_index, uint32_t split_num,
                           const std::string& data, std::string* err = nullptr);

    // 单文件上传
    static bool UploadFile(uint64_t user_id, const std::string& file_name, const std::string& data,
                           dao::MediaFile& out_media, std::string* err = nullptr);

    // 获取媒体文件信息
    static bool GetMediaFile(const std::string& media_id, dao::MediaFile& out_media,
                             std::string* err = nullptr);

    // 初始化临时分片目录清理定时器（可安全调用多次）
    static void InitTempCleanupTimer();

   private:
    static std::string GetStoragePath(const std::string& file_name);
    static std::string GetTempPath(const std::string& upload_id);
    static bool MergeParts(const dao::UploadSession& session, std::string* err);
};

}  // namespace IM::app

#endif  // __IM_APP_MEDIA_SERVICE_HPP__
