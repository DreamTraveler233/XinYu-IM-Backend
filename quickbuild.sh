#!/bin/bash

set -e

# 快速编译脚本 - 专为开发过程中快速迭代而设计
# 特性:
# 1. 自动检测并使用 Ninja (如果可用)
# 2. 使用 ccache 加速重新编译
# 3. 只编译指定的目标而不是整个项目
# 4. 最大化并行编译任务

# 项目根目录
PROJECT_ROOT=$(cd "$(dirname "$0")"; pwd)

echo "进入项目目录: $PROJECT_ROOT"
cd "$PROJECT_ROOT"

# 解析参数
TARGET=""
BUILD_TYPE="Debug"
PARALLEL_JOBS=$(nproc)
CLEAN_BUILD=false

show_help() {
    echo "用法: $0 [选项] [目标]"
    echo ""
    echo "选项:"
    echo "  -h, --help          显示此帮助信息"
    echo "  -c, --clean         执行干净构建"
    echo "  -j N                指定并行编译任务数，默认为 CPU 核心数"
    echo "  -t, --type TYPE     构建类型: Debug 或 Release，默认为 Debug"
    echo ""
    echo "示例:"
    echo "  $0                  # 构建整个项目"
    echo "  $0 im_server        # 只构建 im_server"
    echo "  $0 -c im_server     # 干净构建 im_server"
    echo "  $0 -j4              # 使用 4 个并行任务构建"
}

# 解析命令行参数
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            show_help
            exit 0
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -j)
            PARALLEL_JOBS="$2"
            if ! [[ "$PARALLEL_JOBS" =~ ^[0-9]+$ ]]; then
                echo "错误: -j 参数必须是数字"
                exit 1
            fi
            shift 2
            ;;
        -t|--type)
            BUILD_TYPE="$2"
            if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" ]]; then
                echo "错误: 构建类型必须是 Debug 或 Release"
                exit 1
            fi
            shift 2
            ;;
        -*)
            echo "未知选项: $1"
            show_help
            exit 1
            ;;
        *)
            TARGET="$1"
            shift
            ;;
    esac
done

# 检查依赖
command -v cmake >/dev/null 2>&1 || { echo >&2 "未找到 cmake，请先安装。"; exit 1; }

# 检查 Ninja 是否可用
USE_NINJA=false
if command -v ninja >/dev/null 2>&1; then
    echo "检测到 Ninja，将使用 Ninja 构建系统"
    USE_NINJA=true
elif command -v ninja-build >/dev/null 2>&1; then
    echo "检测到 ninja-build，将使用 Ninja 构建系统"
    USE_NINJA=true
fi

# 创建 build 目录
if [ ! -d build ]; then
    echo "创建 build 目录"
    mkdir build
fi

cd build

# 如果需要干净构建，则删除所有内容
if [ "$CLEAN_BUILD" = true ]; then
    echo "执行干净构建"
    rm -rf *
fi

# 如果 CMakeCache.txt 不存在，则需要重新配置
if [ ! -f CMakeCache.txt ]; then
    echo "配置 CMake..."
    if [ "$USE_NINJA" = true ]; then
        cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE -G Ninja ..
    else
        cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE ..
    fi
fi

echo "开始编译..."
START_TIME=$(date +%s)

if [ "$USE_NINJA" = true ]; then
    if [ -n "$TARGET" ]; then
        ninja -j$PARALLEL_JOBS $TARGET
    else
        ninja -j$PARALLEL_JOBS
    fi
else
    if [ -n "$TARGET" ]; then
        make -j$PARALLEL_JOBS $TARGET
    else
        make -j$PARALLEL_JOBS
    fi
fi

END_TIME=$(date +%s)
ELAPSED=$((END_TIME - START_TIME))

echo "编译完成! 耗时: $ELAPSED 秒"

# 如果指定了可执行目标，则运行它
if [ -n "$TARGET" ]; then
    EXECUTABLE="../bin/$TARGET"
    if [ -f "$EXECUTABLE" ]; then
        echo "正在运行 $EXECUTABLE ..."
        "$EXECUTABLE" -s
    fi
fi