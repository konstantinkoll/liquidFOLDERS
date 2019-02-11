
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
	m_IsSorted = FALSE;
}

BOOL LFFileImportList::IsPathGUID(LPCWSTR pPath)
{
	assert(pPath);

	// Check first and last chars: must be brackets
	const SIZE_T szPath = wcslen(pPath);
	if (szPath<34)
		return FALSE;

	if ((pPath[0]!=L'{') || (pPath[szPath-1]!=L'}'))
		return FALSE;

	// Check all other chars
	for (SIZE_T Ptr=1; Ptr<szPath-1; Ptr++)
		if (!wcschr(L"-0123456789ABCDEFabcdef", pPath[Ptr]))
			return FALSE;

	return TRUE;
}

BOOL LFFileImportList::IsPathEligible(SIZE_T szPath, const WIN32_FIND_DATA& FindData, DWORD Forbidden)
{
	assert((Forbidden & ALWAYSFORBIDDEN)==ALWAYSFORBIDDEN);

	// No forbidden files
	if (FindData.dwFileAttributes & Forbidden)
		return FALSE;

	// No super-hidden files
	if ((FindData.cFileName[0]=='.') && (FindData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN))
		return FALSE;

	// No virtual parent folders
	if ((wcscmp(FindData.cFileName, L".")==0) || (wcscmp(FindData.cFileName, L"..")==0))
		return FALSE;

	// No GUIDs as path name
	if (IsPathGUID(FindData.cFileName))
		return FALSE;

	// Check for max path size
	if (szPath+wcslen(FindData.cFileName)+1>=MAX_PATH)
		return FALSE;

	// Eligible
	return TRUE;
}

BOOL LFFileImportList::AddPath(LPCWSTR pPath)
{
	assert(pPath);
	assert(pPath[0]!=L'\0');

	// Path
	LFFileImportItem Item;
	wcscpy_s(Item.Path, MAX_PATH, pPath);

	// Remove trailing backslash
	WCHAR* pChar = &Item.Path[wcslen(Item.Path)-1];
	if (*pChar==L'\\')
		*pChar = L'\0';

	Item.LastError = LFOk;
	Item.Processed = PROCESSED_UNTOUCHED;
	Item.Flags = 0;

	return LFDynArray::AddItem(Item);
}

BOOL LFFileImportList::AddPath(LPCWSTR pPath, const WIN32_FIND_DATA& FindData)
{
	assert(pPath);
	assert(pPath[0]!=L'\0');
	assert(pPath[wcslen(pPath)-1]!=L'\\');

	// Path
	LFFileImportItem Item;
	wcscpy_s(Item.Path, MAX_PATH, pPath);

	Item.LastError = LFOk;
	Item.Processed = PROCESSED_UNTOUCHED;

	// FindData
	Item.FindData = FindData;
	Item.Flags = FII_FINDDATAVALID;

	return LFDynArray::AddItem(Item);
}

void LFFileImportList::Resolve(BOOL Recursive, LFProgress* pProgress)
{
	// Progress
	ProgressMinorStart(pProgress, 1, m_Items[0].Path);

	// Resolve
	const DWORD Forbidden = Recursive ? ALWAYSFORBIDDEN : ALWAYSFORBIDDEN | FILE_ATTRIBUTE_DIRECTORY;

	UINT Index = 0;
	while (Index<m_ItemCount)
	{
		if (!m_Items[Index].Processed)
		{
			const DWORD Attr = (m_Items[Index].Flags & FII_FINDDATAVALID) ? m_Items[Index].FindData.dwFileAttributes : GetFileAttributes(m_Items[Index].Path);
			if (Attr==INVALID_FILE_ATTRIBUTES)
			{
				m_Items[Index].Processed = PROCESSED_FINISHED;
			}
			else
				if (Attr & FILE_ATTRIBUTE_DIRECTORY)
				{
					// Find and add files
					const SIZE_T szPath = wcslen(m_Items[Index].Path);

					WCHAR DirSpec[MAX_PATH];
					wcscpy_s(DirSpec, MAX_PATH, m_Items[Index].Path);
					wcscat_s(DirSpec, MAX_PATH, L"\\*");

					WIN32_FIND_DATAW FindData;
					HANDLE hFind = FindFirstFile(DirSpec, &FindData);

					if (hFind!=INVALID_HANDLE_VALUE)
						do
						{
							if (IsPathEligible(szPath, FindData, Forbidden))
							{
								WCHAR Filename[MAX_PATH];
								wcscpy_s(Filename, MAX_PATH, m_Items[Index].Path);
								wcscat_s(Filename, MAX_PATH, L"\\");
								wcscat_s(Filename, MAX_PATH, FindData.cFileName);

								AddPath(Filename, FindData);

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

					m_Items[Index].Processed = PROCESSED_FINISHED;
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
	m_Items[Index].Processed = PROCESSED_FINISHED;

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

LFFileImportItem* LFFileImportList::FindPath(LPCWSTR pPath)
{
	assert(pPath);

	// Sort to enable binary search
	if (!m_IsSorted)
	{
		SortItems();

		m_IsSorted = TRUE;
	}

	// Find path in import list using binary search
	INT First = 0;
	INT Last = (INT)m_ItemCount-1;

	while (First<=Last)
	{
		const INT Mid = (First+Last)/2;
		LFFileImportItem* pItem = &m_Items[Mid];

		const INT Result = _wcsicmp(&pPath[4], pItem->Path);
		if (!Result)
			return pItem;

		if (Result<0)
		{
			Last = Mid-1;
		}
		else
		{
			First = Mid+1;
		}
	}

	return NULL;
}
