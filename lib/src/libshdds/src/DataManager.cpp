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

}
void DataManager::SubThreadFunc()
{
    std::cout<<"DataManager::SubThreadFunc"<<std::endl;
    while(true)
    {
        pthread_mutex_lock(&m_data_shm->mutex);//防止读取时刻数据被修改
        pthread_cond_wait(&m_data_shm->cond, &m_data_shm->mutex); // 在调用 pthread_cond_wait 前，这个互斥锁必须由调用线程持有（即已经被锁定）
        auto it = m_sub_map.find(m_data_shm->topic);
        if (it != m_sub_map.end()) 
        {
            std::cout<<"find topic"<<std::endl;
            if(it->second != nullptr)
            {
                std::cout<<"is not nullptr"<<std::endl;
                it->second(m_data_shm->data);
            }
            else
            {
                std::cout<<"is nullptr"<<std::endl;
            }
        }
        // {
        //     // 如果存在，执行对应的回调函数
        //     it->second(m_data_shm->data);
        // } 
        // else 
        // {
        //     // 如果 topic 不存在，输出提示信息
        //     std::cout << "No callback registered for topic: " << m_data_shm->topic << std::endl;
        // }
        
        pthread_mutex_unlock(&m_data_shm->mutex);
    }
}    

void DataManager::write(const std::string& topic_name,const void* p_topic_msg,int len)
{
    pthread_mutex_lock(&m_data_shm->mutex);//确保对共享内存中的数据 (shared->data) 的访问是互斥的，即在任何时刻只有一个进程能修改这个数据
    memcpy(m_data_shm->data,p_topic_msg,len);
    strcpy(m_data_shm->topic,topic_name.c_str());
    printf("Published topic data \n");
    pthread_cond_broadcast(&m_data_shm->cond);  // 通知所有等待的订阅者
    pthread_mutex_unlock(&m_data_shm->mutex);
}
void DataManager::regSubCbk(const std::string& topic,std::function<void(void*)> fun)
{
    if (m_sub_map.find(topic) == m_sub_map.end()) 
    {
        // 如果不存在，添加 topic 和对应的回调函数
        if(fun == nullptr)
        {
            std::cout << "Registered new callback for topic:is nullptr " << topic << std::endl;
        }
        else 
        {
            std::cout << "Registered new callback for topic:is not nullptr " << topic << std::endl;
        }
        m_sub_map[topic] = fun;
        std::cout << "Registered new callback for topic: " << topic << std::endl;
    } 
    else 
    {
        // 如果已存在，输出提示信息
        std::cout << "Callback already registered for topic: " << topic << std::endl;
    } 
}

}