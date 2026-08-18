#pragma once

#include <stdint.h>
#include <pthread.h>
#include <atomic>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <thread>

#include "Macro.h"

namespace shdds
{

constexpr int MAX_TOPICS      = 10;
constexpr int MAX_TOPIC_NAME  = 32;
// 每个 topic 的消息描述符深度(最多积压多少"条"消息)。发布方突发超出该深度时,
// 最旧的未读消息会被覆盖, 订阅方通过序号 gap 检测到会打印丢包告警
constexpr int DESC_DEPTH      = 256;
// 每个 topic 的变长数据池(字节), 循环使用。单条消息上限即池大小;
// 池回卷覆盖掉未读 payload 时, 订阅方按描述符 offset 检测到会跳过并告警
constexpr int TOPIC_POOL_SIZE = 64 * 1024;
// 单条消息上限(= 数据池大小)
constexpr int MAX_DATA_SIZE   = TOPIC_POOL_SIZE;

constexpr uint32_t SHM_MAGIC   = 0x53484444;  // "SHDD", 标记共享内存已完成初始化
constexpr uint32_t SHM_VERSION = 3;
constexpr const char* SHM_NAME = "/myshm";

// 一条消息的元数据; payload 变长存放在 pool 里
struct MsgDesc
{
    uint64_t seq;                // 消息序号
    uint64_t offset;             // payload 在 pool 中的单调偏移(取模后为实际位置)
    uint32_t len;
    uint32_t rsv;
};

// 共享内存布局: 所有进程按相同偏移解释, 不要在其中放指针/std 类型
struct Topic
{
    char name[MAX_TOPIC_NAME];
    uint64_t seq;                     // 该 topic 已发布的消息总数(单调递增)
    uint64_t pool_off;                // 已写入 pool 的总字节数(单调递增)
    MsgDesc desc[DESC_DEPTH];         // 消息描述符环形数组
    char pool[TOPIC_POOL_SIZE];       // 变长消息字节池(循环使用)
};

struct shared_struct
{
    pthread_mutex_t mutex;                     // robust + process-shared
    pthread_cond_t  cond;                      // process-shared
    uint32_t magic;                            // 初始化握手标志, 最后写入
    uint32_t version;
    Topic topics[MAX_TOPICS];
};

class DataManager
{
public:
    ~DataManager();  // 用户忘记 deinit 时兜底: 停线程、解除映射
    // is_mgr 仅决定 deinit 时是否 shm_unlink; 初始化顺序任意,
    // 谁先创建共享内存谁负责初始化, 其余进程等待握手标志
    bool init(bool is_mgr);
    void deinit();
    bool write(const std::string& topic_name, const void* p_topic_msg, int len);
    void regSubCbk(const std::string& topic, std::function<void(void*)> fun);
private:
    void SubThreadFunc();
    int findTopicIndex(const std::string& topic_name);  // 调用前需持有共享内存锁
private:
    shared_struct* m_data_shm{nullptr};
    bool m_is_mgr{false};                  // init 时记录, deinit 用
    std::map<std::string, std::function<void(void*)>> m_sub_map;
    std::mutex m_map_mutex;                    // 保护 m_sub_map(进程内)
    std::atomic<bool> start_threads{false};
    std::thread m_sub_thread;
    uint64_t m_last_seq[MAX_TOPICS]{};         // 本进程每个 topic 已消费的序号
private:
    DECLARE_SINGLETON(DataManager)
};


}
