
#pragma once
#include "CStore.h"
#include "LFFileImportList.h"


// CStoreWindows
//

class CStoreWindows : public CStore
{
public:
	CStoreWindows(LFStoreDescriptor* pStoreDescriptor, HMUTEX hMutexForStore);

	// Non-Index operations
	virtual UINT GetFilePath(const REVENANTFILE& File, LPWSTR pPath, SIZE_T cCount) const;

	// Index operations
	virtual UINT Synchronize(LFProgress* pProgress=NULL, BOOL OnInitialize=FALSE);

	// Callbacks
	virtual UINT PrepareImport(LPCWSTR pFilename, LPCSTR pExtension, LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount);

protected:
	// Callbacks
	virtual UINT RenameFile(const REVENANTFILE& File, LFItemDescriptor* pItemDescriptor);
	virtual UINT DeleteFile(const REVENANTFILE& File);
	virtual void SetAttributesFromStore(LFItemDescriptor* pItemDescriptor);
	virtual BOOL SynchronizeFile(const REVENANTFILE& File);

private:
	LFFileImportList* m_pFileImportList;
};
