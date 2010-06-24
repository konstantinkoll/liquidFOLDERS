#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFItemDescriptor.h"
#include "Mutex.h"
#include "ShellProperties.h"
#include "Stores.h"
#include "StoreCache.h"
#include <assert.h>
#include <io.h>
#include <iostream>
#include <malloc.h>
#include <objbase.h>
#include <shellapi.h>
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>


extern HMODULE LFCoreModuleHandle;
extern HANDLE Mutex_Stores;
extern LFMessageIDs LFMessages;

static char KeyChars[38] = { LFKeyChars };


// Verzeichnis-Funktionen
//

DWORD CreateDir(LPCSTR lpPath)
{
	SECURITY_ATTRIBUTES sa;
	SECURITY_DESCRIPTOR sd;
	PACL pAcl = NULL;
	DWORD cbAcl = 0;
	DWORD dwNeeded = 0;
	DWORD dwError = 0;
	HANDLE hToken;
	PTOKEN_USER ptu = NULL;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
		return GetLastError();

	GetTokenInformation( hToken, TokenUser, NULL, 0, &dwNeeded);
	if (GetLastError()!=ERROR_INSUFFICIENT_BUFFER)
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	ptu = (TOKEN_USER*)malloc(dwNeeded);
	if (!GetTokenInformation(hToken, TokenUser, ptu, dwNeeded, &dwNeeded))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	cbAcl = sizeof(ACL)+((sizeof(ACCESS_ALLOWED_ACE)-sizeof(DWORD))+GetLengthSid(ptu->User.Sid));
	pAcl = (ACL*)malloc(cbAcl);

	if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	if (!AddAccessAllowedAce(pAcl, ACL_REVISION,GENERIC_ALL | STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL, ptu->User.Sid))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	InitializeSecurityDescriptor(&sd, SECURITY_DESCRIPTOR_REVISION);
	SetSecurityDescriptorDacl(&sd, TRUE, pAcl, FALSE);
	SetSecurityDescriptorOwner(&sd, ptu->User.Sid, FALSE);
	SetSecurityDescriptorGroup(&sd, NULL, FALSE); 
	SetSecurityDescriptorSacl(&sd, FALSE, NULL, FALSE);

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = &sd;
	sa.bInheritHandle = TRUE;

	CreateDirectoryA(lpPath, &sa);
	dwError = GetLastError();

Cleanup:
	if (ptu)
		free(ptu);
	if (pAcl)
		free(pAcl);

	CloseHandle(hToken);
	return dwError;
}

void RemoveDir(LPCSTR lpPath)
{
	// Dateien löschen
	char DirSpec[MAX_PATH];
	strcpy_s(DirSpec, MAX_PATH, lpPath);
	strcat_s(DirSpec, "*");

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = FindFirstFileA(DirSpec, &FindFileData);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
FileFound:
		if ((FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_VIRTUAL))==0)
		{
			char fn[MAX_PATH];
			strcpy_s(fn, MAX_PATH, lpPath);
			strcat_s(fn, MAX_PATH, FindFileData.cFileName);
			DeleteFileA(fn);
		}

		if (FindNextFileA(hFind, &FindFileData)!=0)
			goto FileFound;

		FindClose(hFind);
	}

	// Verzeichnis löschen
	RemoveDirectoryA(lpPath);
}

unsigned int CopyDir(LPCSTR lpPathSrc, LPCSTR lpPathDst)
{
	char DirSpec[MAX_PATH];
	strcpy_s(DirSpec, MAX_PATH, lpPathSrc);
	strcat_s(DirSpec, MAX_PATH, "*");

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = FindFirstFileA(DirSpec, &FindFileData);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
FileFound:
		if ((FindFileData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE | FILE_ATTRIBUTE_VIRTUAL))==0)
		{
			char fns[MAX_PATH];
			strcpy_s(fns, MAX_PATH, lpPathSrc);
			strcat_s(fns, MAX_PATH, FindFileData.cFileName);

			char fnd[MAX_PATH];
			strcpy_s(fnd, MAX_PATH, lpPathDst);
			strcat_s(fnd, MAX_PATH, FindFileData.cFileName);

			if (!CopyFileA(fns, fnd, FALSE))
			{
				FindClose(hFind);
				return LFCannotCopyIndex;
			}
		}

		if (FindNextFileA(hFind, &FindFileData)!=0)
			goto FileFound;

		FindClose(hFind);
	}

	return LFOk;
}

