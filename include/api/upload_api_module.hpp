#ifndef __IM_API_UPLOAD_API_MODULE_HPP__
#define __IM_API_UPLOAD_API_MODULE_HPP__

#include "other/module.hpp"

namespace IM::api {

class UploadApiModule : public IM::Module {
   public:
    UploadApiModule();
    ~UploadApiModule() override = default;

    bool onServerReady() override;
};

}  // namespace IM::api

#endif // __IM_API_UPLOAD_API_MODULE_HPP__
