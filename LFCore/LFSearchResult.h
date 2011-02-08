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
	void RemoveItemDescriptor(unsigned int idx, bool updatecount=true);
	void RemoveFlaggedItemDescriptors(bool updatecount=true);
	void KeepRange(int first, int last);
	void Sort(unsigned int attr, bool descending);
	void Group(unsigned int attr,unsigned int icon, bool groupone, LFFilter* f);
	void GroupArray(unsigned int attr, unsigned int icon, LFFilter* f);
	void SetContext(LFFilter* f);

	wchar_t m_Name[256];
	bool m_RawCopy;
	int m_Context;
	unsigned int m_GroupAttribute;
	bool m_HasCategories;
	DWORD m_QueryTime;
	unsigned int m_FileCount;
	__int64 m_FileSize;
	unsigned int m_StoreCount;
	char m_StoreID[LFKeySize];

private:
	int Compare(int eins, int zwei, unsigned int attr, bool descending);
	void Heap(int wurzel, int anz, unsigned int attr, bool descending);
	unsigned int Aggregate(unsigned int write, unsigned int read1, unsigned int read2, void* c,
		unsigned int attr, unsigned int icon, bool groupone, LFFilter* f);
};
