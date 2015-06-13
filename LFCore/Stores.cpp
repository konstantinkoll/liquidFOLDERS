
#include "stdafx.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "Mutex.h"
#include "PIDL.h"
#include "Stores.h"
#include "StoreCache.h"
#include <assert.h>


extern HMODULE LFCoreModuleHandle;
extern HANDLE Mutex_Stores;
extern LFMessageIDs LFMessages;
extern char KeyChars[38];


// Verzeichnis-Funktionen
//

__forceinline DWORD CreateDir(LPWSTR lpPath)
{
	return CreateDirectory(lpPath, NULL) ? ERROR_SUCCESS : GetLastError();
}

bool RemoveDir(LPWSTR lpPath)
{
	bool Result = true;

	// Dateien l�schen
	wchar_t DirSpec[MAX_PATH];
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
				wchar_t fn[MAX_PATH];
				wcscpy_s(fn, MAX_PATH, lpPath);
				wcscat_s(fn, MAX_PATH, FindFileData.cFileName);

				if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				{
					if ((wcscmp(FindFileData.cFileName, L".")!=0) && (wcscmp(FindFileData.cFileName, L"..")!=0))
					{
						wcscat_s(fn, MAX_PATH, L"\\");
						if (!RemoveDir(fn))
							Result = false;
					}
				}
				else
				{
					if (!DeleteFile(fn))
						Result = false;
				}
			}
		}
		while (FindNextFile(hFind, &FindFileData)!=0);

		FindClose(hFind);
	}

	// Verzeichnis l�schen
	if (Result)
		if (!RemoveDirectory(lpPath))
			Result = false;

	return Result;
}

unsigned int CopyDir(LPWSTR lpPathSrc, LPWSTR lpPathDst)
{
	wchar_t DirSpec[MAX_PATH];
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
				wchar_t fns[MAX_PATH];
				wcscpy_s(fns, MAX_PATH, lpPathSrc);
				wcscat_s(fns, MAX_PATH, FindFileData.cFileName);

				wchar_t fnd[MAX_PATH];
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

bool DirFreeSpace(LPWSTR lpPath, unsigned int Required)
{
	ULARGE_INTEGER FreeBytesAvailable;
	return GetDiskFreeSpaceEx(lpPath, &FreeBytesAvailable, NULL, NULL) ? FreeBytesAvailable.QuadPart>=Required : false;
}

bool DirWriteable(LPWSTR lpPath)
{
	wchar_t Filename[MAX_PATH];
	wcscpy_s(Filename, MAX_PATH, lpPath);
	wcscat_s(Filename, MAX_PATH, L"LF_TEST.BIN");

	HANDLE h = CreateFile(Filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h==INVALID_HANDLE_VALUE)
		return false;

	CloseHandle(h);
	return DeleteFile(Filename)==TRUE;
}

void SanitizeFileName(wchar_t* dst, size_t cCount, wchar_t* src)
{
	wcscpy_s(dst, cCount, src);

	while (*dst!=L'\0')
	{
		if ((*dst<L' ') || (wcschr(L"<>:\"/\\|?*", *dst)))
			*dst = L'_';
		dst++;
	}
}

unsigned int CreateStoreDirectories(LFStoreDescriptor* s)
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
		wchar_t tmpStr[MAX_PATH];
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

__forceinline void CreateStoreIndex(LFStoreDescriptor* s, bool UseMainIndex, unsigned int &Result)
{
	assert(s);

	CIndex idx(s, UseMainIndex);
	if (!idx.Create())
		Result = LFIndexCreateError;
}

void GetFilePath(wchar_t* DatPath, LFCoreAttributes* ca, wchar_t* dst, size_t cCount)
{
	wchar_t buf1[3] = L" \\";
	buf1[0] = ca->FileID[0];

	wchar_t buf2[LFKeySize-1];
	MultiByteToWideChar(CP_ACP, 0, &ca->FileID[1], -1, buf2, LFKeySize-1);

	wcscpy_s(dst, cCount, L"\\\\?\\");
	wcscat_s(dst, cCount, DatPath);
	wcscat_s(dst, cCount, buf1);
	wcscat_s(dst, cCount, buf2);
}

void GetFileLocation(wchar_t* DatPath, LFCoreAttributes* ca, wchar_t* dst, size_t cCount)
{
	wchar_t buf1[MAX_PATH];
	SanitizeFileName(buf1, MAX_PATH, ca->FileName);

	GetFilePath(DatPath, ca, dst, cCount);
	wcscat_s(dst, cCount, L"\\");
	wcsncat_s(dst, cCount, buf1, 127);

	if (ca->FileFormat[0]!='\0')
	{
		wchar_t buf2[LFExtSize];
		MultiByteToWideChar(CP_ACP, 0, ca->FileFormat, -1, buf2, LFExtSize);

		wcscat_s(dst, cCount, L".");
		wcscat_s(dst, cCount, buf2);
	}
}

bool FileExists(LPWSTR lpPath)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(lpPath, &FindFileData);

	bool Result = (hFind!=INVALID_HANDLE_VALUE);
	if (Result)
		FindClose(hFind);

	return Result;
}

