#include "stdafx.h"
#include "LFCore.h"
#include "LFFileImportList.h"
#include <assert.h>
#include <malloc.h>


LFFileImportList::LFFileImportList()
	: LFDynArray()
{
	m_FileCount = 0;
	m_FileSize = 0;
}

bool LFFileImportList::AddPath(wchar_t* path)
{
	assert(path);

	LFFIL2_Item item;
	ZeroMemory(&item, sizeof(item));
	wcscpy_s(item.Path, MAX_PATH, path);

	return LFDynArray::AddItem(item);
}

void LFFileImportList::Resolve(bool recursive)
{
	unsigned int a = 0;

	while (a<m_ItemCount)
	{
		if (!m_Items[a].Processed)
		{
			DWORD attr = GetFileAttributes(m_Items[a].Path);
			if (attr==INVALID_FILE_ATTRIBUTES)
			{
				m_Items[a].Processed = true;
			}
			else
				if (attr & FILE_ATTRIBUTE_DIRECTORY)
				{
					// Dateien suchen und hinzufügen
					wchar_t DirSpec[MAX_PATH];
					wcscpy_s(DirSpec, MAX_PATH, m_Items[a].Path);
					wcscat_s(DirSpec, MAX_PATH, L"\\*");

					WIN32_FIND_DATAW FindFileData;
					HANDLE hFind = FindFirstFile(DirSpec, &FindFileData);

					if (hFind!=INVALID_HANDLE_VALUE)
					{
FileFound:
						if (((FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_VIRTUAL | FILE_ATTRIBUTE_SYSTEM))==0) &&
							(wcscmp(FindFileData.cFileName, L".")!=0) && (wcscmp(FindFileData.cFileName, L"..")!=0) &&
							((!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) || (recursive)))
						{
							wchar_t fn[MAX_PATH];
							wcscpy_s(fn, MAX_PATH, m_Items[a].Path);
							wcscat_s(fn, MAX_PATH, L"\\");
							wcscat_s(fn, MAX_PATH, FindFileData.cFileName);

							AddPath(fn);
						}
					}

					if (FindNextFile(hFind, &FindFileData)!=0)
						goto FileFound;

					FindClose(hFind);

					m_Items[a].Processed = true;
				}
		}

		a++;
	}
}

void LFFileImportList::SetError(unsigned int idx, unsigned int Result, LFProgress* pProgress)
{
	if (Result>LFOk)
		m_LastError = Result;
	m_Items[idx].LastError = Result;
	m_Items[idx].Processed = true;

	if (pProgress)
	{
		if (Result>LFCancel)
			pProgress->ProgressState = LFProgressError;

		wcscpy_s(pProgress->Object, 256, m_Items[idx].Path);
		pProgress->MinorCurrent++;
		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			m_LastError = LFCancel;
	}
}
