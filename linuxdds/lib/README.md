# libshdds 设计原理

基于 Linux POSIX 共享内存（`shm_open` + `mmap`）的进程间发布/订阅库。
零拷贝数据面 + 条件变量通知，支持任意启动顺序、丢包检测、latched 最新帧。

## 分层结构

```text
shdds.h            模板 API: Publisher<T> / Subscriber<T>, 按 sizeof(T) 发布 POD 数据
   │               init/deinit + detail 窄接口(隐藏内部实现)
   │
DataManager        核心: 共享内存映射、环形缓冲读写、订阅分发线程
```

用户进程通过 `libshdds.so` 映射同一块共享内存；所有同步原语都是
`PTHREAD_PROCESS_SHARED`，因此可以跨进程使用。

## 共享内存布局

```text
/dev/shm/myshm  →  struct shared_struct
├─ pthread_mutex_t mutex          robust + process-shared
├─ pthread_cond_t  cond           process-shared, 只作"叫醒"用
├─ uint32_t magic                 初始化握手标志(最后写入)
├─ uint32_t version               布局版本, 不一致拒绝接入
├─ char topics[10][32]            topic 名, 写一次不再修改
├─ uint64_t seq[10]               每 topic 已发布总数(单调递增序号)
└─ Slot  ring[10][256]            每 topic 独立环形缓冲
      └─ Slot { uint64_t seq; uint32_t len; char data[1024]; }
```

总大小约 2.6 MB。容量参数（topic 数、名称长度、单帧大小、环深）都是
[DataManager.h](src/libshdds/src/DataManager.h) 里的常量。

## 关键机制

### 1. 初始化握手（启动顺序任意）

- `shm_open(O_RDWR|O_CREAT|O_EXCL)` 成功的一方是**创建者**：`ftruncate`、初始化
  互斥锁/条件变量，全部完成后**最后**写 `magic`（`__ATOMIC_RELEASE`）；
- `EEXIST` 的一方是**后来者**：映射后自旋等 `magic`（`__ATOMIC_ACQUIRE`，5 秒超时），
  再校验 `version`；
- 因此不存在"某个进程必须先启动"的要求，`init(is_mgr)` 的参数不再决定初始化角色，
  仅在 `deinit` 时决定是否 `shm_unlink`（init 时记录，deinit 无需再传参）。

### 2. 发布：写环 + 递增序号 + 广播

发布路径要解决的问题是：**多进程间传数据，怎么做到够快、写方永不被拖住、
丢了能被发现**。`write()` 的每一部分都是沿着这个思考链条推出来的：

1. **数据放哪？** 进程间传数据要么过内核（管道/socket，两次拷贝+系统调用），
   要么共享内存（双方直接读写同一块物理内存，零系统调用）。选共享内存——
   这是低延迟的前提，也是整个库的地基。
2. **缓冲区什么结构？** 定长数组 + 取模下标构成环：第 N 条放 `ring[N % 256]`，
   写满后新数据自然覆盖最旧的槽位。为什么不阻塞写方？因为发布往往是实时
   路径（电机状态、传感器帧），订阅方卡住不该反过来拖死发布方；为什么不
   丢新数据？对状态类数据而言，最新帧永远比旧帧有价值。覆盖最旧 + 写方
   永不阻塞，是唯一同时满足这两点的结构。
3. **覆盖 = 可能丢，怎么让丢失可见？** 给消息编号。维护一个单调递增的总序号
   `seq[idx]`（该 topic 一共发过多少条），每个槽位再自带 `slot.seq` 戳。
   订阅方拿自己的消费游标和总序号一比：差几条就追几条，差超过环深就是
   被覆盖了——丢包从"静默发生"变成"精确可数的告警"。编号是检测的前提。
4. **多进程并发写怎么不撕裂？** 一把 process-shared 互斥锁包住整个写入。
   读者也持同一把锁，因此任何时刻看到的都是一致快照，不存在读到半包。
   代价是锁内一次 ≤1KB 的 memcpy，微秒级——用极小的临界区换正确性。
