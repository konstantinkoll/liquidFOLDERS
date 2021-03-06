
#pragma once
#include "LF.h"
#include "LFDynArray.h"


#define REMOVEITEMS(KeepCondition, KeepOps) \
	UINT WriteIdx = 0; \
	for (UINT ReadIdx=0; ReadIdx<m_ItemCount; ReadIdx++) \
		if (KeepCondition) \
		{ \
			(m_Items[WriteIdx++]=m_Items[ReadIdx])KeepOps; \
		} \
		else \
		{ \
			assert(LFIsFile(m_Items[ReadIdx])); \
			LFFreeItemDescriptor(m_Items[ReadIdx]); \
		} \
	m_ItemCount = WriteIdx;

class LFSearchResult sealed : public LFDynArray<LFItemDescriptor*, 64, 2048>
{
public:
	LFSearchResult(ITEMCONTEXT Context=LFContextAllFiles);
	LFSearchResult(LFSearchResult* pSearchResult);
	~LFSearchResult();

	void FinishQuery(LFFilter* pFilter);
	BOOL AddItem(LFItemDescriptor* pItemDescriptor);
	BOOL AddStoreDescriptor(LFStoreDescriptor& StoreDescriptor);
	void RemoveFlaggedItems(BOOL UpdateSummary=TRUE);
	void KeepRange(UINT First, UINT Last);
	void SortItems(ATTRIBUTE Attr, BOOL Descending);
	void GroupItems(ATTRIBUTE Attr, BOOL GroupSingle, LFFilter* pFilter);
	void GroupArray(ATTRIBUTE Attr, LFFilter* pFilter);
	void UpdateFolderColors(const LFSearchResult* pRawFiles);

	UINT m_LastError;

	WCHAR m_Name[256];
	WCHAR m_Hint[256];
	DWORD m_QueryTime;
	ITEMCONTEXT m_Context;
	UINT m_IconID;

	BOOL m_RawCopy;
	BOOL m_HasCategories;

	LFFileSummary m_FileSummary;

protected:
	void UpdateFileSummary(BOOL Close=TRUE);

private:
	static void InitializeFileSummary(LFFileSummary& FileSummary);
	static void AddStoreToSummary(LFFileSummary& FileSummary, const LFStoreDescriptor& StoreDescriptor);
	static void AddFileToSummary(LFFileSummary& FileSummary, LFItemDescriptor* pItemDescriptor);
	static void CloseFileSummary(LFFileSummary& FileSummary);
	static INT __stdcall CompareItems(const LFItemDescriptor** pData1, const LFItemDescriptor** pData2, const SortParameters& Parameters);
	static INT CompareItemsSecondary(const LFItemDescriptor** pData1, const LFItemDescriptor** pData2, SortParameters Parameters, ATTRIBUTE Attr, BOOL Descending=FALSE);
	UINT Aggregate(UINT WriteIndex, UINT ReadIndex1, UINT ReadIndex2, LPVOID pCategorizer, ATTRIBUTE Attr, BOOL GroupSingle, LFFilter* pFilter);
};

inline void LFSearchResult::InitializeFileSummary(LFFileSummary& FileSummary)
{
	ZeroMemory(&FileSummary, sizeof(FileSummary));

	FileSummary.Context = LFContextAuto;
	FileSummary.OnlyTimebasedMediaFiles = TRUE;
}

inline void LFSearchResult::AddStoreToSummary(LFFileSummary& FileSummary, const LFStoreDescriptor& StoreDescriptor)
{
	FileSummary.FileCount += StoreDescriptor.Statistics.FileCount[0];
	FileSummary.FileSize += StoreDescriptor.Statistics.FileSize[0];

	FileSummary.Context = LFContextStores;
}

inline INT LFSearchResult::CompareItemsSecondary(const LFItemDescriptor** pData1, const LFItemDescriptor** pData2, SortParameters Parameters, ATTRIBUTE Attr, BOOL Descending)
{
	Parameters.Attr = Attr;
	Parameters.Descending = Descending;

	return CompareItems(pData1, pData2, Parameters);
}
