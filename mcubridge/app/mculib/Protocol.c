#include "GlobalDataStu.h"
#include "Protocol.h"


#define DATA_LEN 0
#define DATA_DATA 2


S32 PackBatComm(U8* const pBuf, const void* const pMsg, U8* const pPackLen)
{
	if ((pBuf == NULL) || (pPackLen == NULL))
	{
		return -1;
	}
	Battery* pBat = (Battery*)(pMsg);
	U8* pParLen = pBuf + DATA_LEN;
	U8* pFmt = pBuf + DATA_DATA;

	*pParLen += FmtU8(pBat->soc, &pFmt);
	*pParLen += FmtU8(pBat->soh, &pFmt);
	// *pParLen += FmtU32(pBat->remainCap, &pFmt);
	*pPackLen += *pParLen;

	return 0;
}

S32 UnPackBatComm(const U8* const pBuf, void* const pMsg)
{
	if ((pMsg == NULL) || (pBuf == NULL))
	{
		return -1;
	}
	Battery* pBat = (Battery*)(pMsg);

	const U8* pFmt = pBuf + DATA_DATA;

	pBat->soc = UnFmtU8(&pFmt);

	pBat->soh = UnFmtU8(&pFmt);

	// pBat->remainCap = UnFmtU32(&pFmt);

	return 0;
}

