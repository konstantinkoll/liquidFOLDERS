
#include "stdafx.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "Mutex.h"
#include "PIDL.h"
#include "Stores.h"
#include "StoreCache.h"
#include <assert.h>


extern HMODULE LFCoreModuleHandle;
extern LFMessageIDs LFMessages;
extern CHAR KeyChars[38];


// Verzeichnis-Funktionen
//

__forceinline DWORD CreateDir(LPWSTR lpPath)
{
	return CreateDirectory(lpPath, NULL) ? ERROR_SUCCESS : GetLastError();
}

BOOL RemoveDir(LPWSTR lpPath)
{
	BOOL Result = TRUE;

	// Dateien löschen
	WCHAR DirSpec[MAX_PATH];
	wcscpy_s(DirSpec, MAX_PATH, lpPath);
	wcscat_s(DirSpec, MAX_PATH, L"*");

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(DirSpec, &FindFileData);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_VIRTUAL))==0)
			{
				WCHAR fn[MAX_PATH];
				wcscpy_s(fn, MAX_PATH, lpPath);
				wcscat_s(fn, MAX_PATH, FindFileData.cFileName);

				if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if ((wcscmp(FindFileData.cFileName, L".")!=0) && (wcscmp(FindFileData.cFileName, L"..")!=0))
					{
						wcscat_s(fn, MAX_PATH, L"\\");
						if (!RemoveDir(fn))
							Result = FALSE;
					}
				}
				else
				{
					if (!DeleteFile(fn))
						Result = FALSE;
				}
			}
		}
		while (FindNextFile(hFind, &FindFileData)!=0);

		FindClose(hFind);
	}

	// Verzeichnis löschen
	if (Result)
		if (!RemoveDirectory(lpPath))
			Result = FALSE;

	return Result;
}

UINT CopyDir(LPWSTR lpPathSrc, LPWSTR lpPathDst)
{
	WCHAR DirSpec[MAX_PATH];
	wcscpy_s(DirSpec, MAX_PATH, lpPathSrc);
	wcscat_s(DirSpec, MAX_PATH, L"*");

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(DirSpec, &FindFileData);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
		do
		{
			if ((FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_VIRTUAL))==0)
			{
				WCHAR fns[MAX_PATH];
				wcscpy_s(fns, MAX_PATH, lpPathSrc);
				wcscat_s(fns, MAX_PATH, FindFileData.cFileName);

				WCHAR fnd[MAX_PATH];
				wcscpy_s(fnd, MAX_PATH, lpPathDst);
				wcscat_s(fnd, MAX_PATH, FindFileData.cFileName);

				if (!CopyFile(fns, fnd, FALSE))
				{
					FindClose(hFind);
					return LFCannotCopyIndex;
				}
			}
		}
		while (FindNextFile(hFind, &FindFileData)!=0);

		FindClose(hFind);
	}

	return LFOk;
}

BOOL DirFreeSpace(LPWSTR lpPath, UINT Required)
{
	ULARGE_INTEGER FreeBytesAvailable;
	return GetDiskFreeSpaceEx(lpPath, &FreeBytesAvailable, NULL, NULL) ? FreeBytesAvailable.QuadPart>=Required : FALSE;
}

BOOL DirWriteable(LPWSTR lpPath)
{
	WCHAR Filename[MAX_PATH];
	wcscpy_s(Filename, MAX_PATH, lpPath);
	wcscat_s(Filename, MAX_PATH, L"LF_TEST.BIN");

	HANDLE h = CreateFile(Filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h==INVALID_HANDLE_VALUE)
		return FALSE;

	CloseHandle(h);
	return DeleteFile(Filename)==TRUE;
}

void SanitizeFileName(WCHAR* dst, SIZE_T cCount, WCHAR* src)
{
	wcscpy_s(dst, cCount, src);

	while (*dst!=L'\0')
	{
		if ((*dst<L' ') || (wcschr(L"<>:\"/\\|?*", *dst)))
			*dst = L'_';
		dst++;
	}
}

