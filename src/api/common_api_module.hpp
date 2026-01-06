#ifndef __IM_API_COMMON_API_MODULE_HPP__
#define __IM_API_COMMON_API_MODULE_HPP__

#include "infra/module/module.hpp"
#include"domain/service/common_service.hpp"
#include"domain/service/user_service.hpp"

namespace IM::api {

class CommonApiModule : public IM::Module {
   public:
    CommonApiModule(IM::domain::service::ICommonService::Ptr common_service,
                    IM::domain::service::IUserService::Ptr user_service);
    ~CommonApiModule() override = default;

    bool onServerReady() override;

    private:
    IM::domain::service::ICommonService::Ptr m_common_service;
    IM::domain::service::IUserService::Ptr m_user_service;
};

}  // namespace IM::api

#endif // __IM_API_COMMON_API_MODULE_HPP__
