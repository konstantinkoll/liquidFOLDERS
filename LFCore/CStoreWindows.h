
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
	virtual UINT Synchronize(LFProgress* pProgress=NULL, BOOL OnInitialize=FALSE);

	// Callbacks
	virtual UINT PrepareImport(LPCWSTR pFilename, LPCSTR pExtension, LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount);

protected:
	// Callbacks
	virtual UINT GetFileLocation(const LFCoreAttributes& CoreAttributes, LPCVOID pStoreData, LPWSTR pPath, SIZE_T cCount) const;
	virtual UINT RenameFile(const LFCoreAttributes& CoreAttributes, LPVOID pStoreData, LFItemDescriptor* pItemDescriptor);
	virtual UINT DeleteFile(const LFCoreAttributes& CoreAttributes, LPCVOID pStoreData);
	virtual void SetAttributesFromStore(LFItemDescriptor* pItemDescriptor);
	virtual BOOL SynchronizeFile(LFCoreAttributes& CoreAttributes, LPCVOID pStoreData);

private:
	LFFileImportList* m_pFileImportList;
};