unsigned int PrepareImport(LFStoreDescriptor* slot, LFItemDescriptor* i, wchar_t* Dst, size_t cCount)
{
	RANDOMIZE();

	SetAttribute(i, LFAttrStoreID, slot->StoreID);
	i->CoreAttributes.FileID[0] = RAND_CHAR();
	i->CoreAttributes.FileID[LFKeySize-1] = '\0';

	wchar_t Buf[3] = L" \\";
	Buf[0] = i->CoreAttributes.FileID[0];

	wchar_t Path[2*MAX_PATH];
	wcscpy_s(Path, 2*MAX_PATH, slot->DatPath);
	wcscat_s(Path, 2*MAX_PATH, Buf);

	DWORD Result = CreateDir(Path);
	if ((Result!=ERROR_SUCCESS) && (Result!=ERROR_ALREADY_EXISTS))
		return LFIllegalPhysicalPath;

	do
	{
		for (unsigned int a=1; a<LFKeySize-1; a++)
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

__forceinline void SendLFNotifyMessage(unsigned int Msg)
{
	SendNotifyMessage(HWND_BROADCAST, Msg, NULL, NULL);
}

void SendShellNotifyMessage(unsigned int Msg, char* StoreID, LPITEMIDLIST oldpidl, LPITEMIDLIST oldpidlDelegate)
{
	LPITEMIDLIST pidl;
	LPITEMIDLIST pidlDelegate;
	if (GetPIDLForStore(StoreID, &pidl, &pidlDelegate))
	{
		SHChangeNotify(Msg, SHCNF_IDLIST | SHCNF_FLUSH, oldpidl ? oldpidl : pidl, (Msg==SHCNE_RENAMEFOLDER) ? pidl : NULL);
		FreePIDL(pidl);

		if (pidlDelegate)
		{
			SHChangeNotify(Msg, SHCNF_IDLIST | SHCNF_FLUSH, oldpidlDelegate ? oldpidlDelegate : pidlDelegate, (Msg==SHCNE_RENAMEFOLDER) ? pidlDelegate : NULL);
			FreePIDL(pidlDelegate);
		}
	}
}


// Stores
//

LFCORE_API unsigned int LFGetFileLocation(LFItemDescriptor* i, wchar_t* dst, size_t cCount, bool CheckExists, bool RemoveNew, bool Extended)
{
	if ((i->Type & LFTypeMask)!=LFTypeFile)
		return LFIllegalItemType;

	if (i->CoreAttributes.Flags & LFFlagLink)
	{
		*dst = L'\0';
		return LFOk;
	}

	bool Changed = false;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	unsigned int Result = LFIllegalKey;
	LFStoreDescriptor* slot = FindStore(i->StoreID);

	if (slot)
		if (LFIsStoreMounted(slot))
		{
			wchar_t tmpPath[2*MAX_PATH];
			GetFileLocation(slot->DatPath, &i->CoreAttributes, tmpPath, 2*MAX_PATH);
			Result = LFOk;

			if (CheckExists || (RemoveNew && (i->CoreAttributes.Flags & LFFlagNew)))
			{
				bool Exists = FileExists(tmpPath);

				if ((Exists!=((i->CoreAttributes.Flags & LFFlagMissing)==0)) || (RemoveNew && (i->CoreAttributes.Flags & LFFlagNew)))
				{
					// Update index
					OPEN_STORE(i->StoreID, true, Result = Result);

					if (idx1)
						idx1->UpdateSystemFlags(i, Exists, RemoveNew);
					if (idx2)
						idx2->UpdateSystemFlags(i, Exists, RemoveNew);

					CLOSE_STORE();

					Changed = true;
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

	ReleaseMutex(Mutex_Stores);

	if (Changed)
		SendLFNotifyMessage(LFMessages.StatisticsChanged);

	return Result;
}

LFCORE_API unsigned int LFGetStoreSettings(char* StoreID, LFStoreDescriptor* s)
{
	assert(StoreID);

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	LFStoreDescriptor* slot = FindStore(StoreID[0]=='\0' ? DefaultStore : StoreID);
	if (slot)
		*s = *slot;

	ReleaseMutex(Mutex_Stores);
	return (slot ? LFOk : LFIllegalKey);
}

LFCORE_API unsigned int LFGetStoreSettings(GUID guid, LFStoreDescriptor* s)
{
	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	LFStoreDescriptor* slot = FindStore(guid);
	if (slot)
		*s = *slot;

	ReleaseMutex(Mutex_Stores);
	return (slot ? LFOk : LFIllegalKey);
}

LFCORE_API bool LFIsStoreMounted(LFStoreDescriptor* s)
{
	return s ? IsStoreMounted(s) : false;
}

LFCORE_API unsigned int LFGetStoreIcon(LFStoreDescriptor* s)
{
	return s ? s->Source+1 : IDI_STR_UNKNOWN;
}

LFCORE_API unsigned int LFCreateStore(LFStoreDescriptor* s)
{
	// GUID generieren
	CoCreateGuid(&s->guid);

	// Pfad erg�nzen
	AppendGUID(s, s->DatPath);

	SetStoreAttributes(s);

	// Ggf. Name setzen
	if (s->StoreName[0]==L'\0')
		LoadString(LFCoreModuleHandle, IDS_NEWSTORE, s->StoreName, 256);

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	// CreateTime und MaintenanceTime setzen
	GetSystemTimeAsFileTime(&s->CreationTime);
	s->MaintenanceTime = s->CreationTime;
	s->Flags &= ~LFStoreFlagUnchecked;

	// SynchronizeTime zur�cksetzen
	ZeroMemory(&s->SynchronizeTime, sizeof(FILETIME));

	// Key generieren
	CreateNewStoreID(s->StoreID);

	// Index-Version
	s->IndexVersion = CurIdxVersion;

	// Store speichern
	unsigned int Result = UpdateStore(s);
	if (Result==LFOk)
	{
		HANDLE StoreLock;
		if (GetMutexForStore(s, &StoreLock))
		{
			ReleaseMutex(Mutex_Stores);

			Result = CreateStoreDirectories(s);
			if (Result==LFOk)
			{
				CreateStoreIndex(s, true, Result);
				CreateStoreIndex(s, false, Result);
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
		ReleaseMutex(Mutex_Stores);
	}

	return Result;
}

LFCORE_API unsigned int LFMakeDefaultStore(char* StoreID)
{
	assert(StoreID);

	if (strcmp(StoreID, DefaultStore)==0)
		return LFOk;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	unsigned int Result = LFIllegalKey;
	LFStoreDescriptor* slot = FindStore(StoreID);

	if (slot)
		Result = MakeDefaultStore(slot);

	ReleaseMutex(Mutex_Stores);

	if (Result==LFOk)
		SendLFNotifyMessage(LFMessages.DefaultStoreChanged);

	return Result;
}

LFCORE_API unsigned int LFMakeStoreSearchable(char* StoreID, bool Searchable)
{
	assert(StoreID);

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	unsigned int Result = LFIllegalKey;
	HANDLE StoreLock = NULL;
	LFStoreDescriptor* slot = FindStore(StoreID, &StoreLock);
	if (!StoreLock)
	{
		ReleaseMutex(Mutex_Stores);
		return LFMutexError;
	}
	if (slot)
	{
		LFStoreDescriptor victim = *slot;

		#define EXIT(Result) { ReleaseMutexForStore(StoreLock); ReleaseMutex(Mutex_Stores); return Result; }

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
		ReleaseMutex(Mutex_Stores);
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
				wchar_t path[MAX_PATH];
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

LFCORE_API unsigned int LFSetStoreAttributes(char* StoreID, wchar_t* name, wchar_t* comment, bool InternalCall)
{
	assert(StoreID);

	if ((!name) && (!comment))
		return LFOk;
	if (name)
		if (name[0]==L'\0')
			return LFIllegalValue;

	LPITEMIDLIST oldpidl;
	LPITEMIDLIST oldpidlDelegate;
	GetPIDLForStore(StoreID, &oldpidl, &oldpidlDelegate);

	if (!GetMutex(Mutex_Stores))
	{
		FreePIDL(oldpidl);
		FreePIDL(oldpidlDelegate);
		return LFMutexError;
	}

	unsigned int Result = LFIllegalKey;
	LFStoreDescriptor* slot = FindStore(StoreID);
	if (slot)
	{
		if (name)
			wcscpy_s(slot->StoreName, 256, name);

		if (comment)
			wcscpy_s(slot->StoreComment, 256, comment);

		Result = UpdateStore(slot);
	}

	ReleaseMutex(Mutex_Stores);

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

	FreePIDL(oldpidl);
	FreePIDL(oldpidlDelegate);
	return Result;
}

LFCORE_API unsigned int LFDeleteStore(char* StoreID, LFProgress* pProgress)
{
	assert(StoreID);

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	unsigned int Result = LFIllegalKey;
	HANDLE StoreLock = NULL;
	LFStoreDescriptor* slot = FindStore(StoreID, &StoreLock);
	if (!StoreLock)
	{
		ReleaseMutex(Mutex_Stores);
		return LFMutexError;
	}
	if (slot)
	{
		// Progress
		if (pProgress)
		{
			pProgress->MinorCount = 2;
			pProgress->MinorCurrent = 0;
			pProgress->NoMinorCounter = true;
			wcscpy_s(pProgress->Object, 256, slot->StoreName);
			if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			{
				ReleaseMutex(Mutex_Stores);
				ReleaseMutexForStore(StoreLock);
				return LFCancel;
			}
		}

		// Delete
		if (slot->DatPath[0]!=L'\0')
		{
			wchar_t Path[MAX_PATH];
			wcscpy_s(Path, MAX_PATH, slot->DatPath);
			wcscat_s(Path, MAX_PATH, L"*");

			if (FileExists(Path))
				if (!DirWriteable(slot->DatPath))
				{
					ReleaseMutex(Mutex_Stores);
					ReleaseMutexForStore(StoreLock);
					return LFDriveWriteProtected;
				}
		}

		LFStoreDescriptor victim = *slot;
		Result = DeleteStore(slot);
		ReleaseMutex(Mutex_Stores);
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
				wchar_t path[MAX_PATH];
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
		ReleaseMutex(Mutex_Stores);
	}

	return Result;
}

unsigned int RunMaintenance(LFStoreDescriptor* s, bool Scheduled, LFProgress* pProgress)
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

	// Verzeichnisse pr�fen
	unsigned int Result = CreateStoreDirectories(s);
	if (Result!=LFOk)
		ABORT(Result);

	// Progress
	if (pProgress)
	{
		pProgress->MinorCurrent++;
		pProgress->NoMinorCounter = true;
		if (SendMessage(pProgress->hWnd, WM_UPDATEPROGRESS, (WPARAM)pProgress, NULL))
			return LFCancel;
	}

	// Index pr�fen
	CIndex* idx = new CIndex(s, (s->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexHybrid);

	bool Repaired = false;
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
		s->IndexVersion = CurIdxVersion;

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
	Result = UpdateStore(s, false);
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
	LFMaintenanceList* ml = LFAllocMaintenanceList();

	if (!GetMutex(Mutex_Stores))
	{
		ml->m_LastError = LFMutexError;
		return ml;
	}

	char* IDs;
	unsigned int count = FindStores(&IDs);
	ReleaseMutex(Mutex_Stores);

	if (count)
	{
		if (pProgress)
			pProgress->MajorCount = count;

		char* ptr = IDs;
		for (unsigned int a=0; a<count; a++)
		{
			if (!GetMutex(Mutex_Stores))
			{
				free(IDs);
				ml->m_LastError = LFMutexError;
				return ml;
			}

			HANDLE StoreLock = NULL;
			LFStoreDescriptor* slot = FindStore(ptr, &StoreLock);
			ReleaseMutex(Mutex_Stores);

			if ((!slot) || (!StoreLock))
				continue;

			ml->AddStore(RunMaintenance(slot, true, pProgress), slot->StoreName, ptr, LFGetStoreIcon(slot));
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

unsigned int OpenStore(char* StoreID, bool WriteAccess, CIndex* &Index1, CIndex* &Index2, LFStoreDescriptor** s, HANDLE* lock)
{
	Index1 = Index2 = NULL;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	LFStoreDescriptor* slot = FindStore(StoreID, lock);
	ReleaseMutex(Mutex_Stores);

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
		unsigned int Result = RunMaintenance(slot, false);
		if ((Result!=LFOk) && (Result!=LFDriveWriteProtected))
		{
			ReleaseMutexForStore(*lock);
			return Result;
		}
	}

	// Open index(es)
	if (WriteAccess)
	{
		Index1 = new CIndex(slot, true);
		if ((slot->Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid)
			Index2 = new CIndex(slot, false);
	}
	else
	{
		Index1 = new CIndex(slot, (slot->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexHybrid);
	}

	return LFOk;
}
