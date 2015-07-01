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

BOOL LFFileImportList::AddPath(WCHAR* path)
{
	assert(path);

	LFFIL2_Item item;
	ZeroMemory(&item, sizeof(item));
	wcscpy_s(item.Path, MAX_PATH, path);

	return LFDynArray::AddItem(item);
}

void LFFileImportList::Resolve(BOOL recursive)
{
	UINT a = 0;

	while (a<m_ItemCount)
	{
		if (!m_Items[a].Processed)
		{
			DWORD Attr = GetFileAttributes(m_Items[a].Path);
			if (Attr==INVALID_FILE_ATTRIBUTES)
			{
				m_Items[a].Processed = TRUE;
			}
			else
				if (Attr & FILE_ATTRIBUTE_DIRECTORY)
				{
					// Dateien suchen und hinzufügen
					WCHAR DirSpec[MAX_PATH];
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
							WCHAR fn[MAX_PATH];
							wcscpy_s(fn, MAX_PATH, m_Items[a].Path);
							wcscat_s(fn, MAX_PATH, L"\\");
							wcscat_s(fn, MAX_PATH, FindFileData.cFileName);

							AddPath(fn);
						}
					}

					if (FindNextFile(hFind, &FindFileData)!=0)
						goto FileFound;

					FindClose(hFind);

					m_Items[a].Processed = TRUE;
				}
		}

		a++;
	}
}

void LFFileImportList::SetError(UINT idx, UINT Result, LFProgress* pProgress)
{
	if (Result>LFOk)
		m_LastError = Result;
	m_Items[idx].LastError = Result;
	m_Items[idx].Processed = TRUE;

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
