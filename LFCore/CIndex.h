
#pragma once
#include "LFCore.h"
#include "CHeapfile.h"
#include "IndexTables.h"


// CIndex
//

#define IndexMaintenanceSteps     (IDXTABLECOUNT*2+1)

class CStore;

class CIndex
{
public:
	CIndex(CStore* pStore, BOOL IsMainIndex, UINT StoreDataSize);
	~CIndex();

	// Index management
	BOOL Initialize();
	UINT MaintenanceAndStatistics(BOOL Scheduled, BOOL* pRepaired, LFProgress* pProgress=NULL);

	// Operations on index only
	UINT Add(LFItemDescriptor* pItemDescriptor);
	void Query(LFFilter* pFilter, LFSearchResult* pSearchResult);
	void AddToSearchResult(LFTransactionList* pTransactionList, LFSearchResult* pSearchResult);
	void ResolveLocations(LFTransactionList* pTransactionList);
	void SendTo(LFTransactionList* pTransactionList, CHAR* pStoreID, LFProgress* pProgress=NULL);
	BOOL UpdateMissingFlag(LFItemDescriptor* pItemDescriptor, BOOL Exists, BOOL RemoveNew);
	void UpdateItemState(LFTransactionList* pTransactionList, UINT Flags, FILETIME* pTransactionTime);

	// Operations with callbacks to CStore object
	void Update(LFTransactionList* pTransactionList, LFVariantData* pVariantData1, LFVariantData* pVariantData2=NULL, LFVariantData* pVariantData3=NULL);
	void Delete(LFTransactionList* pTransactionList, LFProgress* pProgress=NULL);

protected:
	BOOL LoadTable(UINT TableID, UINT* pResult=NULL);
	void AddFileToStatistics(LFCoreAttributes* PtrM);
	void RemoveFileFromStatistics(LFCoreAttributes* PtrM);

	CStore* p_Store;
	LFStoreDescriptor* p_StoreDescriptor;
	BOOL m_IsMainIndex;
	UINT m_AdditionalDataSize;
	CHeapfile* m_pTable[IDXTABLECOUNT];

private:
	WCHAR m_IdxPath[MAX_PATH];
};
