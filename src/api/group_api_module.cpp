#include "api/group_api_module.hpp"

#include "base/macro.hpp"
#include "common/common.hpp"
#include "http/http_server.hpp"
#include "http/http_servlet.hpp"
#include "system/application.hpp"
#include "util/util.hpp"

namespace IM::api {

static auto g_logger = IM_LOG_NAME("root");

GroupApiModule::GroupApiModule() : Module("api.group", "0.1.0", "builtin") {}

bool GroupApiModule::onServerReady() {
    std::vector<IM::TcpServer::ptr> httpServers;
    if (!IM::Application::GetInstance()->getServer("http", httpServers)) {
        IM_LOG_WARN(g_logger) << "no http servers found when registering group routes";
        return true;
    }

    for (auto& s : httpServers) {
        auto http = std::dynamic_pointer_cast<IM::http::HttpServer>(s);
        if (!http) continue;
        auto dispatch = http->getServletDispatch();

        // 群组申请
        dispatch->addServlet(
            "/api/v1/group-apply/agree",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                res->setBody(Ok());
                return 0;
            });
        dispatch->addServlet(
            "/api/v1/group-apply/all",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                Json::Value d;
                d["list"] = Json::Value(Json::arrayValue);
                res->setBody(Ok(d));
                return 0;
            });
        dispatch->addServlet(
            "/api/v1/group-apply/create",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                res->setBody(Ok());
                return 0;
            });
        dispatch->addServlet(
            "/api/v1/group-apply/decline",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                res->setBody(Ok());
                return 0;
            });
        dispatch->addServlet(
            "/api/v1/group-apply/delete",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                res->setBody(Ok());
                return 0;
            });
        dispatch->addServlet(
            "/api/v1/group-apply/list",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                Json::Value d;
                d["list"] = Json::Value(Json::arrayValue);
                res->setBody(Ok(d));
                return 0;
            });
        dispatch->addServlet(
            "/api/v1/group-apply/unread-num",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                Json::Value d;
                d["count"] = 0;
                res->setBody(Ok(d));
                return 0;
            });

        // 群组公告
        dispatch->addServlet(
            "/api/v1/group-notice/edit",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                res->setBody(Ok());
                return 0;
            });

        // 群组投票
        dispatch->addServlet(
            "/api/v1/group-vote/create",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                res->setBody(Ok());
                return 0;
            });
        dispatch->addServlet(
            "/api/v1/group-vote/detail",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                Json::Value d(Json::objectValue);
                res->setBody(Ok(d));
                return 0;
            });
        dispatch->addServlet(
            "/api/v1/group-vote/submit",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                res->setBody(Ok());
                return 0;
            });

        // 群组主要接口
        dispatch->addServlet(
            "/api/v1/group/assign-admin",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                res->setBody(Ok());
                return 0;
            });
        dispatch->addServlet("/api/v1/group/create", [this](IM::http::HttpRequest::ptr /*req*/,
                                                        IM::http::HttpResponse::ptr res,
                                                        IM::http::HttpSession::ptr /*session*/) {
            res->setHeader("Content-Type", "application/json");
            Json::Value d(Json::objectValue);
            d["group_id"] = static_cast<Json::Int64>(0);
            res->setBody(Ok(d));
            return 0;
        });
        dispatch->addServlet("/api/v1/group/detail", [this](IM::http::HttpRequest::ptr /*req*/,
                                                        IM::http::HttpResponse::ptr res,
                                                        IM::http::HttpSession::ptr /*session*/) {
            res->setHeader("Content-Type", "application/json");
            Json::Value d(Json::objectValue);
            res->setBody(Ok(d));
            return 0;
        });
        dispatch->addServlet("/api/v1/group/dismiss", [this](IM::http::HttpRequest::ptr /*req*/,
                                                         IM::http::HttpResponse::ptr res,
                                                         IM::http::HttpSession::ptr /*session*/) {
            res->setHeader("Content-Type", "application/json");
            res->setBody(Ok());
            return 0;
        });
        dispatch->addServlet(
            "/api/v1/group/get-invite-friends",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                Json::Value d;
                d["list"] = Json::Value(Json::arrayValue);
                res->setBody(Ok(d));
                return 0;
            });
        dispatch->addServlet("/api/v1/group/handover", [this](IM::http::HttpRequest::ptr /*req*/,
                                                          IM::http::HttpResponse::ptr res,
                                                          IM::http::HttpSession::ptr /*session*/) {
            res->setHeader("Content-Type", "application/json");
            res->setBody(Ok());
            return 0;
        });
        dispatch->addServlet("/api/v1/group/invite", [this](IM::http::HttpRequest::ptr /*req*/,
                                                        IM::http::HttpResponse::ptr res,
                                                        IM::http::HttpSession::ptr /*session*/) {
            res->setHeader("Content-Type", "application/json");
            res->setBody(Ok());
            return 0;
        });
        dispatch->addServlet("/api/v1/group/list", [this](IM::http::HttpRequest::ptr /*req*/,
                                                      IM::http::HttpResponse::ptr res,
                                                      IM::http::HttpSession::ptr /*session*/) {
            res->setHeader("Content-Type", "application/json");
            Json::Value d;
            d["list"] = Json::Value(Json::arrayValue);
            res->setBody(Ok(d));
            return 0;
        });
        dispatch->addServlet(
            "/api/v1/group/member-list",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                Json::Value d;
                d["list"] = Json::Value(Json::arrayValue);
                res->setBody(Ok(d));
                return 0;
            });
        dispatch->addServlet("/api/v1/group/mute", [this](IM::http::HttpRequest::ptr /*req*/,
                                                      IM::http::HttpResponse::ptr res,
                                                      IM::http::HttpSession::ptr /*session*/) {
            res->setHeader("Content-Type", "application/json");
            res->setBody(Ok());
            return 0;
        });
        dispatch->addServlet("/api/v1/group/no-speak", [this](IM::http::HttpRequest::ptr /*req*/,
                                                          IM::http::HttpResponse::ptr res,
                                                          IM::http::HttpSession::ptr /*session*/) {
            res->setHeader("Content-Type", "application/json");
            res->setBody(Ok());
            return 0;
        });
        dispatch->addServlet("/api/v1/group/overt", [this](IM::http::HttpRequest::ptr /*req*/,
                                                       IM::http::HttpResponse::ptr res,
                                                       IM::http::HttpSession::ptr /*session*/) {
            res->setHeader("Content-Type", "application/json");
            res->setBody(Ok());
            return 0;
        });
        dispatch->addServlet(
            "/api/v1/group/overt-list",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                Json::Value d;
                d["list"] = Json::Value(Json::arrayValue);
                res->setBody(Ok(d));
                return 0;
            });
        dispatch->addServlet(
            "/api/v1/group/remark-update",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                res->setBody(Ok());
                return 0;
            });
        dispatch->addServlet(
            "/api/v1/group/remove-member",
            [this](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
               IM::http::HttpSession::ptr /*session*/) {
                res->setHeader("Content-Type", "application/json");
                res->setBody(Ok());
                return 0;
            });
        dispatch->addServlet("/api/v1/group/secede", [this](IM::http::HttpRequest::ptr /*req*/,
                                                        IM::http::HttpResponse::ptr res,
                                                        IM::http::HttpSession::ptr /*session*/) {
            res->setHeader("Content-Type", "application/json");
            res->setBody(Ok());
            return 0;
        });
        dispatch->addServlet("/api/v1/group/setting", [this](IM::http::HttpRequest::ptr /*req*/,
                                                         IM::http::HttpResponse::ptr res,
                                                         IM::http::HttpSession::ptr /*session*/) {
            res->setHeader("Content-Type", "application/json");
            Json::Value d;
            d["notify"] = true;
            d["mute"] = false;
            res->setBody(Ok(d));
            return 0;
        });
    }
    return true;
}

}  // namespace IM::api
