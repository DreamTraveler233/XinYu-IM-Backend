# XinYu IM Backend

XinYu IM Backend 是一个采用 C++17 构建的实时即时通讯服务端，提供 HTTP API、WebSocket 网关、媒体上传、联系人/群组管理、消息流以及静态资源托管能力。核心进程为 `im_server`，通过模块化架构（API → 应用服务 → 仓储/基础设施）实现清晰的业务层次，可在单机或多实例部署场景中运行。

## 功能亮点
- **多协议接入**：`server.yaml` 同时定义 REST(HTTP) 与 WebSocket 服务，满足同步请求与长连接推送场景。
- **完备的业务域**：包含用户、联系人、会话、群聊、消息、媒体上传、文章/表情占位模块以及静态文件托管模块。
- **分层设计**：`src/api`, `src/app`, `src/infra`, `src/ds` 等目录分别承载接口、领域服务、仓储/适配器与底层数据结构，实现解耦。
- **可配置运行时**：所有运行参数基于 YAML (`conf/*.yaml`) 管理，涵盖服务器监听、线程池、日志、加密密钥、MySQL/Redis 等资源。
- **生产级依赖**：使用 Protobuf、OpenSSL、libevent、hiredis、jwt-cpp、yaml-cpp、tinyxml2 等成熟组件，支持 Ragel 生成状态机与 ZooKeeper 服务发现。
- **脚本与基准**：提供 `autobuild.sh`、`scripts/*` 工具链以及 `bench_results/` 样例报告，便于持续集成与性能回归。

## 仓库结构概览
| 路径 | 说明 |
| --- | --- |
| `src/` | 主要业务实现，按 api/app/infra/... 分层；`src/bootstrap/im_server.cpp` 为入口。 |
| `include/` | 对应头文件，供共享库 `lib/libIM.so` 与可执行程序使用。 |
| `migrations/` | MySQL 初始化与增量脚本（按编号依次执行）。 |
| `tests/` | 单元 & 集成测试（产物输出到 `bin/tests/`）。 |
| `bin/` | 构建输出：`im_server`、`lib/`、测试二进制及样例配置 `bin/config/*.yaml`。 |
| `apps/work/IM/` | 运行时工作区（PID、日志、测试数据等），可通过配置调整。 |
| `scripts/` | 辅助脚本：`bench/`, `docker/`, `sql/`, `tooling/` 等。 |
| `bench_results/` | 基准测试报告与图表示例。 |

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
1. **克隆与子模块**（如使用）：
   ```bash
   git clone https://github.com/.../XinYu-IM-Backend.git
   cd XinYu-IM-Backend
   ```
2. **准备数据库**：
   ```bash
   mysql -u root -p < migrations/001_init_db.sql
   mysql -u root -p < migrations/002_table.sql
   # 依次执行到 090_upload.sql
   ```
3. **配置文件**：
   - 默认读取 `./conf` 目录，可先复制样例：
     ```bash
     mkdir -p conf
     cp -r bin/config/* conf/
     ```
   - 根据环境修改各 YAML（参考下文“配置说明”）。
4. **构建**：
   - 快捷脚本：`./autobuild.sh`
   - 或使用标准 CMake：
     ```bash
     mkdir -p build && cd build
     cmake -DCMAKE_BUILD_TYPE=Release ..
     cmake --build . -- -j$(nproc)
     cd ..
     ```
5. **运行服务**：
   ```bash
   ./bin/im_server -s -c conf        # 前台运行
   ./bin/im_server -d -c conf        # 以守护进程运行
   ```
   - `-s`/`-d` 控制前台或后台；`-c` 指定配置目录；`-h` 查看内置帮助。
   - 运行时会在 `server.work_path`（默认 `apps/work/IM`）下写入 PID、日志与临时文件。

## 配置说明（conf/*.yaml）
- `system.yaml`
  - `server.work_path` / `pid_file`：运行时工作目录与 PID 文件。
  - `auth.jwt`：JWT 密钥、签发者与有效期。
  - `machine.code`：雪花/ID 预留字段，用于多实例区分。
  - `crypto.*`：RSA 密钥路径 & padding。
  - `mysql.dbs.default`：主库连接信息与连接池大小。
  - `websocket.message.max_size` 等：WS 层参数。
- `server.yaml`：HTTP / WebSocket 监听地址、KeepAlive、超时、线程池绑定。
- `workers.yaml`：`accept`, `http_worker`, `ws_worker` 的 worker/thread 数量。
- `log.yaml`：日志级别、输出到文件/STDOUT、滚动策略（小时/大小）。
- `media.yaml`：上传根目录、分片大小、临时目录清理策略。
- `email.yaml` / `sms.yaml`：验证码、SMTP/短信网关配置，可禁用对应通道。

> 将敏感信息（数据库密码、JWT secret、SMTP/短信凭据、RSA 密钥）替换为生产值，并确保 `server.work_path` 与媒体目录具备写权限。

## 运行与部署提示
- `apps/work/IM` 需要可写权限；若部署到 `/var/lib/xinyu-im` 等目录，请在 `system.yaml` 中同步更新。
- 如果启用 ZooKeeper，请在 `system.yaml` 配置 `service_discovery.zk`。
- WebSocket 与 HTTP 默认监听 `0.0.0.0:8080/8081`（可在 `server.yaml` 调整）。
- 静态资源与上传文件默认写入 `media.upload_base_dir`（建议使用绝对路径或挂载的对象存储网关）。
- `ENABLE_QT_CLIENT=ON` 可在同一 CMake 工程中构建桌面客户端（会尝试添加 `../XinYu-IM-QtClient`）。

## 测试
- 构建后测试产物位于 `bin/tests/`（详见 `tests/README.md`）。
- 示例运行：
  ```bash
  ./bin/tests/test_media
  ```
- 新增测试：在 `tests/unit` 或 `tests/integration` 添加 `*.cpp` 并更新 `tests/CMakeLists.txt`。

## 基准与辅助脚本
- `bench_results/<timestamp>/`：包含 API 压测、媒体上传测试的原始日志、CSV、HTML 报告与图表，可作为性能基线。
- `scripts/bench/`：压测驱动脚本；`scripts/docker/`：容器化模板；`scripts/sql/`：数据库工具；`scripts/tooling/`：开发者辅助脚本。

## 常见问题
- **缺少 Ragel/Protobuf**：确保已安装 `ragel`、`protobuf-compiler` 并在 `PATH`/`PKG_CONFIG_PATH` 中可见。
- **连接数据库失败**：检查 `mysql.dbs.default`、网络连通性与权限；可使用 `mysql --defaults-extra-file` 进行验证。
- **工作目录不可写**：修改 `system.yaml` 的 `server.work_path` 或为当前用户授予写权限。
- **日志未生成**：确认 `log.yaml` 路径相对于运行目录存在，并确保 `log/` 文件夹可写。

## 贡献
欢迎通过 Issue/PR 报告问题或提交改进。请在提交代码前运行相关测试与格式化工具（`.clang-format`, `.clang-tidy`, `.pre-commit-config.yaml`）。
