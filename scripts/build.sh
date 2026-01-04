#!/bin/bash

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# 获取脚本所在目录的绝对路径
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

BUILD_DIR="$PROJECT_ROOT/build"
BUILD_TYPE="Release"
CLEAN_BUILD=false

# 帮助信息
usage() {
    echo "用法: $0 [选项]"
    echo "选项:"
    echo "  -d, --debug    构建 Debug 版本 (默认: Release)"
    echo "  -c, --clean    构建前清理 build 目录"
    echo "  -h, --help     显示帮助信息"
    exit 1
}

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -h|--help)
            usage
            ;;
        *)
            echo "未知选项: $1"
            usage
            ;;
    esac
done

echo -e "${YELLOW}开始构建项目 (类型: $BUILD_TYPE)...${NC}"

# 1. 清理 build 目录
if [ "$CLEAN_BUILD" = true ] && [ -d "$BUILD_DIR" ]; then
    echo -e "${YELLOW}正在清理 build 目录...${NC}"
    rm -rf "$BUILD_DIR"
fi

# 2. 创建 build 目录
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR"

# 3. 运行 CMake
echo -e "${YELLOW}正在运行 CMake 配置...${NC}"
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..

if [ $? -ne 0 ]; then
    echo -e "${RED}CMake 配置失败！${NC}"
    exit 1
fi

# 4. 编译项目
echo -e "${YELLOW}正在编译项目 (使用 $(nproc) 个核心)...${NC}"
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}编译失败！${NC}"
    exit 1
fi

echo -e "\n${GREEN}构建成功！${NC}"
echo -e "可执行文件位于: ${YELLOW}$PROJECT_ROOT/bin/im_server${NC}"
