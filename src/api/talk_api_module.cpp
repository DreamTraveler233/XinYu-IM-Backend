#include "api/talk_api_module.hpp"

#include "app/talk_service.hpp"
#include "base/macro.hpp"
#include "common/common.hpp"
#include "http/http_server.hpp"
#include "http/http_servlet.hpp"
#include "system/application.hpp"
#include "util/util.hpp"

namespace CIM::api {

static auto g_logger = CIM_LOG_NAME("root");

TalkApiModule::TalkApiModule() : Module("api.talk", "0.1.0", "builtin") {}

bool TalkApiModule::onServerReady() {
    std::vector<CIM::TcpServer::ptr> httpServers;
    if (!CIM::Application::GetInstance()->getServer("http", httpServers)) {
        CIM_LOG_WARN(g_logger) << "no http servers found when registering talk routes";
        return true;
    }

    for (auto& s : httpServers) {
        auto http = std::dynamic_pointer_cast<CIM::http::HttpServer>(s);
        if (!http) continue;
        auto dispatch = http->getServletDispatch();

        dispatch->addServlet("/api/v1/talk/session-clear-unread-num",
                             [](CIM::http::HttpRequest::ptr, CIM::http::HttpResponse::ptr res,
                                CIM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 res->setBody(Ok());
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/talk/session-create",
                             [](CIM::http::HttpRequest::ptr, CIM::http::HttpResponse::ptr res,
                                CIM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 Json::Value d(Json::objectValue);
                                 d["session_id"] = static_cast<Json::Int64>(0);
                                 res->setBody(Ok(d));
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/talk/session-delete",
                             [](CIM::http::HttpRequest::ptr, CIM::http::HttpResponse::ptr res,
                                CIM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 res->setBody(Ok());
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/talk/session-disturb",
                             [](CIM::http::HttpRequest::ptr, CIM::http::HttpResponse::ptr res,
                                CIM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");
                                 res->setBody(Ok());
                                 return 0;
                             });
        dispatch->addServlet("/api/v1/talk/session-list", [](CIM::http::HttpRequest::ptr req,
                                                             CIM::http::HttpResponse::ptr res,
                                                             CIM::http::HttpSession::ptr) {
            res->setHeader("Content-Type", "application/json");

            // 获取用户ID
            auto uid_result = GetUidFromToken(req, res);
            if (!uid_result.ok) {
                res->setStatus(ToHttpStatus(uid_result.code));
                res->setBody(Error(uid_result.code, uid_result.err));
                return 0;
            }

            // 获取会话列表
            auto result = CIM::app::TalkService::getSessionListByUserId(uid_result.data);
            if (!result.ok) {
                res->setStatus(ToHttpStatus(result.code));
                res->setBody(Error(result.code, result.err));
                return 0;
            }

            Json::Value data;
            data["id"] = Json::Value();          // 会话id
            data["talk_mode"] = Json::Value();   // 会话模式
            data["to_from_id"] = Json::Value();  // 目标用户ID
            data["is_top"] = Json::Value();      // 是否置顶
            data["is_disturb"] = Json::Value();  // 是否免打扰
            data["is_robot"] = Json::Value();    // 是否机器人
            data["name"] = Json::Value();        // 会话名称
            data["avatar"] = Json::Value();      // 会话头像
            data["remark"] = Json::Value();      // 会话备注
            data["unread_num"] = Json::Value();  // 未读消息数
            data["msg_text"] = Json::Value();    // 最后一条消息预览文本
            data["updated_at"] = Json::Value();  // 最后更新时间
            Json::Value items;
            items["items"] = data;
            res->setBody(Ok(items));
            return 0;
        });
        dispatch->addServlet("/api/v1/talk/session-top",
                             [](CIM::http::HttpRequest::ptr req, CIM::http::HttpResponse::ptr res,
                                CIM::http::HttpSession::ptr) {
                                 res->setHeader("Content-Type", "application/json");

                                 uint64_t to_from_id;
                                 uint8_t talk_mode, action;
                                 Json::Value body;
                                 if (ParseBody(req->getBody(), body)) {
                                    to_from_id = CIM::JsonUtil::GetUint64(body, "to_from_id");
                                    talk_mode = static_cast<uint8_t>(CIM::JsonUtil::GetUint32(body, "talk_mode"));
                                    action = static_cast<uint8_t>(CIM::JsonUtil::GetUint32(body, "action"));
                                 }

                                 // 设置是否置顶
                                 auto result = CIM::app::TalkService::setSessionTop(to_from_id, talk_mode, action);
                                 if (!result.ok) {
                                     res->setStatus(ToHttpStatus(result.code));
                                     res->setBody(Error(result.code, result.err));
                                     return 0;
                                 }

                                 res->setBody(Ok());
                                 return 0;
                             });
    }

    CIM_LOG_INFO(g_logger) << "talk routes registered";
    return true;
}

}  // namespace CIM::api
