
#pragma once
#include "LF.h"
#include "LFDynArray.h"


class LFSearchResult : public LFDynArray<LFItemDescriptor*>
{
public:
	LFSearchResult(UINT Context);
	LFSearchResult(LFFilter* pFilter);
	LFSearchResult(LFSearchResult* pSearchResult);
	~LFSearchResult();

	void SetMetadataFromFilter(LFFilter* pFilter);
	BOOL AddItem(LFItemDescriptor* i);
	BOOL AddStoreDescriptor(LFStoreDescriptor* pStoreDescriptor);
	void RemoveItem(UINT Index, BOOL UpdateCount=TRUE);
	void RemoveFlaggedItems(BOOL UpdateCount=TRUE);
	void KeepRange(INT First, INT Last);
	void Sort(UINT Attr, BOOL Descending);
	void Group(UINT Attr, BOOL GroupOne, LFFilter* pFilter);
	void GroupArray(UINT Attr, LFFilter* pFilter);

	WCHAR m_Name[256];
	WCHAR m_Hint[256];
	DWORD m_QueryTime;
	UINT m_Context;
	UINT m_GroupAttribute;

	BOOL m_RawCopy;
	BOOL m_HasCategories;

	UINT m_StoreCount;
	UINT m_FileCount;
	INT64 m_FileSize;

private:
	INT Compare(LFItemDescriptor* i1, LFItemDescriptor* i2, UINT Attr, BOOL Descending);
	void Heap(UINT Wurzel, const UINT Anz, const UINT Attr, const BOOL Descending);
	UINT Aggregate(UINT WriteIndex, UINT ReadIndex1, UINT ReadIndex2, void* pCategorizer, UINT Attr, BOOL GroupOne, LFFilter* pFilter);
};
