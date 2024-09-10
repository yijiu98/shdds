#include <iostream>
#include "shdds.h"
#include <thread>
#include <memory>  // 包含智能指针的头文件
#include "GlobalDataStu.h"
#include <csignal>
#include <unistd.h>
// 信号处理函数
void signalHandler(int signum) 
{
    shdds::deinit(true);
    exit(signum);
}
void batRetCbk(void* pMsg)
{
    Battery* msg = (Battery*)pMsg;
    printf("recv battery soc:%d,soh:%d \n",msg->soc,msg->soh);
}


int main(int argc, char **argv)
{
    signal(SIGINT, signalHandler);
    shdds::init(true);

    std::shared_ptr<shdds::Publisher<CutMotor>> m_Pub_Cut_Motor = std::make_shared<shdds::Publisher<CutMotor>>("cutMotor");
    CutMotor cutMotor = 
    {
        .state=1,
        .rpm=2400
    };

    std::shared_ptr<shdds::Publisher<LeftMotor>> m_Pub_Left_Motor = std::make_shared<shdds::Publisher<LeftMotor>>("leftMotor");
    LeftMotor leftMotor = 
    {
        .state=2,
        .rpm=9600
    };


    std::shared_ptr<shdds::Subscriber<Battery>> m_Sub_Battery = std::make_shared<shdds::Subscriber<Battery>>("battery");
    std::function<void(void*)> cb = std::bind(batRetCbk,std::placeholders::_1);
    m_Sub_Battery->subscribe(cb);
    while(true)
    {
          // 获取当前时间戳（微秒级别）并存储为 long long int
        auto now = std::chrono::high_resolution_clock::now();
        long long int time_stamp = std::chrono::duration_cast<std::chrono::microseconds>(now.time_since_epoch()).count();
        cutMotor.timestamp=time_stamp;
        cutMotor.rpm++;
        m_Pub_Cut_Motor->publish(cutMotor);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));



        leftMotor.rpm++;
        m_Pub_Left_Motor->publish(leftMotor);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    // shdds::publish();
    exit(1);
}