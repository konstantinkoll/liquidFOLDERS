
#pragma once
#include "LFCore.h"
#include "IdxTables.h"


class LFCore_API CIndex
{
public:
	CIndex(char* _Path, char* _StoreID);
	~CIndex();

	void AddItem(LFItemDescriptor* i);
	void Update(LFItemDescriptor* i);
	void Update(LFTransactionList* li);
	void Remove(LFItemDescriptor* i);
	void Remove(LFTransactionList* li);
	void RemoveTrash();
	void Compact(bool ForceAllTables);
	void Retrieve(LFFilter* f, LFSearchResult* res);

protected:
	void LoadTable(unsigned int ID);

private:
	CHeapfile* Tables[IdxTableCount];
	char Path[MAX_PATH];
	char StoreID[LFKeySize];
};
