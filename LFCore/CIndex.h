
#pragma once
#include "LFCore.h"
#include "IdxTables.h"


// CIndex
//

#define IndexOk                        0
#define IndexPartiallyRepaired         1
#define IndexFullyRepaired             2
#define IndexError                     3
#define IndexReindexRequired           4

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
	void Retrieve(LFFilter* f, LFSearchResult* res);
	void RetrieveDomains(unsigned int* cnt);

protected:
	bool LoadTable(unsigned int ID, unsigned int* res=NULL);

private:
	CHeapfile* Tables[IdxTableCount];
	char Path[MAX_PATH];
	char StoreID[LFKeySize];
};
