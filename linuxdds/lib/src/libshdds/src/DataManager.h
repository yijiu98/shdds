#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include "Macro.h"
#include <map>
#include <functional>
#include <atomic>
#include <string>

namespace shdds
{

constexpr int MAX_TOPICS = 10;
typedef struct CutMotor
{
 	unsigned int state;
    unsigned int rpm;
}CutMotor;

typedef struct {
    pthread_mutex_t mutex; // 互斥锁
    pthread_cond_t cond;   // 条件变量
    char topics[MAX_TOPICS][16];          // 多个 topic 的名称
    char data[MAX_TOPICS][1024];          // 每个 topic 对应的数据
    int last_updated_topic_index; // 记录最后一个更新的 topic 索引
} shared_struct;   

class DataManager
{
public:
    bool init(bool is_mgr);
    void deinit(bool is_mgr);
    void write(const std::string& topic_name,const void* p_topic_msg,int len);
    void regSubCbk(const std::string& topic,std::function<void(void*)> fun);
private:
    void SubThreadFunc();
    int findTopicIndex(const std::string& topic_name);  // 查找 topic 的下标
private:
    shared_struct* m_data_shm{nullptr};
    std::map<std::string,std::function<void(void*)>> m_sub_map;
    std::atomic<bool> start_threads{true}; 
private:
    DECLARE_SINGLETON(DataManager)
};


}