#pragma once
#include <memory>
#include <string>
#include "shdds.h"
#include "DDS.h"

#define MAX_INS (4)

template <class T>
class MsgPublishToLinux
{
  public:

    MsgPublishToLinux(const std::string & topic_name):mTopicName(topic_name)
    {
      gIns[gInsNum] = this;

      getInterface()->subscribe(topic_name.c_str(),MsgPublishToLinux::MsgPubCallback);

      mPub = std::make_shared<shdds::Publisher<T>>(topic_name);

      gInsNum++;
    }

    static MsgPublishToLinux* gIns[MAX_INS];
    static int gInsNum;

  private:

    static void MsgPubCallback(void * pMsg)
    {
      for(int i = 0;i<gInsNum; i++)
      {
        if(gIns[i] != nullptr) 
        {
        //   if(topicName == gIns[i]->mTopicName)
          {
            (gIns[i]->mPub)->publish(*(T*)pMsg);
          }
        }
      }
    }

    std::shared_ptr<shdds::Publisher<T>> mPub;
    std::string mTopicName;
};
template<typename T> MsgPublishToLinux<T>* MsgPublishToLinux<T>::gIns[MAX_INS];
template<typename T> int MsgPublishToLinux<T>::gInsNum = 0;


template <class T>
class MsgSubFromLinux
{
public:
    MsgSubFromLinux(const std::string & topic_name):mTopicName(topic_name)
    {
        mSub = std::make_shared<shdds::Subscriber<T>>(topic_name);

        // 使用 lambda 表达式将 void* 转换为 T*
        std::function<void(void*)> fr = [this](void* ptr) {
            T* pMsg = static_cast<T*>(ptr);  // 将 void* 转换为 T*
            this->MsgSubCallback(pMsg);  // 调用原来的回调
        };

        mSub->subscribe(fr);  // 将转换后的回调传入 subscribe 函数
    }

private:
    void MsgSubCallback(T* pMsg)
    {
        getInterface()->publish(mTopicName.c_str(), pMsg);  
    }
    
    std::shared_ptr<shdds::Subscriber<T>> mSub;
    std::string mTopicName;
};


 class BridgeMgr
{
  public:
    BridgeMgr();
    ~BridgeMgr();

  private:
    void PublishToLinux();
    void SubFromLinux();

    std::shared_ptr<MsgPublishToLinux<Battery>> mBattery;

    std::shared_ptr<MsgSubFromLinux<LeftMotor>> mLeftMotor;
    
};