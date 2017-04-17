
#pragma once
#include "LF.h"
#include "LFDynArray.h"


class LFSearchResult : public LFDynArray<LFItemDescriptor*, 64, 2048>
{
public:
	LFSearchResult(BYTE Context=LFContextAllFiles);
	LFSearchResult(LFSearchResult* pSearchResult);
	~LFSearchResult();

	void FinishQuery(LFFilter* pFilter);
	BOOL AddItem(LFItemDescriptor* pItemDescriptor);
	BOOL AddStoreDescriptor(LFStoreDescriptor* pStoreDescriptor);
	void RemoveItem(UINT Index, BOOL UpdateCount=TRUE);
	void RemoveFlaggedItems(BOOL UpdateCount=TRUE);
	void KeepRange(INT First, INT Last);
	void Sort(UINT Attr, BOOL Descending);
	void Group(UINT Attr, BOOL GroupSingle, LFFilter* pFilter);
	void GroupArray(UINT Attr, LFFilter* pFilter);

	UINT m_LastError;

	WCHAR m_Name[256];
	WCHAR m_Hint[256];
	DWORD m_QueryTime;
	BYTE m_Context;
	UINT m_IconID;

	BOOL m_RawCopy;
	BOOL m_HasCategories;

	UINT m_StoreCount;
	LFFileSummary m_FileSummary;

protected:
	BYTE m_AutoContext;

private:
	static void InitFileSummary(LFFileSummary& FileSummary);
	void AddFileToSummary(LFFileSummary& FileSummary, LFItemDescriptor* pItemDescriptor);
	void RemoveFileFromSummary(LFFileSummary& FileSummary, LFItemDescriptor* pItemDescriptor);
	INT Compare(LFItemDescriptor* pItem1, LFItemDescriptor* pItem2, UINT Attr, BOOL Descending) const;
	void Heap(UINT Wurzel, const UINT Anz, const UINT Attr, const BOOL Descending);
	UINT Aggregate(UINT WriteIndex, UINT ReadIndex1, UINT ReadIndex2, void* pCategorizer, UINT Attr, BOOL GroupSingle, LFFilter* pFilter);
};

inline void LFSearchResult::InitFileSummary(LFFileSummary& FileSummary)
{
	ZeroMemory(&FileSummary, sizeof(FileSummary));

	FileSummary.OnlyMediaFiles = TRUE;
}
