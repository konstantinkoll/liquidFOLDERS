#pragma once
#include "liquidFOLDERS.h"

#define LFSR_FirstAlloc          1024
#define LFSR_SubsequentAlloc     1024

#define LFSR_MemoryAlignment     8

// LFSearchResult
// Speichert ein Suchergebnis ab als Array von Zeigern auf LFItemDescriptor. Das Array
// wächst dynamisch, zunächst zum LFSR_FirstAlloc, dann jedes Mal um LFSR_SubsequentAlloc
// Plätze.

class LFSearchResult
{
public:
	LFSearchResult(int ctx);
	LFSearchResult(int ctx, LFSearchResult* res);
	virtual ~LFSearchResult();

	bool AddItemDescriptor(LFItemDescriptor* i);
	bool AddStoreDescriptor(LFStoreDescriptor* s, LFFilter* f);
	void AddDrives(LFFilter* filter);
	void AddBacklink(char* StoreID, LFFilter* f);
	void RemoveItemDescriptor(unsigned int idx);
	void RemoveFlaggedItemDescriptors();
	void Sort(unsigned int attr, bool descending, bool categories);
	void Group(unsigned int attr, bool descending);

	LFItemDescriptor** m_Items;
	bool m_HasCategories;
	bool m_HidingItems;
	DWORD m_QueryTime;
	unsigned int m_LastError;
	unsigned int m_ItemCount;
	unsigned int m_FileCount;
	__int64 m_FileSize;
	int m_Context;
	int m_ContextView;
	unsigned int m_RecommendedView;
	char m_StoreID[LFKeySize];

protected:
	unsigned int m_Allocated;

private:
	int Compare(int eins, int zwei, unsigned int attr, bool descending, bool categories);
	void Heap(int wurzel, int anz, unsigned int attr, bool descending, bool categories);

	bool m_RawCopy;
};
