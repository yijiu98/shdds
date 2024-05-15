#include "ShddsMgr.h"
#include "PublishMgr.h"
#include "SubscribeMgr.h"
namespace shdds
{

ShddsMgr::ShddsMgr():m_pub_mgr(std::make_shared<PublishMgr>())
                    ,m_sub_mgr(std::make_shared<SubscribeMgr>())
{
    
}

bool ShddsMgr::init(bool is_mgr)
{
    m_pub_mgr->init(is_mgr);
    m_sub_mgr->init(is_mgr);
    return true;
}

void ShddsMgr::deinit(bool is_mgr)
{

}

void ShddsMgr::publish(const std::string& topic_name,const void* p_topic_msg,int len)
{
    m_pub_mgr->publish(topic_name,p_topic_msg,len);
}

void ShddsMgr::regsSubCbk(const std::string& topic,std::function<void(void*)> fun)
{
    m_sub_mgr->regsSubCbk(topic,fun);
}

}