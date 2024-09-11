
#include "DDS.h"
#include "CommMgr.h"

DDS gCommIns = { Publish, Subscribe };

DDS* getInterface(void)
{
	return &gCommIns;
}