5. **写完怎么通知订阅方？** 旧设计让通知承载数据（广播里带 topic 下标），
   结果丢通知 = 丢数据。现在的思路是**通知与数据分离**：条件变量只当门铃，
   数据真相永远在序号里。广播丢了、没人等着，都无所谓——订阅方超时醒来
   照样按序号把数据追回来。叫醒机制只影响延迟，不影响正确性。
6. **为什么按 data → len → slot.seq → seq[idx] 的顺序写？** 提交点思想：
   `seq[idx]` 是读者判断"有新数据"的唯一依据，必须在槽位内容完整之后
   才递增，如同数据库先写日志再提交。

最终代码：

```text
未初始化 / 空指针 / 长度 >1024 / 名称 ≥32  →  锁外快速失败, 返回 false (不截断)
lock(mutex)
  idx  = 查找或创建 topic             (首次 publish 即隐式注册, 多进程同名复用)
  next = seq[idx]
  slot = ring[idx][next % 256]
  slot: memcpy data → 写 len → 写 slot.seq = next+1
  seq[idx] = next+1                   (提交点)
  pthread_cond_broadcast(cond)        (叫醒所有进程的订阅线程)
unlock
```

多发布者并发时，锁把 publish 串行化，消息按获锁先后交错入环，每条完整、
序号连续；但库不做发布权仲裁，业务上应避免同 topic 多发布者。

### 3. 订阅：序号追赶，而不是"等通知拿数据"

订阅线程每轮：

```text
lock(mutex)
  对每个已注册回调的 topic:
      latest = seq[i]
      if latest - last > 256: 打印 "N message(s) dropped", 跳到 latest-256
      while last < latest:
          校验 slot.seq == last+1 后拷到本地队列   ← 拷出, 不在锁内回调
          last++
  if 没有新数据: pthread_cond_timedwait(200ms)     ← 检查与等待都在锁内, 无丢失唤醒窗口
unlock
在锁外逐条执行用户回调                                ← 回调里可再 publish, 不会死锁
```

要点：

- **条件变量丢了通知不丢数据**。唤醒后按序号追赶，数据才是真相；这与旧实现
  "广播里隐式携带 last_updated_topic_index" 有本质区别；
- **丢包可检测**：突发超过环深时旧数据被覆盖，订阅方通过序号 gap 发现并打印数量，
  不再静默丢失；
- 200ms 超时等待兼作 `deinit` 停止信号的响应窗口。

### 4. Latched（新订阅者立取最新帧）

`regSubCbk` 注册时读取该 topic 最新槽位（带 `slot.seq` 校验），立即回调一次，
同时把消费游标推到最新，避免分发线程重复投递。
等价于 DDS 的 `TRANSIENT_LOCAL` + depth 1。历史消息不回放。

### 5. Robust 互斥锁

互斥锁带 `PTHREAD_MUTEX_ROBUST`：持锁进程崩溃后，其他进程加锁得到
`EOWNERDEAD`，调用 `pthread_mutex_consistent` 恢复后继续，不会全员永久卡死。

### 6. 生命周期

- 订阅线程为 `std::thread` 成员，`deinit` 置停止标志 → 广播唤醒 → `join` →
  `munmap`；杜绝线程访问已解除映射内存的段错误；
- 单例使用 C++11 magic static，析构时兜底 `deinit`，用户忘调也不会 `std::terminate`。

## 使用约束

- 发布的数据必须是 **POD**（按 `sizeof(T)` 原样拷贝，含指针/std 类型的结构体无意义）；
- 单帧最大 `MAX_DATA_SIZE`(1024) 字节，topic 名最长 31 字符，超限拒绝并返回 `false`；
- 同一 topic 多发布者：消息会按锁序交错写入同一环（不丢、不错乱），
  但库不做发布权仲裁，业务上应避免；
- 跨进程传递的是原始字节，要求各端**结构体布局一致**（同编译器/对齐/位宽），
  不同架构（如 x86 与 ARM MCU）之间请走 mcudds/mcubridge 的协议序列化。

## 性能特征

- 数据面：一次 `memcpy` 进环 + 一次 `memcpy` 出环，无系统调用；
- 实测本机（B660M 平台）pub→sub 回调延迟约 30~180 µs；
- 突发容量：每 topic 256 帧，超出覆盖最旧并告警。
