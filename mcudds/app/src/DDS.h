#ifndef __DDS_H_
#define __DDS_H_
#include "GlobalDataStu.h"
#include "CommMgr.h"
#include "CommMap.h"
typedef void(* SubCallback)(void* subMsg);
typedef struct
{
	S32(* publish)(const char* topicName, void* topicMsg);
	S32(* subscribe)(const char* topicName, SubCallback cbk);
}DDS;

extern  DDS * getInterface(void);
#endif