
#pragma once
#include "LFCore.h"
#include "CIndex.h"
#include "LFMaintenanceList.h"
#include "Mutex.h"


// CStore
//

class CStore
{
friend class CIndex;

public:
	CStore(LFStoreDescriptor* pStoreDescriptor, HMUTEX hMutexForStore, UINT AdditionalDataSize=0);
	virtual ~CStore();

	UINT Open(BOOL WriteAccess);
	void Close();

	// Non-Index operations
	virtual UINT Initialize(LFProgress* pProgress=NULL);
	virtual UINT PrepareDelete();
	virtual UINT CommitDelete();

	UINT MaintenanceAndStatistics(BOOL Scheduled=FALSE, LFProgress* pProgress=NULL);
	void ScheduledMaintenance(LFMaintenanceList* pMaintenanceList, LFProgress* pProgress=NULL);
	UINT UpdateStatistics();
	UINT GetFileLocation(LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount) const;

	// Index operations
	virtual UINT Synchronize(BOOL OnInitialize=FALSE, LFProgress* pProgress=NULL);
	virtual UINT ImportFile(LPCWSTR pPath, LFItemDescriptor* pItemTemplate, BOOL Move=FALSE, BOOL RetrieveMetadata=TRUE);

	BOOL UpdateMissingFlag(LFItemDescriptor* pItemDescriptor, BOOL Exists, BOOL RemoveNew);
	UINT CommitImport(LFItemDescriptor* pItemDescriptor, BOOL Commit=TRUE, LPCWSTR pPath=NULL, BOOL OnInitialize=FALSE);
	void Query(LFFilter* pFilter, LFSearchResult* pSearchResult);
	void DoTransaction(LFTransactionList* pTransactionList, UINT TransactionType, LFProgress* pProgress=NULL, UINT_PTR Parameter=0, LFVariantData* pVariantData1=NULL, LFVariantData* pVariantData2=NULL, LFVariantData* pVariantData3=NULL);

	// Callbacks
	virtual UINT CreateDirectories();
	virtual UINT PrepareImport(LPCWSTR pFilename, LPCSTR pExtension, LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount);

	LFStoreDescriptor* p_StoreDescriptor;

protected:
	// Callbacks
	virtual UINT DeleteDirectories();
	virtual UINT GetFileLocation(LFCoreAttributes* pCoreAttributes, LPCVOID pStoreData, LPWSTR pPath, SIZE_T cCount) const;
	virtual UINT RenameFile(LFCoreAttributes* pCoreAttributes, LPVOID pStoreData, LFItemDescriptor* pItemDescriptor);
	virtual UINT DeleteFile(LFCoreAttributes* pCoreAttributes, LPCVOID pStoreData);
	virtual void SetAttributesFromStore(LFItemDescriptor* pItemDescriptor);
	virtual BOOL SynchronizeFile(LFCoreAttributes* pCoreAttributes, LPCVOID pStoreData);

	// Aux functions
	UINT PrepareImport(LPCWSTR pSourcePath, LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount);
	void GetInternalFilePath(LFCoreAttributes* pCoreAttributes, LPWSTR pPath, SIZE_T cCount) const;
	void CreateNewFileID(LPSTR pFileID) const;

	UINT m_AdditionalDataSize;
	CIndex* m_pIndexMain;
	CIndex* m_pIndexAux;

private:
	HMUTEX hMutex;
	BOOL m_WriteAccess;
};

inline void CStore::ScheduledMaintenance(LFMaintenanceList* pMaintenanceList, LFProgress* pProgress)
{
	pMaintenanceList->AddItem(p_StoreDescriptor->StoreName, p_StoreDescriptor->Comments, p_StoreDescriptor->StoreID, MaintenanceAndStatistics(TRUE, pProgress), LFGetStoreIcon(p_StoreDescriptor));
}

inline UINT CStore::UpdateStatistics()
{
	assert(m_pIndexMain);

	return m_pIndexMain->UpdateStatistics();
}

inline UINT CStore::GetFileLocation(LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount) const
{
	assert(pItemDescriptor);
	assert(pPath);

	return GetFileLocation(&pItemDescriptor->CoreAttributes, &pItemDescriptor->StoreData, pPath, cCount);
}
