
#pragma once
#include "CStore.h"
#include "LFFileImportList.h"


// CStoreWindows
//

class CStoreWindows : public CStore
{
public:
	CStoreWindows(LFStoreDescriptor* pStoreDescriptor, HMUTEX hMutexForStore);

	// Index operations
	virtual UINT Synchronize(BOOL OnInitialize=FALSE, LFProgress* pProgress=NULL);

	// Callbacks
	virtual UINT PrepareImport(LFItemDescriptor* pItemDescriptor, WCHAR* pPath, SIZE_T cCount);

protected:
	// Callbacks
	virtual UINT GetFileLocation(LFCoreAttributes* pCoreAttributes, void* pStoreData, WCHAR* pPath, SIZE_T cCount) const;
	virtual UINT RenameFile(LFCoreAttributes* pCoreAttributes, void* pStoreData, LFItemDescriptor* pItemDescriptor);
	virtual UINT DeleteFile(LFCoreAttributes* pCoreAttributes, void* pStoreData);
	virtual BOOL SynchronizeFile(LFCoreAttributes* pCoreAttributes, void* pStoreData, LFProgress* pProgress=NULL);

private:
	LFFileImportList* m_pFileImportList;
};
