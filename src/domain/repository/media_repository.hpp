#ifndef __IM_DOMAIN_REPOSITORY_UPLOAD_REPOSITORY_HPP__
#define __IM_DOMAIN_REPOSITORY_UPLOAD_REPOSITORY_HPP__

#include <memory>
#include <string>

#include "model/media_file.hpp"
#include "model/upload_session.hpp"

namespace IM::domain::repository {

class IMediaRepository {
   public:
    using Ptr = std::shared_ptr<IMediaRepository>;
    virtual ~IMediaRepository() = default;

    // 创建媒体文件记录
    virtual bool CreateMediaFile(const model::MediaFile& f, std::string* err = nullptr) = 0;

    // 根据 upload_id 获取媒体文件记录
    virtual bool GetMediaFileByUploadId(const std::string& upload_id, model::MediaFile& out,
                                        std::string* err = nullptr) = 0;

    // 根据 id 获取媒体文件记录
    virtual bool GetMediaFileById(const std::string& id, model::MediaFile& out,
                                  std::string* err = nullptr) = 0;

    // 创建上传会话记录
    virtual bool CreateMediaSession(const model::UploadSession& s, std::string* err = nullptr) = 0;

    // 根据 upload_id 获取上传会话记录
    virtual bool GetMediaSessionByUploadId(const std::string& upload_id, model::UploadSession& out,
                                           std::string* err = nullptr) = 0;

    // 更新已上传分片数量
    virtual bool UpdateUploadedCount(const std::string& upload_id, uint32_t count,
                                     std::string* err = nullptr) = 0;

    // 更新上传会话状态
    virtual bool UpdateMediaSessionStatus(const std::string& upload_id, uint8_t status,
                                          std::string* err = nullptr) = 0;
};

}  // namespace IM::domain::repository

#endif  // __IM_DOMAIN_REPOSITORY_UPLOAD_REPOSITORY_HPP__