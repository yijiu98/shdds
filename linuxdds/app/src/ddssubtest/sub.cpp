#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <mutex>
#include <thread>

#include "GlobalDataStu.h"
#include "shdds.h"

namespace
{
std::atomic<bool> g_running{true};

// 以下统计只在订阅回调(单线程)里写, 退出汇总时主线程读, 用 mutex 保护
std::mutex g_stat_mu;
unsigned long g_recv = 0;
unsigned long g_lost = 0;      // 通过序号 gap 推算出的丢包数
long long g_lat_min = -1;
long long g_lat_max = 0;
long long g_lat_sum = 0;

unsigned int g_expect = 0;     // 期望的下一条序号, 仅回调线程访问
bool g_first = true;

void onSignal(int)
{
    g_running = false;
}

void cutCbk(void* pMsg)
{
    CutMotor* msg = static_cast<CutMotor*>(pMsg);

    // rpm 是发布端写入的递增序号; 晚加入的订阅者第一帧是 latched 的最新帧,
    // 以它为基线, 历史消息不计入丢包
    if (g_first)
    {
        g_expect = msg->rpm;
        g_first = false;
        printf("[sub] first frame seq %u (latched 基线)\n", msg->rpm);
    }
    if (msg->rpm != g_expect)
    {
        unsigned long gap = msg->rpm - g_expect;
        printf("[sub] gap: expect seq %u, got %u (%lu lost)\n", g_expect, msg->rpm, gap);
        {
            std::lock_guard<std::mutex> lk(g_stat_mu);
            g_lost += gap;
        }
        g_expect = msg->rpm;
    }
    g_expect++;

    long long now = std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    long long lat = now - msg->timestamp;
    {
        std::lock_guard<std::mutex> lk(g_stat_mu);
        g_recv++;
        g_lat_sum += lat;
        if (g_lat_min < 0 || lat < g_lat_min) g_lat_min = lat;
        if (lat > g_lat_max) g_lat_max = lat;
    }
    printf("[sub] recv cutMotor state:%u seq:%u latency:%lld us\n", msg->state, msg->rpm, lat);
}
} // namespace

// 用法: ddssubtest
// 订阅 cutMotor 并校验序号连续性、统计延迟; 同时周期性发布 battery 作为回环演示
int main(int /*argc*/, char** /*argv*/)
{
    signal(SIGINT, onSignal);
    signal(SIGTERM, onSignal);

    if (!shdds::init(false))
    {
        fprintf(stderr, "[sub] shdds init failed\n");
        return 1;
    }

    shdds::Subscriber<CutMotor> subCut("cutMotor");
    subCut.subscribe(cutCbk);

    shdds::Publisher<Battery> pubBattery("battery");
    Battery battery{1, 98};
    while (g_running)
    {
        battery.soh++;
        pubBattery.publish(battery);
        for (int i = 0; i < 30 && g_running; ++i)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    {
        std::lock_guard<std::mutex> lk(g_stat_mu);
        printf("[sub] total recv %lu, lost %lu", g_recv, g_lost);
        if (g_recv > 0)
        {
            printf(", latency min/avg/max = %lld/%lld/%lld us",
                   g_lat_min, g_lat_sum / (long long)g_recv, g_lat_max);
        }
        printf("\n");
    }

    shdds::deinit();
    return 0;
}
