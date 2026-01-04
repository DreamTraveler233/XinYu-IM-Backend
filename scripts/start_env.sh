#!/bin/bash

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}正在检查并启动 IM 服务器所需环境...${NC}"

# 函数：检查并启动服务
check_and_start() {
    local service_name=$1
    local display_name=$2
    local port=$3

    echo -n "检查 $display_name (端口 $port)... "
    
    # 检查端口是否在监听
    if netstat -tuln | grep -q ":$port "; then
        echo -e "${GREEN}已运行${NC}"
    else
        echo -e "${YELLOW}未运行，尝试启动...${NC}"
        sudo systemctl start $service_name
        
        # 等待启动
        sleep 2
        
        if netstat -tuln | grep -q ":$port "; then
            echo -e "${GREEN}$display_name 启动成功${NC}"
        else
            echo -e "${RED}$display_name 启动失败，请检查日志 (sudo journalctl -u $service_name)${NC}"
        fi
    fi
}

# 1. 检查 Zookeeper
check_and_start "zookeeper" "Zookeeper" "2181"

# 2. 检查 Redis
check_and_start "redis-server" "Redis" "6379"

# 3. 检查 MySQL
check_and_start "mysql" "MySQL" "3306"

# 4. 检查 Nginx
check_and_start "nginx" "Nginx" "80" # 或者 9000，取决于你的配置

echo -e "\n${GREEN}环境检查完毕！${NC}"
echo -e "你可以运行以下命令启动 IM 服务器："
echo -e "${YELLOW}./bin/im_server -d"
