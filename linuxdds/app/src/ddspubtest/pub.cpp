#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <thread>

#include "GlobalDataStu.h"
#include "shdds.h"

namespace
{
std::atomic<bool> g_running{true};

void onSignal(int)
{
    g_running = false;
}

void batteryCbk(void* pMsg)
{
    Battery* msg = static_cast<Battery*>(pMsg);
    printf("[pub] recv battery soc:%u soh:%u\n", msg->soc, msg->soh);
}
} // namespace

// 用法: ddspubtest [interval_ms=1000] [count=0 表示一直发]
// 启动顺序任意: pub/sub 谁先启动都可以, is_mgr 只决定退出时谁 shm_unlink
int main(int argc, char** argv)
{
    int interval_ms = (argc > 1) ? atoi(argv[1]) : 1000;
    long count      = (argc > 2) ? atol(argv[2]) : 0;

    signal(SIGINT, onSignal);
    signal(SIGTERM, onSignal);

    if (!shdds::init(true))
    {
        fprintf(stderr, "[pub] shdds init failed\n");
        return 1;
    }

    shdds::Publisher<CutMotor> pubCut("cutMotor");
    shdds::Subscriber<Battery> subBattery("battery");
    subBattery.subscribe(batteryCbk);

    CutMotor msg{};
    msg.state = 1;
    long sent = 0;
    while (g_running && (count == 0 || sent < count))
    {
        msg.rpm = static_cast<unsigned int>(sent + 1);  // rpm 兼作序号, 订阅端据此检查连续性
        msg.timestamp = std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();
        if (!pubCut.publish(msg))
        {
            fprintf(stderr, "[pub] publish cutMotor failed (seq %ld)\n", sent + 1);
        }
        sent++;
        if (interval_ms > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
        }
    }

    printf("[pub] sent %ld message(s)\n", sent);
    shdds::deinit();  // init(true) 记录为 mgr, 退出时清理 /dev/shm/myshm
    return 0;
}
