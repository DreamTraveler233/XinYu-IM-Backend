#ifndef __IM_API_USER_API_MODULE_HPP__
#define __IM_API_USER_API_MODULE_HPP__

#include "domain/service/user_service.hpp"
#include "infra/module/module.hpp"

namespace IM::api {

class UserApiModule : public IM::Module {
   public:
    UserApiModule(const IM::domain::service::IUserService::Ptr& user_service);
    ~UserApiModule() override = default;

    bool onServerReady() override;
    bool onServerUp() override;

   private:
    IM::domain::service::IUserService::Ptr m_user_service;
};

}  // namespace IM::api

#endif  // __IM_API_USER_API_MODULE_HPP__