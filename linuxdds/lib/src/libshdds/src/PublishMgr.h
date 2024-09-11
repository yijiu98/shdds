#pragma once
#include "DataManager.h"
namespace shdds
{
    
class PublishMgr
{
public:
    bool init(bool is_mgr=0);
    void deinit(bool is_mgr=0);
    void publish(const std::string& topic_name,const void* p_topic_msg,int len);

};


}