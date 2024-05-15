#include <iostream>
#include "shdds.h"
#include <thread>
#include <thread>
#include <memory>  // 包含智能指针的头文件
#include "GlobalDataStu.h"

void cutRetCbk(void* pMsg)
{
    CutMotor* msg = (CutMotor*)pMsg;
    printf("recv cutmotor state:%d,rpm:%d \n",msg->state,msg->rpm);
}


int main(int argc, char **argv)
{
    shdds::init(false);
    std::shared_ptr<shdds::Subscriber<CutMotor>> m_Sub_Cut_Motor = std::make_shared<shdds::Subscriber<CutMotor>>("cutMotor");
    std::function<void(void*)> cb = std::bind(cutRetCbk,std::placeholders::_1);
    m_Sub_Cut_Motor->subscribe(cb);



    std::shared_ptr<shdds::Publisher<Battery>> m_Pub_Battery = std::make_shared<shdds::Publisher<Battery>>("battery");
    Battery battery = 
    {
        .soc=1,
        .soh=98
    };
    while(1)
    {
        battery.soh++;
        m_Pub_Battery->publish(battery);
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    exit(1);
}