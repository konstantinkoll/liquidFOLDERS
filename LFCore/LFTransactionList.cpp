#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFTransactionList.h"
#include <assert.h>
#include <malloc.h>

LFTransactionList::LFTransactionList()
{
	m_LastError = LFOk;
	m_Entries = NULL;
	m_Count = 0;
	m_Allocated = 0;
	m_Changes = false;
}

LFTransactionList::~LFTransactionList()
{
	if (m_Entries)
	{
		for (unsigned int a=0; a<m_Count; a++)
			LFFreeItemDescriptor(m_Entries[a].Item);
		_aligned_free(m_Entries);
	}
}

bool LFTransactionList::AddItemDescriptor(LFItemDescriptor* i, unsigned int UserData)
{
	assert(i);

	if (!m_Entries)
	{
		m_Entries = static_cast<LFTL_Entry*>(_aligned_malloc(LFTL_FirstAlloc*sizeof(LFTL_Entry), LFTL_MemoryAlignment));
		if (!m_Entries)
		{
			m_LastError = LFMemoryError;
			return false;
		}
		m_Allocated = LFTL_FirstAlloc;
	}
	
	if (m_Count==m_Allocated)
	{
		m_Entries = static_cast<LFTL_Entry*>(_aligned_realloc(m_Entries, (m_Allocated+LFTL_SubsequentAlloc)*sizeof(LFTL_Entry), LFTL_MemoryAlignment));
		if (!m_Entries)
		{
			m_LastError = LFMemoryError;
			return false;
		}
		m_Allocated += LFTL_SubsequentAlloc;
	}

	i->RefCount++;
	m_Entries[m_Count].Item = i;
	m_Entries[m_Count].LastError = LFOk;
	m_Entries[m_Count++].UserData = UserData;

	return true;
}

void LFTransactionList::RemoveEntry(unsigned int idx)
{
	assert(idx<m_Count);

	LFFreeItemDescriptor(m_Entries[idx].Item);

	if (idx<--m_Count)
		m_Entries[idx] = m_Entries[m_Count];
}

void LFTransactionList::RemoveFlaggedEntries()
{
	unsigned int idx = 0;
	
	while (idx<m_Count)
	{
		if (m_Entries[idx].Item->DeleteFlag)
		{
			RemoveEntry(idx);
		}
		else
		{
			idx++;
		}
	}
}

void LFTransactionList::RemoveErrorEntries()
{
	unsigned int idx = 0;
	
	while (idx<m_Count)
	{
		if (m_Entries[idx].LastError!=LFOk)
		{
			RemoveEntry(idx);
		}
		else
		{
			idx++;
		}
	}
}
