#include "DataManager.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include <iostream>
#include <vector>

namespace shdds
{

namespace
{

// 加锁; 若上一个持锁进程崩溃, 恢复互斥锁并标记状态一致
bool lock_robust(pthread_mutex_t* m)
{
    int rc = pthread_mutex_lock(m);
    if (rc == EOWNERDEAD)
    {
        fprintf(stderr, "[shdds] previous lock owner crashed, mutex recovered\n");
        pthread_mutex_consistent(m);
        return true;
    }
    if (rc != 0)
    {
        fprintf(stderr, "[shdds] pthread_mutex_lock failed: %d\n", rc);
        return false;
    }
    return true;
}

struct PendingMsg
{
    std::string topic;
    uint32_t len;
    char data[MAX_DATA_SIZE];
};

} // namespace

DataManager::DataManager()
{
}

DataManager::~DataManager()
{
    deinit();
}

bool DataManager::init(bool is_mgr)
{
    if (m_data_shm != nullptr)
    {
        return true;
    }
    m_is_mgr = is_mgr;

    // O_EXCL  "排他创建"：必须和 O_CREAT 一起用，文件不存在 → 创建成功，返回 fd;文件已存在 → 打开失败,errno = EEXIST。
    bool creator = false;
    int fd = shm_open(SHM_NAME, O_RDWR | O_CREAT | O_EXCL, 0666);
    if (fd >= 0)
    {
        creator = true;//此次创建的，此次后面接着就初始化
    }
    else if (errno == EEXIST)
    {
        fd = shm_open(SHM_NAME, O_RDWR, 0666);
        if (fd == -1)
        {
            perror("shm_open");
            return false;
        }
    }
    else
    {
        perror("shm_open");
        return false;
    }

    if (creator && ftruncate(fd, sizeof(shared_struct)) == -1)
    {
        perror("ftruncate");
        close(fd);
        return false;
    }

    void* p = mmap(NULL, sizeof(shared_struct), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    if (p == MAP_FAILED)
    {
        perror("mmap");
        return false;
    }
    m_data_shm = static_cast<shared_struct*>(p);

    if (creator)
    {
        memset(m_data_shm, 0, sizeof(shared_struct));  // magic 保持 0, 最后才置位

        pthread_mutexattr_t mutexAttr;//初始化互斥锁
        pthread_mutexattr_init(&mutexAttr);
        pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);//跨进程共享
        pthread_mutexattr_setrobust(&mutexAttr, PTHREAD_MUTEX_ROBUST);//健壮锁
        pthread_mutex_init(&m_data_shm->mutex, &mutexAttr);
        pthread_mutexattr_destroy(&mutexAttr);

        pthread_condattr_t condAttr;//初始化条件变量
        pthread_condattr_init(&condAttr);
        pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&m_data_shm->cond, &condAttr);
        pthread_condattr_destroy(&condAttr);

        m_data_shm->version = SHM_VERSION;
        // release 语义保证上面的初始化先于 magic 对其他进程可见
        __atomic_store_n(&m_data_shm->magic, SHM_MAGIC, __ATOMIC_RELEASE);//原子性
    }
    else
    {
        // 等待创建者完成初始化, 最多 5 秒
        bool ready = false;
        for (int i = 0; i < 500; ++i)
        {
            if (__atomic_load_n(&m_data_shm->magic, __ATOMIC_ACQUIRE) == SHM_MAGIC)
            {
                ready = true;
                break;
            }
            usleep(10 * 1000);
        }
        if (!ready)
        {
            fprintf(stderr, "[shdds] timeout waiting for shared memory init\n");
            munmap(m_data_shm, sizeof(shared_struct));
            m_data_shm = nullptr;
            return false;
        }
        if (m_data_shm->version != SHM_VERSION)
        {
            fprintf(stderr, "[shdds] shm layout version mismatch: %u != %u\n",
                    m_data_shm->version, SHM_VERSION);
            munmap(m_data_shm, sizeof(shared_struct));
            m_data_shm = nullptr;
            return false;
        }
    }

