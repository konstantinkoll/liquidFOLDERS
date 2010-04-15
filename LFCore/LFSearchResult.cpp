#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFSearchResult.h"
#include "StoreCache.h"
#include <malloc.h>

LFSearchResult::LFSearchResult(int ctx)
{
	m_RawCopy = true;
	m_LastError = LFOk;
	m_Context = ctx;
	m_ContextView = ctx;
	m_RecommendedView = LFViewDetails;
	m_HasCategories = FALSE;
	m_QueryTime = 0;
	m_Files = NULL;
	m_Count = 0;
	m_Allocated = 0;
}

LFSearchResult::LFSearchResult(int ctx, LFSearchResult* res, bool AllowEmptyDrives)
{
	m_RawCopy = false;
	m_LastError = res->m_LastError;
	m_Context = res->m_Context;
	m_ContextView = ctx;
	m_RecommendedView = res->m_RecommendedView;
	m_HasCategories = res->m_HasCategories;
	m_QueryTime = res->m_QueryTime;
	m_Files = static_cast<LFItemDescriptor**>(_aligned_malloc(res->m_Count*sizeof(LFItemDescriptor*), LFSR_MemoryAlignment));
	m_Allocated = res->m_Count;
	if (m_Files)
	{
		if ((AllowEmptyDrives) || (res->m_Context!=LFContextStores))
		{
			memcpy(m_Files, res->m_Files, res->m_Count*sizeof(LFItemDescriptor*));
			m_Count = res->m_Count;
			for (unsigned int a=0; a<res->m_Count; a++)
				m_Files[a]->RefCount++;
		}
		else
		{
			m_Count = 0;
			for (unsigned int a=0; a<res->m_Count; a++)
			{
				if (((res->m_Files[a]->Type & LFTypeMask)!=LFTypeDrive) || !(res->m_Files[a]->Type & LFTypeNotMounted))
				{
					m_Files[m_Count] = res->m_Files[a];
					m_Files[m_Count]->RefCount++;
					m_Count++;
				}
			}
		}
	}
	else
	{
		m_LastError = LFMemoryError;
		m_Count = 0;
		m_Allocated =0;
	}
}

LFSearchResult::~LFSearchResult()
{
	if (m_Files)
	{
		for (unsigned int a=0; a<m_Count; a++)
			LFFreeItemDescriptor(m_Files[a]);
		_aligned_free(m_Files);
	}
}

bool LFSearchResult::AddItemDescriptor(LFItemDescriptor* i)
{
	if (!m_Files)
	{
		m_Files = static_cast<LFItemDescriptor**>(_aligned_malloc(LFSR_FirstAlloc*sizeof(LFItemDescriptor*), LFSR_MemoryAlignment));
		if (!m_Files)
		{
			m_LastError = LFMemoryError;
			return false;
		}
		m_Allocated = LFSR_FirstAlloc;
	}
	
	if (m_Count==m_Allocated)
	{
		m_Files = static_cast<LFItemDescriptor**>(_aligned_realloc(m_Files, (m_Allocated+LFSR_SubsequentAlloc)*sizeof(LFItemDescriptor*), LFSR_MemoryAlignment));
		if (!m_Files)
		{
			m_LastError = LFMemoryError;
			return false;
		}
		m_Allocated += LFSR_SubsequentAlloc;
	}

	if (m_RawCopy)
		i->Position = m_Count;
	m_Files[m_Count++] = i;

	return true;
}

bool LFSearchResult::AddStoreDescriptor(LFStoreDescriptor* s)
{
	return AddItemDescriptor(StoreDescriptor2ItemDescriptor(s));
}

void LFSearchResult::RemoveItemDescriptor(unsigned int idx)
{
	assert(idx<m_Count);

	LFFreeItemDescriptor(m_Files[idx]);

	if (idx<--m_Count)
	{
		m_Files[idx] = m_Files[m_Count];
		if (m_RawCopy)
			m_Files[idx]->Position = idx;
	}
}

void LFSearchResult::RemoveFlaggedItemDescriptors()
{
	unsigned int idx = 0;
	
	while (idx<m_Count)
	{
		if (m_Files[idx]->DeleteFlag)
		{
			RemoveItemDescriptor(idx);
		}
		else
		{
			idx++;
		}
	}
}
