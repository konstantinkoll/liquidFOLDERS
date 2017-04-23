
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

	// Callbacks
	virtual UINT CreateDirectories();
	virtual UINT PrepareImport(LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount);

protected:
	virtual UINT DeleteDirectories();
};
