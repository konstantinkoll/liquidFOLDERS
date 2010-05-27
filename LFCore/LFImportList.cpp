#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFImportList.h"
#include <assert.h>
#include <malloc.h>

LFImportList::LFImportList()
{
	m_LastError = LFOk;
	m_Entries = NULL;
	m_Count = 0;
	m_Allocated = 0;
}

LFImportList::~LFImportList()
{
	if (m_Entries)
	{
		for (unsigned int a=0; a<m_Count; a++)
			if (m_Entries[a])
				free(m_Entries[a]);
		_aligned_free(m_Entries);
	}
}

bool LFImportList::AddPath(wchar_t* path)
{
	assert(path);

	if (!m_Entries)
	{
		m_Entries = static_cast<wchar_t**>(_aligned_malloc(LFIL_FirstAlloc*sizeof(wchar_t*), LFIL_MemoryAlignment));
		if (!m_Entries)
		{
			m_LastError = LFMemoryError;
			return false;
		}
		m_Allocated = LFIL_FirstAlloc;
	}

	if (m_Count==m_Allocated)
	{
		m_Entries = static_cast<wchar_t**>(_aligned_realloc(m_Entries, (m_Allocated+LFIL_SubsequentAlloc)*sizeof(wchar_t*), LFIL_MemoryAlignment));
		if (!m_Entries)
		{
			m_LastError = LFMemoryError;
			return false;
		}
		m_Allocated += LFIL_SubsequentAlloc;
	}

	size_t sz = wcslen(path)+1;
	m_Entries[m_Count] = (wchar_t*)malloc(sz*sizeof(wchar_t));
	wcscpy_s(m_Entries[m_Count], sz, path);
	m_Count++;

	return true;
}
