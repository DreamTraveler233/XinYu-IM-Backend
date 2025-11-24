#ifndef __IM_API_UPLOAD_API_MODULE_HPP__
#define __IM_API_UPLOAD_API_MODULE_HPP__

#include "domain/service/media_service.hpp"
#include "other/module.hpp"

namespace IM::api {

class UploadApiModule : public IM::Module {
   public:
    UploadApiModule(IM::domain::service::IMediaService::Ptr media_service);
    ~UploadApiModule() override = default;

    bool onServerReady() override;

   private:
    IM::domain::service::IMediaService::Ptr m_media_service;
};

}  // namespace IM::api

#endif  // __IM_API_UPLOAD_API_MODULE_HPP__
