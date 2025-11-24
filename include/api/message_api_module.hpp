#ifndef __IM_API_MESSAGE_API_MODULE_HPP__
#define __IM_API_MESSAGE_API_MODULE_HPP__

#include "domain/service/message_service.hpp"
#include "other/module.hpp"

namespace IM::api {

class MessageApiModule : public IM::Module {
   public:
    MessageApiModule(IM::domain::service::IMessageService::Ptr message_service);
    ~MessageApiModule() override = default;

    bool onServerReady() override;

   private:
    IM::domain::service::IMessageService::Ptr m_message_service;
};

}  // namespace IM::api

#endif  // __IM_API_MESSAGE_API_MODULE_HPP__