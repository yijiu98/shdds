# shdds

## 介绍

基于 linux 共享内存发布订阅模型中间件

## 工程说明

- `linuxdds/`：共享内存 dds 动态库源码（lib/src/libshdds）与使用示例（app/src/ddspubtest、ddssubtest）
- `mcudds/`：MCU 端 dds（纯 C 接口）
- `mcubridge/`：MCU <-> Linux 桥接程序
- `ginclude/`：仓库级公共数据结构头文件

## 编译与测试

根目录提供统一脚本 `build.sh`：

```bash
./build.sh          # 构建全部模块 (linuxdds -> mcudds -> mcubridge)
./build.sh linux    # 只构建 linuxdds (库 + pub/sub 测试程序)
./build.sh test     # 构建并运行 pub/sub 通信测试 (约 10 秒)
./build.sh clean    # 清理所有构建产物
./build.sh --toolchain <toolchain.cmake>   # 交叉编译
```

产物位置：`linuxdds/out/lib/libshdds.so`、`linuxdds/out/bin/ddspubtest`、`ddssubtest`。

## 使用说明

支持多进程间通讯，需要定义相同 topic 和数据结构体（见 ginclude/GlobalDataStu.h）。
pub/sub 启动顺序任意：谁先创建共享内存谁自动完成初始化，其余进程等待就绪标志；
后启动的订阅者会立即收到所订阅 topic 的最新一帧（latched）。
`init` 的 `is_mgr` 参数仅决定退出时是否清理共享内存文件（shm_unlink），`deinit` 无需传参。

## 待解决问题

1. qos（目前已具备：每 topic 环形缓冲 + 序号、丢包检测告警、latched；待完善：可靠性策略可配置等）

## 版本更新记录

- 新增 mcu 端 dds 设计（mcudds）
- 新增 mcubridge 桥接程序
- 共享内存核心重写：序号 + 环形缓冲、初始化顺序任意、robust 互斥锁、latched
- 构建系统重构：根目录统一 build.sh
