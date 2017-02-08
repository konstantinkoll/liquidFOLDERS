
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

LFCORE_API void LFDoFileImport(LFFileImportList* pFileImportList, BOOL Recursive, const CHAR* pStoreID, LFItemDescriptor* pItemTemplate, BOOL Move, LFProgress* pProgress)
{
	pFileImportList->DoFileImport(Recursive, pStoreID, pItemTemplate, Move, pProgress);
}


// LFFileImportList
//

LFFileImportList::LFFileImportList()
	: LFDynArray()
{
	m_LastError = LFOk;
}

BOOL LFFileImportList::AddPath(WCHAR* Path, WIN32_FIND_DATA* pFindFileData)
{
	assert(Path);

	LFFileImportListItem Item;

	wcscpy_s(Item.Path, MAX_PATH, Path);

	// Remove trailing backslash
	if (Item.Path[0]!=L'\0')
	{
		WCHAR* Ptr = &Item.Path[wcslen(Item.Path)-1];
		if (*Ptr==L'\\')
			*Ptr = L'\0';
	}

	// FindFileData
	if (pFindFileData)
	{
		Item.FindFileData = *pFindFileData;
		Item.FindFileDataPresent = TRUE;
	}
	else
	{
		Item.FindFileDataPresent = FALSE;
	}

	Item.LastError = LFOk;
	Item.Processed = FALSE;

	return LFDynArray::AddItem(Item);
}

void LFFileImportList::Resolve(BOOL Recursive, LFProgress* pProgress)
{
	// Init progress
	if (pProgress)
	{
		wcscpy_s(pProgress->Object, 256, m_Items[0].Path);

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
			DWORD Attr = m_Items[Index].FindFileDataPresent ? m_Items[Index].FindFileData.dwFileAttributes : GetFileAttributes(m_Items[Index].Path);
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
								((FindFileData.cFileName[0]!='.') || ((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)==0)) &&
								(((FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)==0) || Recursive) &&
								(wcslen(m_Items[Index].Path)+wcslen(FindFileData.cFileName)<MAX_PATH-1))
								{
									WCHAR Filename[MAX_PATH];
									wcscpy_s(Filename, MAX_PATH, m_Items[Index].Path);
									wcscat_s(Filename, MAX_PATH, L"\\");
									wcscat_s(Filename, MAX_PATH, FindFileData.cFileName);

									AddPath(Filename, &FindFileData);

									// Update
									if (pProgress)
									{
										pProgress->MinorCount = m_ItemCount;
										UpdateProgress(pProgress);
									}
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

void LFFileImportList::Heap(UINT Wurzel, const UINT Anz)
{
	LFFileImportListItem Item = m_Items[Wurzel];
	UINT Parent = Wurzel;
	UINT Child;

	while ((Child=(Parent+1)*2)<Anz)
	{
		if (_wcsicmp(m_Items[Child-1].Path, m_Items[Child].Path)>0)
			Child--;

		m_Items[Parent] = m_Items[Child];
		Parent = Child;
	}

	if (Child==Anz)
	{
		if (_wcsicmp(m_Items[--Child].Path, Item.Path)>=0)
		{
			m_Items[Parent] = m_Items[Child];
			m_Items[Child] = Item;

			return;
		}

		Child = Parent;
	}
	else
	{
		if (Parent==Wurzel)
			return;

		if (_wcsicmp(m_Items[Parent].Path, Item.Path)>=0)
		{
			m_Items[Parent] = Item;

			return;
		}

		Child = (Parent-1)/2;
	}

	while (Child!=Wurzel)
	{
		Parent = (Child-1)/2;

		if (_wcsicmp(m_Items[Parent].Path, Item.Path)>=0)
			break;

		m_Items[Child] = m_Items[Parent];
		Child = Parent;
	}

	m_Items[Child] = Item;
}

void LFFileImportList::Sort()
{
	if (m_ItemCount>1)
	{
		for (INT a=m_ItemCount/2-1; a>=0; a--)
			Heap(a, m_ItemCount);

		for (INT a=m_ItemCount-1; a>0; a--)
		{
			LFFileImportListItem Temp = m_Items[0];
			m_Items[0] = m_Items[a];
			m_Items[a] = Temp;

			Heap(0, a);
		}
	}
}

WCHAR* LFFileImportList::GetFileName(UINT Index)
{
	WCHAR* Ptr = wcsrchr(m_Items[Index].Path, L'\\');

	return Ptr ? Ptr+1 : m_Items[Index].Path;
}

void LFFileImportList::SetError(UINT Index, UINT Result, LFProgress* pProgress)
{
	if (Result!=LFOk)
		m_LastError = Result;

	m_Items[Index].LastError = Result;
	m_Items[Index].Processed = TRUE;

	if (pProgress)
	{
		wcscpy_s(pProgress->Object, 256, GetFileName(Index));
		pProgress->MinorCurrent++;

		if (Result>LFCancel)
			pProgress->ProgressState = LFProgressError;

		if (UpdateProgress(pProgress))
			m_LastError = LFCancel;
	}
}

void LFFileImportList::DoFileImport(BOOL Recursive, const CHAR* pStoreID, LFItemDescriptor* pItemTemplate, BOOL Move, LFProgress* pProgress)
{
	// Store
	CHAR StoreID[LFKeySize] = "";
	if (pStoreID)
		strcpy_s(StoreID, LFKeySize, pStoreID);

	if (StoreID[0]=='\0')
		if ((m_LastError=LFGetDefaultStore(StoreID))!=LFOk)
			return;

	CStore* pStore;
	if ((m_LastError=OpenStore(StoreID, TRUE, &pStore))==LFOk)
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
					wcscpy_s(pProgress->Object, 256, GetFileName(a));

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
}
