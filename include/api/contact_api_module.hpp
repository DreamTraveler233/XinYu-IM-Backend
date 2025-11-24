#ifndef __IM_API_CONTACT_API_MODULE_HPP__
#define __IM_API_CONTACT_API_MODULE_HPP__

#include "domain/service/contact_service.hpp"
#include "domain/service/user_service.hpp"
#include "other/module.hpp"

namespace IM::api {

class ContactApiModule : public IM::Module {
   public:
    ContactApiModule(IM::domain::service::IContactService::Ptr contact_service,
                     IM::domain::service::IUserService::Ptr user_service);
    ~ContactApiModule() override = default;

    bool onServerReady() override;

   private:
    IM::domain::service::IContactService::Ptr m_contact_service;
    IM::domain::service::IUserService::Ptr m_user_service;
};

}  // namespace IM::api

#endif  // __IM_API_CONTACT_API_MODULE_HPP__