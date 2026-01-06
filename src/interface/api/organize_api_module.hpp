#ifndef __IM_API_ORGANIZE_API_MODULE_HPP__
#define __IM_API_ORGANIZE_API_MODULE_HPP__

#include "infra/module/module.hpp"

namespace IM::api {

class OrganizeApiModule : public IM::Module {
   public:
    OrganizeApiModule();
    ~OrganizeApiModule() override = default;

    bool onServerReady() override;
};

}  // namespace IM::api

#endif // __IM_API_ORGANIZE_API_MODULE_HPP__