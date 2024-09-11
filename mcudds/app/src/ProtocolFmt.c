#include "ProtocolFmt.h"



U8 FmtU8(U8 src, U8** pdes)
{
	**pdes = src & 0xFF;
	(*pdes)++;

	return U8_BYTE;
}


U8  FmtU16(U16 src, U8** pdes)
{
	for (U8 i = 0; i < U16_BYTE; i++)
	{
		**pdes = (src >> (BIT_8 * i)) & 0xFF;
		(*pdes)++;
	}
	return U16_BYTE;
}


U8  FmtU32(U32 src, U8** pdes)
{
	for (U8 i = 0; i < U32_BYTE; i++)
	{
		**pdes = (src >> (BIT_8 * i)) & 0xFF;
		(*pdes)++;
	}

	return U32_BYTE;
}
U8  FmtF32(Float src, U8** pdes)
{
	for (U8 i = 0; i < F32_BYTE; i++)
	{
		**pdes = (src.byte[i]) & 0xFF;
		(*pdes)++;
	}

	return F32_BYTE;
}

U8  FmtU64(U64 src, U8** pdes)
{

	for (U8 i = 0; i < U64_BYTE; i++)
	{
		**pdes = (src >> (BIT_8 * i)) & 0xFF;
		(*pdes)++;
	}

	return U64_BYTE;
}


U8 UnFmtU8(const U8** pSrc)
{
	U8 ret = **pSrc & 0xFF;
	(*pSrc)++;

	return ret;
}

U16 UnFmtU16(const U8** pSrc)
{
	U16 ret = 0x00;

	for (U8 i = 0; i < U16_BYTE; i++)
	{
		ret |= ((U16) * *pSrc & 0xFF) << (i * BIT_8);
		(*pSrc)++;

	}

	return ret;
}


U32 UnFmtU32(const U8** pSrc)
{
	U32 ret = 0x00;

	for (U8 i = 0; i < U32_BYTE; i++)
	{
		ret |= ((U32) * *pSrc & 0xFF) << (i * BIT_8);
		(*pSrc)++;

	}

	return ret;
}

F32 UnFmtF32(const U8** pSrc)
{
	Float ret = {0};

	for (U8 i = 0; i < F32_BYTE; i++)
	{
		ret.byte[i]=(U32) * *pSrc & 0xFF;
		(*pSrc)++;
	}

	return ret.value;
}
U64 UnFmtU64(const U8** pSrc)
{
	U64 ret = 0x00;

	for (U8 i = 0; i < U64_BYTE; i++)
	{
		ret |= ((U64) * *pSrc & 0xFF) << (i * BIT_8);
		(*pSrc)++;
	}

	return ret;
}

D64  SavePrec(D64 para, U64 mult)
{
	return (para + 0.1 / mult) * mult;
}
