
#include "stdafx.h"
#include "LFCore.h"
#include "LFVariantData.h"
#include "Mutex.h"
#include "ShellProperties.h"
#include "Stores.h"
#include "StoreCache.h"
#include "Transaction.h"
#include <assert.h>


extern HANDLE Mutex_Stores;
extern LFMessageIDs LFMessages;


LFCore_API unsigned int LFTransactionRename(char* StoreID, char* FileID, wchar_t* NewName)
{
	assert(StoreID);
	assert(FileID);
	assert(NewName);

	OPEN_STORE(StoreID, true,);

	if (idx1)
		res = idx1->Rename(FileID, NewName);

	if ((idx2) && (res==LFOk))
		res = idx2->Rename(FileID, NewName);

	CLOSE_STORE();
	return res;
}


// LFFileImportList
//

LFCore_API void LFTransactionImport(char* key, LFFileImportList* il, LFItemDescriptor* it, bool recursive, bool move, LFProgress* pProgress)
{
	assert(il);

	// Store finden
	char StoreID[LFKeySize] = "";
	if (key)
		strcpy_s(StoreID, LFKeySize, key);

	if (StoreID[0]=='\0')
		if (GetMutex(Mutex_Stores))
		{
			strcpy_s(StoreID, LFKeySize, DefaultStore);
			ReleaseMutex(Mutex_Stores);
		}
		else
		{
			il->m_LastError = LFMutexError;
			return;
		}

	if (StoreID[0]=='\0')
	{
		il->m_LastError = LFNoDefaultStore;
		return;
	}

	// Import
	OPEN_STORE(StoreID, true, il->m_LastError = res);

	// Progress, prepare import list
	if (pProgress)
	{
		wcscpy_s(pProgress->Object, 256, il->m_Items[0].Path);
		pProgress->MinorCount = 1;
		pProgress->MinorCurrent = 0;
		SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL);
	}

	il->Resolve(recursive);

	if (pProgress)
		pProgress->MinorCount = il->m_ItemCount;

	// Process
	for (unsigned int a=0; a<il->m_ItemCount; a++)
	{
		if (!il->m_Items[a].Processed)
		{
			// Progress
			if (pProgress)
			{
				wcscpy_s(pProgress->Object, 256, il->m_Items[a].Path);
				if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
				{
					il->m_LastError = LFCancel;
					break;
				}
			}

			LFItemDescriptor* i = LFAllocItemDescriptor(it);
			i->CoreAttributes.Flags = LFFlagNew;
			SetNameExtAddFromFile(i, il->m_Items[a].Path);
			SetAttributesFromFile(i, il->m_Items[a].Path);

			wchar_t Path[2*MAX_PATH];
			res = PrepareImport(slot, i, Path, 2*MAX_PATH);
			if (res==LFOk)
				if (!(move ? MoveFile(il->m_Items[a].Path, Path) : CopyFile(il->m_Items[a].Path, Path, FALSE)))
				{
					wchar_t* LastBackslash = wcsrchr(Path, L'\\');
					if (LastBackslash)
						*(LastBackslash+1) = L'\0';
					RemoveDir(Path);

					res = LFCannotImportFile;
				}
				else
				{
					il->m_FileCount++;
					il->m_FileSize += i->CoreAttributes.FileSize;

					if (idx1)
						idx1->AddItem(i);
					if (idx2)
						idx2->AddItem(i);
				}

			il->SetError(a, res, pProgress);
			LFFreeItemDescriptor(i);
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

	// Done
	if (pProgress)
	{
		pProgress->Object[0] = L'\0';
		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			il->m_LastError = LFCancel;
	}

	CLOSE_STORE();

	SendLFNotifyMessage(LFMessages.StatisticsChanged, NULL);
}


// LFTransactionList
//

void UpdateVolume(LFTransactionList* tl, unsigned int idx, LFVariantData* value)
{
	unsigned int result = LFIllegalAttribute;

	if (value->Attr==LFAttrFileName)
	{
		WCHAR szVolumeRoot[4] = L" :\\";
		szVolumeRoot[0] = tl->m_Items[idx].Item->CoreAttributes.FileID[0];

		result = SetVolumeLabel(szVolumeRoot, value->UnicodeString) ? LFOk : LFDriveNotReady;

		if (result==LFOk)
			wcscpy_s(tl->m_Items[idx].Item->CoreAttributes.FileName, 256, value->UnicodeString);
	}
	else
	{
		result = LFIllegalAttribute;
	}

	if (result==LFOk)
	{
		tl->m_Changes = true;
	}
	else
	{
		tl->m_LastError = tl->m_Items[idx].LastError = result;
	}

	tl->m_Items[idx].Processed = true;
}

void UpdateStore(LFTransactionList* tl, unsigned int idx, LFVariantData* value, bool& Updated)
{
	unsigned int result = LFIllegalAttribute;

	switch (value->Attr)
	{
	case LFAttrFileName:
		result = LFSetStoreAttributes(tl->m_Items[idx].Item->StoreID, value->UnicodeString, NULL, NULL, true);
		break;
	case LFAttrComments:
		result = LFSetStoreAttributes(tl->m_Items[idx].Item->StoreID, NULL, value->UnicodeString, NULL, true);
		break;
	default:
		result = LFIllegalAttribute;
	}

	if (result==LFOk)
	{
		LFSetAttributeVariantData(tl->m_Items[idx].Item, value);
		tl->m_Changes = true;
		Updated |= true;
	}
	else
	{
		tl->m_LastError = tl->m_Items[idx].LastError = result;
	}

	tl->m_Items[idx].Processed = true;
}

LFCore_API void LFTransactionUpdate(LFTransactionList* tl, HWND hWndSource, LFVariantData* value1, LFVariantData* value2, LFVariantData* value3)
{
	assert(tl);

	bool StoresUpdated = false;
	bool FilesUpdated = false;

	// Reset
	tl->Reset();

	// Process
	for (unsigned int a=0; a<tl->m_ItemCount; a++)
		if (tl->m_Items[a].LastError==LFOk)
			switch (tl->m_Items[a].Item->Type & LFTypeMask)
			{
			case LFTypeVolume:
				UpdateVolume(tl, a, value1);
				if (value2)
					UpdateVolume(tl, a, value2);
				if (value3)
					UpdateVolume(tl, a, value3);
				break;
			case LFTypeStore:
				UpdateStore(tl, a, value1, StoresUpdated);
				if (value2)
					UpdateStore(tl, a, value2, StoresUpdated);
				if (value3)
					UpdateStore(tl, a, value3, StoresUpdated);
				break;
			case LFTypeFile:
				if (!tl->m_Items[a].Processed)
				{
					OPEN_STORE(tl->m_Items[a].Item->StoreID, true, tl->SetError(tl->m_Items[a].Item->StoreID, res))

					if (idx1)
						idx1->Update(tl, value1, value2, value3);
					if (idx2)
						idx2->Update(tl, value1, value2, value3);

					CLOSE_STORE();

					FilesUpdated = true;
				}

				break;
			default:
				tl->m_LastError = tl->m_Items[a].LastError = LFIllegalItemType;
				tl->m_Items[a].Processed = true;
			}

	// Update messages
	if (StoresUpdated)
	{
		SendLFNotifyMessage(LFMessages.StoreAttributesChanged, hWndSource);
		SendShellNotifyMessage(SHCNE_UPDATEDIR);
	}
	if (FilesUpdated)
		SendLFNotifyMessage(LFMessages.StatisticsChanged, NULL);
}

LFCore_API void LFTransactionArchive(LFTransactionList* tl)
{
	assert(tl);

	// Reset
	tl->Reset();

	// Process
	for (unsigned int a=0; a<tl->m_ItemCount; a++)
		if ((tl->m_Items[a].LastError==LFOk) && (!tl->m_Items[a].Processed))
		{
			if ((tl->m_Items[a].Item->Type & LFTypeMask)==LFTypeFile)
			{
				OPEN_STORE(tl->m_Items[a].Item->StoreID, true, tl->SetError(tl->m_Items[a].Item->StoreID, res));

				if (idx1)
					idx1->Archive(tl);
				if (idx2)
					idx2->Archive(tl);

				CLOSE_STORE();
			}
			else
			{
				tl->SetError(a, LFIllegalItemType);
			}
		}

	SendLFNotifyMessage(LFMessages.StatisticsChanged, NULL);
}

LFCore_API void LFTransactionDelete(LFTransactionList* tl, bool PutInTrash, LFProgress* pProgress)
{
	assert(tl);

	// Reset
	tl->Reset();

	// Progress
	if (pProgress)
	{
		pProgress->MinorCount = tl->m_ItemCount;
		pProgress->MinorCurrent = 0;
	}

	// Process
	for (unsigned int a=0; a<tl->m_ItemCount; a++)
		if ((tl->m_Items[a].LastError==LFOk) && (!tl->m_Items[a].Processed))
		{
			if ((tl->m_Items[a].Item->Type & LFTypeMask)==LFTypeFile)
			{
				OPEN_STORE(tl->m_Items[a].Item->StoreID, true, tl->SetError(tl->m_Items[a].Item->StoreID, res));

				if (idx1)
					idx1->Delete(tl, PutInTrash, pProgress);
				if (idx2)
					idx2->Delete(tl, PutInTrash, pProgress);

				CLOSE_STORE();
			}
			else
			{
				tl->SetError(a, LFIllegalItemType, pProgress);
			}

			if (pProgress)
				if (pProgress->UserAbort)
					break;
		}

	SendLFNotifyMessage(LFMessages.StatisticsChanged, NULL);
}

LFCore_API void LFTransactionRestore(LFTransactionList* tl, unsigned int Flags)
{
	assert(tl);

	if ((Flags==0) || ((Flags & (LFFlagArchive | LFFlagTrash))!=Flags))
	{
		tl->m_LastError = LFIllegalValue;
		return;
	}

	LFVariantData value1;
	value1.Attr = LFAttrFlags;
	LFGetNullVariantData(&value1);

	value1.IsNull = false;
	value1.Flags.Flags = 0;
	value1.Flags.Mask = Flags;

	LFVariantData value2;
	value2.Attr = LFAttrArchiveTime;
	LFGetNullVariantData(&value2);
	value2.IsNull = false;

	LFVariantData value3;
	value3.Attr = LFAttrDeleteTime;
	LFGetNullVariantData(&value3);
	value3.IsNull = false;

	LFTransactionUpdate(tl, NULL, &value1, (Flags & LFFlagArchive) ? &value2 : NULL, (Flags & LFFlagTrash) ? &value3 : NULL);
}

LFCore_API void LFTransactionResolvePhysicalLocations(LFTransactionList* tl, bool IncludePIDL)
{
	// Reset
	tl->Reset();

	// Retrieve physical paths
	for (unsigned int a=0; a<tl->m_ItemCount; a++)
		if ((tl->m_Items[a].LastError==LFOk) && (!tl->m_Items[a].Processed))
			if ((tl->m_Items[a].Item->Type & LFTypeMask)==LFTypeFile)
			{
				OPEN_STORE(tl->m_Items[a].Item->StoreID, false, tl->SetError(tl->m_Items[a].Item->StoreID, res));

				if (idx1)
					idx1->ResolvePhysicalLocations(tl);

				CLOSE_STORE();
			}
			else
			{
				tl->m_LastError = tl->m_Items[a].LastError = LFIllegalItemType;
				tl->m_Items[a].Processed = true;
			}

	// PIDLs
	if (IncludePIDL)
	{
		IShellFolder* pDesktop = NULL;
		if (SUCCEEDED(SHGetDesktopFolder(&pDesktop)))
		{
			for (unsigned int a=0; a<tl->m_ItemCount; a++)
				if ((tl->m_Items[a].Processed) && (tl->m_Items[a].LastError==LFOk))
					if (FAILED(pDesktop->ParseDisplayName(NULL, NULL, &tl->m_Items[a].Path[4], NULL, &tl->m_Items[a].pidlFQ, NULL)))
						tl->m_Items[a].LastError = tl->m_LastError = LFIllegalPhysicalPath;

			pDesktop->Release();
		}
	}

	tl->m_Resolved = true;
}


// LFFileIDList
//

LFCore_API void LFTransactionImport(char* key, LFFileIDList* il, bool move, LFProgress* pProgress)
{
	assert(il);

	// Reset
	il->Reset();

	// Store finden
	char StoreID[LFKeySize] = "";
	if (key)
		strcpy_s(StoreID, LFKeySize, key);

	if (StoreID[0]=='\0')
		if (GetMutex(Mutex_Stores))
		{
			strcpy_s(StoreID, LFKeySize, DefaultStore);
			ReleaseMutex(Mutex_Stores);
		}
		else
		{
			il->m_LastError = LFMutexError;
			return;
		}

	if (StoreID[0]=='\0')
	{
		il->m_LastError = LFNoDefaultStore;
		return;
	}

	// Import
	OPEN_STORE_PREFIX(StoreID, true, il->m_LastError = res, slotDst, idxDst1, idxDst2);

	// Progress
	if (pProgress)
	{
		pProgress->MinorCount = il->m_ItemCount;
		pProgress->MinorCurrent = 0;
	}

	// Process
	for (unsigned int a=0; a<il->m_ItemCount; a++)
	{
		if ((il->m_Items[a].LastError==LFOk) && (!il->m_Items[a].Processed))
		{
			if (strcmp(StoreID, il->m_Items[a].StoreID)==0)
			{
				il->SetError(a, LFCancel, pProgress);
			}
			else
			{
				OPEN_STORE_PREFIX(il->m_Items[a].StoreID, false, il->SetError(il->m_Items[a].StoreID, res, pProgress), slotSrc, idxSrc1, idxSrc2);

				if (idxSrc1)
					idxSrc1->TransferTo(idxDst1, idxDst2, slotDst, il, slotSrc, move, pProgress);

				CLOSE_STORE_PREFIX(idxSrc1, idxSrc2);
			}
		}

		// Progress
		if (pProgress)
			if (pProgress->UserAbort)
				break;
	}

	// Done
	if (pProgress)
	{
		pProgress->Object[0] = L'\0';
		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			il->m_LastError = LFCancel;
	}

	CLOSE_STORE_PREFIX(idxDst1, idxDst2);

	SendLFNotifyMessage(LFMessages.StatisticsChanged, NULL);
}

LFCore_API void LFTransactionDelete(LFFileIDList* il, bool PutInTrash, LFProgress* pProgress)
{
	assert(il);

	// Reset
	il->Reset();

	// Progress
	if (pProgress)
	{
		pProgress->MinorCount = il->m_ItemCount;
		pProgress->MinorCurrent = 0;
	}

	// Process
	for (unsigned int a=0; a<il->m_ItemCount; a++)
		if ((il->m_Items[a].LastError==LFOk) && (!il->m_Items[a].Processed))
		{
			OPEN_STORE(il->m_Items[a].StoreID, true, il->SetError(il->m_Items[a].StoreID, res));

			if (idx1)
				idx1->Delete(il, PutInTrash, pProgress);
			if (idx2)
				idx2->Delete(il, PutInTrash, pProgress);

			CLOSE_STORE();

			if (pProgress)
				if (pProgress->UserAbort)
					break;
		}

	SendLFNotifyMessage(LFMessages.StatisticsChanged, NULL);
}

LFCore_API void LFTransactionAddToSearchResult(LFFileIDList* il, LFSearchResult* sr)
{
	assert(il);

	// Reset
	il->Reset();

	// Process
	for (unsigned int a=0; a<il->m_ItemCount; a++)
		if ((il->m_Items[a].LastError==LFOk) && (!il->m_Items[a].Processed))
		{
			OPEN_STORE(il->m_Items[a].StoreID, false, il->SetError(il->m_Items[a].StoreID, res));

			if (idx1)
				idx1->AddToSearchResult(il, sr);

			CLOSE_STORE();
		}
}
