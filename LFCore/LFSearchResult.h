#pragma once
#include "liquidFOLDERS.h"
#include "DynArray.h"


class LFSearchResult : public DynArray<LFItemDescriptor*>
{
public:
	LFSearchResult(int ctx);
	LFSearchResult(LFFilter* f);
	LFSearchResult(LFSearchResult* res);
	~LFSearchResult();

	void SetMetadataFromFilter(LFFilter* f);
	bool AddItemDescriptor(LFItemDescriptor* i);
	bool AddStoreDescriptor(LFStoreDescriptor* s);
	void AddVolumes();
	void RemoveItemDescriptor(unsigned int idx, bool updatecount=true);
	void RemoveFlaggedItemDescriptors(bool updatecount=true);
	void KeepRange(int first, int last);
	void Sort(unsigned int attr, bool descending);
	void Group(unsigned int attr, bool groupone, LFFilter* f);
	void GroupArray(unsigned int attr, LFFilter* f);

	wchar_t m_Name[256];
	wchar_t m_Hint[256];
	bool m_RawCopy;
	unsigned int m_Context;
	unsigned int m_GroupAttribute;
	bool m_HasCategories;
	DWORD m_QueryTime;
	unsigned int m_FileCount;
	__int64 m_FileSize;
	unsigned int m_StoreCount;

private:
	int Compare(LFItemDescriptor* d1, LFItemDescriptor* d2, unsigned int attr, bool descending);
	void Heap(unsigned int wurzel, const unsigned int anz, const unsigned int attr, const bool descending);
	unsigned int Aggregate(unsigned int write, unsigned int read1, unsigned int read2, void* c,
		unsigned int attr, bool groupone, LFFilter* f);
};
