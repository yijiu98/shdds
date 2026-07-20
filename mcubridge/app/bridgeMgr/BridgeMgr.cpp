#include "BridgeMgr.h"



BridgeMgr::BridgeMgr()
{
    // linux 侧共享内存由 mgr 进程(如 ddspubtest)负责清理, 桥接进程用 false
    shdds::init(false);
    PublishToLinux();
    SubFromLinux();
}

BridgeMgr::~BridgeMgr()
{
    
}

void BridgeMgr::PublishToLinux()
{
    mBattery = std::make_shared<MsgPublishToLinux<Battery>>(Topic_BATTERY);
}

void BridgeMgr::SubFromLinux()
{
    mLeftMotor = std::make_shared<MsgSubFromLinux<LeftMotor>>(Topic_LeftMotor);
}


