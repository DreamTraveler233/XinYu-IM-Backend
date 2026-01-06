#include "interface/api/static_file_module.hpp"

#include "core/config/config.hpp"
#include "core/net/http/http_server.hpp"
#include "core/net/http/servlets/static_servlet.hpp"
#include "core/system/application.hpp"
#include "core/system/env.hpp"

namespace IM::api {

static auto g_upload_base_dir = IM::Config::Lookup<std::string>(
    "media.upload_base_dir", std::string("data/uploads"), "base dir for uploaded media files");

StaticFileModule::StaticFileModule() : Module("StaticFileModule", "1.0", "StaticFileModule") {}

bool StaticFileModule::onLoad() {
    return true;
}

bool StaticFileModule::onServerReady() {
    std::vector<IM::TcpServer::ptr> svrs;
    if (!IM::Application::GetInstance()->getServer("http", svrs)) {
        return true;
    }

    // Resolve base dir
    std::string path = g_upload_base_dir->getValue();
    if (path.empty()) {
        return true;
    }
    if (path[0] != '/') {
        path = IM::EnvMgr::GetInstance()->getAbsoluteWorkPath(path);
    }

    auto servlet = std::make_shared<IM::http::StaticServlet>(path, "/media/");

    for (auto& i : svrs) {
        IM::http::HttpServer::ptr http_server = std::dynamic_pointer_cast<IM::http::HttpServer>(i);
        if (http_server) {
            http_server->getServletDispatch()->addGlobServlet("/media/*", servlet);
        }
    }
    return true;
}

}  // namespace IM::api
