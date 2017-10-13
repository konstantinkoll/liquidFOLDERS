
#pragma once
#include "LFCore.h"
#include "CHeapfile.h"
#include "TableIndexes.h"


// CIndex
//

#define IndexMaintenanceSteps     (IDXTABLECOUNT*2+1)

class CStore;

class CIndex
{
public:
	CIndex(CStore* pStore, BOOL IsMainIndex, BOOL WriteAccess, UINT StoreDataSize);
	~CIndex();

	// Index management
	BOOL Initialize();
	UINT MaintenanceAndStatistics(BOOL Scheduled, BOOL* pRepaired, LFProgress* pProgress=NULL);

	// Operations on index only
	UINT Add(LFItemDescriptor* pItemDescriptor);
	void Query(LFFilter* pFilter, LFSearchResult* pSearchResult, BOOL UpdateStatistics=FALSE);
	UINT UpdateStatistics();
	void AddToSearchResult(LFTransactionList* pTransactionList, LFSearchResult* pSearchResult);
	void ResolveLocations(LFTransactionList* pTransactionList);
	void SendTo(LFTransactionList* pTransactionList, LPCSTR pStoreID, LFProgress* pProgress=NULL);
	BOOL ExistingFileID(LPCSTR pFileID);
	BOOL UpdateMissingFlag(LFItemDescriptor* pItemDescriptor, BOOL Exists, BOOL RemoveNew);
	void UpdateItemState(LFTransactionList* pTransactionList, const FILETIME& TransactionTime, BYTE Flags);

	// Operations with callbacks to CStore object
	void Update(LFTransactionList* pTransactionList, LFVariantData* pVariantData1, LFVariantData* pVariantData2=NULL, LFVariantData* pVariantData3=NULL, BOOL MakeTask=FALSE);
	UINT Synchronize(LFProgress* pProgress=NULL);
	void Delete(LFTransactionList* pTransactionList, LFProgress* pProgress=NULL);

protected:
	BOOL LoadTable(UINT TableID, BOOL Initialize=FALSE, UINT* pResult=NULL);
	void AddFileToStatistics(LFCoreAttributes* PtrM) const;
	void RemoveFileFromStatistics(LFCoreAttributes* PtrM) const;

	CStore* p_Store;
	LFStoreDescriptor* p_StoreDescriptor;
	BOOL m_IsMainIndex;
	UINT m_AdditionalDataSize;
	BOOL m_WriteAccess;
	CHeapfile* m_pTable[IDXTABLECOUNT];

private:
	void ResetStatistics();
	void ResetStatistics(BOOL& DoReset);
	BOOL InspectForUpdate(LFVariantData* pVariantData, BOOL& IncludeSlaves, BOOL& DoRename);

	WCHAR m_IdxPath[MAX_PATH];
};

inline void CIndex::ResetStatistics()
{
	assert(p_StoreDescriptor);

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