bool DirFreeSpace(LPCSTR lpPathSrc, unsigned int Required)
{
	ULARGE_INTEGER FreeBytesAvailable;
	if (!GetDiskFreeSpaceExA(lpPathSrc, &FreeBytesAvailable, NULL, NULL))
		return false;

	return FreeBytesAvailable.QuadPart>=Required;
}

bool DirWriteable(LPCSTR lpPath)
{
	char Filename[MAX_PATH];
	strcpy_s(Filename, MAX_PATH, lpPath);
	strcat_s(Filename, MAX_PATH, "LF_TEST.BIN");

	HANDLE h = CreateFileA(Filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (h==INVALID_HANDLE_VALUE)
		return false;

	CloseHandle(h);
	return DeleteFileA(Filename)==TRUE;
}

unsigned int ValidateStoreDirectories(LFStoreDescriptor* s)
{
	// Store phys. anlegen
	if (s->DatPath[0]!='\0')
	{
		DWORD res = CreateDir(s->DatPath);
			if ((res!=ERROR_SUCCESS) && (res!=ERROR_ALREADY_EXISTS))
				return LFIllegalPhysicalPath;
	}

	if (s->DatPath[0]!='\0')
	{
		DWORD res = CreateDir(s->IdxPathMain);
			if ((res!=ERROR_SUCCESS) && (res!=ERROR_ALREADY_EXISTS))
				return LFIllegalPhysicalPath;

		// Store auf externen Medien verstecken
		if ((s->StoreMode!=LFStoreModeInternal) && (res==ERROR_SUCCESS))
			SetFileAttributesA(s->DatPath, FILE_ATTRIBUTE_HIDDEN);
	}

	// Hilfsindex anlegen
	if (s->StoreMode==LFStoreModeHybrid)
	{
		char tmpStr[MAX_PATH];
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

void CreateStoreIndex(char* _Path, char* _StoreID, unsigned int &res)
{
	if (_Path[0]=='\0')
		return;

	CIndex idx(_Path, _StoreID);
	if (!idx.Create())
		res = LFIndexNotCreated;
}

void GetFileLocation(char* _Path, char* _FileID, char* _FileFormat, char* dst, size_t cCount)
{
	char buf[3] = " \\";
	buf[0] = _FileID[0];

	strcpy_s(dst, cCount, _Path);
	strcat_s(dst, cCount, buf);
	strcat_s(dst, cCount, &_FileID[1]);

	if (_FileFormat[0]!='\0')
	{
		strcat_s(dst, cCount, ".");
		strcat_s(dst, cCount, _FileFormat);
	}
}

bool FileExists(char* path)
{
	return (_access(path, 0)==0);
}

unsigned int PrepareImport(LFStoreDescriptor* slot, LFItemDescriptor* i, char* Dst, size_t cCount)
{
	SetAttribute(i, LFAttrStoreID, slot->StoreID);

	char Path[MAX_PATH];

	SYSTEMTIME st;
	GetSystemTime(&st);
	srand(st.wMilliseconds*rand());

ChooseAgain:
	for (unsigned int a=0; a<LFKeyLength; a++)
	{
		int r = rand()%sizeof(KeyChars);
		i->CoreAttributes.FileID[a] = KeyChars[r];
	}

	i->CoreAttributes.FileID[LFKeyLength] = 0;
	GetFileLocation(slot->DatPath, i->CoreAttributes.FileID, "*", Path, MAX_PATH);

	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind = FindFirstFileA(Path, &FindFileData);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
		FindClose(hFind);
		goto ChooseAgain;
	}

	if (Dst)
	{
		GetFileLocation(slot->DatPath, i->CoreAttributes.FileID, i->CoreAttributes.FileFormat, Path, MAX_PATH);
		strcpy_s(Dst, cCount, Path);
	}

	*strrchr(Path, '\\') = '\0';
	DWORD res = CreateDir(Path);
	return ((res==ERROR_SUCCESS) || (res==ERROR_ALREADY_EXISTS)) ? LFOk : LFIllegalPhysicalPath;
}


// Stores
//

LFCore_API unsigned int LFGetFileLocation(LFItemDescriptor* i, char* dst, size_t cCount)
{
	if ((i->Type & LFTypeMask)!=LFTypeFile)
		return LFIllegalKey;
	if (i->StoreID=='\0')
		return LFIllegalKey;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	unsigned int res = LFIllegalKey;
	LFStoreDescriptor* slot = FindStore(i->StoreID);

	if (slot)
		if (IsStoreMounted(slot))
		{
			GetFileLocation(slot->DatPath, i->CoreAttributes.FileID, i->CoreAttributes.FileFormat, dst, cCount);
			res = LFOk;
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
	if (key[0]=='\0')
		return LFIllegalKey;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	LFStoreDescriptor* slot = FindStore(key);
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
		LoadStringW(LFCoreModuleHandle, IDS_StoreDefaultName, s->StoreName, 256);

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	// CreateTime und MaintenanceTime setzen
	SYSTEMTIME st;
	GetLocalTime(&st);
	SystemTimeToFileTime(&st, &s->CreationTime);
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
				CreateStoreIndex(s->IdxPathMain, s->StoreID, res);
				CreateStoreIndex(s->IdxPathAux, s->StoreID, res);
			}

			ReleaseMutex(StoreLock);
			SendNotifyMessage(HWND_BROADCAST, LFMessages.StoresChanged, s->StoreMode==LFStoreModeInternal ? LFMSGF_IntStores : LFMSGF_ExtHybStores , (LPARAM)hWndSource);
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
			SendMessage(HWND_BROADCAST, LFMessages.DefaultStoreChanged, LFMSGF_IntStores, (LPARAM)hWndSource);
	}

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
		SendMessage(HWND_BROADCAST, LFMessages.StoreAttributesChanged, LFMSGF_ExtHybStores, (LPARAM)hWndSource);

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

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	unsigned int res = LFIllegalKey;
	LFStoreDescriptor* slot = FindStore(key);
	if (slot)
	{
		if (name)
			wcscpy_s(slot->StoreName, 256, name);
		if (comment)
			wcscpy_s(slot->Comment, 256, comment);
		res = UpdateStore(slot);
	}

	unsigned int Mode = slot->StoreMode;
	ReleaseMutex(Mutex_Stores);

	if ((res==LFOk) && (!InternalCall))
		SendMessage(HWND_BROADCAST, LFMessages.StoreAttributesChanged, Mode==LFStoreModeInternal ? LFMSGF_IntStores : LFMSGF_ExtHybStores, (LPARAM)hWndSource);

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
		if (slot->DatPath[0]!='\0')
			if (!DirWriteable(slot->DatPath))
			{
				ReleaseMutex(Mutex_Stores);
				ReleaseMutexForStore(StoreLock);
				return LFDriveWriteProtected;
			}

		LFStoreDescriptor victim = *slot;
		res = DeleteStore(slot);
		ReleaseMutex(Mutex_Stores);
		ReleaseMutexForStore(StoreLock);

		if (res==LFOk)
		{
			SendNotifyMessage(HWND_BROADCAST, LFMessages.StoresChanged, victim.StoreMode==LFStoreModeInternal ? LFMSGF_IntStores : LFMSGF_ExtHybStores, (LPARAM)hWndSource);

			if (victim.IdxPathAux[0]!='\0')
			{
				char path[MAX_PATH];
				GetAutoPath(&victim, path);

				RemoveDir(victim.IdxPathAux);
				RemoveDir(path);
			}

			if (victim.IdxPathMain[0]!='\0')
				RemoveDir(victim.IdxPathMain);

			if (victim.DatPath[0]!='\0')
			{
				for (unsigned a=0; a<sizeof(KeyChars); a++)
				{
					char subpath[3];
					subpath[0] = KeyChars[a];
					subpath[1] = '\\';
					subpath[2] = '\0';

					char path[MAX_PATH];
					strcpy_s(path, MAX_PATH, victim.DatPath);
					strcat_s(path, MAX_PATH, subpath);

					RemoveDir(path);
				}

				RemoveDir(victim.DatPath);
			}
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
	swprintf_s(caption, 256 , tmp, s->CoreAttributes.FileName);

	wchar_t msg[256];
	LoadString(LFCoreModuleHandle, IDS_DeleteStoreMessage, msg, 256);

	return MessageBox(hWnd, msg, caption, MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING)==IDYES;
}

LFCore_API bool LFAskDeleteStore(LFStoreDescriptor* s, HWND hWnd)
{
	wchar_t caption[256];
	wchar_t tmp[256];
	LoadString(LFCoreModuleHandle, IDS_DeleteStoreCaption, tmp, 256);
	swprintf_s(caption, 256 , tmp, s->StoreName);

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
	CIndex* idx = new CIndex((s->StoreMode!=LFStoreModeHybrid) ? s->IdxPathMain : s->IdxPathAux, s->StoreID);
	switch (idx->Check(scheduled))
	{
	case IndexNotEnoughFreeDiscSpace:
		delete idx;
		return LFNotEnoughFreeDiscSpace;
	case IndexReindexRequired:
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

	SYSTEMTIME st;
	GetLocalTime(&st);
	SystemTimeToFileTime(&st, &s->MaintenanceTime);
	s->NeedsCheck = false;

	return LFOk;
}

LFCore_API unsigned int LFStoreMaintenance(char* key)
{
	if (!key)
		return LFIllegalKey;
	if (key[0]=='\0')
		return LFIllegalKey;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	HANDLE StoreLock = NULL;
	LFStoreDescriptor* slot = FindStore(key, &StoreLock);
	ReleaseMutex(Mutex_Stores);

	if (!slot)
		return LFIllegalKey;
	if (!StoreLock)
		return LFMutexError;

	unsigned int res = RunMaintenance(slot, true);
	ReleaseMutexForStore(StoreLock);

	return res;
}

LFCore_API unsigned int LFStoreMaintenance(unsigned int* Repaired, unsigned int* NoAccess, unsigned int* NoFreeSpace, unsigned int* RepairError)
{
	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

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
				return LFMutexError;
			}

			HANDLE StoreLock = NULL;
			LFStoreDescriptor* slot = FindStore(ptr, &StoreLock);
			ReleaseMutex(Mutex_Stores);

			if ((!slot) || (!StoreLock))
				continue;

			switch (LFStoreMaintenance(ptr))
			{
			case LFOk:
				if (Repaired)
					(*Repaired)++;
				break;
			case LFIllegalPhysicalPath:
			case LFDriveWriteProtected:
				if (NoAccess)
					(*NoAccess)++;
				break;
			case LFNotEnoughFreeDiscSpace:
				if (NoFreeSpace)
					(*NoFreeSpace)++;
				break;
			default:
				if (RepairError)
					(*RepairError)++;
			}
			ReleaseMutexForStore(StoreLock);

			ptr += LFKeySize;
		}

		free(keys);
	}

	return LFOk;
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
		Index1 = new CIndex(s->IdxPathMain, s->StoreID);
		if (s->StoreMode==LFStoreModeHybrid)
			Index2 = new CIndex(s->IdxPathAux, s->StoreID);
	}
	else
	{
		Index1 = new CIndex(s->StoreMode==LFStoreModeHybrid ? s->IdxPathAux : s->IdxPathMain, s->StoreID);
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

LFCore_API unsigned int LFImportFiles(char* key, LFFileImportList* il, LFItemDescriptor* it, bool move)
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

	if (store[0]=='\0')
		return LFNoDefaultStore;

	// Importliste vorbereiten
	il->Resolve();

	// Import
	CIndex* idx1;
	CIndex* idx2;
	LFStoreDescriptor* slot;
	HANDLE StoreLock = NULL;
	unsigned int res = OpenStore(&store[0], true, idx1, idx2, &slot, &StoreLock);
	if (res==LFOk)
	{
		for (unsigned int a=0; a<il->m_Count; a++)
			if (il->m_Entries[a])
			{
				LFItemDescriptor* i = LFAllocItemDescriptor(it);
				i->CoreAttributes.Flags = LFFlagNew;
				SetNameExtFromFile(i, il->m_Entries[a]);
				SetAttributesFromFile(i, il->m_Entries[a]);

				char CopyToA[MAX_PATH];
				res = PrepareImport(slot, i, CopyToA, MAX_PATH);
				if (res!=LFOk)
				{
					LFFreeItemDescriptor(i);
					break;
				}

				wchar_t CopyToW[MAX_PATH];
				size_t sz = strlen(CopyToA)+1;
				MultiByteToWideChar(CP_ACP, 0, CopyToA, (int)sz, &CopyToW[0], (int)sz);

				BOOL shres = move ? MoveFile(il->m_Entries[a], CopyToW) : CopyFile(il->m_Entries[a], CopyToW, FALSE);
				if (!shres)
				{
					LFFreeItemDescriptor(i);
					res = LFIllegalPhysicalPath;
					break;
				}

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

	return res;
}
