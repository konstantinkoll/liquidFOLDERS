
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
	CStore(LFStoreDescriptor* pStoreDescriptor, HMUTEX hMutexForStore, UINT StoreDataSize=0);
	virtual ~CStore();

	UINT Open(BOOL WriteAccess);
	void Close();

	// Non-Index operations
	virtual UINT Initialize(LFProgress* pProgress=NULL);
	virtual UINT PrepareDelete();
	virtual UINT CommitDelete();

	UINT MaintenanceAndStatistics(BOOL Scheduled=FALSE, LFProgress* pProgress=NULL);
	void ScheduledMaintenance(LFMaintenanceList* pMaintenanceList, LFProgress* pProgress=NULL);
	UINT GetFileLocation(LFItemDescriptor* pItemDescriptor, WCHAR* pPath, SIZE_T cCount);

	// Index operations
	virtual UINT Synchronize(LFProgress* pProgress=NULL);
	virtual UINT ImportFile(WCHAR* pPath, LFItemDescriptor* pItemTemplate, BOOL Move=FALSE, BOOL Metadata=TRUE);

	BOOL UpdateMissingFlag(LFItemDescriptor* pItemDescriptor, BOOL Exists, BOOL RemoveNew);
	UINT PrepareImport(LFItemDescriptor* pItemDescriptor, WCHAR* pPath, SIZE_T cCount);
	UINT CommitImport(LFItemDescriptor* pItemDescriptor, BOOL Commit=TRUE, WCHAR* pPath=NULL);
	void Query(LFFilter* pFilter, LFSearchResult* pSearchResult);
	void DoTransaction(LFTransactionList* pTransactionList, UINT TransactionType, LFProgress* pProgress=NULL, UINT_PTR Parameter=0, LFVariantData* pVariantData1=NULL, LFVariantData* pVariantData2=NULL, LFVariantData* pVariantData3=NULL);

	// Callbacks
	virtual UINT CreateDirectories();

	LFStoreDescriptor* p_StoreDescriptor;

protected:
	// Callbacks
	virtual UINT DeleteDirectories();
	virtual UINT GetFileLocation(LFCoreAttributes* pCoreAttributes, void* pStoreData, WCHAR* pPath, SIZE_T cCount);
	virtual UINT RenameFile(LFCoreAttributes* pCoreAttributes, void* pStoreData, WCHAR* pNewName);
	virtual UINT DeleteFile(LFCoreAttributes* pCoreAttributes, void* pStoreData);

	// Aux functions
	void GetInternalFilePath(LFCoreAttributes* pCoreAttributes, WCHAR* pPath, SIZE_T cCount);

	UINT m_AdditionalDataSize;

private:
	HMUTEX hMutex;
	CIndex* m_pIndexMain;
	CIndex* m_pIndexAux;
	BOOL m_WriteAccess;
};
