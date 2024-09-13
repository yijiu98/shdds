#include "ProtocolPackUnpack.h"
#include "ProtocolFmt.h"
S32 PackHead(U8* const pBuf, const CommCmd* const pCmd, U8* const pPackLen)
{
	if ((pBuf == NULL) || (pPackLen == NULL) || (pCmd == NULL))
	{
		return -1;
	}
	pBuf[PAR_LEN] = 2;//长度和命令位共2位
	pBuf[CMD_TYPE] = *pCmd & 0xFF;
	*pPackLen = 0;

	return 0;
}

S32 UnPackHead(const U8* const pBuf, CommCmd* const pCmd, U8* const pUnPackLen)
{
	if ((pBuf == NULL) || (pCmd == NULL) || (pUnPackLen == NULL))
	{
		return -1;
	}
	*pCmd = (CommCmd)pBuf[CMD_TYPE];

	return 0;
}