#!/bin/bash
#如果脚本中的任何命令返回非零退出状态（错误），则立即退出脚本
set -e
echo "start lib build bash--------------------------"
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
    #使用 CMake 生成 Makefile，并使用 Release 构建类型和 g++ 编译器进行构建(在..上层目录中找cmakelists文件)
    #生成构建系统文件
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_COMPILER=g++

    echo "build (ie 'make')"
    #执行构建操作
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
