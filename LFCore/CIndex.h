
#pragma once
#include "LFCore.h"
#include "IdxTables.h"


// CIndex
//

#define IndexOk                        0
#define IndexPartiallyRepaired         1
#define IndexFullyRepaired             2
#define IndexNoAccess                  3
#define IndexError                     4
#define IndexCannotCreate              5
#define IndexCompleteReindexRequired   6
#define IndexNotEnoughFreeDiscSpace    7

class CIndex
{
public:
	CIndex(wchar_t* _Path, char* _StoreID, wchar_t* _DatPath);
	~CIndex();

	bool Create();
	unsigned int Check(bool scheduled=false);
	void Reindex();
	void AddItem(LFItemDescriptor* i);
	void Update(LFItemDescriptor* i, bool IncludeSlaves=true);
	bool UpdateMissing(LFItemDescriptor* i, bool Exists);
	void Update(LFTransactionList* tl, LFVariantData* value1, LFVariantData* value2=NULL, LFVariantData* value3=NULL);
	void Delete(LFTransactionList* tl, wchar_t* DatPath=NULL);
	void Delete(LFFileIDList* il, bool PutInTrash=true, wchar_t* DatPath=NULL);
	unsigned int Rename(char* FileID, wchar_t* NewName, wchar_t* DatPath=NULL);
	void Retrieve(LFFilter* f, LFSearchResult* res);
	unsigned int RetrieveStats(unsigned int* cnt, __int64* size);

protected:
	bool LoadTable(unsigned int ID, unsigned int* res=NULL);
	unsigned int RenamePhysicalFile(LFCoreAttributes* PtrM, wchar_t* NewName, wchar_t* DatPath);
	unsigned int DeletePhysicalFile(LFCoreAttributes* PtrM, wchar_t* DatPath);

private:
	CHeapfile* Tables[IdxTableCount];
	wchar_t Path[MAX_PATH];
	char StoreID[LFKeySize];
	wchar_t DatPath[MAX_PATH];
};
