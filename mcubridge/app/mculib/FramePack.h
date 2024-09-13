#ifndef __FRAME_PACK_H_
#define __FRAME_PACK_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "GlobalDataStu.h"
#include "stdio.h"

extern S32 FramePack(U8* pSrc, U8* pLen);
extern S32 FrameUnPack(U8* pSrc, U8* pLen);

#ifdef __cplusplus
}
#endif

#endif