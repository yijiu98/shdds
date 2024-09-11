#pragma once

#include "DataManager.h"
namespace shdds
{
    
class SubscribeMgr
{
public:
    bool init(bool is_mgr=0);
    void deinit(bool is_mgr=0);
    void regsSubCbk(const std::string& topic,std::function<void(void*)> fun);
private:
    DataManager* m_data_mgr{nullptr};
};


}

