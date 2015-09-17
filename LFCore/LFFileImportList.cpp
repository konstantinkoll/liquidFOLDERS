
#include "stdafx.h"
#include "LFCore.h"
#include "LFFileImportList.h"
#include "Stores.h"
#include <assert.h>


extern LFMessageIDs LFMessages;


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

LFCORE_API BOOL LFAddImportPath(LFFileImportList* pFileImportList, WCHAR* pPath)
{
	assert(pFileImportList);
	assert(pPath);

	return pFileImportList->AddPath(pPath);
}

LFCORE_API UINT LFDoFileImport(LFFileImportList* pFileImportList, BOOL Recursive, CHAR* pStoreID, LFItemDescriptor* pItemTemplate, BOOL Move, LFProgress* pProgress)
{
	return pFileImportList->DoFileImport(Recursive, pStoreID, pItemTemplate, Move, pProgress);
}


// LFFileImportList
//

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
		UpdateProgress(pProgress);
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
						UpdateProgress(pProgress);
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
	if (Result!=LFOk)
		m_LastError = Result;

	m_Items[Index].LastError = Result;
	m_Items[Index].Processed = TRUE;

	if (pProgress)
	{
		wcscpy_s(pProgress->Object, 256, m_Items[Index].Path);
		pProgress->MinorCurrent++;

		if (Result>LFCancel)
			pProgress->ProgressState = LFProgressError;

		if (UpdateProgress(pProgress))
			m_LastError = LFCancel;
	}
}

UINT LFFileImportList::DoFileImport(BOOL Recursive, CHAR* pStoreID, LFItemDescriptor* pItemTemplate, BOOL Move, LFProgress* pProgress)
{
	UINT Result;

	// Store
	CHAR StoreID[LFKeySize] = "";
	if (pStoreID)
		strcpy_s(StoreID, LFKeySize, pStoreID);

	if (StoreID[0]=='\0')
		if ((Result=LFGetDefaultStore(StoreID))!=LFOk)
			return Result;

	CStore* pStore;
	if ((Result=OpenStore(StoreID, TRUE, &pStore))==LFOk)
	{
		// Resolve
		Resolve(Recursive, pProgress);

		// Process files
		for (UINT a=0; a<m_ItemCount; a++)
		{
			if (!m_Items[a].Processed)
			{
				// Progress
				if (pProgress)
				{
					wcscpy_s(pProgress->Object, 256, m_Items[a].Path);
					if (UpdateProgress(pProgress))
					{
						m_LastError = LFCancel;
						break;
					}
				}

				// Metadata
				LFItemDescriptor* pItemDescriptor = LFCloneItemDescriptor(pItemTemplate);

				SetError(a, pStore->ImportFile(m_Items[a].Path, pItemDescriptor, Move), pProgress);
	
				LFFreeItemDescriptor(pItemDescriptor);
			}
			else
			{
				// Resolved paths
				if (pProgress)
					pProgress->MinorCurrent++;
			}

			// Progress
			if (pProgress)
				if (pProgress->UserAbort)
					break;
		}

		delete pStore;
	}

	// Finish
	if (pProgress)
	{
		pProgress->Object[0] = L'\0';
		if (UpdateProgress(pProgress))
			m_LastError = LFCancel;
	}

	SendLFNotifyMessage(LFMessages.StatisticsChanged);

	return Result;
}
