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

constexpr int MAX_TOPICS     = 10;
constexpr int MAX_TOPIC_NAME = 32;
constexpr int MAX_DATA_SIZE  = 1024;
// 每个 topic 的环形缓冲深度。发布方突发超出该深度时,最旧的未读消息会被覆盖,
// 订阅方通过序号 gap 检测到会打印丢包告警
constexpr int RING_DEPTH     = 256;

constexpr uint32_t SHM_MAGIC   = 0x53484444;  // "SHDD", 标记共享内存已完成初始化
constexpr uint32_t SHM_VERSION = 2;
constexpr const char* SHM_NAME = "/myshm";

struct Slot
{
    uint64_t seq;              // 该槽位存放的消息序号
    uint32_t len;
    char data[MAX_DATA_SIZE];
};

// 共享内存布局: 所有进程按相同偏移解释, 不要在其中放指针/std 类型
struct shared_struct
{
    pthread_mutex_t mutex;                     // robust + process-shared
    pthread_cond_t  cond;                      // process-shared
    uint32_t magic;                            // 初始化握手标志, 最后写入
    uint32_t version;
    char topics[MAX_TOPICS][MAX_TOPIC_NAME];
    uint64_t seq[MAX_TOPICS];                  // 每 topic 已发布的消息总数(单调递增)
    Slot ring[MAX_TOPICS][RING_DEPTH];         // 每 topic 独立环形缓冲
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
