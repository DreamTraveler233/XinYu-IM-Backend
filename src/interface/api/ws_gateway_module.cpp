#include "interface/api/ws_gateway_module.hpp"

#include <jwt-cpp/jwt.h>

#include <atomic>
#include <unordered_map>
#include <vector>

#include "core/base/macro.hpp"
#include "common/common.hpp"
#include "core/config/config.hpp"
#include "core/net/core/address.hpp"
#include "core/net/rock/rock_stream.hpp"
#include "core/net/http/ws_server.hpp"
#include "core/net/http/ws_servlet.hpp"
#include "core/net/http/ws_session.hpp"
#include "core/system/application.hpp"
#include "core/util/util.hpp"
#include "core/util/trace_context.hpp"
#include "domain/repository/talk_repository.hpp"
#include "application/rpc/talk_repository_rpc_client.hpp"

namespace IM::api {

static auto g_logger = IM_LOG_NAME("root");

// 静态 talk repo（用于静态 PushImMessage 调用）
static IM::domain::repository::ITalkRepository::Ptr s_talk_repo = nullptr;

static IM::domain::repository::ITalkRepository::Ptr GetTalkRepoForBroadcast() {
    if (s_talk_repo) return s_talk_repo;
    static IM::domain::repository::ITalkRepository::Ptr s_fallback =
        std::make_shared<IM::app::rpc::TalkRepositoryRpcClient>();
    return s_fallback;
}

WsGatewayModule::WsGatewayModule(IM::domain::service::IUserService::Ptr user_service,
                                 IM::domain::repository::ITalkRepository::Ptr talk_repo)
    : RockModule("ws.gateway", "0.1.0", "builtin"),
      m_user_service(std::move(user_service)),
      m_talk_repo(std::move(talk_repo)) {
    // 保存静态引用，供静态方法使用
    s_talk_repo = m_talk_repo;
}

// 简易查询串解析（假设无需URL解码，前端传递 token 直接可用）
static std::unordered_map<std::string, std::string> ParseQueryKV(const std::string& q) {
    std::unordered_map<std::string, std::string> kv;
    if (q.empty()) return kv;
    size_t start = 0;
    while (start < q.size()) {
        size_t amp = q.find('&', start);
        if (amp == std::string::npos) amp = q.size();
        size_t eq = q.find('=', start);
        if (eq != std::string::npos && eq < amp) {
            std::string k = q.substr(start, eq - start);
            std::string v = q.substr(eq + 1, amp - (eq + 1));
            kv[k] = v;
        } else {
            // 只有key没有value
            std::string k = q.substr(start, amp - start);
            if (!k.empty()) kv[k] = "";
        }
        start = amp + 1;
    }
    return kv;
}

// 连接上下文与会话表（首版进程内，支持多连接）
struct ConnCtx {
    uint64_t uid = 0;
    std::string platform;  // web|pc|app，默认 web
    std::string conn_id;   // 连接唯一ID
};

static std::atomic<uint64_t> s_conn_seq{1};
static IM::RWMutex s_ws_mutex;  // 保护会话表
// 记录连接与上下文、会话弱引用，key: WSSession* 原始地址
struct ConnItem {
    ConnCtx ctx;
    std::weak_ptr<IM::http::WSSession> weak;
};
static std::unordered_map<void*, ConnItem> s_ws_conns;

// Forward declarations for helper functions used in routing helpers
static void SendEvent(IM::http::WSSession::ptr session, const std::string& event,
                      const Json::Value& payload, const std::string& ackid);
static std::vector<IM::http::WSSession::ptr> CollectSessions(uint64_t uid);

namespace {
constexpr uint32_t kCmdDeliverToUser = 101;
constexpr uint32_t kPresenceCmdSetOnline = 201;
constexpr uint32_t kPresenceCmdSetOffline = 202;
constexpr uint32_t kPresenceCmdHeartbeat = 203;
constexpr uint32_t kPresenceCmdGetRoute = 204;

constexpr uint32_t kPresenceTimeoutMs = 300;
constexpr uint32_t kDeliverTimeoutMs = 500;
constexpr uint32_t kPresenceTtlSec = 120;

static IM::RWMutex s_rpc_mutex;
static std::unordered_map<std::string, IM::RockConnection::ptr> s_rpc_conns;
static std::atomic<uint32_t> s_rock_req_sn{1};

static auto g_presence_rpc_addr =
    IM::Config::Lookup("presence.rpc_addr", std::string(""), "presence rpc address ip:port");

static bool SplitIpPort(const std::string& ip_port, std::string& ip, uint16_t& port) {
    auto pos = ip_port.find(':');
    if (pos == std::string::npos) {
        return false;
    }
    ip = ip_port.substr(0, pos);
    try {
        auto p = std::stoul(ip_port.substr(pos + 1));
        if (p == 0 || p > 65535) {
            return false;
        }
        port = static_cast<uint16_t>(p);
        return true;
    } catch (...) {
        return false;
    }
}

static std::string GetLocalRockAddr() {
    std::vector<IM::TcpServer::ptr> rockServers;
    if (!IM::Application::GetInstance()->getServer("rock", rockServers)) {
        return "";
    }
    for (auto& s : rockServers) {
        auto socks = s->getSocks();
        for (auto& sock : socks) {
            auto addr = std::dynamic_pointer_cast<IM::IPv4Address>(sock->getLocalAddress());
            if (!addr) {
                continue;
            }
            auto str = addr->toString();
            if (str.find("127.0.0.1") == 0) {
                continue;
            }
            if (str.find("0.0.0.0") == 0) {
                return IM::GetIPv4() + ":" + std::to_string(addr->getPort());
            }
            return str;
        }
    }
    return "";
}

static IM::RockConnection::ptr GetOrCreateRpcConn(const std::string& ip_port) {
    if (ip_port.empty()) {
        return nullptr;
    }

    {
        IM::RWMutex::ReadLock lock(s_rpc_mutex);
        auto it = s_rpc_conns.find(ip_port);
        if (it != s_rpc_conns.end() && it->second && it->second->isConnected()) {
            return it->second;
        }
    }

    std::string ip;
    uint16_t port = 0;
    if (!SplitIpPort(ip_port, ip, port)) {
        return nullptr;
    }

    auto addr = IM::Address::LookupAnyIpAddress(ip);
    if (!addr) {
        return nullptr;
    }
    addr->setPort(port);

    IM::RockConnection::ptr conn(new IM::RockConnection);
    if (!conn->connect(addr)) {
        return nullptr;
    }
    conn->start();

    IM::RWMutex::WriteLock lock(s_rpc_mutex);
    s_rpc_conns[ip_port] = conn;
    return conn;
}

static IM::ServiceItemInfo::ptr PickService(const std::string& domain, const std::string& service) {
    auto sd = IM::Application::GetInstance()->getServiceDiscovery();
    if (!sd) {
        return nullptr;
    }
    std::unordered_map<
        std::string,
        std::unordered_map<std::string, std::unordered_map<uint64_t, IM::ServiceItemInfo::ptr>>>
        infos;
    sd->listServer(infos);
    auto itD = infos.find(domain);
    if (itD == infos.end()) {
        sd->queryServer(domain, service);
        return nullptr;
    }
    auto itS = itD->second.find(service);
    if (itS == itD->second.end() || itS->second.empty()) {
        sd->queryServer(domain, service);
        return nullptr;
    }
    // 简单挑一个（map 顺序不保证，但足够用于最小可用）
    return itS->second.begin()->second;
}

static IM::RockResult::ptr RockJsonRequest(const std::string& ip_port, uint32_t cmd,
                                          const Json::Value& body, uint32_t timeout_ms) {
    auto conn = GetOrCreateRpcConn(ip_port);
    if (!conn || !conn->isConnected()) {
        return std::make_shared<IM::RockResult>(IM::AsyncSocketStream::NOT_CONNECT, 0, nullptr,
                                                nullptr);
    }
    IM::RockRequest::ptr req = std::make_shared<IM::RockRequest>();
    req->setSn(s_rock_req_sn.fetch_add(1));
    req->setCmd(cmd);
    req->setBody(IM::JsonUtil::ToString(body));
    return conn->request(req, timeout_ms);
}

static std::string PresenceRequestGateway(uint32_t cmd, const Json::Value& body, uint32_t timeout_ms,
                                         int32_t* out_code = nullptr) {
    // 1) 优先使用固定地址（避免动态 queryServer 依赖 ZK 60s tick 的延迟）
    const auto fixed = g_presence_rpc_addr->getValue();
    if (!fixed.empty()) {
        auto rr = RockJsonRequest(fixed, cmd, body, timeout_ms);
        if (!rr || !rr->response) {
            if (out_code) *out_code = 503;
            return "";
        }
        if (out_code) *out_code = rr->response->getResult();
        return rr->response->getBody();
    }

    // 通过服务发现挑一个 presence 实例
    auto info = PickService("im", "svc-presence");
    if (!info) {
        if (out_code) *out_code = 503;
        return "";
    }
    const std::string addr = info->getIp() + ":" + std::to_string(info->getPort());
    auto rr = RockJsonRequest(addr, cmd, body, timeout_ms);
    if (!rr || !rr->response) {
        if (out_code) *out_code = 503;
        return "";
    }
    if (out_code) *out_code = rr->response->getResult();
    return rr->response->getBody();
}

static void PresenceSetOnline(uint64_t uid) {
    const auto local_rpc = GetLocalRockAddr();
    if (uid == 0 || local_rpc.empty()) {
        return;
    }
    Json::Value body;
    body["uid"] = (Json::UInt64)uid;
    body["gateway_rpc"] = local_rpc;
    body["ttl_sec"] = (Json::UInt)kPresenceTtlSec;
    PresenceRequestGateway(kPresenceCmdSetOnline, body, kPresenceTimeoutMs);
}

static void PresenceHeartbeat(uint64_t uid) {
    const auto local_rpc = GetLocalRockAddr();
    if (uid == 0 || local_rpc.empty()) {
        return;
    }
    Json::Value body;
    body["uid"] = (Json::UInt64)uid;
    body["gateway_rpc"] = local_rpc;
    body["ttl_sec"] = (Json::UInt)kPresenceTtlSec;
    PresenceRequestGateway(kPresenceCmdHeartbeat, body, kPresenceTimeoutMs);
}

static void PresenceSetOffline(uint64_t uid) {
    if (uid == 0) {
        return;
    }
    Json::Value body;
    body["uid"] = (Json::UInt64)uid;
    PresenceRequestGateway(kPresenceCmdSetOffline, body, kPresenceTimeoutMs);
}

static std::string PresenceGetRoute(uint64_t uid) {
    if (uid == 0) {
        return "";
    }
    Json::Value body;
    body["uid"] = (Json::UInt64)uid;
    int32_t code = 0;
    const auto rsp_body = PresenceRequestGateway(kPresenceCmdGetRoute, body, kPresenceTimeoutMs, &code);
    if (code != 200 || rsp_body.empty()) {
        return "";
    }
    Json::Value out;
    if (!IM::JsonUtil::FromString(out, rsp_body)) {
        return "";
    }
    return IM::JsonUtil::GetString(out, "gateway_rpc");
}

static void PushToUserLocalOnly(uint64_t uid, const std::string& event, const Json::Value& payload,
                                const std::string& ackid) {
    auto sessions = CollectSessions(uid);
    for (auto& s : sessions) {
        SendEvent(s, event, payload, ackid);
    }
}

static void DeliverToGatewayRpc(const std::string& gateway_rpc, uint64_t uid, const std::string& event,
                                const Json::Value& payload) {
    if (gateway_rpc.empty() || uid == 0 || event.empty()) {
        return;
    }
    Json::Value body;
    body["uid"] = (Json::UInt64)uid;
    body["event"] = event;
    body["payload"] = payload;
    RockJsonRequest(gateway_rpc, kCmdDeliverToUser, body, kDeliverTimeoutMs);
}
}  // namespace

// 发送下行统一封装：{"event":"...","payload":{...},"ackid":"..."}
static void SendEvent(IM::http::WSSession::ptr session, const std::string& event,
                      const Json::Value& payload, const std::string& ackid = "") {
    Json::Value root;
    root["event"] = event;
    root["payload"] = payload.isNull() ? Json::Value(Json::objectValue) : payload;
    if (!ackid.empty()) root["ackid"] = ackid;
    session->sendMessage(IM::JsonUtil::ToString(root));
}

// 根据 uid 收集当前在线的会话（强引用），避免长时间持锁
static std::vector<IM::http::WSSession::ptr> CollectSessions(uint64_t uid) {
    std::vector<IM::http::WSSession::ptr> out;
    {
        IM::RWMutex::ReadLock lock(s_ws_mutex);
        out.reserve(s_ws_conns.size());
        for (auto& kv : s_ws_conns) {
            const auto& item = kv.second;
            if (item.ctx.uid != uid) {
                continue;
            }
            if (auto sp = item.weak.lock()) {
                out.push_back(std::move(sp));
            }
        }
    }
    return out;
}

bool WsGatewayModule::onServerReady() {
    std::vector<IM::TcpServer::ptr> wsServers;
    // 1. 获取所有已注册的WebSocket服务器实例
    if (!IM::Application::GetInstance()->getServer("ws", wsServers)) {
        IM_LOG_WARN(g_logger) << "no ws servers found when registering ws routes";
        return true;
    }

    // 2. 遍历每个WebSocket服务器，注册路由与事件回调
    for (auto& s : wsServers) {
        auto ws = std::dynamic_pointer_cast<IM::http::WSServer>(s);
        if (!ws) continue;
        auto dispatch = ws->getWSServletDispatch();

        /* 注册 WebSocket 路由回调 */
        // 2.1 连接建立回调：鉴权、会话登记、欢迎包
        auto on_connect = [](IM::http::HttpRequest::ptr header,
                             IM::http::WSSession::ptr session) -> int32_t {
            // 读取查询串 ?token=...&platform=...
            const std::string query = header->getQuery();
            auto kv = ParseQueryKV(query);
            const std::string token = IM::GetParamValue<std::string>(kv, "token", "");
            std::string platform = IM::GetParamValue<std::string>(kv, "platform", "web");
            std::string suid;

            // 1) 校验token（JWT），失败则发错误事件并关闭
            if (token.empty() || !VerifyJwt(token, &suid) || suid.empty()) {
                Json::Value err;
                err["error_code"] = 401;
                err["error_message"] = "unauthorized";
                SendEvent(session, "event_error", err);
                return -1;  // 触发上层关闭
            }

            // 2) 解析uid，校验合法性
            uint64_t uid = 0;
            try {
                uid = std::stoull(suid);
            } catch (...) {
                uid = 0;
            }
            if (uid == 0) {
                Json::Value err;
                err["error_code"] = 401;
                err["error_message"] = "invalid uid";
                SendEvent(session, "event_error", err);
                return -1;
            }

            // 3) 构造连接上下文，写锁保护下登记到全局会话表
            ConnCtx ctx;
            ctx.uid = uid;
            ctx.platform = platform.empty() ? std::string("web") : platform;
            ctx.conn_id = std::to_string(s_conn_seq.fetch_add(1));

            {
                IM::RWMutex::WriteLock lock(s_ws_mutex);
                ConnItem item;
                item.ctx = ctx;
                item.weak = session;
                s_ws_conns[(void*)session.get()] = std::move(item);
            }

            // 4) 发送欢迎包，event="connect"
            Json::Value payload;
            payload["uid"] = Json::UInt64(uid);
            payload["platform"] = ctx.platform;
            payload["ts"] = (Json::UInt64)IM::TimeUtil::NowToMS();
            SendEvent(session, "connect", payload);

            // 5) 上报 presence：uid -> 当前网关 Rock RPC 地址
            PresenceSetOnline(uid);
            return 0;
        };

        // 2.2 连接关闭回调：移除会话表
        auto on_close = [this](IM::http::HttpRequest::ptr /*header*/,
                               IM::http::WSSession::ptr session) -> int32_t {
            // 获取连接上下文
            ConnCtx ctx;
            {
                IM::RWMutex::ReadLock lock(s_ws_mutex);
                auto it = s_ws_conns.find((void*)session.get());
                if (it != s_ws_conns.end()) {
                    ctx = it->second.ctx;
                }
            }

            // 执行下线操作：更新用户在线状态为离线
            if (ctx.uid != 0) {
                auto offline_result = m_user_service->Offline(ctx.uid);
                if (!offline_result.ok) {
                    IM_LOG_ERROR(g_logger)
                        << "Offline failed for uid=" << ctx.uid << ", err=" << offline_result.err;
                }

                // presence 下线
                PresenceSetOffline(ctx.uid);
            }

            // 移除会话表
            {
                IM::RWMutex::WriteLock lock(s_ws_mutex);
                s_ws_conns.erase((void*)session.get());
            }
            return 0;
        };

        // 2.3 消息处理回调：事件分发、心跳、回显等
        auto on_message = [](IM::http::HttpRequest::ptr /*header*/,
                             IM::http::WSFrameMessage::ptr msg,
                             IM::http::WSSession::ptr session) -> int32_t {
            // 仅处理文本帧，忽略二进制和控制帧
            if (!msg || msg->getOpcode() != IM::http::WSFrameHead::TEXT_FRAME) {
                return 0;
            }
            const std::string& data = msg->getData();
            Json::Value root;
            // 1) 解析JSON消息体，非对象型忽略
            if (!IM::JsonUtil::FromString(root, data) || !root.isObject()) {
                return 0;  // 非JSON忽略
            }
            // 前端封装为 {"event": event, "payload": payload}
            const std::string event = IM::JsonUtil::GetString(root, "event");
            const Json::Value payload =
                root.isMember("payload") ? root["payload"] : Json::Value(Json::objectValue);

            std::string trace_id = IM::JsonUtil::GetString(root, "trace_id");
            if (trace_id.empty()) {
                trace_id = TraceContext::GenerateTraceId();
            }
            TraceGuard guard(trace_id);

            // 2) 内置事件处理
            if (event == "ping") {
                // 应用层心跳，回复pong
                Json::Value p;
                p["ts"] = (Json::UInt64)IM::TimeUtil::NowToMS();
                SendEvent(session, "pong", p);

                // 续租 presence TTL
                ConnCtx ctx;
                {
                    IM::RWMutex::ReadLock lock(s_ws_mutex);
                    auto it = s_ws_conns.find((void*)session.get());
                    if (it != s_ws_conns.end()) {
                        ctx = it->second.ctx;
                    }
                }
                if (ctx.uid != 0) {
                    PresenceHeartbeat(ctx.uid);
                }
                return 0;
            }
            if (event == "ack") {
                // 收到ACK，可做去重确认，这里暂忽略
                return 0;
            }
            if (event == "echo") {
                // 回显测试
                SendEvent(session, "echo", payload);
                return 0;
            }

            // 转发正在输入事件
            if (event == "im.message.keyboard") {
                if (payload.isMember("talk_mode") && payload.isMember("to_from_id")) {
                    int talk_mode = payload["talk_mode"].asInt();
                    uint64_t to_from_id = 0;
                    if (payload["to_from_id"].isString()) {
                        to_from_id = std::stoull(payload["to_from_id"].asString());
                    } else {
                        to_from_id = payload["to_from_id"].asUInt64();
                    }

                    // 获取当前发送者ID
                    ConnCtx ctx;
                    {
                        IM::RWMutex::ReadLock lock(s_ws_mutex);
                        auto it = s_ws_conns.find((void*)session.get());
                        if (it != s_ws_conns.end()) {
                            ctx = it->second.ctx;
                        }
                    }

                    if (ctx.uid != 0) {
                        Json::Value fwd = payload;
                        fwd["from_id"] = (Json::UInt64)ctx.uid;

                        if (talk_mode == 1) {
                            // 单聊：直接推送给对方
                            PushToUser(to_from_id, "im.message.keyboard", fwd);
                        }
                        // 群聊暂不广播输入状态，避免消息风暴
                    }
                }
                return 0;
            }

            // 3) 其他事件留给后续业务模块拓展，这里仅记录日志
            IM_LOG_DEBUG(g_logger) << "unhandled ws event: " << event;
            return 0;
        };

        // 2.4 注册路径：固定 /wss/default.io，同时开放 /wss/* 以便未来扩展
        dispatch->addServlet("/wss/default.io", on_message, on_connect, on_close);
        dispatch->addGlobServlet("/wss/*", on_message, on_connect, on_close);
    }

    return true;
}

bool WsGatewayModule::onServerUp() {
    registerService("ws", "im", "gateway-ws");
    registerService("rock", "im", "gateway-ws-rpc");

    // 让服务发现开始 watch presence 服务，供 PushToUser 路由查询使用
    if (auto sd = IM::Application::GetInstance()->getServiceDiscovery()) {
        sd->queryServer("im", "svc-presence");
    }
    return true;
}

bool WsGatewayModule::handleRockRequest(IM::RockRequest::ptr request, IM::RockResponse::ptr response,
                                       IM::RockStream::ptr stream) {
    // 处理指令：101 - 跨进程消息投递
    if (request->getCmd() == 101) {
        Json::Value body;
        if (!IM::JsonUtil::FromString(body, request->getBody())) {
            response->setResult(400);
            response->setResultStr("invalid json body");
            return true;
        }

        uint64_t uid = IM::JsonUtil::GetUint64(body, "uid");
        std::string event = IM::JsonUtil::GetString(body, "event");
        Json::Value payload = body["payload"];

        if (uid == 0 || event.empty()) {
            response->setResult(400);
            response->setResultStr("missing uid or event");
            return true;
        }

        // 核心：调用本地推送逻辑
        IM_LOG_INFO(g_logger) << "RPC Deliver: uid=" << uid << " event=" << event;
        PushToUserLocalOnly(uid, event, payload, "");

        response->setResult(200);
        return true;
    }

    return false;
}

bool WsGatewayModule::handleRockNotify(IM::RockNotify::ptr notify, IM::RockStream::ptr stream) {
    return false;
}

// ===== 主动推送接口实现 =====
void WsGatewayModule::PushToUser(uint64_t uid, const std::string& event, const Json::Value& payload,
                                 const std::string& ackid) {
    // 1) 本机有连接则直接推送
    {
        auto sessions = CollectSessions(uid);
        if (!sessions.empty()) {
            for (auto& s : sessions) {
                SendEvent(s, event, payload, ackid);
            }
            return;
        }
    }

    // 2) 本机无连接：查询 presence 路由到目标网关
    const auto gateway_rpc = PresenceGetRoute(uid);
    if (gateway_rpc.empty()) {
        return;
    }

    // 3) 避免误投递到本机导致 RPC 回环
    const auto local_rpc = GetLocalRockAddr();
    if (!local_rpc.empty() && gateway_rpc == local_rpc) {
        return;
    }

    DeliverToGatewayRpc(gateway_rpc, uid, event, payload);
}

void WsGatewayModule::PushImMessage(uint8_t talk_mode, uint64_t to_from_id, uint64_t from_id,
                                    const Json::Value& body) {
    Json::Value payload;
    payload["to_from_id"] = to_from_id;
    payload["from_id"] = from_id;
    payload["talk_mode"] = talk_mode;
    payload["body"] = body;

    if (talk_mode == 1) {
        // 单聊：推送给接收方和发送方
        // 不再在服务端做 ID 交换，统一推送标准 payload
        PushToUser(to_from_id, "im.message", payload);
    } else {
        // 群聊：通过 talk repository 查出会话内用户并广播
        try {
            auto talk_repo = GetTalkRepoForBroadcast();
            if (talk_repo) {
                uint64_t talk_id = 0;
                std::string terr;
                if (talk_repo->getGroupTalkId(to_from_id, talk_id, &terr)) {
                    std::vector<uint64_t> talk_users;
                    std::string lerr;
                    if (talk_repo->listUsersByTalkId(talk_id, talk_users, &lerr)) {
                        for (auto uid : talk_users) {
                            PushToUser(uid, "im.message", payload);
                        }
                        return;
                    }
                }
            }
        } catch (const std::exception& ex) {
            IM_LOG_WARN(g_logger) << "broadcast im.message failed: " << ex.what();
        }
    }
}

}  // namespace IM::api
