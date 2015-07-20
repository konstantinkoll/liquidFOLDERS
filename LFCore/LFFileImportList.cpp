
#include "stdafx.h"
#include "LFCore.h"
#include "LFFileImportList.h"
#include <assert.h>


LFCORE_API LFFileImportList* LFAllocFileImportList(HDROP hDrop)
{
	LFFileImportList* pFileImportList = new LFFileImportList();

	if (hDrop)
	{
		UINT cFiles = DragQueryFile(hDrop, (UINT32)-1, NULL, 0);
		WCHAR FileName[MAX_PATH];

		for (UINT a=0; a<cFiles; a++)
			if (DragQueryFile(hDrop, a, FileName, MAX_PATH))
				pFileImportList->AddPath(FileName);
	}

	return pFileImportList;
}

LFCORE_API void LFFreeFileImportList(LFFileImportList* pFileImportList)
{
	assert(pFileImportList);

	delete pFileImportList;
}

LFCORE_API BOOL LFAddImportPath(LFFileImportList* pFileImportList, WCHAR* Path)
{
	assert(pFileImportList);
	assert(Path);

	return pFileImportList->AddPath(Path);
}


// LFFileImportList
//

LFFileImportList::LFFileImportList()
	: LFDynArray()
{
	m_FileCount = 0;
	m_FileSize = 0;
}

BOOL LFFileImportList::AddPath(WCHAR* Path)
{
	assert(Path);

	LFFileImportListItem Item;

	wcscpy_s(Item.Path, MAX_PATH, Path);
	Item.LastError = LFOk;
	Item.Processed = FALSE;

	return LFDynArray::AddItem(Item);
}

void LFFileImportList::Resolve(BOOL Recursive, LFProgress* pProgress)
{
	// Init progress
	if (pProgress)
	{
		wcscpy_s(pProgress->Object, 256,m_Items[0].Path);

		pProgress->MinorCount = 1;
		pProgress->MinorCurrent = 0;
		SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL);
	}

	// Resolve
	UINT Index = 0;

	while (Index<m_ItemCount)
	{
		if (!m_Items[Index].Processed)
		{
			DWORD Attr = GetFileAttributes(m_Items[Index].Path);
			if (Attr==INVALID_FILE_ATTRIBUTES)
			{
				m_Items[Index].Processed = TRUE;
			}
			else
				if (Attr & FILE_ATTRIBUTE_DIRECTORY)
				{
					// Dateien suchen und hinzufügen
					WCHAR DirSpec[MAX_PATH];
					wcscpy_s(DirSpec, MAX_PATH, m_Items[Index].Path);
					wcscat_s(DirSpec, MAX_PATH, L"\\*");

					WIN32_FIND_DATAW FindFileData;
					HANDLE hFind = FindFirstFile(DirSpec, &FindFileData);

					if (hFind!=INVALID_HANDLE_VALUE)
						do
						{
							if (((FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_VIRTUAL | FILE_ATTRIBUTE_SYSTEM))==0) &&
								(wcscmp(FindFileData.cFileName, L".")!=0) && (wcscmp(FindFileData.cFileName, L"..")!=0) &&
								((!(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) || (Recursive)))
							{
								WCHAR Filename[MAX_PATH];
								wcscpy_s(Filename, MAX_PATH, m_Items[Index].Path);
								wcscat_s(Filename, MAX_PATH, L"\\");
								wcscat_s(Filename, MAX_PATH, FindFileData.cFileName);

								AddPath(Filename);
							}
						}
						while (FindNextFile(hFind, &FindFileData)!=0);

					FindClose(hFind);

					// Update
					if (pProgress)
					{
						pProgress->MinorCount = m_ItemCount;
						SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL);
					}

					m_Items[Index].Processed = TRUE;
				}
		}

		Index++;
	}

	// Progress
	if (pProgress)
		pProgress->MinorCount = m_ItemCount;
}

void LFFileImportList::SetError(UINT Index, UINT Result, LFProgress* pProgress)
{
	m_Items[Index].LastError = m_LastError = Result;
	m_Items[Index].Processed = TRUE;

	if (pProgress)
	{
		wcscpy_s(pProgress->Object, 256, m_Items[Index].Path);
		pProgress->MinorCurrent++;

		if (Result>LFCancel)
			pProgress->ProgressState = LFProgressError;

		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			m_LastError = LFCancel;
	}
}
