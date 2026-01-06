#include "interface/api/common_api_module.hpp"

#include "core/base/macro.hpp"
#include "common/common.hpp"
#include "core/net/http/http_server.hpp"
#include "core/net/http/http_servlet.hpp"
#include "core/system/application.hpp"
#include "core/util/util.hpp"

namespace IM::api {

static auto g_logger = IM_LOG_NAME("root");

CommonApiModule::CommonApiModule(IM::domain::service::ICommonService::Ptr common_service,
                                 IM::domain::service::IUserService::Ptr user_service)
    : Module("api.common", "0.1.0", "builtin"),
      m_common_service(std::move(common_service)),
      m_user_service(std::move(user_service)) {}

bool CommonApiModule::onServerReady() {
    std::vector<IM::TcpServer::ptr> httpServers;
    if (!IM::Application::GetInstance()->getServer("http", httpServers)) {
        IM_LOG_WARN(g_logger) << "no http servers found when registering common routes";
        return true;
    }

    // 初始化验证码清理定时器（幂等）
    m_common_service->InitCleanupTimer();

    for (auto& s : httpServers) {
        auto http = std::dynamic_pointer_cast<IM::http::HttpServer>(s);
        if (!http) continue;
        auto dispatch = http->getServletDispatch();

        // 发送短信验证码
        dispatch->addServlet("/api/v1/common/send-sms", [this](IM::http::HttpRequest::ptr req,
                                                               IM::http::HttpResponse::ptr res,
                                                               IM::http::HttpSession::ptr session) {
            res->setHeader("Content-Type", "application/json");

            std::string mobile, channel;
            Json::Value body;
            if (ParseBody(req->getBody(), body)) {
                mobile = IM::JsonUtil::GetString(body, "mobile");
                channel = IM::JsonUtil::GetString(body, "channel");
            }

            /*判断手机号是否已经注册*/
            auto ret = m_user_service->GetUserByMobile(mobile, channel);
            if (!ret.ok) {
                res->setStatus(ToHttpStatus(ret.code));
                res->setBody(Error(ret.code, ret.err));
                return 0;
            }

            /* 发送短信验证码 */
            auto result = m_common_service->SendSmsCode(mobile, channel, session);
            if (!result.ok) {
                res->setStatus(ToHttpStatus(result.code));
                res->setBody(Error(result.code, result.err));
                return 0;
            }

            Json::Value data;
            data["sms_code"] = result.data.code;
            res->setBody(Ok(data));
            return 0;
        });

        // 注册邮箱服务
        dispatch->addServlet("/api/v1/common/send-email",
                             [this](IM::http::HttpRequest::ptr req, IM::http::HttpResponse::ptr res,
                                    IM::http::HttpSession::ptr session) {
                                 res->setHeader("Content-Type", "application/json");

                                 std::string email, channel;
                                 Json::Value body;
                                 if (ParseBody(req->getBody(), body)) {
                                     email = IM::JsonUtil::GetString(body, "email");
                                     channel = IM::JsonUtil::GetString(body, "channel");
                                 }

                                 /*判断邮箱是否已经注册*/
                                 auto ret = m_user_service->GetUserByEmail(email, channel);
                                 if (!ret.ok) {
                                     res->setStatus(ToHttpStatus(ret.code));
                                     res->setBody(Error(ret.code, ret.err));
                                     return 0;
                                 }

                                 auto result =
                                     m_common_service->SendEmailCode(email, channel, session);
                                 if (!result.ok) {
                                     res->setStatus(ToHttpStatus(result.code));
                                     res->setBody(Error(result.code, result.err));
                                     return 0;
                                 }

                                 Json::Value data;
                                 // 为了方便调试，返回验证码，生产环境应移除
                                 data["code"] = result.data.code;
                                 res->setBody(Ok(data));
                                 return 0;
                             });

        // 验证邮箱验证码
        dispatch->addServlet("/api/v1/common/verify-email",
                             [this](IM::http::HttpRequest::ptr req, IM::http::HttpResponse::ptr res,
                                    IM::http::HttpSession::ptr /*session*/) {
                                 res->setHeader("Content-Type", "application/json");

                                 std::string email, code, channel;
                                 Json::Value body;
                                 if (ParseBody(req->getBody(), body)) {
                                     email = IM::JsonUtil::GetString(body, "email");
                                     code = IM::JsonUtil::GetString(body, "code");
                                     channel = IM::JsonUtil::GetString(body, "channel");
                                 }

                                 auto result =
                                     m_common_service->VerifyEmailCode(email, code, channel);
                                 if (!result.ok) {
                                     res->setStatus(ToHttpStatus(result.code));
                                     res->setBody(Error(result.code, result.err));
                                     return 0;
                                 }

                                 res->setBody(Ok());
                                 return 0;
                             });

        // 测试接口（占位）
        dispatch->addServlet("/api/v1/common/send-test",
                             [](IM::http::HttpRequest::ptr /*req*/, IM::http::HttpResponse::ptr res,
                                IM::http::HttpSession::ptr /*session*/) {
                                 res->setHeader("Content-Type", "application/json");
                                 Json::Value data;
                                 data["echo"] = true;
                                 res->setBody(IM::JsonUtil::ToString(data));
                                 return 0;
                             });
    }
    return true;
}

}  // namespace IM::api