    start_threads.store(true);
    m_sub_thread = std::thread(&DataManager::SubThreadFunc, this);
    return true;
}

void DataManager::deinit()
{
    if (m_data_shm == nullptr)
    {
        return;
    }

    // 唤醒并等待订阅线程退出, 避免线程访问已 munmap 的内存
    start_threads.store(false);
    if (lock_robust(&m_data_shm->mutex))
    {
        pthread_cond_broadcast(&m_data_shm->cond);
        pthread_mutex_unlock(&m_data_shm->mutex);
    }
    if (m_sub_thread.joinable())
    {
        m_sub_thread.join();
    }

    if (munmap(m_data_shm, sizeof(shared_struct)) == -1)
    {
        perror("munmap");
    }
    m_data_shm = nullptr;

    if (m_is_mgr && shm_unlink(SHM_NAME) == -1 && errno != ENOENT)
    {
        perror("shm_unlink");
    }
}

void DataManager::SubThreadFunc()
{
    // 基线: 只接收本进程启动之后发布的新数据, 不回放历史
    if (lock_robust(&m_data_shm->mutex))
    {
        for (int i = 0; i < MAX_TOPICS; ++i)
        {
            m_last_seq[i] = m_data_shm->seq[i];
        }
        pthread_mutex_unlock(&m_data_shm->mutex);
    }

    std::vector<PendingMsg> pending;
    while (start_threads.load())
    {
        pending.clear();

        if (!lock_robust(&m_data_shm->mutex))
        {
            usleep(10 * 1000);
            continue;
        }

        // 锁内只做追赶和拷贝, 不执行用户回调
        for (int i = 0; i < MAX_TOPICS; ++i)
        {
            if (m_data_shm->topics[i][0] == '\0')
            {
                continue;
            }

            bool has_cb;
            {
                std::lock_guard<std::mutex> g(m_map_mutex);
                has_cb = m_sub_map.find(m_data_shm->topics[i]) != m_sub_map.end();
            }
            if (!has_cb)
            {
                m_last_seq[i] = m_data_shm->seq[i];  // 无人订阅的 topic 不积压
                continue;
            }

            uint64_t next = m_last_seq[i];
            uint64_t latest = m_data_shm->seq[i];
            if (latest - next > RING_DEPTH)
            {
                fprintf(stderr, "[shdds] topic %s: %llu message(s) dropped (subscriber too slow)\n",
                        m_data_shm->topics[i],
                        (unsigned long long)(latest - next - RING_DEPTH));
                next = latest - RING_DEPTH;
            }
            while (next < latest)
            {
                Slot* s = &m_data_shm->ring[i][next % RING_DEPTH];
                if (s->seq == next + 1)  // 序号校验, 防止读到被覆盖的槽位
                {
                    PendingMsg msg;
                    msg.topic = m_data_shm->topics[i];
                    msg.len = s->len;
                    memcpy(msg.data, s->data, s->len);
                    pending.push_back(std::move(msg));
                }
                next++;
            }
            m_last_seq[i] = next;
        }

        if (pending.empty())
        {
            // 检查和等待都在锁内完成, 不存在丢失唤醒的窗口;
            // 带超时以便及时响应 deinit 的停止请求
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_nsec += 200 * 1000 * 1000;
            if (ts.tv_nsec >= 1000000000L)
            {
                ts.tv_sec += 1;
                ts.tv_nsec -= 1000000000L;
            }
            pthread_cond_timedwait(&m_data_shm->cond, &m_data_shm->mutex, &ts);
        }
        pthread_mutex_unlock(&m_data_shm->mutex);

        // 锁外执行回调, 回调里可以再 publish, 不会死锁
        for (auto& msg : pending)
        {
            std::function<void(void*)> cb;
            {
                std::lock_guard<std::mutex> g(m_map_mutex);
                auto it = m_sub_map.find(msg.topic);
                if (it != m_sub_map.end())
                {
                    cb = it->second;
                }
            }
            if (cb != nullptr)
            {
                cb(msg.data);
            }
        }
    }
}

