#include <iostream>
#include "shdds.h"
#include <thread>
#include <thread>
#include <memory>  // 包含智能指针的头文件
#include "GlobalDataStu.h"

void cutRetCbk(void* pMsg)
{
    CutMotor* msg = (CutMotor*)pMsg;
    long long int send_time_stamp = msg->timestamp;
    // 获取当前时间戳（微秒级别）
    auto recv_time = std::chrono::high_resolution_clock::now();
    
    long long int recv_time_stamp = std::chrono::duration_cast<std::chrono::microseconds>(recv_time.time_since_epoch()).count();

    // 计算延迟
    long long int latency = recv_time_stamp - send_time_stamp;
    std::cout << "Communication delay: " << latency << " microseconds" << std::endl;
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