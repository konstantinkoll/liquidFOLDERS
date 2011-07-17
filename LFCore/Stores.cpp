
#include "stdafx.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "Mutex.h"
#include "PIDL.h"
#include "ShellProperties.h"
#include "Stores.h"
#include "StoreCache.h"
#include <assert.h>
#include <io.h>
#include <iostream>
#include <malloc.h>
#include <objbase.h>
#include <shellapi.h>
#include <sys/types.h>
#include <sys/stat.h>


extern HMODULE LFCoreModuleHandle;
extern HANDLE Mutex_Stores;
extern LFMessageIDs LFMessages;

static char KeyChars[38] = { LFKeyChars };


// Verzeichnis-Funktionen
//

inline DWORD CreateDir(LPWSTR lpPath)
{
	return CreateDirectory(lpPath, NULL);
}

bool RemoveDir(LPWSTR lpPath)
{
	bool res = true;

	// Dateien löschen
	wchar_t DirSpec[MAX_PATH];
	wcscpy_s(DirSpec, MAX_PATH, lpPath);
	wcscat_s(DirSpec, MAX_PATH, L"*");

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(DirSpec, &FindFileData);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
FileFound:
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
						res = false;
				}
			}
			else
			{
				if (!DeleteFile(fn))
					res = false;
			}
		}

		if (FindNextFile(hFind, &FindFileData)!=0)
			goto FileFound;

		FindClose(hFind);
	}

	// Verzeichnis löschen
	if (res)
		if (!RemoveDirectory(lpPath))
			res = false;

	return res;
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
FileFound:
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

		if (FindNextFile(hFind, &FindFileData)!=0)
			goto FileFound;

		FindClose(hFind);
	}

	return LFOk;
}

