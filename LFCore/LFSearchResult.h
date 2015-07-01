#pragma once
#include "LF.h"
#include "LFDynArray.h"


class LFSearchResult : public LFDynArray<LFItemDescriptor*>
{
public:
	LFSearchResult(INT ctx);
	LFSearchResult(LFFilter* f);
	LFSearchResult(LFSearchResult* Result);
	~LFSearchResult();

	void SetMetadataFromFilter(LFFilter* f);
	BOOL AddItemDescriptor(LFItemDescriptor* i);
	BOOL AddStoreDescriptor(LFStoreDescriptor* s);
	void AddVolumes();
	void RemoveItemDescriptor(UINT idx, BOOL updatecount=TRUE);
	void RemoveFlaggedItemDescriptors(BOOL updatecount=TRUE);
	void KeepRange(INT first, INT last);
	void Sort(UINT Attr, BOOL descending);
	void Group(UINT Attr, BOOL groupone, LFFilter* f);
	void GroupArray(UINT Attr, LFFilter* f);

	WCHAR m_Name[256];
	WCHAR m_Hint[256];
	BOOL m_RawCopy;
	UINT m_Context;
	UINT m_GroupAttribute;
	BOOL m_HasCategories;
	DWORD m_QueryTime;
	UINT m_FileCount;
	INT64 m_FileSize;
	UINT m_StoreCount;

private:
	INT Compare(LFItemDescriptor* d1, LFItemDescriptor* d2, UINT Attr, BOOL descending);
	void Heap(UINT wurzel, const UINT anz, const UINT Attr, const BOOL descending);
	UINT Aggregate(UINT write, UINT read1, UINT read2, void* c, UINT Attr, BOOL groupone, LFFilter* f);
};
