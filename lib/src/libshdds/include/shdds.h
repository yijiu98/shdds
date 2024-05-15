#pragma once
#include <string>
#include <functional>
#include "ShddsMgr.h"
namespace shdds{

/**
*	@note: 	must call once before use.
*	@param	void.
*	@return void.
**/
bool init(bool is_mgr = 0);

void deinit(bool is_mgr = 0);


template<typename T>
class Publisher
{
public:
	Publisher(const std::string& topic):topic_name(topic){}
	void publish(T const& topic_msg)
	{
		std::size_t size = sizeof(T);
		int len = static_cast<int>(size);
		ShddsMgr::Instance()->publish(topic_name,const_cast<void*>((void*)&topic_msg),len);
	}
private:
	std::string topic_name;
};

template<typename T>
class Subscriber
{
public:
	Subscriber(const std::string& topic):topic_name(topic)
	{
		
	}
	void subscribe(std::function<void(void*)> cbk)
	{
		m_sub_cbk = cbk;
		ShddsMgr::Instance()->regsSubCbk(topic_name,m_sub_cbk);
	}
private:
	std::function<void(void*)> m_sub_cbk{nullptr};
	std::string topic_name;
};

}