bool DirFreeSpace(LPWSTR lpPathSrc, unsigned int Required)
{
	ULARGE_INTEGER FreeBytesAvailable;
	if (!GetDiskFreeSpaceEx(lpPathSrc, &FreeBytesAvailable, NULL, NULL))
		return false;

	return FreeBytesAvailable.QuadPart>=Required;
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

unsigned int ValidateStoreDirectories(LFStoreDescriptor* s)
{
	// Store phys. anlegen
	if (s->DatPath[0]!=L'\0')
	{
		DWORD res = CreateDir(s->DatPath);
		if ((res!=ERROR_SUCCESS) && (res!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;
	}

	if (s->DatPath[0]!=L'\0')
	{
		DWORD res = CreateDir(s->IdxPathMain);
		if ((res!=ERROR_SUCCESS) && (res!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;

		// Store auf externen Medien verstecken
		if ((s->StoreMode!=LFStoreModeInternal) && (res==ERROR_SUCCESS))
			SetFileAttributes(s->DatPath, FILE_ATTRIBUTE_HIDDEN);
	}

	// Hilfsindex anlegen
	if (s->StoreMode==LFStoreModeHybrid)
	{
		wchar_t tmpStr[MAX_PATH];
		GetAutoPath(s, tmpStr);
		DWORD res = CreateDir(tmpStr);
		if ((res!=ERROR_SUCCESS) && (res!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;
		res = CreateDir(s->IdxPathAux);
		if ((res!=ERROR_SUCCESS) && (res!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;
	}

	return LFOk;
}

void CreateStoreIndex(wchar_t* _Path, char* _StoreID, wchar_t* _DatPath, unsigned int &res)
{
	if (_Path[0]==L'\0')
		return;

	CIndex idx(_Path, _StoreID, _DatPath);
	if (!idx.Create())
		res = LFIndexCreateError;
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

	bool res = (hFind!=INVALID_HANDLE_VALUE);
	if (res)
		FindClose(hFind);

	return res;
}

unsigned int PrepareImport(LFStoreDescriptor* slot, LFItemDescriptor* i, wchar_t* Dst, size_t cCount)
{
#define RandChar KeyChars[rand()%sizeof(KeyChars)]

	SYSTEMTIME st;
	GetSystemTime(&st);
	srand(st.wMilliseconds*rand());

	SetAttribute(i, LFAttrStoreID, slot->StoreID);
	i->CoreAttributes.FileID[0] = RandChar;
	i->CoreAttributes.FileID[LFKeyLength] = '\0';

	wchar_t Buf[3] = L" \\";
	Buf[0] = i->CoreAttributes.FileID[0];

	wchar_t Path[2*MAX_PATH];
	wcscpy_s(Path, 2*MAX_PATH, slot->DatPath);
	wcscat_s(Path, 2*MAX_PATH, Buf);

	DWORD res = CreateDir(Path);
	if ((res!=ERROR_SUCCESS) && (res!=ERROR_ALREADY_EXISTS))
		return LFIllegalPhysicalPath;

	do
	{
		for (unsigned int a=1; a<LFKeyLength; a++)
			i->CoreAttributes.FileID[a] = RandChar;

		GetFilePath(slot->DatPath, &i->CoreAttributes, Path, 2*MAX_PATH);
	}
	while (FileExists(Path));

	res = CreateDir(Path);
	if ((res!=ERROR_SUCCESS) && (res!=ERROR_ALREADY_EXISTS))
		return LFIllegalPhysicalPath;

	GetFileLocation(slot->DatPath, &i->CoreAttributes, Dst, cCount);
	return LFOk;
}

void SendLFNotifyMessage(unsigned int Msg, unsigned int Flags, HWND hWndSource)
{
	SendNotifyMessage(HWND_BROADCAST, Msg, (WPARAM)Flags, (LPARAM)hWndSource);
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

LFCore_API unsigned int LFGetFileLocation(LFItemDescriptor* i, wchar_t* dst, size_t cCount, bool CheckExists, bool Extended)
{
	if (((i->Type & LFTypeMask)!=LFTypeFile) || (i->StoreID[0]=='\0'))
		return LFIllegalKey;

	if (i->CoreAttributes.Flags & LFFlagLink)
	{
		*dst = L'\0';
		return LFOk;
	}

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	unsigned int res = LFIllegalKey;
	LFStoreDescriptor* slot = FindStore(i->StoreID);

	if (slot)
		if (IsStoreMounted(slot))
		{
			wchar_t tmpPath[2*MAX_PATH];
			GetFileLocation(slot->DatPath, &i->CoreAttributes, tmpPath, 2*MAX_PATH);
			res = LFOk;

			if (CheckExists)
			{
				unsigned int Flags = i->CoreAttributes.Flags;
				bool Exists = FileExists(tmpPath);

				if ((!Exists) || (Flags & (LFFlagNew | LFFlagMissing)))
				{
					// Update index
					CIndex* idx1;
					CIndex* idx2;
					HANDLE StoreLock = NULL;
					if (OpenStore(i->StoreID, true, idx1, idx2, NULL, &StoreLock)==LFOk)
					{
						if (idx1)
						{
							if (!idx1->UpdateMissing(i, Exists))
								res = LFNoFileBody;
							delete idx1;
						}
						if (idx2)
						{
							idx2->Update(i, false);
							delete idx2;
						}
						ReleaseMutexForStore(StoreLock);
					}
				}
			}

			if (res==LFOk)
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
			res = LFStoreNotMounted;
		}

	ReleaseMutex(Mutex_Stores);
	return res;
}

LFCore_API unsigned int LFGetStoreSettings(char* key, LFStoreDescriptor* s)
{
	if (!key)
		return LFIllegalKey;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	LFStoreDescriptor* slot = FindStore(key[0]=='\0' ? DefaultStore : key);
	if (slot)
		*s = *slot;

	ReleaseMutex(Mutex_Stores);
	return (slot ? LFOk : LFIllegalKey);
}

LFCore_API unsigned int LFGetStoreSettings(GUID guid, LFStoreDescriptor* s)
{
	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	LFStoreDescriptor* slot = FindStore(guid);
	if (slot)
		*s = *slot;

	ReleaseMutex(Mutex_Stores);
	return (slot ? LFOk : LFIllegalKey);
}

LFCore_API unsigned int LFCreateStore(LFStoreDescriptor* s, bool MakeDefault, HWND hWndSource)
{
	// GUID generieren
	CoCreateGuid(&s->guid);

	// Pfad ergänzen
	AppendGUID(s, s->DatPath);

	unsigned int res = ValidateStoreSettings(s);
	if (res!=LFOk)
		return res;

	// Ggf. Name setzen
	if (!s->StoreName[0])
		LoadString(LFCoreModuleHandle, IDS_StoreDefaultName, s->StoreName, 256);

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	// CreateTime und MaintenanceTime setzen
	GetSystemTimeAsFileTime(&s->CreationTime);
	s->MaintenanceTime = s->CreationTime;
	s->NeedsCheck = false;

	// Key generieren
	CreateStoreKey(s->StoreID);

	// Index-Version
	s->IndexVersion = CurIdxVersion;

	// Store speichern
	res = UpdateStore(s, MakeDefault);

	if (res==LFOk)
	{
		HANDLE StoreLock;
		if (GetMutexForStore(s, &StoreLock))
		{
			ReleaseMutex(Mutex_Stores);

			res = ValidateStoreDirectories(s);
			if (res==LFOk)
			{
				CreateStoreIndex(s->IdxPathMain, s->StoreID, s->DatPath, res);
				CreateStoreIndex(s->IdxPathAux, s->StoreID, s->DatPath, res);
			}

			ReleaseMutex(StoreLock);
			SendLFNotifyMessage(LFMessages.StoresChanged, s->StoreMode==LFStoreModeInternal ? LFMSGF_IntStores : LFMSGF_ExtHybStores , hWndSource);
			SendShellNotifyMessage(SHCNE_UPDATEDIR);
		}
		else
		{
			res = LFMutexError;
		}
	}
	else
	{
		ReleaseMutex(Mutex_Stores);
	}

	return res;
}

LFCore_API unsigned int LFMakeDefaultStore(char* key, HWND hWndSource, bool InternalCall)
{
	if (!key)
		return LFIllegalKey;
	if (key[0]=='\0')
		return LFIllegalKey;

	if (!InternalCall)
		if (!GetMutex(Mutex_Stores))
			return LFMutexError;

	unsigned int res = LFIllegalKey;
	LFStoreDescriptor* slot = FindStore(key);

	char OldDefaultStore[LFKeySize];
	strcpy_s(OldDefaultStore, LFKeySize, DefaultStore);

	if (slot)
		if (slot->StoreMode!=LFStoreModeInternal)
		{
			res = LFIllegalStoreDescriptor;
		}
		else
		{
			res = LFRegistryError;

			HKEY k;
			if (RegOpenKeyA(HKEY_CURRENT_USER, LFStoresHive, &k)==ERROR_SUCCESS)
			{
				if (RegSetValueExA(k, "DefaultStore", 0, REG_SZ, (BYTE*)key, (DWORD)strlen(key))==ERROR_SUCCESS)
				{
					strcpy_s(DefaultStore, LFKeySize, key);
					res = LFOk;
				}
				RegCloseKey(k);
			}
		}

	if (!InternalCall)
	{
		ReleaseMutex(Mutex_Stores);

		if (res==LFOk)
			SendLFNotifyMessage(LFMessages.DefaultStoreChanged, LFMSGF_IntStores, hWndSource);
	}

	SendShellNotifyMessage(SHCNE_UPDATEITEM, DefaultStore);
	if ((strcmp(DefaultStore, OldDefaultStore)!=0) && (OldDefaultStore[0]!='\0'))
		SendShellNotifyMessage(SHCNE_UPDATEITEM, OldDefaultStore);

	return res;
}

LFCore_API unsigned int LFMakeHybridStore(char* key, HWND hWndSource)
{
	if (!key)
		return LFIllegalKey;
	if (key[0]=='\0')
		return LFIllegalKey;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	unsigned int res = LFIllegalKey;
	HANDLE StoreLock = NULL;
	LFStoreDescriptor* slot = FindStore(key, &StoreLock);
	if (slot)
	{
		if ((slot->StoreMode!=LFStoreModeExternal) || (!IsStoreMounted(slot)))
		{
			ReleaseMutexForStore(StoreLock);
			ReleaseMutex(Mutex_Stores);
			return LFIllegalStoreDescriptor;
		}

		slot->StoreMode = LFStoreModeHybrid;
		res = ValidateStoreSettings(slot);
		if (res!=LFOk)
		{
			ReleaseMutexForStore(StoreLock);
			ReleaseMutex(Mutex_Stores);
			return res;
		}

		#define CheckAbort if (res!=LFOk) { ReleaseMutexForStore(StoreLock); return res; }

		res = UpdateStore(slot);
		ReleaseMutex(Mutex_Stores);
		CheckAbort

		res = ValidateStoreDirectories(slot);
		CheckAbort

		res = CopyDir(slot->IdxPathMain, slot->IdxPathAux);
		CheckAbort
	}

	ReleaseMutexForStore(StoreLock);

	if (res==LFOk)
	{
		SendLFNotifyMessage(LFMessages.StoreAttributesChanged, LFMSGF_ExtHybStores, hWndSource);
		SendShellNotifyMessage(SHCNE_UPDATEITEM, key);
		SendShellNotifyMessage(SHCNE_UPDATEDIR);
	}

	return res;
}

LFCore_API unsigned int LFSetStoreAttributes(char* key, wchar_t* name, wchar_t* comment, HWND hWndSource, bool InternalCall)
{
	if (!key)
		return LFIllegalKey;
	if (key[0]=='\0')
		return LFIllegalKey;
	if ((!name) && (!comment))
		return LFOk;
	if (name)
		if (wcscmp(name, L"")==0)
			return LFIllegalValue;

	LPITEMIDLIST oldpidl;
	LPITEMIDLIST oldpidlDelegate;
	GetPIDLForStore(key, &oldpidl, &oldpidlDelegate);

	if (!GetMutex(Mutex_Stores))
	{
		FreePIDL(oldpidl);
		FreePIDL(oldpidlDelegate);
		return LFMutexError;
	}

	unsigned int res = LFIllegalKey;
	LFStoreDescriptor* slot = FindStore(key);
	if (slot)
	{
		if (name)
			if (wcscmp(name, slot->StoreName)==0)
				name = NULL;
		if (name)
			wcscpy_s(slot->StoreName, 256, name);

		if (comment)
			if (wcscmp(comment, slot->Comment)==0)
				comment = NULL;
		if (comment)
			wcscpy_s(slot->Comment, 256, comment);

		res = (name || comment) ? UpdateStore(slot) : LFOk;
	}

	unsigned int Mode = slot->StoreMode;
	ReleaseMutex(Mutex_Stores);

	if (res==LFOk)
	{
		if (!InternalCall)
			SendLFNotifyMessage(LFMessages.StoreAttributesChanged, Mode==LFStoreModeInternal ? LFMSGF_IntStores : LFMSGF_ExtHybStores, hWndSource);
		if (comment)
			SendShellNotifyMessage(SHCNE_UPDATEITEM, key);
		if (name)
		{
			SendShellNotifyMessage(SHCNE_RENAMEFOLDER, key, oldpidl);
			SendShellNotifyMessage(SHCNE_RENAMEFOLDER, key, oldpidlDelegate);
			SendShellNotifyMessage(SHCNE_UPDATEDIR);
		}
	}

	FreePIDL(oldpidl);
	FreePIDL(oldpidlDelegate);
	return res;
}

LFCore_API unsigned int LFDeleteStore(char* key, HWND hWndSource)
{
	if (!key)
		return LFIllegalKey;
	if (key[0]=='\0')
		return LFIllegalKey;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	unsigned int res = LFIllegalKey;
	HANDLE StoreLock = NULL;
	LFStoreDescriptor* slot = FindStore(key, &StoreLock);
	if ((slot) && (StoreLock))
	{
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
		res = DeleteStore(slot);
		ReleaseMutex(Mutex_Stores);
		ReleaseMutexForStore(StoreLock);

		if (res==LFOk)
		{
			SendLFNotifyMessage(LFMessages.StoresChanged, victim.StoreMode==LFStoreModeInternal ? LFMSGF_IntStores : LFMSGF_ExtHybStores, hWndSource);
			SendShellNotifyMessage(SHCNE_UPDATEDIR);

			if (victim.IdxPathAux[0]!=L'\0')
			{
				wchar_t path[MAX_PATH];
				GetAutoPath(&victim, path);
				RemoveDir(path);
			}

			if (victim.DatPath[0]!=L'\0')
				RemoveDir(victim.DatPath);
		}
	}
	else
	{
		if (slot)
			res = LFMutexError;

		ReleaseMutex(Mutex_Stores);
	}

	return res;
}

LFCore_API bool LFAskDeleteStore(LFItemDescriptor* s, HWND hWnd)
{
	wchar_t caption[256];
	wchar_t tmp[256];
	LoadString(LFCoreModuleHandle, IDS_DeleteStoreCaption, tmp, 256);
	swprintf_s(caption, 256, tmp, s->CoreAttributes.FileName);

	wchar_t msg[256];
	LoadString(LFCoreModuleHandle, IDS_DeleteStoreMessage, msg, 256);

	return MessageBox(hWnd, msg, caption, MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING)==IDYES;
}

LFCore_API bool LFAskDeleteStore(LFStoreDescriptor* s, HWND hWnd)
{
	wchar_t caption[256];
	wchar_t tmp[256];
	LoadString(LFCoreModuleHandle, IDS_DeleteStoreCaption, tmp, 256);
	swprintf_s(caption, 256, tmp, s->StoreName);

	wchar_t msg[256];
	LoadString(LFCoreModuleHandle, IDS_DeleteStoreMessage, msg, 256);

	return MessageBox(hWnd, msg, caption, MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING)==IDYES;
}

unsigned int RunMaintenance(LFStoreDescriptor* s, bool scheduled)
{
	// Verzeichnisse prüfen
	unsigned int res = ValidateStoreDirectories(s);
	if (res!=LFOk)
		return res;

	// Sind die Verzeichnisse beschreibbar?
	if (s->IdxPathMain[0]!='\0')
		if (!DirWriteable(s->IdxPathMain))
			return LFDriveWriteProtected;
	if (s->IdxPathAux[0]!='\0')
		if (!DirWriteable(s->IdxPathAux))
			return LFDriveWriteProtected;

	// Index prüfen
	CIndex* idx = new CIndex((s->StoreMode!=LFStoreModeHybrid) ? s->IdxPathMain : s->IdxPathAux, s->StoreID, s->DatPath);
	switch (idx->Check(scheduled))
	{
	case IndexNoAccess:
		delete idx;
		return LFIndexAccessError;
	case IndexCannotCreate:
		delete idx;
		return LFIndexCreateError;
	case IndexNotEnoughFreeDiscSpace:
		delete idx;
		return LFNotEnoughFreeDiscSpace;
	case IndexCompleteReindexRequired:
		// TODO
	case IndexError:
		delete idx;
		return LFIndexRepairError;
	case IndexFullyRepaired:
		s->IndexVersion = CurIdxVersion;

		res = UpdateStore(s);
		if (res!=LFOk)
		{
			delete idx;
			return res;
		}
	}

	delete idx;

	// Index duplizieren
	if ((s->StoreMode==LFStoreModeHybrid) && IsStoreMounted(s))
	{
		res = CopyDir(s->IdxPathAux, s->IdxPathMain);
		if (res!=LFOk)
			return res;
	}

	GetSystemTimeAsFileTime(&s->MaintenanceTime);
	s->NeedsCheck = false;

	return LFOk;
}

LFCore_API LFMaintenanceList* LFStoreMaintenance(char* key, HWND hWndSource)
{
	LFMaintenanceList* ml = LFAllocMaintenanceList();

	if (!GetMutex(Mutex_Stores))
	{
		ml->m_LastError = LFMutexError;
		return ml;
	}

	HANDLE StoreLock = NULL;
	LFStoreDescriptor* slot = FindStore(key, &StoreLock);
	ReleaseMutex(Mutex_Stores);

	if (!slot)
	{
		ml->m_LastError = LFIllegalKey;
		return ml;
	}
	if (!StoreLock)
	{
		ml->m_LastError = LFMutexError;
		return ml;
	}

	ml->AddStore(RunMaintenance(slot, true), slot->StoreName, key, slot->StoreMode==LFStoreModeInternal ? IDI_STORE_Internal : slot->StoreMode==LFStoreModeRemote ? IDI_STORE_Server : IDI_STORE_Bag);
	ReleaseMutexForStore(StoreLock);

	SendLFNotifyMessage(LFMessages.StoreAttributesChanged, slot->StoreMode==LFStoreModeInternal ? LFMSGF_IntStores : LFMSGF_ExtHybStores, hWndSource);

	return ml;
}

LFCore_API LFMaintenanceList* LFStoreMaintenance(HWND hWndSource)
{
	LFMaintenanceList* ml = LFAllocMaintenanceList();

	if (!GetMutex(Mutex_Stores))
	{
		ml->m_LastError = LFMutexError;
		return ml;
	}

	char* keys;
	unsigned int count = FindStores(&keys);
	ReleaseMutex(Mutex_Stores);

	if (count)
	{
		char* ptr = keys;
		for (unsigned int a=0; a<count; a++)
		{
			if (!GetMutex(Mutex_Stores))
			{
				free(keys);
				ml->m_LastError = LFMutexError;
				return ml;
			}

			HANDLE StoreLock = NULL;
			LFStoreDescriptor* slot = FindStore(ptr, &StoreLock);
			ReleaseMutex(Mutex_Stores);

			if ((!slot) || (!StoreLock))
				continue;

			ml->AddStore(RunMaintenance(slot, true), slot->StoreName, ptr, slot->StoreMode==LFStoreModeInternal ? IDI_STORE_Internal : slot->StoreMode==LFStoreModeRemote ? IDI_STORE_Server : IDI_STORE_Bag);
			ReleaseMutexForStore(StoreLock);

			ptr += LFKeySize;
		}

		free(keys);
	}

	SendLFNotifyMessage(LFMessages.StoreAttributesChanged, LFMSGF_IntStores | LFMSGF_ExtHybStores, hWndSource);

	return ml;
}

unsigned int OpenStore(LFStoreDescriptor* s, bool WriteAccess, CIndex* &Index1, CIndex* &Index2)
{
	Index1 = Index2 = NULL;

	if (WriteAccess && !IsStoreMounted(s))
		return LFStoreNotMounted;

	// Einfache Wartungsarbeiten
	if (s->NeedsCheck)
	{
		unsigned int res = RunMaintenance(s, false);
		if ((res!=LFOk) && (res!=LFDriveWriteProtected))
			return res;
	}

	// Index öffnen
	if (WriteAccess)
	{
		Index1 = new CIndex(s->IdxPathMain, s->StoreID, s->DatPath);
		if (s->StoreMode==LFStoreModeHybrid)
			Index2 = new CIndex(s->IdxPathAux, s->StoreID, s->DatPath);
	}
	else
	{
		Index1 = new CIndex(s->StoreMode==LFStoreModeHybrid ? s->IdxPathAux : s->IdxPathMain, s->StoreID, s->DatPath);
	}

	return LFOk;
}

unsigned int OpenStore(char* key, bool WriteAccess, CIndex* &Index1, CIndex* &Index2, LFStoreDescriptor** s, HANDLE* lock)
{
	Index1 = Index2 = NULL;

	if (!key)
		return LFIllegalKey;
	if (key[0]=='\0')
		return LFIllegalKey;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	LFStoreDescriptor* slot = FindStore(key, lock);
	ReleaseMutex(Mutex_Stores);

	if (!slot)
		return LFIllegalKey;
	if (!*lock)
		return LFMutexError;

	if (s)
		*s = slot;

	unsigned int res = OpenStore(slot, WriteAccess, Index1, Index2);
	if (res!=LFOk)
		ReleaseMutexForStore(*lock);

	return res;
}

LFCore_API unsigned int LFImportFiles(char* key, LFFileImportList* il, LFItemDescriptor* it, bool recursive, bool move)
{
	assert(il);

	// Store finden
	char store[LFKeySize] = "";
	if (key)
		strcpy_s(store, LFKeySize, key);

	if (store[0]=='\0')
		if (GetMutex(Mutex_Stores))
		{
			strcpy_s(store, LFKeySize, DefaultStore);
			ReleaseMutex(Mutex_Stores);
		}
		else
			return LFMutexError;

	if (store[0]=='\0')
	{
		il->m_LastError = LFNoDefaultStore;
		return LFNoDefaultStore;
	}

	// Importliste vorbereiten
	il->Resolve(recursive);

	// Import
	CIndex* idx1;
	CIndex* idx2;
	LFStoreDescriptor* slot;
	HANDLE StoreLock = NULL;
	unsigned int res = OpenStore(store, true, idx1, idx2, &slot, &StoreLock);
	if (res==LFOk)
	{
		for (unsigned int a=0; a<il->m_ItemCount; a++)
			if (il->m_Items[a])
			{
				LFItemDescriptor* i = LFAllocItemDescriptor(it);
				i->CoreAttributes.Flags = LFFlagNew;
				SetNameExtAddFromFile(i, il->m_Items[a]);
				SetAttributesFromFile(i, il->m_Items[a]);

				wchar_t Path[2*MAX_PATH];
				res = PrepareImport(slot, i, Path, 2*MAX_PATH);
				if (res!=LFOk)
				{
					LFFreeItemDescriptor(i);
					break;
				}

				BOOL shres = move ? MoveFile(il->m_Items[a], Path) : CopyFile(il->m_Items[a], Path, FALSE);
				if (!shres)
				{
					wchar_t* LastBackslash = wcsrchr(Path, L'\\');
					if (LastBackslash)
						*(LastBackslash+1) = L'\0';

					RemoveDir(Path);

					LFFreeItemDescriptor(i);
					res = LFIllegalPhysicalPath;
					break;
				}

				il->m_FileCount++;
				il->m_FileSize += i->CoreAttributes.FileSize;

				if (idx1)
					idx1->AddItem(i);
				if (idx2)
					idx2->AddItem(i);
				LFFreeItemDescriptor(i);
			}

		if (idx1)
			delete idx1;
		if (idx2)
			delete idx2;
		ReleaseMutexForStore(StoreLock);
	}

	il->m_LastError = res;
	return res;
}
