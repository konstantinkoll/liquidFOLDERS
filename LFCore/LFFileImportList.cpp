
#include "stdafx.h"
#include "LFCore.h"
#include "LFFileImportList.h"
#include "Progress.h"
#include "Stores.h"


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

LFCORE_API BOOL LFAddImportPath(LFFileImportList* pFileImportList, LPCWSTR pPath)
{
	assert(pFileImportList);
	assert(pPath);

	return pFileImportList->AddPath(pPath);
}

LFCORE_API UINT LFDoFileImport(LFFileImportList* pFileImportList, BOOL Recursive, const STOREID& StoreID, LFItemDescriptor* pItemTemplate, BOOL Move, LFProgress* pProgress)
{
	return pFileImportList->DoFileImport(Recursive, StoreID, pItemTemplate, Move, pProgress);
}


// LFFileImportList
//

LFFileImportList::LFFileImportList()
	: LFDynArray()
{
	m_LastError = LFOk;
}

BOOL LFFileImportList::AddPath(LPCWSTR pPath, WIN32_FIND_DATA* pFindData)
{
	assert(pPath);

	LFFileImportItem Item;

	wcscpy_s(Item.Path, MAX_PATH, pPath);

	// Remove trailing backslash
	if (Item.Path[0]!=L'\0')
	{
		WCHAR* pChar = &Item.Path[wcslen(Item.Path)-1];
		if (*pChar==L'\\')
			*pChar = L'\0';
	}

	// FindData
	if (pFindData)
	{
		Item.FindData = *pFindData;
		Item.FindDataPresent = TRUE;
	}
	else
	{
		Item.FindDataPresent = FALSE;
	}

	Item.LastError = LFOk;
	Item.Processed = FALSE;

	return LFDynArray::AddItem(Item);
}

void LFFileImportList::Resolve(BOOL Recursive, LFProgress* pProgress)
{
	// Progress
	ProgressMinorStart(pProgress, 1, m_Items[0].Path);

	// Resolve
	UINT Index = 0;

	while (Index<m_ItemCount)
	{
		if (!m_Items[Index].Processed)
		{
			DWORD Attr = m_Items[Index].FindDataPresent ? m_Items[Index].FindData.dwFileAttributes : GetFileAttributes(m_Items[Index].Path);
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

					WIN32_FIND_DATAW FindData;
					HANDLE hFind = FindFirstFile(DirSpec, &FindData);

					if (hFind!=INVALID_HANDLE_VALUE)
						do
						{
							if (((FindData.dwFileAttributes & (FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_VIRTUAL | FILE_ATTRIBUTE_SYSTEM))==0) &&
								(wcscmp(FindData.cFileName, L".")!=0) && (wcscmp(FindData.cFileName, L"..")!=0) &&
								((FindData.cFileName[0]!='.') || ((FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)==0)) &&
								(((FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0) || Recursive) &&
								(wcslen(m_Items[Index].Path)+wcslen(FindData.cFileName)<MAX_PATH-1))
								{
									WCHAR Filename[MAX_PATH];
									wcscpy_s(Filename, MAX_PATH, m_Items[Index].Path);
									wcscat_s(Filename, MAX_PATH, L"\\");
									wcscat_s(Filename, MAX_PATH, FindData.cFileName);

									AddPath(Filename, &FindData);

									// Progress
									if (pProgress)
									{
										pProgress->MinorCount = m_ItemCount;
										UpdateProgress(pProgress);
									}
								}
						}
						while (FindNextFile(hFind, &FindData)!=0);

					FindClose(hFind);

					// Progress
					ProgressMinorNext(pProgress);

					m_Items[Index].Processed = TRUE;
				}
		}

		Index++;
	}

	// Progress
	if (pProgress)
		pProgress->MinorCount = m_ItemCount;
}

INT LFFileImportList::CompareItems(LFFileImportItem* pData1, LFFileImportItem* pData2, const SortParameters& /*Parameters*/)
{
	return _wcsicmp(pData1->Path, pData2->Path);
}

LPCWSTR LFFileImportList::GetFileName(UINT Index)
{
	LPCWSTR pChar = wcsrchr(m_Items[Index].Path, L'\\');

	return pChar ? pChar+1 : m_Items[Index].Path;
}

void LFFileImportList::SetError(UINT Index, UINT Result, LFProgress* pProgress)
{
	if (Result!=LFOk)
		m_LastError = Result;

	m_Items[Index].LastError = Result;
	m_Items[Index].Processed = TRUE;

	// Progress
	if (ProgressMinorNext(pProgress, Result, GetFileName(Index)))
		m_LastError = LFCancel;
}

UINT LFFileImportList::DoFileImport(BOOL Recursive, const STOREID& StoreID, LFItemDescriptor* pItemTemplate, BOOL Move, LFProgress* pProgress)
{
	CStore* pStore;
	if ((m_LastError=OpenStore(StoreID, pStore))==LFOk)
	{
		// Resolve
		Resolve(Recursive, pProgress);

		// Process files
		for (UINT a=0; a<m_ItemCount; a++)
		{
			if (!m_Items[a].Processed)
			{
				// Progress
				if (SetProgressObject(pProgress, GetFileName(a)))
				{
					m_LastError = LFCancel;
					break;
				}

				// Metadata
				LFItemDescriptor* pItemDescriptor = LFCloneItemDescriptor(pItemTemplate);

				SetError(a, pStore->ImportFile(m_Items[a].Path, pItemDescriptor, Move), pProgress);
	
				LFFreeItemDescriptor(pItemDescriptor);
			}

			// Progress
			if (AbortProgress(pProgress))
				break;
		}

		delete pStore;
	}

	// Finish
	if (SetProgressObject(pProgress))
		m_LastError = LFCancel;

	SendLFNotifyMessage(LFMessages.StatisticsChanged);

	return m_LastError;
}
