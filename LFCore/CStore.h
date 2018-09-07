
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
	virtual UINT GetFilePath(const REVENANTFILE& File, LPWSTR pPath, SIZE_T cCount) const=0;

	UINT MaintenanceAndStatistics(LFProgress* pProgress=NULL, BOOL Scheduled=FALSE);
	void ScheduledMaintenance(LFMaintenanceList* pMaintenanceList, LFProgress* pProgress=NULL);
	void ResetStatistics() const;
	void ResetStatistics(BOOL& DoReset) const;
	void AddFileToStatistics(const LFCoreAttributes& CoreAttributes) const;
	void AddFileToStatistics(LPCCOREATTRIBUTES pCoreAttributes) const;
	void AddFileToStatistics(LFItemDescriptor* pItemDescriptor) const;
	void RemoveFileFromStatistics(const LFCoreAttributes& CoreAttributes) const;
	void RemoveFileFromStatistics(LPCCOREATTRIBUTES pCoreAttributes) const;
	void RemoveFileFromStatistics(LFItemDescriptor* pItemDescriptor) const;
	UINT UpdateStatistics() const;

	// Index operations
	virtual UINT Synchronize(LFProgress* pProgress=NULL, BOOL OnInitialize=FALSE);
	virtual UINT ImportFile(LPCWSTR pPath, LFItemDescriptor* pItemTemplate, BOOL Move=FALSE, BOOL RetrieveMetadata=TRUE);

	UINT GetFileLocation(LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount, BOOL RemoveNew=TRUE);
	UINT CommitImport(LFItemDescriptor* pItemDescriptor, BOOL Commit=TRUE, LPCWSTR pPath=NULL, BOOL OnInitialize=FALSE);
	void Query(LFFilter* pFilter, LFSearchResult* pSearchResult);
	void DoTransaction(LFTransactionList* pTransactionList, UINT TransactionType, LFProgress* pProgress=NULL, UINT_PTR Parameter=0, const LFVariantData* pVariantData1=NULL, const LFVariantData* pVariantData2=NULL, const LFVariantData* pVariantData3=NULL);

	// Callbacks
	virtual UINT CreateDirectories();
	virtual UINT PrepareImport(LPCWSTR pFilename, LPCSTR pExtension, LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount);

	LFStoreDescriptor* p_StoreDescriptor;

protected:
	// Callbacks
	virtual UINT DeleteDirectories();
	virtual UINT RenameFile(const REVENANTFILE& File, LFItemDescriptor* pItemDescriptor);
	virtual UINT DeleteFile(const REVENANTFILE& File)=0;
	virtual void SetAttributesFromStore(LFItemDescriptor* pItemDescriptor);
	virtual BOOL SynchronizeFile(const REVENANTFILE& File);

	// Aux functions
	void CreateNewFileID(FILEID& FileID) const;
	UINT PrepareImport(LPCWSTR pSourcePath, LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount);
	static UINT DeleteFile();

	UINT m_AdditionalDataSize;
	CIndex* m_pIndexMain;
	CIndex* m_pIndexAux;

private:
	void AddFileToStatistics(LFStatistics& Statistics, const LFCoreAttributes& CoreAttributes) const;
	void RemoveFileFromStatistics(LFStatistics& Statistics, const LFCoreAttributes& CoreAttributes) const;

	HMUTEX hMutex;
	BOOL m_WriteAccess;
};

inline void CStore::ScheduledMaintenance(LFMaintenanceList* pMaintenanceList, LFProgress* pProgress)
{
	pMaintenanceList->AddItem(p_StoreDescriptor, MaintenanceAndStatistics(pProgress, TRUE));
}

inline void CStore::ResetStatistics() const
{
	ZeroMemory(&p_StoreDescriptor->Statistics, sizeof(LFStatistics));
}

inline void CStore::ResetStatistics(BOOL& DoReset) const
{
	if (DoReset)
	{
		ResetStatistics();

		DoReset = FALSE;
	}
}

inline void CStore::AddFileToStatistics(const LFCoreAttributes& CoreAttributes) const
{
	AddFileToStatistics(p_StoreDescriptor->Statistics, CoreAttributes);
}

inline void CStore::AddFileToStatistics(LPCCOREATTRIBUTES pCoreAttributes) const
{
	assert(pCoreAttributes);

	AddFileToStatistics(p_StoreDescriptor->Statistics, *pCoreAttributes);
}

inline void CStore::AddFileToStatistics(LFItemDescriptor* pItemDescriptor) const
{
	assert(pItemDescriptor);

	AddFileToStatistics(p_StoreDescriptor->Statistics, pItemDescriptor->CoreAttributes);
}

inline void CStore::RemoveFileFromStatistics(const LFCoreAttributes& CoreAttributes) const
{
	RemoveFileFromStatistics(p_StoreDescriptor->Statistics, CoreAttributes);
}

inline void CStore::RemoveFileFromStatistics(LPCCOREATTRIBUTES pCoreAttributes) const
{
	assert(pCoreAttributes);

	RemoveFileFromStatistics(p_StoreDescriptor->Statistics, *pCoreAttributes);
}

inline void CStore::RemoveFileFromStatistics(LFItemDescriptor* pItemDescriptor) const
{
	assert(pItemDescriptor);

	RemoveFileFromStatistics(p_StoreDescriptor->Statistics, pItemDescriptor->CoreAttributes);
}

inline UINT CStore::UpdateStatistics() const
{
	assert(m_pIndexMain);

	return m_pIndexMain->UpdateStatistics();
}
