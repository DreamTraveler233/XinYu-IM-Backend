# nginx-dev.sh 使用文档

该脚本用于本地开发环境下便捷地管理 nginx 实例（启动、重载、停止、查看状态、测试配置、查看日志等）。

## 使用方法

```bash
./nginx-dev.sh <command>
```

### 可用命令

- `start`      启动开发用 nginx 实例
- `reload`     重新加载配置（发送 HUP 信号）
- `stop`       优雅停止（QUIT），超时后强制终止（TERM）
- `status`     查看 nginx 运行状态
- `test`       测试配置文件语法
- `logs`       实时查看 error/access 日志（tail -F，Ctrl-C 退出）

### 环境变量覆盖

可通过环境变量覆盖默认路径和参数：

- `NGINX_BIN`   nginx 可执行文件路径（默认：nginx）
- `CONF`        配置文件路径（默认：nginx.dev.conf）
- `PID`         PID 文件路径（默认：nginx.pid）
- `ACCESS_LOG`  访问日志路径（默认：nginx.access.log）
- `ERROR_LOG`   错误日志路径（默认：nginx.error.log）

示例：

```bash
CONF=/path/to/your.conf ./nginx-dev.sh start
```

## 常见用法

- 启动 nginx：
  ```bash
  ./scripts/docker/nginx-dev.sh start
  ```
- 停止 nginx：
  ```bash
  ./scripts/docker/nginx-dev.sh stop
  ```
- 重载配置：
  ```bash
  ./scripts/docker/nginx-dev.sh reload
  ```
- 查看状态：
  ```bash
  ./scripts/docker/nginx-dev.sh status
  ```
- 测试配置：
  ```bash
  ./scripts/docker/nginx-dev.sh test
  ```
- 查看日志：
  ```bash
  ./scripts/docker/nginx-dev.sh logs
  ```

## 注意事项

- 需保证 `nginx` 可用，且配置文件路径正确。
- 日志、PID 文件等默认生成在脚本同级目录。
- 如遇启动失败，请检查 error log。

## 媒体文件静态目录（/media/）

如果你希望将后端上传（`data/uploads`）的文件暴露为 HTTP 静态资源（例如 /media/2025/11/21/xxx.png），你可以在 `nginx.dev.conf` 或独立配置文件中加入如下段落：

```nginx
  location /media/ {
    alias /absolute/path/to/IM-backend/data/uploads/; # 注意 alias 必须以 / 结尾
    expires 30d;
    access_log off;
    add_header Cache-Control "public";
    try_files $uri $uri/ =404;
  }
```

也可以使用 `scripts/docker/nginx.media.conf` 中提供的演示配置文件，默认将 `data/uploads` 目录映射到 `/media`。确保 Nginx 的 `alias` 指向正确的绝对路径，并保证 Nginx 运行用户有权限访问该目录。

---

如有问题请联系维护者。
