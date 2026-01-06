#include "system/application.hpp"

#include <limits.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "base/macro.hpp"
#include "config/config.hpp"
#include "io/fox_thread.hpp"
#include "infra/db/redis.hpp"
#include "net/http/ws_server.hpp"
#include "io/worker.hpp"
#include "net/core/tcp_server.hpp"
#include "ns/name_server_module.hpp"
#include "infra/module/module.hpp"
#include "net/rock/rock_server.hpp"
#include "net/rock/rock_stream.hpp"
#include "system/daemon.hpp"
#include "system/env.hpp"
#include "util/id_worker.hpp"
#include <thread>

namespace IM {
static auto g_logger = IM_LOG_NAME("system");

// 用于在信号处理器中删除 PID 文件（以 C 字符串形式保存，信号处理安全）
static char s_pidfile[PATH_MAX] = {0};

// atexit handler
static void at_exit_cleanup() {
    if (Application::GetInstance()) {
        Application::GetInstance()->cleanup();
    }
}

/**
 * @brief 信号处理器：删除 PID 文件并使信号以默认方式触发（重抛）
 */
static void sig_exit_handler(int sig) {
    if (s_pidfile[0]) {
        ::unlink(s_pidfile);
    }
    // 恢复默认信号处理并再次触发信号以终止进程
    signal(sig, SIG_DFL);
    raise(sig);
}

// 服务器工作目录配置项，决定服务运行时的工作目录路径。
// 默认值："apps/work/IM"
static auto g_server_work_path =
    Config::Lookup("server.work_path", std::string("apps/work/IM"), "server work path");

// 服务器PID文件配置项，指定进程ID文件的名称。
// 默认值："IM.pid"
static auto g_server_pid_file =
    Config::Lookup("server.pid_file", std::string("IM.pid"), "server pid file");

// 服务发现Zookeeper地址配置项，用于分布式服务注册与发现。
// 默认值：空字符串（表示不启用）
static auto g_service_discovery_zk =
    Config::Lookup("service_discovery.zk", std::string(""), "service discovery zookeeper");

// 服务器配置列表，包含所有需要启动的服务器参数。
// 默认值：空列表
static auto g_servers_conf =
    Config::Lookup("servers", std::vector<TcpServerConf>(), "http server config");

static auto g_machine_code =
    Config::Lookup<uint16_t>("machine.code", uint16_t(0), "machine code for id worker");

Application* Application::s_instance = nullptr;

Application::Application() {
    s_instance = this;
}

void Application::cleanup() {
    if (!m_pidfile.empty()) {
        // Only remove pidfile if it actually belongs to this process
        bool removed = false;
        {
            std::ifstream ifs(m_pidfile);
            int pid = -1;
            if (ifs && (ifs >> pid)) {
                if (pid == getpid()) {
                    removed = FSUtil::Unlink(m_pidfile, true);
                } else {
                    IM_LOG_WARN(g_logger)
                        << "pidfile " << m_pidfile << " not belong to current pid: " << pid
                        << " currentpid=" << getpid();
                }
            } else {
                // If cannot read pidfile, still attempt to unlink it
                removed = FSUtil::Unlink(m_pidfile, true);
            }
        }
        if (removed) {
            IM_LOG_INFO(g_logger) << "pidfile removed: " << m_pidfile;
        } else {
            IM_LOG_ERROR(g_logger) << "failed to remove pidfile: " << m_pidfile;
        }
        // reset static c-string to avoid any further accidental uses
        if (sizeof(s_pidfile) > 0) {
            memset(s_pidfile, 0, sizeof(s_pidfile));
        }
    }
    // Attempt to stop the main IO manager to initiate a graceful shutdown
    if (m_mainIOManager) {
        // If we're in the context of the main IOManager (i.e., cleanup called inside it),
        // avoid directly calling stop() to prevent coroutine state assertions. Instead,
        // call stop() from another thread so the main loop can shutdown safely.
        IOManager* current = IOManager::GetThis();
        if (current && current == m_mainIOManager.get()) {
            std::thread([this]() {
                if (m_mainIOManager) {
                    m_mainIOManager->stop();
                }
            })
                .detach();
        } else {
            m_mainIOManager->stop();
        }
    }
}

void Application::Exit(int code) {
    if (GetInstance()) {
        GetInstance()->cleanup();
    }
    _exit(code);
}

/**
     * @brief 应用初始化流程
     * @param argc 命令行参数个数
     * @param argv 命令行参数数组
     * @return 初始化成功返回 true，失败返回 false
     *
     * 主要流程：
     * 1. 解析命令行参数，注册帮助信息
     * 2. 判断是否需要打印帮助信息
     * 3. 加载配置文件
     * 4. 初始化并通知所有模块（参数解析前/后）
     * 5. 判断运行模式（前台/后台），未指定则打印帮助
     * 6. 检查 PID 文件，防止重复启动
     * 7. 创建工作目录
     */
bool Application::init(int argc, char** argv) {
    m_argc = argc;
    m_argv = argv;

    /*添加命令行帮助信息*/
    EnvMgr::GetInstance()->addHelp("s", "start with the terminal");
    EnvMgr::GetInstance()->addHelp("d", "run as daemon");
    EnvMgr::GetInstance()->addHelp("c", "conf path default: ./conf");
    EnvMgr::GetInstance()->addHelp("h", "print help");

    bool is_print_help = false;
    if (!EnvMgr::GetInstance()->init(argc, argv)) {
        is_print_help = true;
    }

    if (EnvMgr::GetInstance()->has("p")) {
        is_print_help = true;
    }

    /*加载配置文件*/
    std::string conf_path = EnvMgr::GetInstance()->getConfigPath();
    IM_LOG_INFO(g_logger) << "load conf path:" << conf_path;
    Config::LoadFromConfigDir(conf_path);

    /*初始化模块管理器并获取所有模块*/
    ModuleMgr::GetInstance()->init();
    std::vector<Module::ptr> modules;
    ModuleMgr::GetInstance()->listAll(modules);

    /*在参数解析前调用各模块的回调函数*/
    for (auto i : modules) {
        i->onBeforeArgsParse(argc, argv);
    }

    /*如果需要打印帮助信息，则打印后返回*/
    if (is_print_help) {
        EnvMgr::GetInstance()->printHelp();
        return false;
    }

    /*在参数解析后调用各模块的回调函数*/
    for (auto i : modules) {
        i->onAfterArgsParse(argc, argv);
    }
    modules.clear();

    /*确定运行类型：1-前台运行，2-后台运行*/
    int run_type = 0;
    if (EnvMgr::GetInstance()->has("s")) {
        run_type = 1;
    }
    if (EnvMgr::GetInstance()->has("d")) {
        run_type = 2;
    }

    /*如果未指定运行类型，则打印帮助信息并返回*/
    if (run_type == 0) {
        EnvMgr::GetInstance()->printHelp();
        return false;
    }

    /*检查服务是否已经在运行*/
    std::string pidfile = g_server_work_path->getValue() + "/" + g_server_pid_file->getValue();
    // 将 pidfile 路径保存到实例，以便后续清理
    m_pidfile = pidfile;
    if (FSUtil::IsRunningPidfile(m_pidfile)) {
        IM_LOG_ERROR(g_logger) << "server is running:" << pidfile;
        return false;
    }

    /*创建工作目录*/
    if (!FSUtil::Mkdir(g_server_work_path->getValue())) {
        IM_LOG_FATAL(g_logger) << "create work path [" << g_server_work_path->getValue() << "] fail"
                               << " errno=" << errno << " errstr=" << strerror(errno);
        return false;
    }
    return true;
}

bool Application::run() {
    bool is_daemon = EnvMgr::GetInstance()->has("d");
    return start_daemon(
        m_argc, m_argv,
        std::bind(&Application::main, this, std::placeholders::_1, std::placeholders::_2),
        is_daemon);
}

/**
     * @brief 应用程序主函数
     * @param argc 主函数参数个数
     * @param argv 主函数参数列表
     * @return 执行结果，正常退出返回0
     *
     * 该函数主要完成以下工作：
     * 1. 忽略SIGPIPE信号
     * 2. 记录日志并加载配置
     * 3. 写入进程ID到pid文件
     * 4. 创建主IO管理器并调度运行协程
     * 5. 添加定时器并启动事件循环
     */
int Application::main(int argc, char** argv) {
    // 忽略SIGPIPE信号，避免socket连接断开时程序异常退出
        {
            struct sigaction sa_pipe;
            memset(&sa_pipe, 0, sizeof(sa_pipe));
            sa_pipe.sa_handler = SIG_IGN;
            sigemptyset(&sa_pipe.sa_mask);
            sa_pipe.sa_flags = 0;
            sigaction(SIGPIPE, &sa_pipe, nullptr);
        }
    IM_LOG_INFO(g_logger) << "main";

    // 获取配置路径并重新加载配置
    std::string conf_path = EnvMgr::GetInstance()->getConfigPath();
    Config::LoadFromConfigDir(conf_path, true);

    // 将当前进程ID写入pid文件
    {
        std::string pidfile = g_server_work_path->getValue() + "/" + g_server_pid_file->getValue();
        std::ofstream ofs(pidfile);
        if (!ofs) {
            IM_LOG_ERROR(g_logger) << "open pidfile " << pidfile << " failed";
            return false;
        }
        ofs << getpid();
    }
    // 在写入 pidfile 后，设置全局静态 s_pidfile，注册退出回调和信号处理
    {
        std::string pidfile = g_server_work_path->getValue() + "/" + g_server_pid_file->getValue();
        // 保存到实例
        m_pidfile = pidfile;
        // 保存到静态 C 字符串，供信号处理使用（确保长度不超过 PATH_MAX）
        if (!pidfile.empty()) {
            strncpy(s_pidfile, pidfile.c_str(), sizeof(s_pidfile) - 1);
            s_pidfile[sizeof(s_pidfile) - 1] = '\0';
        }
        // 注册 atexit 清理函数
        atexit(at_exit_cleanup);
        // 注册信号处理器，确保在收到信号时仍然能清理 pid 文件
            struct sigaction sa_exit;
            memset(&sa_exit, 0, sizeof(sa_exit));
            sa_exit.sa_handler = sig_exit_handler;
            sigemptyset(&sa_exit.sa_mask);
            sa_exit.sa_flags = 0;
            sigaction(SIGINT, &sa_exit, nullptr);
            sigaction(SIGTERM, &sa_exit, nullptr);
            sigaction(SIGQUIT, &sa_exit, nullptr);
            sigaction(SIGHUP, &sa_exit, nullptr);
    }

    // 创建主IO管理器并调度执行协程
    m_mainIOManager.reset(new IOManager(1, true, "main"));
    m_mainIOManager->schedule(std::bind(&Application::run_coroutine, this));

    // 添加一个周期性定时器（目前为空实现）
    m_mainIOManager->addTimer(
        2000,
        []() {
            // IM_LOG_INFO(g_logger) << "hello";
        },
        true);

    // 停止主IO管理器
    m_mainIOManager->stop();
    return 0;
}

/**
     * @brief 协程主流程，负责服务启动的各个核心环节
     * @return 正常返回0，出错直接退出进程
     *
     * 主要流程：
     * 1. 加载所有模块，若有失败则直接退出
     * 2. 初始化工作线程、Fox线程、Redis管理器
     * 3. 解析服务器配置，创建并配置所有服务器实例
     * 4. 初始化服务发现（如Zookeeper）
     * 5. 通知模块服务器已就绪，启动所有服务器
     * 6. 启动Rock负载均衡，通知模块服务器已上线
     */
int Application::run_coroutine() {
    // 获取所有模块并加载
    std::vector<Module::ptr> modules;
    ModuleMgr::GetInstance()->listAll(modules);
    bool has_error = false;
    for (auto& i : modules) {
        if (!i->onLoad()) {
            IM_LOG_ERROR(g_logger)
                << "module name=" << i->getName() << " version=" << i->getVersion()
                << " filename=" << i->getFilename();
            has_error = true;
        }
    }
    // 如果模块加载出错，则直接退出程序
    if (has_error) {
        Application::Exit(0);
    }

    // 初始化工作线程管理器和Fox线程管理器
    WorkerMgr::GetInstance()->init();
    FoxThreadMgr::GetInstance()->init();
    FoxThreadMgr::GetInstance()->start();

    // 不再使用雪花ID生成器，改用数据库自增ID。
    // 如果将来需要恢复分布式ID，可在此处重新初始化 IdWorker。

    // 初始化Redis管理器
    RedisMgr::GetInstance();

    // 获取服务器配置并创建服务器实例
    auto http_confs = g_servers_conf->getValue();
    std::vector<TcpServer::ptr> svrs;
    for (auto& i : http_confs) {
        IM_LOG_DEBUG(g_logger) << std::endl << LexicalCast<TcpServerConf, std::string>()(i);

        // 解析服务器地址配置
        std::vector<Address::ptr> address;
        for (auto& a : i.address) {
            size_t pos = a.find(":");
            if (pos == std::string::npos) {
                // 可能是UnixSocket地址
                IM_LOG_ERROR(g_logger) << "invalid address: " << a;
                address.push_back(UnixAddress::ptr(new UnixAddress(a)));
                continue;
            }

            // 解析IP地址和端口
            int32_t port = atoi(a.substr(pos + 1).c_str());
            // 127.0.0.1
            auto addr = IPAddress::Create(a.substr(0, pos).c_str(), port);
            if (addr) {
                address.push_back(addr);
                continue;
            }

            // 创建IP地址失败，则尝试获取网卡接口地址 eth0:port
            std::vector<std::pair<Address::ptr, uint32_t>> result;
            if (Address::GetInterfaceAddresses(result, a.substr(0, pos))) {
                for (auto& x : result) {
                    auto ipaddr = std::dynamic_pointer_cast<IPAddress>(x.first);
                    if (ipaddr) {
                        ipaddr->setPort(atoi(a.substr(pos + 1).c_str()));
                    }
                    address.push_back(ipaddr);
                }
                continue;
            }

            // 解析域名地址
            auto aaddr = Address::LookupAny(a);
            if (aaddr) {
                address.push_back(aaddr);
                continue;
            }
            IM_LOG_ERROR(g_logger) << "invalid address: " << a;
            Application::Exit(0);
        }

        // 获取IO管理器配置
        // 接收工作器: 负责接收新的客户端连接
        IOManager* accept_worker = IOManager::GetThis();
        // IO工作器: 已建立连接的数据读写
        IOManager* io_worker = IOManager::GetThis();
        // 处理工作器: 负责业务逻辑处理
        IOManager* process_worker = IOManager::GetThis();
        if (!i.accept_worker.empty()) {
            accept_worker = WorkerMgr::GetInstance()->getAsIOManager(i.accept_worker).get();
            if (!accept_worker) {
                IM_LOG_ERROR(g_logger) << "accept_worker: " << i.accept_worker << " not exists";
                Application::Exit(0);
            }
        }
        if (!i.io_worker.empty()) {
            io_worker = WorkerMgr::GetInstance()->getAsIOManager(i.io_worker).get();
            if (!io_worker) {
                IM_LOG_ERROR(g_logger) << "io_worker: " << i.io_worker << " not exists";
                Application::Exit(0);
            }
        }
        if (!i.process_worker.empty()) {
            process_worker = WorkerMgr::GetInstance()->getAsIOManager(i.process_worker).get();
            if (!process_worker) {
                IM_LOG_ERROR(g_logger) << "process_worker: " << i.process_worker << " not exists";
                Application::Exit(0);
            }
        }

        // 根据服务器类型创建对应的服务器实例
        TcpServer::ptr server;
        if (i.type == "http") {
            server.reset(
                new IM::http::HttpServer(i.keepalive, process_worker, io_worker, accept_worker));
        } else if (i.type == "ws") {
            server.reset(new IM::http::WSServer(process_worker, io_worker, accept_worker));
        } else if (i.type == "rock") {
            server.reset(new RockServer("rock", process_worker, io_worker, accept_worker));
        } else if (i.type == "nameserver") {
            server.reset(new RockServer("nameserver", process_worker, io_worker, accept_worker));
            ModuleMgr::GetInstance()->add(std::make_shared<ns::NameServerModule>());
        } else {
            IM_LOG_ERROR(g_logger)
                << "invalid server type=" << i.type << LexicalCast<TcpServerConf, std::string>()(i);
            Application::Exit(0);
        }

        // 设置服务器名称
        if (!i.name.empty()) {
            server->setName(i.name);
        }

        // 绑定服务器地址
        std::vector<Address::ptr> fails;
        if (!server->bind(address, fails, i.ssl)) {
            for (auto& x : fails) {
                IM_LOG_ERROR(g_logger) << "bind address fail:" << *x;
            }
            Application::Exit(0);
        }

        // 加载SSL证书
        if (i.ssl) {
            if (!server->loadCertificates(i.cert_file, i.key_file)) {
                IM_LOG_ERROR(g_logger) << "loadCertificates fail, cert_file=" << i.cert_file
                                       << " key_file=" << i.key_file;
            }
        }

        // 设置服务器配置并添加到服务器列表
        server->setConf(i);
        m_servers[i.type].push_back(server);
        svrs.push_back(server);
    }

    // 初始化服务发现组件
    if (!g_service_discovery_zk->getValue().empty()) {
        m_serviceDiscovery.reset(new ZKServiceDiscovery(g_service_discovery_zk->getValue()));
        m_rockSDLoadBalance.reset(new RockSDLoadBalance(m_serviceDiscovery));

        std::vector<TcpServer::ptr> svrs;
        if (!getServer("http", svrs)) {
            m_serviceDiscovery->setSelfInfo(GetIPv4() + ":0:" + GetHostName());
        } else {
            std::string ip_and_port;
            for (auto& i : svrs) {
                auto socks = i->getSocks();
                for (auto& s : socks) {
                    auto addr = std::dynamic_pointer_cast<IPv4Address>(s->getLocalAddress());
                    if (!addr) {
                        continue;
                    }
                    auto str = addr->toString();
                    if (str.find("127.0.0.1") == 0) {
                        continue;
                    }
                    if (str.find("0.0.0.0") == 0) {
                        ip_and_port = GetIPv4() + ":" + std::to_string(addr->getPort());
                        break;
                    } else {
                        ip_and_port = addr->toString();
                    }
                }
                if (!ip_and_port.empty()) {
                    break;
                }
            }
            m_serviceDiscovery->setSelfInfo(ip_and_port + ":" + GetHostName());
        }
    }

    // 通知所有模块服务器准备就绪
    for (auto& i : modules) {
        i->onServerReady();
    }

    // 启动所有服务器
    for (auto& i : svrs) {
        i->start();
    }

    // 启动Rock服务负载均衡
    if (m_rockSDLoadBalance) {
        m_rockSDLoadBalance->start();
    }

    // 通知所有模块服务器已启动
    for (auto& i : modules) {
        i->onServerUp();
    }
    return 0;
}

bool Application::getServer(const std::string& type, std::vector<TcpServer::ptr>& svrs) {
    auto it = m_servers.find(type);
    if (it == m_servers.end()) {
        return false;
    }
    svrs = it->second;
    return true;
}

void Application::listAllServer(std::map<std::string, std::vector<TcpServer::ptr>>& servers) {
    servers = m_servers;
}

Application* Application::GetInstance() {
    return s_instance;
}

ZKServiceDiscovery::ptr Application::getServiceDiscovery() const {
    return m_serviceDiscovery;
}

RockSDLoadBalance::ptr Application::getRockSDLoadBalance() const {
    return m_rockSDLoadBalance;
}

}  // namespace IM