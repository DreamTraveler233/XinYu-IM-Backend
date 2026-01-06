# scripts 目录说明文档

本目录包含了 XinYu-IM-Backend 项目的构建、环境管理、集群运行及运维辅助脚本。

## 1. 核心运行脚本

### [gateways_run.sh](gateways_run.sh) (网关分离运行)
在阶段 1 完成网关分离后，专用于启停专用的 HTTP 网关与 WebSocket 网关。
- **功能**: 管理分离后的 `gateway_http` 和 `gateway_ws` 进程。
- **用法**:
  - `./gateways_run.sh start`: 启动 HTTP 和 WS 网关。
  - `./gateways_run.sh stop`: 停止网关进程。
  - `./gateways_run.sh status`: 查看网关运行状态及监听端口。
- **配置依赖**: 依赖 `bin/config/gateway_http` 和 `bin/config/gateway_ws`。

### [build.sh](build.sh) (编译构建)
- **功能**: 封装了 CMake 编译流程，生成 `im_server`、`gateway_http` 和 `gateway_ws`。
- **输出**: 编译结果存放在 `build/` 目录，生成的库位于 `lib/`，可执行文件位于 `bin/`。

### [start_env.sh](start_env.sh) & [stop_env.sh](stop_env.sh) (基础环境管理)
- **功能**: 检查并启动/停止项目依赖的中间件（Zookeeper, Redis, MySQL, Nginx）。
- **注意**: 在运行 `im_server` 或 `cluster_run.sh` 之前，必须确保基础环境已启动。

---

## 2. 目录结构

| 子目录 | 说明 |
| :--- | :--- |
| **bench/** | 压力测试相关脚本（基于 wrk），包含报表分析工具。 |
| **docker/** | Docker 部署相关配置，目前主要包含 Nginx 环境的容器化脚本。 |
| **sql/** | 数据库迁移管理（migrate.sh），负责执行 `migrations/` 下的版本更新。 |
| **tooling/** | 辅助工具，如测试脚本、媒体文件目录初始化脚本等。 |

---

## 3. 典型操作流程

### 场景 A：重新编译并本地集群测试
```bash
# 1. 启动中间件
./start_env.sh

# 2. 编译代码
./build.sh

# 3. 运行集群节点
./cluster_run.sh restart

# 4. 检查是否正常
./cluster_run.sh status
```

### 场景 B：数据库版本更新
```bash
cd sql
./migrate.sh
```

---

## 4. 运维注意事项
- **端口冲突**: `cluster_run.sh` 启动的节点端口在 `conf/cluster/` 对应的 `server.yaml` 中配置。若需新增节点，请确保所有端口不冲突。
- **日志查看**: 集群模式下，日志不再输出到标准输出，请查看 `bin/log/nodeX/system.log`。
- **清理**: 如果进程异常无法停止，可以使用 `pkill -9 im_server` 强行清理。
