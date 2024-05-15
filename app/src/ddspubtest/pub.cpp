#include <iostream>
#include "shdds.h"
#include <thread>
#include <memory>  // 包含智能指针的头文件
#include "GlobalDataStu.h"


void batRetCbk(void* pMsg)
{
    Battery* msg = (Battery*)pMsg;
    printf("recv cutmotor soc:%d,soh:%d \n",msg->soc,msg->soh);
}


int main(int argc, char **argv)
{
    shdds::init(true);
    std::shared_ptr<shdds::Publisher<CutMotor>> m_Pub_Cut_Motor = std::make_shared<shdds::Publisher<CutMotor>>("cutMotor");
    CutMotor cutMotor = 
    {
        .state=1,
        .rpm=2400
    };


    std::shared_ptr<shdds::Subscriber<Battery>> m_Sub_Battery = std::make_shared<shdds::Subscriber<Battery>>("battery");
    std::function<void(void*)> cb = std::bind(batRetCbk,std::placeholders::_1);
    m_Sub_Battery->subscribe(cb);
    while(true)
    {
        cutMotor.rpm++;
        m_Pub_Cut_Motor->publish(cutMotor);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    // shdds::publish();
    exit(1);
}