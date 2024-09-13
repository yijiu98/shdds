#ifndef __PROTOCOL_PACK_UNPACK_H_
#define __PROTOCOL_PACK_UNPACK_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "GlobalDataStu.h"
#include "CommMgr.h"
#include "stdio.h"


enum ProtocolFrame
{
	PAR_LEN = 0,
	CMD_TYPE,
	FIRST_DATA,
};

extern S32 PackHead(U8* const pBuf, const CommCmd* const pCmd, U8* const pPackLen);
extern S32 UnPackHead(const U8* const pBuf, CommCmd* const pCmd, U8* const pUnPackLen);

#ifdef __cplusplus
}
#endif


#endif