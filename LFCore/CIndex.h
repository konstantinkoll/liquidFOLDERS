
#pragma once
#include "LFCore.h"
#include "IdxTables.h"


// CIndex
//

#define IndexMaintenanceSteps     (IdxTableCount*2+1)

class CIndex
{
public:
	CIndex(LFStoreDescriptor* _slot, bool ForMainIndex);
	~CIndex();

	bool Create();
	unsigned int Check(bool Scheduled, bool* pRepaired, LFProgress* pProgress=NULL);
	unsigned int AddItem(LFItemDescriptor* i);
	bool UpdateSystemFlags(LFItemDescriptor* i, bool Exists, bool RemoveNew);
	void Update(LFTransactionList* tl, LFVariantData* value1, LFVariantData* value2=NULL, LFVariantData* value3=NULL);
	void Delete(LFTransactionList* tl, bool PutInTrash=true, LFProgress* pProgress=NULL);
	void Delete(LFFileIDList* il, bool PutInTrash=true, LFProgress* pProgress=NULL);
	void ResolvePhysicalLocations(LFTransactionList* tl);
	unsigned int Rename(char* FileID, wchar_t* NewName);
	void Retrieve(LFFilter* f, LFSearchResult* res);
	void AddToSearchResult(LFFileIDList* il, LFSearchResult* res);
	void TransferTo(CIndex* idxDst1, CIndex* idxDst2, LFStoreDescriptor* slotDst, LFFileIDList* il, LFStoreDescriptor* slotSrc, bool move, LFProgress* pProgress=NULL);

protected:
	bool LoadTable(unsigned int ID, unsigned int* res=NULL);
	void AddFileToStatistics(LFCoreAttributes* PtrM);
	void RemoveFileFromStatistics(LFCoreAttributes* PtrM);
	unsigned int RenamePhysicalFile(LFCoreAttributes* PtrM, wchar_t* NewName);
	unsigned int DeletePhysicalFile(LFCoreAttributes* PtrM);

private:
	CHeapfile* Tables[IdxTableCount];
	LFStoreDescriptor* slot;
	bool TrackStats;
	wchar_t IdxPath[MAX_PATH];
};
