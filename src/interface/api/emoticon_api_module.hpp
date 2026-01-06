#ifndef __IM_API_EMOTICON_API_MODULE_HPP__
#define __IM_API_EMOTICON_API_MODULE_HPP__

#include "infra/module/module.hpp"

namespace IM::api {

class EmoticonApiModule : public IM::Module {
   public:
    EmoticonApiModule();
    ~EmoticonApiModule() override = default;

    bool onServerReady() override;
};

}  // namespace IM::api

#endif // __IM_API_EMOTICON_API_MODULE_HPP__