#ifndef __IM_APP_MEDIA_SERVICE_IMPL_HPP__
#define __IM_APP_MEDIA_SERVICE_IMPL_HPP__

#include "domain/repository/media_repository.hpp"
#include "domain/service/media_service.hpp"
namespace IM::app {

class MediaServiceImpl : public IM::domain::service::IMediaService {
   public:
    explicit MediaServiceImpl(IM::domain::repository::IMediaRepository::Ptr media_repo);

    Result<model::UploadSession> InitMultipartUpload(const uint64_t user_id,
                                                     const std::string& file_name,
                                                     const uint64_t file_size) override;
    Result<bool> UploadPart(const std::string& upload_id, const uint32_t split_index,
                            const uint32_t split_num, const std::string& data) override;
    Result<model::MediaFile> UploadFile(const uint64_t user_id, const std::string& file_name,
                                        const std::string& data) override;
    Result<model::MediaFile> GetMediaFile(const std::string& media_id) override;
    Result<model::MediaFile> GetMediaFileByUploadId(const std::string& upload_id) override;
    void InitTempCleanupTimer() override;

   private:
    std::string GetStoragePath(const std::string& file_name) override;
    std::string GetTempPath(const std::string& upload_id) override;
    Result<model::MediaFile> MergeParts(const model::UploadSession& session) override;

   private:
    IM::domain::repository::IMediaRepository::Ptr m_media_repo;
};

}  // namespace IM::app

#endif  // __IM_APP_MEDIA_SERVICE_IMPL_HPP__