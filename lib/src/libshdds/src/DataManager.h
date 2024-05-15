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

namespace shdds
{

typedef struct CutMotor
{
 	unsigned int state;
    unsigned int rpm;
}CutMotor;

typedef struct {
    pthread_mutex_t mutex; // 互斥锁
    pthread_cond_t cond;   // 条件变量
    char topic[16];
    char data[1024];
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
private:
    shared_struct* m_data_shm{nullptr};
    std::map<std::string,std::function<void(void*)>> m_sub_map;
private:
    DECLARE_SINGLETON(DataManager)
};


}