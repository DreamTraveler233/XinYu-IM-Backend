# XinYu IM Backend

XinYu IM Backend 是一个基于 **C++17** 构建的高性能、**分布式架构**即时通讯服务端。项目正在从传统的单体架构演进为微服务体系，目前已实现核心网关层（Gateway）的彻底分离。它不仅提供 RESTful API 和 WebSocket 长连接服务，还集成了媒体处理、关系链管理及分布式消息路由能力。

## 核心亮点

- **分布式网关架构**：
    - **`gateway_http`**：专门处理无状态的 REST 业务请求，负责鉴权、资料同步及业务逻辑触发。
    - **`gateway_ws`**：高性能长连接网关，负责百万级连接维护、心跳检测及下行消息精准推送。
- **跨进程通讯 (Rock RPC)**：基于自定义 Rock 协议实现网关间的信令投递。即使客户端连接在不同的网关节点，系统也能通过内部 RPC 链路实现实时消息送达。
- **服务发现与自治**：深度集成 **ZooKeeper**，实现网关节点的自动注册与发现 (Service Discovery)。系统能够实时感知节点变更，支持动态扩容与负载均衡。
- **高性能底层支撑**：
    - 采用 **Libevent + 协程 (Coroutine)** 模型，兼顾开发效率与极致并发性能。
    - 针对 IM 场景深度优化，支持消息幂等性检查、图片/视频多级缩略图生成及本地/云端存储适配。
- **分层领域模型**：严格遵循分层架构 (API → Application → Infrastructure)，业务逻辑与存储介质彻底解耦，易于扩展与维护。

## 仓库结构概览
| 路径 | 说明 |
| --- | --- |
| `src/interface/` | 接口层：HTTP Servolets、WebSocket 处理及 RPC 模块实现。 |
| `src/application/` | 应用服务层：处理聊天逻辑、好友添加、群组管理等。 |
| `src/infrastructure/` | 基础设施层：MySQL/Redis 仓储适配、加密算法及云存储实现。 |
| `bin/config/` | 配置文件目录：包含 `gateway_http` 和 `gateway_ws` 的独立配置模板。 |
| `bin/` | 构建产物：`gateway_http` (API网关)、`gateway_ws` (WS网关)、`im_server` (兼容单体)。 |
| `scripts/` | 自动化工具：`gateways_run.sh` (网关启停)、`sql/` (数据库迁移)、`bench/` (性能压测)。 |

## 依赖与环境要求
- CMake ≥ 3.10，Make/ninja，ccache(可选)。
- C++17 编译器（推荐 GCC ≥ 9 或 Clang ≥ 11）。
- 系统库：Threads、pkg-config、Protobuf、OpenSSL、Zlib、yaml-cpp、jwt-cpp、sqlite3、mysqlclient、jsoncpp、libevent、hiredis_vip、tinyxml2、ragel、ZooKeeper (`libzookeeper_mt.so`)。
- 服务依赖：MySQL 8+、Redis、（可选）ZooKeeper 用于 `service_discovery.zk`。
- 其它：RSA 密钥对 (`keys/`)、本地或分布式文件存储路径（媒体上传）。

Ubuntu 示例安装命令（按需调整）：
```bash
sudo apt install build-essential cmake pkg-config ccache ragel \
    libprotobuf-dev protobuf-compiler libssl-dev zlib1g-dev libyaml-cpp-dev \
    libjwt-cpp-dev libsqlite3-dev libmysqlclient-dev libjsoncpp-dev \
    libevent-dev libhiredis-dev libtinyxml2-dev libzookeeper-mt-dev
```

## 快速上手

1. **基础环境准备**：
   ```bash
   # 启动 MySQL, Redis, ZooKeeper
   ./scripts/start_env.sh
   # 执行数据库迁移
   ./scripts/sql/migrate.sh apply
   ```

2. **编译代码**：
   ```bash
   ./scripts/build.sh
   ```

3. **启动网关服务**：
   项目目前采用网关分离部署模式，由专用脚本管理：
   ```bash
   # 一键启动 HTTP 与 WebSocket 网关
   ./scripts/gateways_run.sh start
   # 查看运行状态与监听端口
   ./scripts/gateways_run.sh status
   ```

4. **客户端连接**：
   - **HTTP API**: `http://127.0.0.1:8080/api/v1/...`
   - **WebSocket**: `ws://127.0.0.1:8081?token=...&platform=pc`
   - **内部 RPC**: `127.0.0.1:8060` (仅供服务间调用)

## 设计理念与架构演进

项目正依照 **[分布式IM服务器项目计划书.md](分布式IM服务器项目计划书.md)** 进行分阶段重构。

- **Phase 1（当前）**：网关分离。将单体拆分为 `gateway_http` 和 `gateway_ws`，解决单进程并发瓶颈，引入 RPC 投递机制。
- **Phase 2（计划中）**：全局在线路由 (Presence)。利用 Redis 记录用户会话映射，实现多节点间的推送寻址。
- **Phase 3（计划中）**：业务核心拆分。将消息存储、离线记录、关系链提取为独立服务。

## 运行与部署提示
- **相对路径支持**：所有二进制程序均默认从 `bin/config/` 读取配置。
- **日志观察**：网关日志分别记录在 `bin/log/gateway_http/` 和 `bin/log/gateway_ws/`。
- **动态扩容**：若需部署多个 WS 网关实例，物理复制 `bin/config/gateway_ws` 目录并修改 `server.yaml` 中的监听端口及 ZK 注册信息即可。

## 配置说明 (bin/config/)

项目配置文件采用子目录隔离模式，确保不同职责的网关进程互不干扰。

- **`gateway_http/`**：HTTP 网关配置，重点在于 `server.yaml` 中的 `http` 监听。
- **`gateway_ws/`**：WS 网关配置，重点在于 `server.yaml` 中的 `ws` 与 `rock` 监听。

### 核心配置文件：
- **`system.yaml`**：
    - `server.work_path`：网关的运行时工作目录（如 `bin/apps/work/gateway_ws`）。
    - `server.pid_file`：进程 PID 文件名，确保网关可独立启停。
    - `service_discovery.zk`：ZooKeeper 地址，用于节点注册。
    - `mysql.dbs.default` / `redis.nodes`：数据库与缓存连接配置。
- **`server.yaml`**：协议类型（http/ws/rock）、监听地址及超时参数。
- **`log.yaml`**：日志级别与路径。建议设置为 `bin/log/gateway_xxx/` 以便区分。
- **`media.yaml`**：媒体文件上传的基准路径与存储策略。

## 测试与基准

- **单元测试**：构建产出物位于 `bin/tests/`。
  ```bash
  # 示例：运行媒体服务测试
  ./bin/tests/test_media
  ```
- **自动化压测**：
  `scripts/bench/` 提供了基于 `wrk` 的压测脚本。运行后会在 `bench_results/` 生成可视化 HTML 报告。

## 常见问题

- **端口冲突**：确保 `gateway_http` (8080) 与 `gateway_ws` (8081, 8060) 端口配置互不重叠。
- **RPC 无法连通**：检查 `gateway_ws` 的 Rock RPC 端口是否放行，且后端服务能正确从 ZooKeeper 获取 `gateway-ws-rpc` 节点。
- **工作目录写权限**：网关在启动时会尝试锁定 `work_path` 下的 PID 文件，请确保该目录对运行用户可见且可写。

## 贡献

欢迎提交 Issue 或 PR。在提交代码前建议运行 `.clang-format` 进行格式化，并确保所有单元测试通过。
