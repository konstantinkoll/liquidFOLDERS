
#pragma once
#include "LFCore.h"
#include "IdxTables.h"


// CIndex
//

#define IndexOk                        0
#define IndexCancel                    1
#define IndexRepaired                  2
#define IndexNoAccess                  3
#define IndexError                     4
#define IndexCannotCreate              5
#define IndexCompleteReindexRequired   6
#define IndexNotEnoughFreeDiscSpace    7

#define IndexMaintenanceSteps          (IdxTableCount*2+1)

class CIndex
{
public:
	CIndex(wchar_t* _Path, char* _StoreID, wchar_t* _DatPath);
	~CIndex();

	bool Create();
	unsigned int Check(bool scheduled=false, LFProgress* pProgress=NULL);
	void Reindex();
	unsigned int AddItem(LFItemDescriptor* i);
	void Update(LFItemDescriptor* i, bool IncludeSlaves=true);
	bool UpdateMissing(LFItemDescriptor* i, bool Exists);
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
	unsigned int RenamePhysicalFile(LFCoreAttributes* PtrM, wchar_t* NewName);
	unsigned int DeletePhysicalFile(LFCoreAttributes* PtrM);

private:
	CHeapfile* Tables[IdxTableCount];
	wchar_t Path[MAX_PATH];
	char StoreID[LFKeySize];
	wchar_t DatPath[MAX_PATH];
};
