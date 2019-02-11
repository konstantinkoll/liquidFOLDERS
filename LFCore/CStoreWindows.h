
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
	virtual UINT GetFilePath(const HORCRUXFILE& File, LPWSTR pPath, SIZE_T cCount) const;

	// Index operations
	virtual UINT Synchronize(LFProgress* pProgress=NULL, BOOL OnInitialize=FALSE);

	// Callbacks
	virtual UINT PrepareImport(LPCWSTR pFilename, LPCSTR pExtension, LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount);

protected:
	// Callbacks
	virtual UINT RenameFile(const HORCRUXFILE& File, LFItemDescriptor* pItemDescriptor);
	virtual UINT DeleteFile(const HORCRUXFILE& File);
	virtual void SetAttributesFromStore(LFItemDescriptor* pItemDescriptor);
	virtual void SynchronizeMatch(const HORCRUXFILE& File);
	virtual BOOL SynchronizeCommit(const HORCRUXFILE& File);

	SIZE_T m_szDatPath;

private:
	LPCWSTR ExtractFileName(LPCWSTR pPath);
	LFFileImportItem* FindSimilarFile(const HORCRUXFILE& File);

	LFFileImportList* m_pFileImportList;
};

inline LPCWSTR CStoreWindows::ExtractFileName(LPCWSTR pData)
{
	assert(pData);

	LPCWSTR pStr = wcsrchr(pData, L'\\');

	return pStr ? pStr+1 : pData;
}