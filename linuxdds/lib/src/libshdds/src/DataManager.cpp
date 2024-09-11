#include "DataManager.h"

#include <iostream>
#include <thread>
namespace shdds
{



DataManager::DataManager()
{
}
bool DataManager::init(bool is_mgr)
{
    std::cout<<"DataManager::init"<<std::endl;
    if(m_data_shm != nullptr)
    {
        return true;
    }
    int fd = shm_open("/myshm", O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        perror("shm_open");
        return EXIT_FAILURE;
    }

    // 设置共享内存大小
    if (ftruncate(fd, sizeof(shared_struct)) == -1) {
        perror("ftruncate");
        return EXIT_FAILURE;
    }

    // 映射共享内存
    m_data_shm = (shared_struct *)mmap(NULL, sizeof(shared_struct), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (m_data_shm == MAP_FAILED) {
        perror("mmap");
        return EXIT_FAILURE;
    }
    if(is_mgr)
    {
        std::cout<<"DataManager::init mutex"<<std::endl;
        pthread_mutexattr_t mutexAttr;
        pthread_mutexattr_init(&mutexAttr);
        pthread_mutexattr_setpshared(&mutexAttr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&m_data_shm->mutex, &mutexAttr);

        pthread_condattr_t condAttr;
        pthread_condattr_init(&condAttr);
        pthread_condattr_setpshared(&condAttr, PTHREAD_PROCESS_SHARED);
        pthread_cond_init(&m_data_shm->cond, &condAttr);
    }
    std::thread subThread (&DataManager::SubThreadFunc ,this);
    if (subThread.joinable())
    {
        subThread.detach();
    }
    return true;
}
void DataManager::deinit(bool is_mgr)
{
    // 停止线程
    start_threads.store(false);

    // 解除映射的共享内存
    if (m_data_shm != nullptr)
    {
        if (munmap(m_data_shm, sizeof(shared_struct)) == -1) {
            perror("munmap");
        }
        m_data_shm = nullptr;
    }


    // 仅在管理进程中移除共享内存对象
    if (is_mgr)
    {
        if (shm_unlink("/myshm") == -1) {
            perror("shm_unlink");
        }
    }
}



void DataManager::SubThreadFunc()
{
    std::cout<<"DataManager::SubThreadFunc"<<std::endl;
    while(start_threads.load())
    {
        pthread_mutex_lock(&m_data_shm->mutex);//防止读取时刻数据被修改
        pthread_cond_wait(&m_data_shm->cond, &m_data_shm->mutex); // 在调用 pthread_cond_wait 前，这个互斥锁必须由调用线程持有（即已经被锁定）
         // 获取最近更新的 topic 索引
        int updated_index = m_data_shm->last_updated_topic_index;

        // 确保该索引合法
        if (updated_index >= 0 && updated_index < MAX_TOPICS && strlen(m_data_shm->topics[updated_index]) > 0)
        {
            auto it = m_sub_map.find(m_data_shm->topics[updated_index]);
            if (it != m_sub_map.end() && it->second != nullptr)
            {
                // 执行该 topic 对应的回调
                it->second(m_data_shm->data[updated_index]);
            }
        }
        // for (int i = 0; i < MAX_TOPICS; ++i)
        // {
        //     if (strlen(m_data_shm->topics[i]) > 0)  // 如果该 topic 不为空
        //     {
        //         auto it = m_sub_map.find(m_data_shm->topics[i]);
        //         if (it != m_sub_map.end() && it->second != nullptr)
        //         {
        //             it->second(m_data_shm->data[i]);
        //         }
        //     }
        // }
        
        pthread_mutex_unlock(&m_data_shm->mutex);
    }
}    



void DataManager::regSubCbk(const std::string& topic,std::function<void(void*)> fun)
{
    if (m_sub_map.find(topic) == m_sub_map.end()) 
    {
        m_sub_map[topic] = fun;
        std::cout << "Registered new callback for topic: " << topic << std::endl;
    } 
    else 
    {
        // 如果已存在，输出提示信息
        std::cout << "Callback already registered for topic: " << topic << std::endl;
    } 
}
int DataManager::findTopicIndex(const std::string& topic_name)
{
    // 查找是否已经存在该 topic
    for (int i = 0; i < MAX_TOPICS; ++i)
    {
        if (strcmp(m_data_shm->topics[i], topic_name.c_str()) == 0)
        {
            // 找到了已有的 topic，返回它的索引
            return i;
        }
    }

    // 如果没有找到相应的 topic，查找空位来创建新的 topic
    for (int i = 0; i < MAX_TOPICS; ++i)
    {
        if (m_data_shm->topics[i][0] == '\0')  // 检查是否为空字符串，空表示该位置可用
        {
            // 在此空位置创建新的 topic
            strcpy(m_data_shm->topics[i], topic_name.c_str());
            return i;  // 返回新创建的 topic 的索引
        }
    }

    // 如果没有找到空位，返回 -1，表示 topic 空间已满
    return -1;
}
void DataManager::write(const std::string& topic_name, const void* p_topic_msg, int len)
{
    pthread_mutex_lock(&m_data_shm->mutex);

    // 查找或创建 topic
    int topic_index = findTopicIndex(topic_name);
    if (topic_index == -1)
    {
        // 如果没有找到 topic，且无法创建新的 topic（因为空间满了）
        printf("无法创建新 topic，最大容量已满或无法找到 topic\n");
        pthread_mutex_unlock(&m_data_shm->mutex);
        return;
    }

    // 写入数据到对应的 topic
    memcpy(m_data_shm->data[topic_index], p_topic_msg, len);
    m_data_shm->last_updated_topic_index = topic_index;
    // 广播通知订阅者
    pthread_cond_broadcast(&m_data_shm->cond);
    
    pthread_mutex_unlock(&m_data_shm->mutex);
}

}