##!/bin/bash
set -e

# 定义当前目录下的extern路径和上一级目录的lib/build路径
EXTERN_DIR="./extern"
LIB_BUILD_DIR="../lib/build"
echo "start app build bash--------------------------"
# 检查extern目录是否存在
if [ ! -d "$EXTERN_DIR" ]; then
  echo "extern目录不存在，正在创建..."
  mkdir -p "$EXTERN_DIR"
fi

# 删除extern目录下的所有内容
echo "正在清空extern目录..."
rm -rf "$EXTERN_DIR"/*

# 检查lib/build目录是否存在
if [ ! -d "$LIB_BUILD_DIR" ]; then
  echo "错误：lib/build目录不存在。"
  exit 1
fi

# 从lib/build拷贝所有文件到extern目录
echo "正在从 $LIB_BUILD_DIR 拷贝文件到 $EXTERN_DIR ..."
cp -R "$LIB_BUILD_DIR/"* "$EXTERN_DIR"

echo "操作完成。"


build()
{
    if [ ! -x "build" ]; then
        echo "create 'build' folder"
        mkdir -p build
    fi
    
    if [ ! -x "out" ]; then
        echo "create 'out' folder"
        mkdir -p out
    fi

    echo "enter 'out' folder"
    cd out

    echo "generate makefile with flags and target toolchain"
    # cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/opt/arm_toolchain_allwinner_a40i/x64-arm-Linux.cmake
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++

    echo "build (ie 'make')"
    cmake --build .
}


builduttest()
{
    if [ ! -x "build" ]; then
        echo "create 'build' folder"
        mkdir -p build
    fi
    
    if [ ! -x "out" ]; then
        echo "create 'out' folder"
        mkdir -p out
    fi

    echo "enter 'out' folder"
    cd out

    echo "generate makefile with flags and target toolchain"
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=/opt/arm_toolchain_allwinner_a40i/x64-arm-Linux.cmake -DUT_TEST=ON

    echo "build (ie 'make')"
    cmake --build .
}


clean()
{
    if [ -x "build" ]; then
        echo "remove 'build' folder"
        rm -rf build
    fi
    
    if [ -x "out" ]; then
        echo "remove 'out' folder"
        rm -rf out
    fi
}


case "$1" in
    clean)
        echo "clean:"
        clean
        ;;
    ut)
        echo "ut:"
        builduttest
        ;;
    *)
        echo "build:"
        build
        ;;
esac
