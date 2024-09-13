#ifndef __PROTOCOLFMT_H_
#define __PROTOCOLFMT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "GlobalDataStu.h"

#define U8_BYTE (1)
#define U16_BYTE (2)
#define U32_BYTE (4)
#define F32_BYTE (4)
#define U64_BYTE (8)
#define BIT_8 (8)

extern U8  FmtU8(U8 src, U8** pdes);

extern U8  FmtU16(U16 src, U8** pdes);

extern U8  FmtU32(U32 src, U8** pdes);

extern U8  FmtF32(Float src, U8** pdes);

extern U8  FmtU64(U64 src, U8** pdes);

extern U8  UnFmtU8(const U8** pSrc);

extern U16 UnFmtU16(const U8** pSrc);

extern U32 UnFmtU32(const U8** pSrc);

extern F32 UnFmtF32(const U8** pSrc);

extern U64 UnFmtU64(const U8** pSrc);

extern D64 SavePrec(D64 para, U64 mult);

#ifdef __cplusplus
}
#endif

#endif