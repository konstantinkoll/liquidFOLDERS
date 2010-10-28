
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
	CIndex(char* _Path, char* _StoreID, char* _DatPath);
	~CIndex();

	bool Create();
	unsigned int Check(bool scheduled=false);
	void Reindex();
	void AddItem(LFItemDescriptor* i);
	void Update(LFItemDescriptor* i, bool IncludeSlaves=true);
	void Update(LFTransactionList* tl, LFVariantData* value1, LFVariantData* value2=NULL, LFVariantData* value3=NULL);
	void Delete(LFTransactionList* tl, char* DatPath=NULL);
	void Delete(LFFileIDList* il, bool PutInTrash=true, char* DatPath=NULL);
	unsigned int Rename(char* FileID, wchar_t* NewName, char* DatPath=NULL);
	void Retrieve(LFFilter* f, LFSearchResult* res);
	unsigned int RetrieveStats(unsigned int* cnt, __int64* size);

protected:
	bool LoadTable(unsigned int ID, unsigned int* res=NULL);
	bool DeleteFile(LFCoreAttributes* PtrM, char* DatPath);

private:
	CHeapfile* Tables[IdxTableCount];
	char Path[MAX_PATH];
	char StoreID[LFKeySize];
	char DatPath[MAX_PATH];
};