void DataManager::regSubCbk(const std::string& topic, std::function<void(void*)> fun)
{
    {
        std::lock_guard<std::mutex> g(m_map_mutex);
        if (m_sub_map.find(topic) != m_sub_map.end())
        {
            std::cout << "Callback already registered for topic: " << topic << std::endl;
            return;
        }
        m_sub_map[topic] = fun;
        std::cout << "Registered new callback for topic: " << topic << std::endl;
    }

    // latched: 若该 topic 已有数据, 立即向新订阅者投递最新一帧
    if (m_data_shm == nullptr)
    {
        return;
    }
    char data[MAX_DATA_SIZE];
    uint32_t len = 0;
    bool have_last = false;
    if (lock_robust(&m_data_shm->mutex))
    {
        for (int i = 0; i < MAX_TOPICS; ++i)
        {
            if (strncmp(m_data_shm->topics[i], topic.c_str(), MAX_TOPIC_NAME) != 0)
            {
                continue;
            }
            uint64_t latest = m_data_shm->seq[i];
            m_last_seq[i] = latest;  // 从最新开始消费, 避免订阅线程重复投递
            if (latest > 0)
            {
                Slot* s = &m_data_shm->ring[i][(latest - 1) % RING_DEPTH];
                if (s->seq == latest)  // 序号校验, 槽位可能已被覆盖
                {
                    memcpy(data, s->data, s->len);
                    len = s->len;
                    have_last = true;
                }
            }
            break;
        }
        pthread_mutex_unlock(&m_data_shm->mutex);
    }
    if (have_last)
    {
        fun(data);  // 锁外回调
    }
}

int DataManager::findTopicIndex(const std::string& topic_name)
{
    for (int i = 0; i < MAX_TOPICS; ++i)
    {
        if (strncmp(m_data_shm->topics[i], topic_name.c_str(), MAX_TOPIC_NAME) == 0)
        {
            return i;
        }
    }

    for (int i = 0; i < MAX_TOPICS; ++i)
    {
        if (m_data_shm->topics[i][0] == '\0')
        {
            snprintf(m_data_shm->topics[i], MAX_TOPIC_NAME, "%s", topic_name.c_str());
            return i;
        }
    }

    return -1;
}

bool DataManager::write(const std::string& topic_name, const void* p_topic_msg, int len)
{
    if (m_data_shm == nullptr || p_topic_msg == nullptr || len <= 0)
    {
        return false;
    }
    if (len > MAX_DATA_SIZE)
    {
        fprintf(stderr, "[shdds] topic %s: msg len %d exceeds max %d, dropped\n",
                topic_name.c_str(), len, MAX_DATA_SIZE);
        return false;
    }
    if (topic_name.size() >= MAX_TOPIC_NAME)
    {
        fprintf(stderr, "[shdds] topic name too long (max %d): %s\n",
                MAX_TOPIC_NAME - 1, topic_name.c_str());
        return false;
    }

    if (!lock_robust(&m_data_shm->mutex))
    {
        return false;
    }

    int topic_index = findTopicIndex(topic_name);
    if (topic_index == -1)
    {
        fprintf(stderr, "[shdds] no free topic slot (max %d), cannot publish %s\n",
                MAX_TOPICS, topic_name.c_str());
        pthread_mutex_unlock(&m_data_shm->mutex);
        return false;
    }

    uint64_t next = m_data_shm->seq[topic_index];
    Slot* s = &m_data_shm->ring[topic_index][next % RING_DEPTH];
    memcpy(s->data, p_topic_msg, len);//存入数据
    s->len = len;//存数据长度
    s->seq = next + 1;//这个槽里装的是第几条
    m_data_shm->seq[topic_index] = next + 1;//总序号，这个topic一共发到第几条了

    pthread_cond_broadcast(&m_data_shm->cond);
    pthread_mutex_unlock(&m_data_shm->mutex);
    return true;
}

}
