#!/bin/bash

# 颜色定义
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}正在停止 IM 服务器相关环境服务...${NC}"

stop_service() {
    local service_name=$1
    local display_name=$2
    echo -n "停止 $display_name... "
    sudo systemctl stop $service_name
    echo -e "${RED}已停止${NC}"
}

stop_service "zookeeper" "Zookeeper"
stop_service "redis-server" "Redis"
stop_service "mysql" "MySQL"
stop_service "nginx" "Nginx"

echo -e "\n${RED}所有环境服务已关闭。${NC}"
