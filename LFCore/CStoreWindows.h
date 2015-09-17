
#pragma once
#include "CStore.h"


// CStoreWindows
//

class CStoreWindows : public CStore
{
public:
	CStoreWindows(LFStoreDescriptor* pStoreDescriptor, HMUTEX hMutexForStore);
};
