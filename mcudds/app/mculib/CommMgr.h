#ifndef __COMMMGR_H__
#define __COMMMGR_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "GlobalDataStu.h"



typedef S32(*PackFun)(U8* const pBuf, const void* const pMsg, U8* const pPackLen);
typedef S32(*UnPackFun)(const U8* const pBuf, void* const pMsg);
typedef void (*SendFun)(U8* buf, U8 len);
typedef void(* SubCallback)(void* subMsg);
typedef enum commCmd
{
    PRO_INVALID         =0x00,
    PRO_BAT             ,
    
                 
}CommCmd;

typedef struct CommMapElemet
{
	const char* name;
	CommCmd cmd;

	PackFun pack;
	UnPackFun unPack;

    SubCallback subCbk;


}CommMapElemet;

typedef struct CommMgr
{
    S32(*m_packHead)(U8* const pBuf, const CommCmd* const pCmd, U8* const pPackLen);
    S32(*m_unPackHead)(const U8* const pBuf, CommCmd* const pCmd, U8* const pUnPackLen);

    CommMapElemet*                      m_CommMap;
    U8                                  m_CommMapSize;

    S32(*m_framePack)(U8* pSrc, U8* pLen);
    S32(*m_frameUnPack)(U8* pSrc, U8* pLen);
    SendFun        m_sendFun;
}CommMgr;

extern struct CommMgr gCommMgr;

extern  S32 Publish(const char* commName, void* commMsg);
extern  S32 Subscribe(const char* commName, SubCallback callback);
extern  void RegsCmdAgentRecvFun(U8 * recvBuf, U8 * pLen);
extern  void RegsCmdAgentSendFun(SendFun fun);//reg send function


#ifdef __cplusplus
}
#endif

#endif