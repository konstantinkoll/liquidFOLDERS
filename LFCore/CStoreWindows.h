
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
	virtual UINT PrepareImport(LPCWSTR pFilename, LPCSTR pExtension, LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount);

protected:
	// Callbacks
	virtual UINT GetFileLocation(LFCoreAttributes* pCoreAttributes, LPCVOID pStoreData, LPWSTR pPath, SIZE_T cCount) const;
	virtual UINT RenameFile(LFCoreAttributes* pCoreAttributes, LPVOID pStoreData, LFItemDescriptor* pItemDescriptor);
	virtual UINT DeleteFile(LFCoreAttributes* pCoreAttributes, LPCVOID pStoreData);
	virtual void SetAttributesFromStore(LFItemDescriptor* pItemDescriptor);
	virtual BOOL SynchronizeFile(LFCoreAttributes* pCoreAttributes, LPCVOID pStoreData);

private:
	LFFileImportList* m_pFileImportList;
};
