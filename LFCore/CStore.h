
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
	UINT GetFileLocation(LFItemDescriptor* pItemDescriptor, WCHAR* pPath, SIZE_T cCount) const;

	// Index operations
	virtual UINT Synchronize(BOOL OnInitialize=FALSE, LFProgress* pProgress=NULL);
	virtual UINT ImportFile(WCHAR* pPath, LFItemDescriptor* pItemTemplate, BOOL Move=FALSE, BOOL Metadata=TRUE);

	BOOL UpdateMissingFlag(LFItemDescriptor* pItemDescriptor, BOOL Exists, BOOL RemoveNew);
	UINT CommitImport(LFItemDescriptor* pItemDescriptor, BOOL Commit=TRUE, WCHAR* pPath=NULL, BOOL OnInitialize=FALSE);
	void Query(LFFilter* pFilter, LFSearchResult* pSearchResult);
	void DoTransaction(LFTransactionList* pTransactionList, UINT TransactionType, LFProgress* pProgress=NULL, UINT_PTR Parameter=0, LFVariantData* pVariantData1=NULL, LFVariantData* pVariantData2=NULL, LFVariantData* pVariantData3=NULL);

	// Callbacks
	virtual UINT CreateDirectories();
	virtual UINT PrepareImport(LFItemDescriptor* pItemDescriptor, WCHAR* pPath, SIZE_T cCount);

	LFStoreDescriptor* p_StoreDescriptor;

protected:
	// Callbacks
	virtual UINT DeleteDirectories();
	virtual UINT GetFileLocation(LFCoreAttributes* pCoreAttributes, void* pStoreData, WCHAR* pPath, SIZE_T cCount) const;
	virtual UINT RenameFile(LFCoreAttributes* pCoreAttributes, void* pStoreData, LFItemDescriptor* pItemDescriptor);
	virtual UINT DeleteFile(LFCoreAttributes* pCoreAttributes, void* pStoreData);
	virtual BOOL SynchronizeFile(LFCoreAttributes* pCoreAttributes, void* pStoreData);

	// Aux functions
	void GetInternalFilePath(LFCoreAttributes* pCoreAttributes, WCHAR* pPath, SIZE_T cCount) const;
	void CreateNewFileID(CHAR* pFileID) const;

	UINT m_AdditionalDataSize;
	CIndex* m_pIndexMain;
	CIndex* m_pIndexAux;

private:
	HMUTEX hMutex;
	BOOL m_WriteAccess;
};
