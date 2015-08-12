
#pragma once
#include "LFCore.h"
#include "CHeapfile.h"
#include "IndexTables.h"


// CIndex
//

#define IndexMaintenanceSteps     (IDXTABLECOUNT*2+1)

class CIndex
{
public:
	CIndex(LFStoreDescriptor* _slot, BOOL ForMainIndex);
	~CIndex();

	BOOL Create();
	UINT Check(BOOL Scheduled, BOOL* pRepaired, LFProgress* pProgress=NULL);
	UINT AddItem(LFItemDescriptor* i);
	BOOL UpdateSystemFlags(LFItemDescriptor* i, BOOL Exists, BOOL RemoveNew);
	void Update(LFTransactionList* tl, LFVariantData* v1, LFVariantData* v2=NULL, LFVariantData* v3=NULL);
	void Archive(LFTransactionList* tl);
	void Delete(LFTransactionList* tl, BOOL PutInTrash=TRUE, LFProgress* pProgress=NULL);
	void ResolvePhysicalLocations(LFTransactionList* tl);
	void Retrieve(LFFilter* f, LFSearchResult* Result);
	void AddToSearchResult(LFTransactionList* il, LFSearchResult* Result);
	void TransferTo(CIndex* idxDst1, CIndex* idxDst2, LFStoreDescriptor* slotDst, LFTransactionList* il, LFStoreDescriptor* slotSrc, BOOL move, LFProgress* pProgress=NULL);

protected:
	BOOL LoadTable(UINT TableID, UINT* Result=NULL);
	void AddFileToStatistics(LFCoreAttributes* PtrM);
	void RemoveFileFromStatistics(LFCoreAttributes* PtrM);
	UINT RenamePhysicalFile(LFCoreAttributes* PtrM, WCHAR* NewName);
	UINT DeletePhysicalFile(LFCoreAttributes* PtrM);

private:
	CHeapfile* Tables[IDXTABLECOUNT];
	LFStoreDescriptor* slot;
	BOOL TrackStats;
	WCHAR IdxPath[MAX_PATH];
};
