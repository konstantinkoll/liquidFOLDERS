
#pragma once
#include "CStore.h"


// CStoreInternal
//

class CStoreInternal : public CStore
{
public:
	CStoreInternal(LFStoreDescriptor* pStoreDescriptor, HMUTEX hMutexForStore);

	// Non-Index operations
	virtual UINT PrepareDelete();

protected:
	// Callbacks
	virtual UINT CreateDirectories();
	virtual UINT DeleteDirectories();
};
