#pragma once

#include <functional>
#include <string>

namespace shdds
{

// is_mgr 只决定 deinit 时是否 shm_unlink 清理共享内存文件;
// 初始化顺序任意, 谁先创建共享内存谁负责初始化
bool init(bool is_mgr = false);
void deinit();

namespace detail
{
// 模板 API 与内部实现(DataManager)之间的窄接口, 由 shdds.cpp 实现
bool publish(const std::string& topic_name, const void* p_topic_msg, int len);
void subscribe(const std::string& topic_name, std::function<void(void*)> cbk);
} // namespace detail

template <typename T>
class Publisher
{
public:
    explicit Publisher(const std::string& topic) : topic_name(topic) {}
    // 数据必须是 POD, 按 sizeof(T) 原样拷贝; 失败(超长/未初始化等)返回 false
    bool publish(T const& topic_msg)
    {
        return detail::publish(topic_name, &topic_msg, static_cast<int>(sizeof(T)));
    }

private:
    std::string topic_name;
};

template <typename T>
class Subscriber
{
public:
    explicit Subscriber(const std::string& topic) : topic_name(topic) {}
    // 每个 topic 仅可注册一次; 注册时若已有数据会立即回调最新一帧(latched)
    void subscribe(std::function<void(void*)> cbk)
    {
        detail::subscribe(topic_name, std::move(cbk));
    }

private:
    std::string topic_name;
};

} // namespace shdds
