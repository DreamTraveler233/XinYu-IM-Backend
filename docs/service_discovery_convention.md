# 服务发现命名与路径规范 (XinYu-IM)

本规范定义了 IM 系统中各服务在 ZooKeeper 中的注册路径、命名规则及数据格式，确保分布式环境下的可发现性。

## 1. 注册核心路径

所有服务实例均注册在以下根路径下：

` /sylar/{domain}/{service}/providers/{ip_port} `

- **sylar**: 框架默认命名空间（后续可平滑迁移至 `xinyu`）。
- **domain**: 业务域或服务大类（如 `im`, `gateway`, `media`）。
- **service**: 具体服务名称（如 `ws`, `http`, `message`）。
- **ip_port**: 实例标识，格式为 `IP:Port`。

## 2. 现有服务命名约定

| 服务组件 | 进程类型 | Domain | Service | 节点内容 (Data) | 说明 |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **WebSocket 网关** | gateway_ws | `im` | `ws` | `ws` | 承载长连接 |
| **HTTP API** | gateway_http | `im` | `http` | `http` | 承载 REST API |
| **Rock RPC** | (内嵌) | `im` | `rock` | `rock` | 内部通信 |

## 3. 节点数据格式

ZooKeeper 中的节点数据暂存为服务类型简写（如 `ws`, `http`），后续将升级为 JSON 格式以支持更多元数据：

```json
{
  "type": "ws",
  "weight": 100,
  "start_time": "2026-01-06 10:00:00",
  "version": "1.0.0"
}
```

## 4. 生命周期管理

- **临时节点 (EPHEMERAL)**: 所有服务实例必须以临时节点形式注册。一旦进程退出或由于网络问题与 ZooKeeper 断开连接，节点应在 Session 超时后自动删除。
- **自动注册**: 服务在启动并成功监听端口后，由 `ModuleMgr` 或 `TcpServer` 触发注册逻辑。
- **自动下线**: 服务正常停止时，应主动删除对应的 ZK 节点。

## 5. 查询与发现

调用方应通过 `ZKServiceDiscovery` 监听对应路径及其子节点（Watch 机制）。当子节点列表发生变化时，本地缓存的负载均衡列表应立即刷新。
