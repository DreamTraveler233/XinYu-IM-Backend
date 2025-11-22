#include "api/article_api_module.hpp"

#include "base/macro.hpp"
#include "common/common.hpp"
#include "http/http_server.hpp"
#include "http/http_servlet.hpp"
#include "system/application.hpp"
#include "util/util.hpp"

namespace IM::api {
static auto g_logger = IM_LOG_NAME("root");

ArticleApiModule::ArticleApiModule() : Module("api.article", "0.1.0", "builtin") {}

bool ArticleApiModule::onServerReady() {
    std::vector<IM::TcpServer::ptr> httpServers;
    if (!IM::Application::GetInstance()->getServer("http", httpServers)) {
        IM_LOG_WARN(g_logger) << "no http servers found when registering article routes";
        return true;
    }

    for (auto& s : httpServers) {
        auto http = std::dynamic_pointer_cast<IM::http::HttpServer>(s);
        if (!http) continue;
        auto dispatch = http->getServletDispatch();

        // 文章附件
        dispatch->addServlet("/api/v1/article-annex/delete",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 res->setBody(Ok());
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/article-annex/forever-delete",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 res->setBody(Ok());
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/article-annex/recover",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 res->setBody(Ok());
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/article-annex/recover-list",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 Json::Value d;
                                 d["list"] = Json::Value(Json::arrayValue);
                                 res->setBody(Ok(d));
                                 return 0;
                             });

        // 文章分类
        dispatch->addServlet("/api/v1/article/classify/delete",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 res->setBody(Ok());
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/article/classify/edit",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 res->setBody(Ok());
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/article/classify/list",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 Json::Value d;
                                 d["list"] = Json::Value(Json::arrayValue);
                                 res->setBody(Ok(d));
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/article/classify/sort",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 res->setBody(Ok());
                                 return 0;
                             });

        // 文章核心接口
        dispatch->addServlet("/api/v1/article/delete",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 res->setBody(Ok());
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/article/detail",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 Json::Value d(Json::objectValue);
                                 res->setBody(Ok(d));
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/article/editor",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 res->setBody(Ok());
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/article/forever-delete",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 res->setBody(Ok());
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/article/list",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 Json::Value d;
                                 d["list"] = Json::Value(Json::arrayValue);
                                 res->setBody(Ok(d));
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/article/move",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 res->setBody(Ok());
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/article/recover",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 res->setBody(Ok());
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/article/recover-list",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 Json::Value d;
                                 d["list"] = Json::Value(Json::arrayValue);
                                 res->setBody(Ok(d));
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/article/tags",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 res->setBody(Ok());
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/article/asterisk",
                             [](IM::http::HttpRequest::ptr, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 res->setBody(Ok());
                                 return 0;
                             });
    }
    return true;
}

}  // namespace IM::api
