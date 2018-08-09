
#pragma once
#include "LFCore.h"
#include "CHeapfile.h"
#include "TableIndexes.h"


// CIndex
//

#define IndexMaintenanceSteps     (IDXTABLECOUNT*2+1)

class CStore;

class CIndex sealed
{
public:
	CIndex(CStore* pStore, BOOL IsMainIndex, BOOL WriteAccess, UINT StoreDataSize);
	~CIndex();

	// Index management
	BOOL Initialize();
	UINT MaintenanceAndStatistics(BOOL Scheduled, BOOL& Repaired, LFProgress* pProgress=NULL);

	// Operations on index only
	UINT Add(LFItemDescriptor* pItemDescriptor);
	void Query(LFFilter* pFilter, LFSearchResult* pSearchResult, BOOL UpdateStatistics=FALSE);
	UINT UpdateStatistics();
	void AddToSearchResult(LFTransactionList* pTransactionList, LFSearchResult* pSearchResult);
	void ResolveLocations(LFTransactionList* pTransactionList);
	void SendTo(LFTransactionList* pTransactionList, const STOREID& StoreID, LFProgress* pProgress=NULL);
	BOOL ExistingFileID(const FILEID& FileID);
	void UpdateUserContext(LFTransactionList* pTransactionList, BYTE UserContextID);
	BOOL UpdateMissingFlag(LFItemDescriptor* pItemDescriptor, BOOL Exists, BOOL RemoveNew);
	void UpdateItemState(LFTransactionList* pTransactionList, const FILETIME& TransactionTime, BYTE Flags);

	// Operations with callbacks to CStore object
	void Update(LFTransactionList* pTransactionList, const LFVariantData* pVariantData1, const LFVariantData* pVariantData2=NULL, const LFVariantData* pVariantData3=NULL, BOOL MakeTask=FALSE);
	UINT Synchronize(LFProgress* pProgress=NULL);
	void Delete(LFTransactionList* pTransactionList, LFProgress* pProgress=NULL);

protected:
	void SetError(LFTransactionList* pTransactionList, UINT Result, LFProgress* pProgress=NULL) const;
	BOOL LoadTable(UINT TableID, BOOL Initialize=FALSE, UINT* pResult=NULL);
	void AddFileToStatistics(LFCoreAttributes* PtrM) const;
	void RemoveFileFromStatistics(LFCoreAttributes* PtrM) const;

	CStore* p_Store;
	LFStoreDescriptor* p_StoreDescriptor;
	UINT m_StoreTypeFlags;
	BOOL m_IsMainIndex;
	BOOL m_WriteAccess;
	UINT m_AdditionalDataSize;
	CHeapfile* m_pTable[IDXTABLECOUNT];

private:
	void ResetStatistics();
	void ResetStatistics(BOOL& DoReset);
	BOOL InspectForUpdate(const LFVariantData* pVData, BOOL& IncludeSlaves, BOOL& DoRename);

	WCHAR m_IdxPath[MAX_PATH];
};

inline void CIndex::SetError(LFTransactionList* pTransactionList, UINT Result, LFProgress* pProgress) const
{
	assert(pTransactionList);

	pTransactionList->SetError(p_StoreDescriptor->StoreID, Result, pProgress);
}

inline void CIndex::ResetStatistics()
{
	ZeroMemory(&p_StoreDescriptor->Statistics, sizeof(LFStatistics));
}

inline void CIndex::ResetStatistics(BOOL& DoReset)
{
	if (DoReset)
	{
		ResetStatistics();

		DoReset = FALSE;
	}
}
