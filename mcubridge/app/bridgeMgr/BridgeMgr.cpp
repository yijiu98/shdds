#include "BridgeMgr.h"



BridgeMgr::BridgeMgr()
{
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


