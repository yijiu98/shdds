#!/bin/bash
# shdds 统一构建/测试脚本
#
# 用法:
#   ./build.sh                      构建全部模块 (linuxdds -> mcudds -> mcubridge)
#   ./build.sh linux                只构建 linuxdds (共享内存库 + pub/sub 测试程序)
#   ./build.sh mcudds               只构建 mcudds
#   ./build.sh bridge               只构建 mcubridge (会自动先构建 linuxdds)
#   ./build.sh test                 构建 linuxdds 并运行 pub/sub 通信测试 (约 10 秒)
#   ./build.sh clean                清理所有构建产物
#   ./build.sh --toolchain <file>   使用指定的交叉编译 toolchain 文件构建
#
# 示例: ./build.sh --toolchain /opt/arm_toolchain_allwinner_a40i/x64-arm-Linux.cmake linux

set -e

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
JOBS="$(nproc)"
TOOLCHAIN=""

if [ "$1" = "--toolchain" ]; then
    TOOLCHAIN="$2"
    shift 2
fi

CMAKE_ARGS=(-DCMAKE_BUILD_TYPE=Release)
if [ -n "$TOOLCHAIN" ]; then
    CMAKE_ARGS+=(-DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN")
fi

# 配置 cmake；若构建目录里有别的路径留下的旧缓存，先清掉
configure() {
    local src_dir="$1"
    local out_dir="$2"
    if [ -f "$out_dir/CMakeCache.txt" ] && ! grep -q "^CMAKE_HOME_DIRECTORY:INTERNAL=$src_dir$" "$out_dir/CMakeCache.txt"; then
        echo "检测到旧的 cmake 缓存，清理 $out_dir"
        rm -rf "$out_dir"
    fi
    cmake -S "$src_dir" -B "$out_dir" "${CMAKE_ARGS[@]}"
    cmake --build "$out_dir" -j"$JOBS"
}

build_linux() {
    echo "========== build linuxdds =========="
    configure "$ROOT_DIR/linuxdds" "$ROOT_DIR/linuxdds/out"
}

build_mcudds() {
    echo "========== build mcudds =========="
    configure "$ROOT_DIR/mcudds" "$ROOT_DIR/mcudds/out"
}

build_bridge() {
    echo "========== build mcubridge =========="
    if [ ! -f "$ROOT_DIR/linuxdds/out/lib/libshdds.so" ]; then
        echo "mcubridge 依赖 libshdds，先构建 linuxdds"
        build_linux
    fi
    configure "$ROOT_DIR/mcubridge" "$ROOT_DIR/mcubridge/out"
}

run_test() {
    build_linux

    local bin_dir="$ROOT_DIR/linuxdds/out/bin"
    local pub_log="$ROOT_DIR/linuxdds/out/pub.log"
    local sub_log="$ROOT_DIR/linuxdds/out/sub.log"

    # 清理上次可能残留的共享内存，避免初始化状态污染
    rm -f /dev/shm/myshm

    echo "========== run pub/sub test =========="
    echo "启动 ddspubtest (管理进程)..."
    "$bin_dir/ddspubtest" >"$pub_log" 2>&1 &
    local pub_pid=$!

    # 等管理进程完成共享内存和互斥锁初始化
    sleep 1

    echo "启动 ddssubtest，运行 10 秒..."
    timeout --signal=SIGINT 10 "$bin_dir/ddssubtest" >"$sub_log" 2>&1 || true

    kill -INT "$pub_pid" 2>/dev/null || true
    wait "$pub_pid" 2>/dev/null || true

    echo "---------- ddspubtest 日志 (尾部) ----------"
    tail -n 10 "$pub_log" || true
    echo "---------- ddssubtest 日志 (尾部) ----------"
    tail -n 20 "$sub_log" || true
    echo "完整日志: $pub_log / $sub_log"
}

clean() {
    echo "clean all build output"
    rm -rf "$ROOT_DIR/linuxdds/out" \
           "$ROOT_DIR/linuxdds/lib/build" "$ROOT_DIR/linuxdds/lib/out" \
           "$ROOT_DIR/linuxdds/app/build" "$ROOT_DIR/linuxdds/app/out" \
           "$ROOT_DIR/mcudds/out" "$ROOT_DIR/mcudds/build" \
           "$ROOT_DIR/mcubridge/out" "$ROOT_DIR/mcubridge/build"
}

case "$1" in
    linux)
        build_linux
        ;;
    mcudds)
        build_mcudds
        ;;
    bridge)
        build_bridge
        ;;
    test)
        run_test
        ;;
    clean)
        clean
        ;;
    all|"")
        build_linux
        build_mcudds
        build_bridge
        ;;
    *)
        echo "未知命令: $1"
        sed -n '2,15p' "$0"
        exit 1
        ;;
esac
