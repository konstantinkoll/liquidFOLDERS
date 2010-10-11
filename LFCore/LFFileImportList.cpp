#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFFileImportList.h"
#include <assert.h>
#include <malloc.h>

LFFileImportList::LFFileImportList()
	: DynArray()
{
}

LFFileImportList::~LFFileImportList()
{
	if (m_Items)
		for (unsigned int a=0; a<m_ItemCount; a++)
			if (m_Items[a])
				free(m_Items[a]);
}

bool LFFileImportList::AddPath(wchar_t* path)
{
	assert(path);

	size_t sz = wcslen(path)+1;
	wchar_t* i = (wchar_t*)malloc(sz*sizeof(wchar_t));
	wcscpy_s(i, sz, path);

	if (!DynArray::AddItem(i))
	{
		free(i);
		return false;
	}

	return true;
}

void LFFileImportList::Resolve()
{
	unsigned int a = 0;

	while (a<m_ItemCount)
	{
		if (m_Items[a])
		{
			DWORD attr = GetFileAttributes(m_Items[a]);
			if (attr==INVALID_FILE_ATTRIBUTES)
			{
				free(m_Items[a]);
				m_Items[a] = NULL;
			}
			else
				if (attr & FILE_ATTRIBUTE_DIRECTORY)
				{
					// Dateien suchen und hinzufügen
					wchar_t DirSpec[MAX_PATH];
					wcscpy_s(DirSpec, MAX_PATH, m_Items[a]);
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
							wcscpy_s(fn, MAX_PATH, m_Items[a]);
							wcscat_s(fn, MAX_PATH, L"\\");
							wcscat_s(fn, MAX_PATH, FindFileData.cFileName);
							AddPath(&fn[0]);
						}
					}

					if (FindNextFileW(hFind, &FindFileData)!=0)
						goto FileFound;

					FindClose(hFind);

					free(m_Items[a]);
					m_Items[a] = NULL;
				}
		}

		a++;
	}
}
