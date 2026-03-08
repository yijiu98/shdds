#ifndef __COMMMAP_H__
#define __COMMMAP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "CommMgr.h"
#include "Protocol.h"

#define		BATTERY						    "battery"

										//name ,		head					,pack,						unpack,						subcbk,	
#define TOP_BAT_MAP					{BATTERY,					PRO_BAT,				PackBatComm,				UnPackBatComm			,	NULL,}


#ifdef __cplusplus
}
#endif
#endif