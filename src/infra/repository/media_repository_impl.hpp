#ifndef __IM_INFRA_REPOSITORY_UPLOAD_REPOSITORY_IMPL_HPP__
#define __IM_INFRA_REPOSITORY_UPLOAD_REPOSITORY_IMPL_HPP__

#include "infra/db/mysql.hpp"
#include "domain/repository/media_repository.hpp"

namespace IM::infra::repository {

class MediaRepositoryImpl : public IM::domain::repository::IMediaRepository {
   public:
    explicit MediaRepositoryImpl(std::shared_ptr<IM::MySQLManager> db_manager);

    bool CreateMediaFile(const model::MediaFile& f, std::string* err = nullptr) override;
    bool GetMediaFileByUploadId(const std::string& upload_id, model::MediaFile& out,
                                std::string* err = nullptr) override;
    bool GetMediaFileById(const std::string& id, model::MediaFile& out,
                          std::string* err = nullptr) override;
    bool CreateMediaSession(const model::UploadSession& s, std::string* err = nullptr) override;
    bool GetMediaSessionByUploadId(const std::string& upload_id, model::UploadSession& out,
                                   std::string* err = nullptr) override;
    bool UpdateUploadedCount(const std::string& upload_id, uint32_t count,
                             std::string* err = nullptr) override;
    bool UpdateMediaSessionStatus(const std::string& upload_id, uint8_t status,
                                  std::string* err = nullptr) override;

   private:
    std::shared_ptr<IM::MySQLManager> m_db_manager;
};

}  // namespace IM::infra::repository

#endif  // __IM_INFRA_REPOSITORY_UPLOAD_REPOSITORY_IMPL_HPP__