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
	LFSearchResult(int ctx, LFSearchResult* res, bool AllowEmptyDrives=true);
	virtual ~LFSearchResult();

	bool AddItemDescriptor(LFItemDescriptor* i);
	bool AddStoreDescriptor(LFStoreDescriptor* s);
	void RemoveItemDescriptor(UINT idx);
	void RemoveFlaggedItemDescriptors();

	LFItemDescriptor** m_Files;
	BOOL m_HasCategories;
	DWORD m_QueryTime;
	unsigned int m_LastError;
	unsigned int m_Count;
	int m_Context;
	int m_ContextView;
	unsigned int m_RecommendedView;

protected:
	unsigned int m_Allocated;

private:
	bool m_RawCopy;
};
