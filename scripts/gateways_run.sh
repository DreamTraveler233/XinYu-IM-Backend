#!/bin/bash

# 网关分离部署启动脚本

PROJECT_ROOT=$(pwd)
HTTP_BIN="${PROJECT_ROOT}/bin/gateway_http"
WS_BIN="${PROJECT_ROOT}/bin/gateway_ws"
HTTP_CONF="${PROJECT_ROOT}/bin/config/gateway_http"
WS_CONF="${PROJECT_ROOT}/bin/config/gateway_ws"

GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

function start() {
    echo -e "${YELLOW}正在启动 HTTP 网关...${NC}"
    "${HTTP_BIN}" -c "${HTTP_CONF}" -d
    
    echo -e "${YELLOW}正在启动 WebSocket 网关...${NC}"
    "${WS_BIN}" -c "${WS_CONF}" -d
    
    echo -e "${GREEN}网关进程已在后台启动${NC}"
}

function stop() {
    echo -e "${YELLOW}正在停止网关进程...${NC}"
    pkill -f "gateway_http -c ${HTTP_CONF}"
    pkill -f "gateway_ws -c ${WS_CONF}"
    echo -e "${GREEN}信号已发送${NC}"
}

function status() {
    echo -e "${YELLOW}网关运行状态：${NC}"
    ps aux | grep -v grep | grep -E "gateway_http|gateway_ws"
    
    echo -e "\n${YELLOW}监听端口状态：${NC}"
    netstat -tunlp | grep -E "8080|8081|8060"
}

case "$1" in
    start)
        start
        ;;
    stop)
        stop
        ;;
    status)
        status
        ;;
    restart)
        stop
        sleep 1
        start
        ;;
    *)
        echo "用法: $0 {start|stop|status|restart}"
        exit 1
esac
