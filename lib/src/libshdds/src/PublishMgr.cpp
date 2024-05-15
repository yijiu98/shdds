#include "PublishMgr.h"

namespace shdds
{
const char* shddsname = "/shdds";
bool PublishMgr::init(bool is_mgr)
{
    DataManager::Instance()->init(is_mgr);
    return true;
}

void PublishMgr::deinit(bool is_mgr)
{
    
}

void PublishMgr::publish(const std::string& topic_name,const void* p_topic_msg,int len)
{
    DataManager::Instance()->write(topic_name,p_topic_msg,len);
}

}