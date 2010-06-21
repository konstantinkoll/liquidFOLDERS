#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFFileImportList.h"
#include <assert.h>
#include <malloc.h>

LFFileImportList::LFFileImportList()
{
	m_LastError = LFOk;
	m_Entries = NULL;
	m_Count = 0;
	m_Allocated = 0;
}

LFFileImportList::~LFFileImportList()
{
	if (m_Entries)
	{
		for (unsigned int a=0; a<m_Count; a++)
			if (m_Entries[a])
				free(m_Entries[a]);
		_aligned_free(m_Entries);
	}
}

bool LFFileImportList::AddPath(wchar_t* path)
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

void LFFileImportList::Resolve()
{
	unsigned int a = 0;

	while (a<m_Count)
	{
		if (m_Entries[a])
		{
			DWORD attr = GetFileAttributes(m_Entries[a]);
			if (attr==INVALID_FILE_ATTRIBUTES)
			{
				free(m_Entries[a]);
				m_Entries[a] = NULL;
			}
			else
				if (attr & FILE_ATTRIBUTE_DIRECTORY)
				{
					// Dateien suchen und hinzufügen
					wchar_t DirSpec[MAX_PATH];
					wcscpy_s(DirSpec, MAX_PATH, m_Entries[a]);
					wcscat_s(DirSpec, MAX_PATH, L"\\*");

					WIN32_FIND_DATAW FindFileData;
					HANDLE hFind = FindFirstFileW(DirSpec, &FindFileData);

					if (hFind!=INVALID_HANDLE_VALUE)
					{
FileFound:
						if (((FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_VIRTUAL))==0) &&
							(wcscmp(FindFileData.cFileName, L".")!=0) && (wcscmp(FindFileData.cFileName, L"..")!=0))
						{
							wchar_t fn[MAX_PATH];
							wcscpy_s(fn, MAX_PATH, m_Entries[a]);
							wcscat_s(fn, MAX_PATH, L"\\");
							wcscat_s(fn, MAX_PATH, FindFileData.cFileName);
							AddPath(&fn[0]);
						}
					}

					if (FindNextFileW(hFind, &FindFileData)!=0)
						goto FileFound;

					FindClose(hFind);

					free(m_Entries[a]);
					m_Entries[a] = NULL;
				}
		}

		a++;
	}
}
