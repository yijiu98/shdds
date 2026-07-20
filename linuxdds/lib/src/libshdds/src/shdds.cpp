#include "shdds.h"

#include "DataManager.h"

namespace shdds
{

bool init(bool is_mgr)
{
    return DataManager::Instance()->init(is_mgr);
}

void deinit()
{
    DataManager::Instance()->deinit();
}

namespace detail
{

bool publish(const std::string& topic_name, const void* p_topic_msg, int len)
{
    return DataManager::Instance()->write(topic_name, p_topic_msg, len);
}

void subscribe(const std::string& topic_name, std::function<void(void*)> cbk)
{
    DataManager::Instance()->regSubCbk(topic_name, std::move(cbk));
}

} // namespace detail

} // namespace shdds
