
#pragma once
#include "CStore.h"


// CStoreInternal
//

class CStoreInternal : public CStore
{
public:
	CStoreInternal(LFStoreDescriptor* pStoreDescriptor, HMUTEX hMutexForStore);

	// Non-Index operations
	virtual UINT GetFilePath(const REVENANTFILE& File, LPWSTR pPath, SIZE_T cCount) const;
	virtual UINT PrepareDelete();

	// Callbacks
	virtual UINT CreateDirectories();
	virtual UINT PrepareImport(LPCWSTR pFilename, LPCSTR pExtension, LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount);

protected:
	// Callbacks
	virtual UINT DeleteDirectories();
	virtual UINT DeleteFile(const REVENANTFILE& File);

private:
	void GetInternalFilePath(const LFCoreAttributes& CoreAttributes, LPWSTR pPath, SIZE_T cCount) const;
};
