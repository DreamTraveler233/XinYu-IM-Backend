#ifndef __IM_API_STATIC_FILE_MODULE_HPP__
#define __IM_API_STATIC_FILE_MODULE_HPP__

#include "infra/module/module.hpp"

namespace IM::api {

class StaticFileModule : public IM::Module {
public:
    typedef std::shared_ptr<StaticFileModule> ptr;
    StaticFileModule();
    virtual bool onLoad() override;
    virtual bool onServerReady() override;
};

}

#endif
