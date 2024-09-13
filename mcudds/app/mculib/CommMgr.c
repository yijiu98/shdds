#include  "CommMgr.h"
#include <stdio.h>
#include <string.h>
#include "CommMap.h"
#include "FramePack.h"
#include "ProtocolPackUnpack.h"
#define BUF_SIZE (100)



CommMapElemet gCommMap[] =
{

    TOP_BAT_MAP,
    
};


struct CommMgr gCommMgr = { PackHead, UnPackHead,gCommMap, sizeof(gCommMap) / sizeof(CommMapElemet), FramePack,FrameUnPack,NULL };

static S32 getIndexByName(const char* commName)
{
    S32 ret = -1;
    if (commName == NULL)
    {
        return ret;
    }
    for (U8 i = 0; i < gCommMgr.m_CommMapSize; i++)
    {
        if (gCommMgr.m_CommMap[i].name != NULL)
        {
            if (strcmp(gCommMgr.m_CommMap[i].name, commName) == 0)
            {
                ret = i;
                break;
            }
        }
    }
    return ret;
}

static S32 getIndexByPackCmd(CommCmd cmd)
{
    S32 ret = -1;
    if (cmd == PRO_INVALID)
    {
        return ret;
    }
    for (U8 i = 0; i < gCommMgr.m_CommMapSize; i++)
    {
        if (gCommMgr.m_CommMap[i].cmd == cmd)
        {
            ret = i;
            break;
        }
    }
    return ret;
}
S32 Publish(const char* commName, void* commMsg)
{
    if ((commName == NULL))
    {
        return -1;
    }
    S32  index = getIndexByName(commName);
    if (index == -1)
    {
        return -1;
    }
    U8  writeNum = 0;
    U8  writeBuf[BUF_SIZE] = { 0 };
    gCommMgr.m_packHead(writeBuf+2, &(gCommMgr.m_CommMap[index].cmd), &writeNum); //commamd byte
    gCommMgr.m_CommMap[index].pack(writeBuf+2, commMsg, &writeNum);
    gCommMgr.m_framePack(writeBuf, &writeNum);
    if (gCommMgr.m_sendFun != NULL)
    {
        gCommMgr.m_sendFun(writeBuf, writeNum);
    }
    return 0;
}

S32 Subscribe(const char* commName, SubCallback callback)
{
    if ((commName == NULL) || (callback == NULL))
    {
        return -1;
    }
    S32  index = getIndexByName(commName);
    if (index == -1)
    {
        return -1;
    }
    gCommMgr.m_CommMap[index].subCbk = callback;

    return 0;
}

void RegsCmdAgentRecvFun(U8* recvBuf, U8* pLen)
{
    S32 index = 0;

    index= gCommMgr.m_frameUnPack(recvBuf, pLen);
    if (index == -1)
    {
        return;
    }

    CommCmd unPackCmd = PRO_INVALID;

    U8 unPackLen = 0;
    gCommMgr.m_unPackHead(recvBuf+2, &unPackCmd, &unPackLen);

    index = getIndexByPackCmd(unPackCmd);
    if (index == -1)
    {
        return;
    }

    if (gCommMgr.m_CommMap[index].subCbk == NULL)
    {
        return;
    }

    U8 unPackMsg[BUF_SIZE] = { 0 };

    gCommMgr.m_CommMap[index].unPack(recvBuf+2, unPackMsg);

    gCommMgr.m_CommMap[index].subCbk(unPackMsg);

}

void RegsCmdAgentSendFun(SendFun fun)
{
	if (fun != NULL)
	{
		gCommMgr.m_sendFun = fun;
	}
}
