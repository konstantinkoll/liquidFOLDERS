#pragma once
#include "liquidFOLDERS.h"
#include "DynArray.h"


class LFSearchResult : public DynArray<LFItemDescriptor*>
{
public:
	LFSearchResult(int ctx);
	LFSearchResult(LFSearchResult* res);
	~LFSearchResult();

	bool AddItemDescriptor(LFItemDescriptor* i);
	bool AddStoreDescriptor(LFStoreDescriptor* s, LFFilter* f);
	void AddDrives(LFFilter* filter);
	void AddBacklink(char* StoreID, LFFilter* f);
	void RemoveItemDescriptor(unsigned int idx, bool updatecount=true);
	void RemoveFlaggedItemDescriptors(bool updatecount=true);
	void KeepRange(int first, int last);
	void Sort(unsigned int attr, bool descending, bool categories);
	void Group(unsigned int attr,unsigned int icon, bool groupone, LFFilter* f);
	void GroupArray(unsigned int attr, unsigned int icon, LFFilter* f);
	void SetContext(LFFilter* f);

	bool m_HasCategories;
	bool m_HidingItems;
	DWORD m_QueryTime;
	unsigned int m_LastError;
	unsigned int m_FileCount;
	unsigned int m_StoreCount;
	__int64 m_FileSize;
	int m_Context;
	int m_ContextView;
	char m_StoreID[LFKeySize];
	bool m_RawCopy;

private:
	int Compare(int eins, int zwei, unsigned int attr, bool descending, bool categories);
	void Heap(int wurzel, int anz, unsigned int attr, bool descending, bool categories);
	unsigned int Aggregate(unsigned int write, unsigned int read1, unsigned int read2, void* c,
		unsigned int attr, unsigned int icon, bool groupone, LFFilter* f);
};
