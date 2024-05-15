#include "SubscribeMgr.h"

namespace shdds
{


bool SubscribeMgr::init(bool is_mgr)
{
    
    return true;
}

void SubscribeMgr::deinit(bool is_mgr)
{

}
void SubscribeMgr::regsSubCbk(const std::string& topic,std::function<void(void*)> fun)
{
    DataManager::Instance()->regSubCbk(topic,fun);
}

}