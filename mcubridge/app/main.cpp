
#include <iostream>
#include <memory> 
#include "BridgeMgr.h"

void BatteryMsgCbk(void* pMsg)
{
    Battery gBattery = *(Battery*)pMsg;
    printf("battery data:\n");
}


int main(int argc, char **argv)
{
    auto pBridge = std::make_shared<BridgeMgr>();



    // getInterface()->subscribe( BATTERY, ( SubCallback ) BatteryMsgCbk );
    while(1)
    {
        // Battery bat =
        // {
        //     .soc=1,               
        //     .soh=1,               
        // };
        // getInterface()->publish(BATTERY,&bat);
    }
    return 0;
}