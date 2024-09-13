
#ifndef __PROTOCOL_H_
#define __PROTOCOL_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "stdio.h"
#include "ProtocolFmt.h"



S32 PackBatComm(U8* const pBuf, const void* const pMsg, U8* const pPackLen);
S32 UnPackBatComm(const U8* const pBuf, void* const pMsg);


#ifdef __cplusplus
}
#endif


#endif

