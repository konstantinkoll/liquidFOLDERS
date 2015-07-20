
#include "stdafx.h"
#include "LFCore.h"
#include "LFVariantData.h"
#include "Mutex.h"
#include "ShellProperties.h"
#include "Stores.h"
#include "StoreCache.h"
#include "Transaction.h"
#include <assert.h>


extern LFMessageIDs LFMessages;


LFCORE_API UINT LFTransactionRename(CHAR* StoreID, CHAR* FileID, WCHAR* NewName)
{
	assert(StoreID);
	assert(FileID);
	assert(NewName);

	OPEN_STORE(StoreID, TRUE,);

	if (idx1)
		Result = idx1->Rename(FileID, NewName);

	if ((idx2) && (Result==LFOk))
		Result = idx2->Rename(FileID, NewName);

	CLOSE_STORE();
	return Result;
}


// LFFileImportList
//

LFCORE_API void LFTransactionImport(CHAR* key, LFFileImportList* il, LFItemDescriptor* it, BOOL recursive, BOOL move, LFProgress* pProgress)
{
	assert(il);

	// Store finden
	CHAR StoreID[LFKeySize] = "";
	if (key)
		strcpy_s(StoreID, LFKeySize, key);

	if (StoreID[0]=='\0')
		if (GetMutexForStores())
		{
			strcpy_s(StoreID, LFKeySize, DefaultStore);
			ReleaseMutexForStores();
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
	OPEN_STORE(StoreID, TRUE, il->m_LastError = Result);

	// Progress, prepare import list
	il->Resolve(recursive, pProgress);

	// Process
	for (UINT a=0; a<il->m_ItemCount; a++)
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

			LFItemDescriptor* i = LFCloneItemDescriptor(it);
			i->CoreAttributes.Flags = LFFlagNew;
			SetNameExtAddFromFile(i, il->m_Items[a].Path);
			SetAttributesFromFile(i, il->m_Items[a].Path);

			WCHAR Path[2*MAX_PATH];
			Result = PrepareImport(slot, i, Path, 2*MAX_PATH);
			if (Result==LFOk)
				if (!(move ? MoveFile(il->m_Items[a].Path, Path) : CopyFile(il->m_Items[a].Path, Path, FALSE)))
				{
					WCHAR* LastBackslash = wcsrchr(Path, L'\\');
					if (LastBackslash)
						*(LastBackslash+1) = L'\0';
					RemoveDir(Path);

					Result = LFCannotImportFile;
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

			il->SetError(a, Result, pProgress);
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

	SendLFNotifyMessage(LFMessages.StatisticsChanged);
}


// LFTransactionList
//

void UpdateVolume(LFTransactionList* tl, UINT idx, LFVariantData* v)
{
	UINT Result = LFIllegalAttribute;

	if (v->Attr==LFAttrFileName)
	{
		WCHAR szVolumeRoot[4] = L" :\\";
		szVolumeRoot[0] = tl->m_Items[idx].pItemDescriptor->CoreAttributes.FileID[0];

		Result = SetVolumeLabel(szVolumeRoot, v->UnicodeString) ? LFOk : LFDriveNotReady;

		if (Result==LFOk)
			wcscpy_s(tl->m_Items[idx].pItemDescriptor->CoreAttributes.FileName, 256, v->UnicodeString);
	}
	else
	{
		Result = LFIllegalAttribute;
	}

	if (Result==LFOk)
	{
		tl->m_Modified = TRUE;
	}
	else
	{
		tl->m_LastError = tl->m_Items[idx].LastError = Result;
	}

	tl->m_Items[idx].Processed = TRUE;
}

void UpdateStore(LFTransactionList* tl, UINT idx, LFVariantData* v, BOOL& Updated)
{
	UINT Result = LFIllegalAttribute;

	switch (v->Attr)
	{
	case LFAttrFileName:
		Result = LFSetStoreAttributes(tl->m_Items[idx].pItemDescriptor->StoreID, v->UnicodeString, NULL, TRUE);
		break;
	case LFAttrComments:
		Result = LFSetStoreAttributes(tl->m_Items[idx].pItemDescriptor->StoreID, NULL, v->UnicodeString, TRUE);
		break;
	default:
		Result = LFIllegalAttribute;
	}

	if (Result==LFOk)
	{
		LFSetAttributeVariantData(tl->m_Items[idx].pItemDescriptor, *v);
		tl->m_Modified = TRUE;
		Updated |= TRUE;
	}
	else
	{
		tl->m_LastError = tl->m_Items[idx].LastError = Result;
	}

	tl->m_Items[idx].Processed = TRUE;
}

LFCORE_API void LFTransactionUpdate(LFTransactionList* tl, LFVariantData* v1, LFVariantData* v2, LFVariantData* v3)
{
	assert(tl);
	assert(v1);

	BOOL StoresUpdated = FALSE;
	BOOL FilesUpdated = FALSE;

	// Process
	for (UINT a=0; a<tl->m_ItemCount; a++)
		if (tl->m_Items[a].LastError==LFOk)
			switch (tl->m_Items[a].pItemDescriptor->Type & LFTypeMask)
			{
			case LFTypeVolume:
				UpdateVolume(tl, a, v1);
				if (v2)
					UpdateVolume(tl, a, v2);
				if (v3)
					UpdateVolume(tl, a, v3);
				break;
			case LFTypeStore:
				UpdateStore(tl, a, v1, StoresUpdated);
				if (v2)
					UpdateStore(tl, a, v2, StoresUpdated);
				if (v3)
					UpdateStore(tl, a, v3, StoresUpdated);
				break;
			case LFTypeFile:
				if (!tl->m_Items[a].Processed)
				{
					OPEN_STORE(tl->m_Items[a].pItemDescriptor->StoreID, TRUE, tl->SetError(tl->m_Items[a].pItemDescriptor->StoreID, Result))

					if (idx1)
						idx1->Update(tl, v1, v2, v3);
					if (idx2)
						idx2->Update(tl, v1, v2, v3);

					CLOSE_STORE();

					FilesUpdated = TRUE;
				}

				break;
			default:
				tl->m_LastError = tl->m_Items[a].LastError = LFIllegalItemType;
				tl->m_Items[a].Processed = TRUE;
			}

	// Update messages
	if (StoresUpdated)
	{
		SendLFNotifyMessage(LFMessages.StoreAttributesChanged);
		SendShellNotifyMessage(SHCNE_UPDATEDIR);
	}
	if (FilesUpdated)
		SendLFNotifyMessage(LFMessages.StatisticsChanged);
}

LFCORE_API void LFTransactionArchive(LFTransactionList* tl)
{
	assert(tl);

	// Process
	for (UINT a=0; a<tl->m_ItemCount; a++)
		if ((tl->m_Items[a].LastError==LFOk) && (!tl->m_Items[a].Processed))
		{
			if ((tl->m_Items[a].pItemDescriptor->Type & LFTypeMask)==LFTypeFile)
			{
				OPEN_STORE(tl->m_Items[a].pItemDescriptor->StoreID, TRUE, tl->SetError(tl->m_Items[a].pItemDescriptor->StoreID, Result));

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

	SendLFNotifyMessage(LFMessages.StatisticsChanged);
}

LFCORE_API void LFTransactionDelete(LFTransactionList* tl, BOOL PutInTrash, LFProgress* pProgress)
{
	assert(tl);

	// Progress
	if (pProgress)
	{
		pProgress->MinorCount = tl->m_ItemCount;
		pProgress->MinorCurrent = 0;
	}

	// Process
	for (UINT a=0; a<tl->m_ItemCount; a++)
		if ((tl->m_Items[a].LastError==LFOk) && (!tl->m_Items[a].Processed))
		{
			if ((tl->m_Items[a].pItemDescriptor->Type & LFTypeMask)==LFTypeFile)
			{
				OPEN_STORE(tl->m_Items[a].pItemDescriptor->StoreID, TRUE, tl->SetError(tl->m_Items[a].pItemDescriptor->StoreID, Result));

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

	SendLFNotifyMessage(LFMessages.StatisticsChanged);
}

LFCORE_API void LFTransactionRestore(LFTransactionList* tl, UINT Flags)
{
	assert(tl);

	if ((Flags==0) || ((Flags & (LFFlagArchive | LFFlagTrash))!=Flags))
	{
		tl->m_LastError = LFIllegalValue;
		return;
	}

	LFVariantData v1;
	LFInitVariantData(v1, LFAttrFlags);
	v1.Flags.Flags = 0;
	v1.Flags.Mask = Flags;
	v1.IsNull = FALSE;

	LFVariantData v2;
	LFInitVariantData(v2, LFAttrArchiveTime);
	v2.IsNull = FALSE;

	LFVariantData v3;
	LFInitVariantData(v3, LFAttrDeleteTime);
	v3.IsNull = FALSE;

	LFTransactionUpdate(tl, &v1, (Flags & LFFlagArchive) ? &v2 : NULL, (Flags & LFFlagTrash) ? &v3 : NULL);
}

LFCORE_API void LFTransactionResolvePhysicalLocations(LFTransactionList* tl, BOOL IncludePIDL)
{
	// Retrieve physical paths
	for (UINT a=0; a<tl->m_ItemCount; a++)
		if ((tl->m_Items[a].LastError==LFOk) && (!tl->m_Items[a].Processed))
			if ((tl->m_Items[a].pItemDescriptor->Type & LFTypeMask)==LFTypeFile)
			{
				OPEN_STORE(tl->m_Items[a].pItemDescriptor->StoreID, FALSE, tl->SetError(tl->m_Items[a].pItemDescriptor->StoreID, Result));

				if (idx1)
					idx1->ResolvePhysicalLocations(tl);

				CLOSE_STORE();
			}
			else
			{
				tl->m_LastError = tl->m_Items[a].LastError = LFIllegalItemType;
				tl->m_Items[a].Processed = TRUE;
			}

	// PIDLs
	if (IncludePIDL)
		for (UINT a=0; a<tl->m_ItemCount; a++)
			if ((tl->m_Items[a].Processed) && (tl->m_Items[a].LastError==LFOk))
				if (FAILED(SHParseDisplayName(&tl->m_Items[a].Path[4], NULL, &tl->m_Items[a].pidlFQ, 0, NULL)))
					tl->m_Items[a].LastError = tl->m_LastError = LFIllegalPhysicalPath;

	tl->m_Resolved = TRUE;
}


// LFFileIDList
//

LFCORE_API void LFTransactionImport(CHAR* key, LFTransactionList* il, BOOL move, LFProgress* pProgress)
{
	assert(il);

	// Store finden
	CHAR StoreID[LFKeySize] = "";
	if (key)
		strcpy_s(StoreID, LFKeySize, key);

	if (StoreID[0]=='\0')
		if (GetMutexForStores())
		{
			strcpy_s(StoreID, LFKeySize, DefaultStore);
			ReleaseMutexForStores();
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
	OPEN_STORE_PREFIX(StoreID, TRUE, il->m_LastError = Result, slotDst, idxDst1, idxDst2);

	// Progress
	if (pProgress)
	{
		pProgress->MinorCount = il->m_ItemCount;
		pProgress->MinorCurrent = 0;
	}

	// Process
	for (UINT a=0; a<il->m_ItemCount; a++)
	{
		if ((il->m_Items[a].LastError==LFOk) && (!il->m_Items[a].Processed))
		{
			if (strcmp(StoreID, il->m_Items[a].StoreID)==0)
			{
				il->SetError(a, LFCancel, pProgress);
			}
			else
			{
				OPEN_STORE_PREFIX(il->m_Items[a].StoreID, FALSE, il->SetError(il->m_Items[a].StoreID, Result, pProgress), slotSrc, idxSrc1, idxSrc2);

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

	SendLFNotifyMessage(LFMessages.StatisticsChanged);
}


LFCORE_API void LFTransactionAddToSearchResult(LFTransactionList* il, LFSearchResult* sr)
{
	assert(il);

	// Process
	for (UINT a=0; a<il->m_ItemCount; a++)
		if ((il->m_Items[a].LastError==LFOk) && (!il->m_Items[a].Processed))
		{
			OPEN_STORE(il->m_Items[a].StoreID, FALSE, il->SetError(il->m_Items[a].StoreID, Result));

			if (idx1)
				idx1->AddToSearchResult(il, sr);

			CLOSE_STORE();
		}
}
