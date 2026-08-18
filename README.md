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

## linuxdds 架构

```text
┌────────────── 进程 A (发布方) ──────────────┐   ┌────────────── 进程 B (订阅方) ──────────────┐
│  应用层: ddspubtest / mcubridge / 业务进程   │   │  应用层: ddssubtest / 业务进程               │
│────────────────────────────────────────────│   │────────────────────────────────────────────│
│  API 层 (shdds.h, 模板, 类型安全)            │   │  API 层 (shdds.h)                            │
│    Publisher<T>::publish()                  │   │    Subscriber<T>::subscribe(cbk)             │
│    shdds::init(is_mgr) / deinit()           │   │                                              │
│────────────────────────────────────────────│   │────────────────────────────────────────────│
│  detail 窄接口 (publish/subscribe, 隐藏 ABI) │   │  detail 窄接口                                │
│────────────────────────────────────────────│   │────────────────────────────────────────────│
│  核心层: DataManager (进程内单例)            │   │  核心层: DataManager                          │
│    write(): 持锁 → 写描述符+字节池 → 广播    │   │    SubThreadFunc: 等待条件变量 →              │
│                                            │   │      锁内按序号追赶拷贝 → 锁外执行回调         │
│    regSubCbk(): 注册回调, 立即投            │   │    (回调里可再 publish, 不会死锁)             │
│      递最新一帧 (latched)                   │   │                                              │
└───────────────────────┬────────────────────┘   └───────────────────────┬────────────────────────┘
                        │ mmap (PROT_READ|PROT_WRITE, MAP_SHARED)        │
┌───────────────────────▼────────────────────────────────────────────────▼───────────────────────┐
│                        /dev/shm/myshm  ——  shared_struct (~700KB)                               │
│                                                                                                │
│  pthread_mutex (ROBUST + PROCESS_SHARED)   pthread_cond (PROCESS_SHARED)                        │
│  magic / version : 初始化握手 + 布局版本校验                                                      │
│                                                                                                │
│  Topic[MAX_TOPICS=10]:                                                                         │
│  ┌──────────────────────────────────────────────────────────────────────────┐                  │
│  │ name[32] │ seq(已发总数) │ pool_off(池已写字节)                            │                  │
│  │ desc[256] : {seq, offset, len}  —— 消息描述符环(积压上限 256 条)           │                  │
│  │ pool[64KB]: 变长消息字节池, 循环使用(单条上限 64KB)                        │                  │
│  └──────────────────────────────────────────────────────────────────────────┘                  │
└────────────────────────────────────────────────────────────────────────────────────────────────┘
```

关键机制：

- **初始化顺序任意**：`shm_open(O_CREAT|O_EXCL)` 抢创建者，创建者初始化互斥锁/条件变量后最后写 `magic`（release 语义），其余进程轮询等待握手标志并校验 `version`
- **发布路径**：`publish` → 持锁 → payload 写入字节池（跨尾部回卷时分两段拷贝）→ 写描述符（seq 最后写）→ `cond_broadcast` 唤醒所有进程的订阅线程
- **订阅路径**：每进程一个订阅线程，条件变量带超时等待；锁内只做序号追赶和拷贝（不执行用户回调），锁外逐个回调——回调里可安全地再 `publish`
- **丢包检测**：订阅端序号 gap 超 256 条（描述符环被覆盖）或 payload 被字节池回卷覆盖时，打印 dropped/overwritten 告警
- **崩溃恢复**：robust 互斥锁，持锁进程崩溃后其他进程锁时收到 `EOWNERDEAD`，调用 `pthread_mutex_consistent` 恢复继续运行
- **生命周期**：`munmap` 只解除本进程映射；`shm_unlink` 由 `init(true)` 的 mgr 进程在 `deinit` 时执行（unlink 后已有映射仍有效，仅阻止新进程加入）

## 使用说明

支持多进程间通讯，需要定义相同 topic 和数据结构体（见 ginclude/GlobalDataStu.h）。
pub/sub 启动顺序任意：谁先创建共享内存谁自动完成初始化，其余进程等待就绪标志；
后启动的订阅者会立即收到所订阅 topic 的最新一帧（latched）。
`init` 的 `is_mgr` 参数仅决定退出时是否清理共享内存文件（shm_unlink），`deinit` 无需传参。

## 待解决问题

1. qos（目前已具备：每 topic 描述符环 + 变长字节池（单条上限 64KB）+ 序号、丢包检测告警、latched；待完善：可靠性策略可配置等）

## 版本更新记录

- 新增 mcu 端 dds 设计（mcudds）
- 新增 mcubridge 桥接程序
- 共享内存核心重写：序号 + 环形缓冲、初始化顺序任意、robust 互斥锁、latched
- 构建系统重构：根目录统一 build.sh
