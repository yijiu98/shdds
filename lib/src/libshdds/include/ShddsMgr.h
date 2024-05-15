#pragma once

#include "Macro.h"
#include <functional>
#include <memory>  // 包含智能指针的头文件

namespace shdds
{
class PublishMgr;
class SubscribeMgr; 

class ShddsMgr
{
public:
    bool init(bool is_mgr=0);
    void deinit(bool is_mgr=0);
    void publish(const std::string& topic_name,const void* p_topic_msg,int len);
    void regsSubCbk(const std::string& topic,std::function<void(void*)> fun);

private:
    std::shared_ptr<PublishMgr> m_pub_mgr{};
    std::shared_ptr<SubscribeMgr> m_sub_mgr{};


private:
    DECLARE_SINGLETON(ShddsMgr)
};


}

