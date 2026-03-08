#include "FramePack.h"


#define FRAME_INCREASE (3)

S32 FramePack(U8* pSrc, U8* pLen)
{
	U8 Crc = 0;
	if ((pSrc == NULL) || (pLen == NULL))
	{
		return -1;
	}
	pSrc[0] = 0xA5;
	pSrc[1] = 0x5A;
	*pLen  += FRAME_INCREASE;
	pSrc[2] += 3;
	for (U8 i = 0; i < *pLen-1; i++)
	{
		Crc ^= *pSrc++;
	}
	*pSrc = Crc;
	return 0;
}

S32 FrameUnPack(U8* pSrc, U8* pLen)
{
	if ((pSrc == NULL) || (pLen == NULL))
	{
		return -1;
	}

	U8 crc = 0;
	if ((pSrc[2] != *pLen) || (*pLen < FRAME_INCREASE))
	{
		return -1;
	}
	for (U8 i = 0; i < *pLen - 1; i++)
	{
		crc ^= *pSrc++;
	}
	if (crc != *pSrc)
	{
		return -1;
	}


	return 0;
}