UINT CreateStoreDirectories(LFStoreDescriptor* s)
{
	assert(s);

	// Create data path
	if (s->DatPath[0]!=L'\0')
	{
		DWORD Result = CreateDir(s->DatPath);
		if ((Result!=ERROR_SUCCESS) && (Result!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;
	}

	if (s->IdxPathMain[0]!=L'\0')
	{
		DWORD Result = CreateDir(s->IdxPathMain);
		if ((Result!=ERROR_SUCCESS) && (Result!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;

		// Hide store on external volumes
		if (((s->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal) && (Result==ERROR_SUCCESS))
			SetFileAttributes(s->DatPath, FILE_ATTRIBUTE_HIDDEN);
	}

	// Create aux index
	if ((s->Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid)
	{
		WCHAR tmpStr[MAX_PATH];
		GetAutoPath(s, tmpStr);

		DWORD Result = CreateDir(tmpStr);
		if ((Result!=ERROR_SUCCESS) && (Result!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;

		Result = CreateDir(s->IdxPathAux);
		if ((Result!=ERROR_SUCCESS) && (Result!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;
	}

	return LFOk;
}

__forceinline void CreateStoreIndex(LFStoreDescriptor* s, BOOL UseMainIndex, UINT &Result)
{
	assert(s);

	CIndex idx(s, UseMainIndex);
	if (!idx.Create())
		Result = LFIndexCreateError;
}

void GetFilePath(WCHAR* DatPath, LFCoreAttributes* ca, WCHAR* dst, SIZE_T cCount)
{
	WCHAR buf1[3] = L" \\";
	buf1[0] = ca->FileID[0];

	WCHAR buf2[LFKeySize-1];
	MultiByteToWideChar(CP_ACP, 0, &ca->FileID[1], -1, buf2, LFKeySize-1);

	wcscpy_s(dst, cCount, L"\\\\?\\");
	wcscat_s(dst, cCount, DatPath);
	wcscat_s(dst, cCount, buf1);
	wcscat_s(dst, cCount, buf2);
}

void GetFileLocation(WCHAR* DatPath, LFCoreAttributes* ca, WCHAR* dst, SIZE_T cCount)
{
	WCHAR buf1[MAX_PATH];
	SanitizeFileName(buf1, MAX_PATH, ca->FileName);

	GetFilePath(DatPath, ca, dst, cCount);
	wcscat_s(dst, cCount, L"\\");
	wcsncat_s(dst, cCount, buf1, 127);

	if (ca->FileFormat[0]!='\0')
	{
		WCHAR buf2[LFExtSize];
		MultiByteToWideChar(CP_ACP, 0, ca->FileFormat, -1, buf2, LFExtSize);

		wcscat_s(dst, cCount, L".");
		wcscat_s(dst, cCount, buf2);
	}
}

BOOL FileExists(LPWSTR lpPath)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(lpPath, &FindFileData);

	BOOL Result = (hFind!=INVALID_HANDLE_VALUE);
	if (Result)
		FindClose(hFind);

	return Result;
}

UINT PrepareImport(LFStoreDescriptor* slot, LFItemDescriptor* i, WCHAR* Dst, SIZE_T cCount)
{
	RANDOMIZE();

	SetAttribute(i, LFAttrStoreID, slot->StoreID);
	i->CoreAttributes.FileID[0] = RAND_CHAR();
	i->CoreAttributes.FileID[LFKeySize-1] = '\0';

	WCHAR Buf[3] = L" \\";
	Buf[0] = i->CoreAttributes.FileID[0];

	WCHAR Path[2*MAX_PATH];
	wcscpy_s(Path, 2*MAX_PATH, slot->DatPath);
	wcscat_s(Path, 2*MAX_PATH, Buf);

	DWORD Result = CreateDir(Path);
	if ((Result!=ERROR_SUCCESS) && (Result!=ERROR_ALREADY_EXISTS))
		return LFIllegalPhysicalPath;

	do
	{
		for (UINT a=1; a<LFKeySize-1; a++)
			i->CoreAttributes.FileID[a] = RAND_CHAR();

		GetFilePath(slot->DatPath, &i->CoreAttributes, Path, 2*MAX_PATH);
	}
	while (FileExists(Path));

	Result = CreateDir(Path);
	if ((Result!=ERROR_SUCCESS) && (Result!=ERROR_ALREADY_EXISTS))
		return LFIllegalPhysicalPath;

	GetFileLocation(slot->DatPath, &i->CoreAttributes, Dst, cCount);
	return LFOk;
}

__forceinline void SendLFNotifyMessage(UINT Msg)
{
	SendNotifyMessage(HWND_BROADCAST, Msg, NULL, NULL);
}

void SendShellNotifyMessage(UINT Msg, CHAR* StoreID, LPITEMIDLIST oldpidl, LPITEMIDLIST oldpidlDelegate)
{
	LPITEMIDLIST pidl;
	LPITEMIDLIST pidlDelegate;
	if (GetPIDLsForStore(StoreID, &pidl, &pidlDelegate))
	{
		SHChangeNotify(Msg, SHCNF_IDLIST | SHCNF_FLUSH, oldpidl ? oldpidl : pidl, (Msg==SHCNE_RENAMEFOLDER) ? pidl : NULL);
		CoTaskMemFree(pidl);

		if (pidlDelegate)
		{
			SHChangeNotify(Msg, SHCNF_IDLIST | SHCNF_FLUSH, oldpidlDelegate ? oldpidlDelegate : pidlDelegate, (Msg==SHCNE_RENAMEFOLDER) ? pidlDelegate : NULL);
			CoTaskMemFree(pidlDelegate);
		}
	}
}


// Stores
//

LFCORE_API UINT LFGetFileLocation(LFItemDescriptor* i, WCHAR* dst, SIZE_T cCount, BOOL CheckExists, BOOL RemoveNew, BOOL Extended)
{
	if ((i->Type & LFTypeMask)!=LFTypeFile)
		return LFIllegalItemType;

	if (i->CoreAttributes.Flags & LFFlagLink)
	{
		*dst = L'\0';
		return LFOk;
	}

	BOOL Changed = FALSE;

	if (!GetMutexForStores())
		return LFMutexError;

	UINT Result = LFIllegalKey;
	LFStoreDescriptor* slot = FindStore(i->StoreID);

	if (slot)
		if (LFIsStoreMounted(slot))
		{
			WCHAR tmpPath[2*MAX_PATH];
			GetFileLocation(slot->DatPath, &i->CoreAttributes, tmpPath, 2*MAX_PATH);
			Result = LFOk;

			if (CheckExists || (RemoveNew && (i->CoreAttributes.Flags & LFFlagNew)))
			{
				BOOL Exists = FileExists(tmpPath);

				if ((Exists!=((i->CoreAttributes.Flags & LFFlagMissing)==0)) || (RemoveNew && (i->CoreAttributes.Flags & LFFlagNew)))
				{
					// Update index
					OPEN_STORE(i->StoreID, TRUE, Result = Result);

					if (idx1)
						idx1->UpdateSystemFlags(i, Exists, RemoveNew);
					if (idx2)
						idx2->UpdateSystemFlags(i, Exists, RemoveNew);

					CLOSE_STORE();

					Changed = TRUE;
				}

				if (CheckExists && !Exists)
					Result = LFNoFileBody;
			}

			if (Result==LFOk)
				if (Extended)
				{
					wcscpy_s(dst, cCount, tmpPath);
				}
				else
					if (wcslen(&tmpPath[4])<=MAX_PATH)
					{
						wcscpy_s(dst, cCount, &tmpPath[4]);
					}
					else
					{
						GetShortPathName(&tmpPath[4], dst, (DWORD)cCount);
					}
		}
		else
		{
			Result = LFStoreNotMounted;
		}

	ReleaseMutexForStores();

	if (Changed)
		SendLFNotifyMessage(LFMessages.StatisticsChanged);

	return Result;
}

LFCORE_API UINT LFGetStoreSettings(CHAR* StoreID, LFStoreDescriptor* s)
{
	assert(StoreID);

	if (!GetMutexForStores())
		return LFMutexError;

	LFStoreDescriptor* slot = FindStore(StoreID[0]=='\0' ? DefaultStore : StoreID);
	if (slot)
		*s = *slot;

	ReleaseMutexForStores();
	return (slot ? LFOk : LFIllegalKey);
}

LFCORE_API UINT LFGetStoreSettings(GUID guid, LFStoreDescriptor* s)
{
	if (!GetMutexForStores())
		return LFMutexError;

	LFStoreDescriptor* slot = FindStore(guid);
	if (slot)
		*s = *slot;

	ReleaseMutexForStores();
	return (slot ? LFOk : LFIllegalKey);
}

LFCORE_API BOOL LFIsStoreMounted(LFStoreDescriptor* s)
{
	return s ? IsStoreMounted(s) : FALSE;
}

LFCORE_API UINT LFGetStoreIcon(LFStoreDescriptor* s)
{
	return s->Source;
}

LFCORE_API UINT LFCreateStore(LFStoreDescriptor* s)
{
	// GUID generieren
	CoCreateGuid(&s->guid);

	// Pfad ergänzen
	AppendGUID(s, s->DatPath);

	SetStoreAttributes(s);

	// Ggf. Name setzen
	if (s->StoreName[0]==L'\0')
		LoadString(LFCoreModuleHandle, IDS_NEWSTORE, s->StoreName, 256);

	if (!GetMutexForStores())
		return LFMutexError;

	// CreateTime und MaintenanceTime setzen
	GetSystemTimeAsFileTime(&s->CreationTime);
	s->MaintenanceTime = s->CreationTime;
	s->Flags &= ~LFStoreFlagUnchecked;

	// SynchronizeTime zurücksetzen
	ZeroMemory(&s->SynchronizeTime, sizeof(FILETIME));

	// Key generieren
	CreateNewStoreID(s->StoreID);

	// Index-Version
	s->IndexVersion = CURIDXVERSION;

	// Store speichern
	UINT Result = UpdateStore(s);
	if (Result==LFOk)
	{
		HANDLE StoreLock;
		if (GetMutexForStore(s, &StoreLock))
		{
			ReleaseMutexForStores();

			Result = CreateStoreDirectories(s);
			if (Result==LFOk)
			{
				CreateStoreIndex(s, TRUE, Result);
				CreateStoreIndex(s, FALSE, Result);
			}

			ReleaseMutex(StoreLock);
			SendLFNotifyMessage(LFMessages.StoresChanged);
			SendShellNotifyMessage(SHCNE_UPDATEDIR);
		}
		else
		{
			Result = LFMutexError;
		}
	}
	else
	{
		ReleaseMutexForStores();
	}

	return Result;
}

LFCORE_API UINT LFMakeDefaultStore(CHAR* StoreID)
{
	assert(StoreID);

	if (strcmp(StoreID, DefaultStore)==0)
		return LFOk;

	if (!GetMutexForStores())
		return LFMutexError;

	UINT Result = LFIllegalKey;
	LFStoreDescriptor* slot = FindStore(StoreID);

	if (slot)
		Result = MakeDefaultStore(slot);

	ReleaseMutexForStores();

	if (Result==LFOk)
		SendLFNotifyMessage(LFMessages.DefaultStoreChanged);

	return Result;
}

LFCORE_API UINT LFMakeStoreSearchable(CHAR* StoreID, BOOL Searchable)
{
	assert(StoreID);

	if (!GetMutexForStores())
		return LFMutexError;

	UINT Result = LFIllegalKey;
	HANDLE StoreLock = NULL;
	LFStoreDescriptor* slot = FindStore(StoreID, &StoreLock);
	if (!StoreLock)
	{
		ReleaseMutexForStores();
		return LFMutexError;
	}
	if (slot)
	{
		LFStoreDescriptor victim = *slot;

		#define EXIT(Result) { ReleaseMutexForStore(StoreLock); ReleaseMutexForStores(); return Result; }

		switch (slot->Mode & LFStoreModeIndexMask)
		{
		case LFStoreModeIndexHybrid:
			if (Searchable)
				EXIT(LFOk);

			// Delete if not mounted
			if (!LFIsStoreMounted(slot))
			{
				Result = DeleteStore(slot);
				if (Result==LFOk)
				{
					SendLFNotifyMessage(LFMessages.StoresChanged);
					SendShellNotifyMessage(SHCNE_UPDATEDIR);
				}

				EXIT(Result);
			}

			Result = DeleteStoreSettingsFromRegistry(slot);
			if (Result!=LFOk)
				EXIT(Result);

			// Convert to external store
			slot->Mode = (slot->Mode & ~LFStoreModeIndexMask) | LFStoreModeIndexExternal;
			break;
		case LFStoreModeIndexExternal:
			if (!Searchable)
				EXIT(LFOk);

			assert(LFIsStoreMounted(slot));

			// Convert to hybrid store
			slot->Mode = (slot->Mode & ~LFStoreModeIndexMask) | LFStoreModeIndexHybrid;
			break;
		default:
			EXIT(LFIllegalStoreDescriptor);
		}

		SetStoreAttributes(slot);

		Result = UpdateStore(slot);
		ReleaseMutexForStores();
		if (Result!=LFOk)
			goto Abort;

		if ((slot->Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid)
		{
			Result = CreateStoreDirectories(slot);
			if (Result!=LFOk)
				goto Abort;

			Result = CopyDir(slot->IdxPathMain, slot->IdxPathAux);
		}
		else
		{
			if (victim.IdxPathAux[0]!=L'\0')
			{
				WCHAR path[MAX_PATH];
				GetAutoPath(&victim, path);
				RemoveDir(path);
			}
		}
	}

Abort:
	ReleaseMutexForStore(StoreLock);

	if (Result==LFOk)
	{
		SendLFNotifyMessage(LFMessages.StoreAttributesChanged);
		SendShellNotifyMessage(SHCNE_UPDATEITEM, StoreID);
		SendShellNotifyMessage(SHCNE_UPDATEDIR);
	}

	return Result;
}

LFCORE_API UINT LFSetStoreAttributes(CHAR* StoreID, WCHAR* name, WCHAR* comment, BOOL InternalCall)
{
	assert(StoreID);

	if ((!name) && (!comment))
		return LFOk;
	if (name)
		if (name[0]==L'\0')
			return LFIllegalValue;

	LPITEMIDLIST oldpidl;
	LPITEMIDLIST oldpidlDelegate;
	GetPIDLsForStore(StoreID, &oldpidl, &oldpidlDelegate);

	if (!GetMutexForStores())
	{
		CoTaskMemFree(oldpidl);
		CoTaskMemFree(oldpidlDelegate);
		return LFMutexError;
	}

	UINT Result = LFIllegalKey;
	LFStoreDescriptor* slot = FindStore(StoreID);
	if (slot)
	{
		if (name)
			wcscpy_s(slot->StoreName, 256, name);

		if (comment)
			wcscpy_s(slot->StoreComment, 256, comment);

		Result = UpdateStore(slot);
	}

	ReleaseMutexForStores();

	if (Result==LFOk)
	{
		if (!InternalCall)
			SendLFNotifyMessage(LFMessages.StoreAttributesChanged);
		if (comment)
			SendShellNotifyMessage(SHCNE_UPDATEITEM, StoreID);
		if (name)
		{
			SendShellNotifyMessage(SHCNE_RENAMEFOLDER, StoreID, oldpidl);
			SendShellNotifyMessage(SHCNE_RENAMEFOLDER, StoreID, oldpidlDelegate);
			SendShellNotifyMessage(SHCNE_UPDATEDIR);
		}
	}

	CoTaskMemFree(oldpidl);
	CoTaskMemFree(oldpidlDelegate);

	return Result;
}

LFCORE_API UINT LFDeleteStore(CHAR* StoreID, LFProgress* pProgress)
{
	assert(StoreID);

	if (!GetMutexForStores())
		return LFMutexError;

	UINT Result = LFIllegalKey;
	HANDLE StoreLock = NULL;
	LFStoreDescriptor* slot = FindStore(StoreID, &StoreLock);
	if (!StoreLock)
	{
		ReleaseMutexForStores();
		return LFMutexError;
	}
	if (slot)
	{
		// Progress
		if (pProgress)
		{
			pProgress->MinorCount = 2;
			pProgress->MinorCurrent = 0;
			pProgress->NoMinorCounter = TRUE;
			wcscpy_s(pProgress->Object, 256, slot->StoreName);
			if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			{
				ReleaseMutexForStores();
				ReleaseMutexForStore(StoreLock);
				return LFCancel;
			}
		}

		// Delete
		if (slot->DatPath[0]!=L'\0')
		{
			WCHAR Path[MAX_PATH];
			wcscpy_s(Path, MAX_PATH, slot->DatPath);
			wcscat_s(Path, MAX_PATH, L"*");

			if (FileExists(Path))
				if (!DirWriteable(slot->DatPath))
				{
					ReleaseMutexForStores();
					ReleaseMutexForStore(StoreLock);
					return LFDriveWriteProtected;
				}
		}

		LFStoreDescriptor victim = *slot;
		Result = DeleteStore(slot);
		ReleaseMutexForStores();
		ReleaseMutexForStore(StoreLock);

		if (Result==LFOk)
		{
			SendLFNotifyMessage(LFMessages.StoresChanged);
			SendShellNotifyMessage(SHCNE_RMDIR, victim.StoreID);
			SendShellNotifyMessage(SHCNE_UPDATEDIR);

			// Progress
			if (pProgress)
			{
				pProgress->MinorCurrent++;
				SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL);
			}

			if (victim.IdxPathAux[0]!=L'\0')
			{
				WCHAR path[MAX_PATH];
				GetAutoPath(&victim, path);
				RemoveDir(path);
			}

			if (victim.DatPath[0]!=L'\0')
				RemoveDir(victim.DatPath);
		}
		else
		{
			pProgress->MinorCount++;
		}

		// Progress
		if (pProgress)
		{
			pProgress->MinorCurrent++;
			SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL);
		}
	}
	else
	{
		ReleaseMutexForStores();
	}

	return Result;
}

UINT RunMaintenance(LFStoreDescriptor* s, BOOL Scheduled, LFProgress* pProgress)
{
	assert(s);

	// Progress
	if (pProgress)
	{
		pProgress->MinorCount = IndexMaintenanceSteps+2;
		pProgress->MinorCurrent = 0;
		wcscpy_s(pProgress->Object, 256, s->StoreName);
		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			return LFCancel;
	}

	#define ABORT(Result) { if (pProgress) pProgress->ProgressState = LFProgressError; return Result; }

	// Verzeichnisse prüfen
	UINT Result = CreateStoreDirectories(s);
	if (Result!=LFOk)
		ABORT(Result);

	// Progress
	if (pProgress)
	{
		pProgress->MinorCurrent++;
		pProgress->NoMinorCounter = TRUE;
		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			return LFCancel;
	}

	// Index prüfen
	CIndex* idx = new CIndex(s, (s->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexHybrid);

	BOOL Repaired = FALSE;
	Result = idx->Check(Scheduled, &Repaired, pProgress);
	delete idx;

	if (Result!=LFOk)
	{
		if (Result==LFCancel)
			return LFCancel;

		ABORT(Result);
	}

	// Index-Version aktualisieren
	if (Repaired)
		s->IndexVersion = CURIDXVERSION;

	// Index duplizieren
	if (((s->Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid) && LFIsStoreMounted(s))
	{
		if (!DirWriteable(s->IdxPathMain))
			ABORT(LFDriveWriteProtected);

		Result = CopyDir(s->IdxPathAux, s->IdxPathMain);
		if (Result!=LFOk)
			ABORT(Result);
	}

	// Wartung vermerken
	s->Flags &= ~LFStoreFlagUnchecked;

	if (Scheduled || Repaired)
		GetSystemTimeAsFileTime(&s->MaintenanceTime);

	// Store aktualisieren
	Result = UpdateStore(s, FALSE);
	if (Result!=LFOk)
		ABORT(Result);

	// Progress
	if (pProgress)
	{
		pProgress->MinorCurrent++;
		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			return LFCancel;
	}

	return LFOk;
}

LFCORE_API LFMaintenanceList* LFStoreMaintenance(LFProgress* pProgress)
{
	LFMaintenanceList* ml = new LFMaintenanceList();

	if (!GetMutexForStores())
	{
		ml->m_LastError = LFMutexError;
		return ml;
	}

	CHAR* IDs;
	UINT count = FindStores(&IDs);
	ReleaseMutexForStores();

	if (count)
	{
		if (pProgress)
			pProgress->MajorCount = count;

		CHAR* ptr = IDs;
		for (UINT a=0; a<count; a++)
		{
			if (!GetMutexForStores())
			{
				free(IDs);
				ml->m_LastError = LFMutexError;
				return ml;
			}

			HANDLE StoreLock = NULL;
			LFStoreDescriptor* slot = FindStore(ptr, &StoreLock);
			ReleaseMutexForStores();

			if ((!slot) || (!StoreLock))
				continue;

			ml->AddItem(slot->StoreName, slot->StoreComment, ptr, RunMaintenance(slot, TRUE, pProgress), LFGetStoreIcon(slot));

			ReleaseMutexForStore(StoreLock);

			ptr += LFKeySize;

			if (pProgress)
			{
				if (pProgress->UserAbort)
					break;

				pProgress->MajorCurrent++;
			}
		}

		free(IDs);
	}

	SendLFNotifyMessage(LFMessages.StoreAttributesChanged);

	return ml;
}

UINT OpenStore(CHAR* StoreID, BOOL WriteAccess, CIndex* &Index1, CIndex* &Index2, LFStoreDescriptor** s, HANDLE* lock)
{
	Index1 = Index2 = NULL;

	if (!GetMutexForStores())
		return LFMutexError;

	LFStoreDescriptor* slot = FindStore(StoreID, lock);
	ReleaseMutexForStores();

	if (!slot)
		return LFIllegalKey;
	if (!*lock)
		return LFMutexError;

	if (s)
		*s = slot;

	if (WriteAccess && !LFIsStoreMounted(slot))
	{
		ReleaseMutexForStore(*lock);
		return LFStoreNotMounted;
	}

	// Run non-scheduled maintenance
	if (slot->Flags & LFStoreFlagUnchecked)
	{
		UINT Result = RunMaintenance(slot, FALSE);
		if ((Result!=LFOk) && (Result!=LFDriveWriteProtected))
		{
			ReleaseMutexForStore(*lock);
			return Result;
		}
	}

	// Open index(es)
	if (WriteAccess)
	{
		Index1 = new CIndex(slot, TRUE);
		if ((slot->Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid)
			Index2 = new CIndex(slot, FALSE);
	}
	else
	{
		Index1 = new CIndex(slot, (slot->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexHybrid);
	}

	return LFOk;
}
