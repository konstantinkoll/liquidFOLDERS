
#pragma once
#include "LFCore.h"
#include "IdxTables.h"


// CIndex
//

#define IndexOk                        0
#define IndexRepaired                  1
#define IndexError                     2
#define IndexReindexRequired           3

class CIndex
{
public:
	CIndex(char* _Path, char* _StoreID);
	~CIndex();

	bool Create();
	unsigned int Check(bool scheduled=false);
	void Reindex();
	void AddItem(LFItemDescriptor* i);
	void Update(LFItemDescriptor* i);
	void Update(LFTransactionList* li);
	void Remove(LFItemDescriptor* i);
	void Remove(LFTransactionList* li);
	void RemoveTrash();
	void Compact(bool ForceAllTables);
	void Retrieve(LFFilter* f, LFSearchResult* res);

protected:
	bool LoadTable(unsigned int ID, unsigned int* res=NULL);

private:
	CHeapfile* Tables[IdxTableCount];
	char Path[MAX_PATH];
	char StoreID[LFKeySize];
};
