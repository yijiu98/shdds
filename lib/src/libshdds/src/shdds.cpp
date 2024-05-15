#include "shdds.h"
#include <iostream>
#include "ShddsMgr.h"


namespace shdds{


bool init(bool is_mgr)
{
    // std::cout<<"hello world"<<std::endl;
    ShddsMgr::Instance()->init(is_mgr);
    return true;
}
void deinit(bool is_mgr)
{
    ShddsMgr::Instance()->deinit();
    std::cout<<"hello world"<<std::endl;
}


}