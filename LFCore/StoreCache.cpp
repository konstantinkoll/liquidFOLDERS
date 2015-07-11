
#include "stdafx.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "Mutex.h"
#include "Stores.h"
#include "StoreCache.h"
#include <assert.h>
#include <io.h>
#include <malloc.h>
#include <shellapi.h>
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>


#pragma data_seg(".shared")

BOOL Initialized = FALSE;
CHAR KeyChars[38] = { LFKeyChars };

#pragma data_seg()


#pragma bss_seg(".stores")

CHAR DefaultStore[LFKeySize];
UINT StoreCount;
LFStoreDescriptor StoreCache[MaxStores];

#pragma data_seg()
#pragma comment(linker, "/SECTION:.stores,RWS")


extern HANDLE Mutex_Stores;
extern HMODULE LFCoreModuleHandle;
extern LFMessageIDs LFMessages;
extern UINT VolumeTypes[26];


#define StoreDescriptorFileSize sizeof(LFStoreDescriptor)- \
	sizeof(LFStoreDescriptor().IdxPathMain)-sizeof(LFStoreDescriptor().IdxPathAux)- \
	sizeof(LFStoreDescriptor().Source)- \
	sizeof(LFStoreDescriptor().FileCount)-sizeof(LFStoreDescriptor().FileSize)
#define StoreDescriptorRequiredFileSize StoreDescriptorFileSize- \
	sizeof(LFStoreDescriptor().SynchronizeTime)


// Default store handling
//

LFCORE_API BOOL LFDefaultStoreAvailable()
{
	BOOL Result = FALSE;

	if (GetMutex(Mutex_Stores))
	{
		Result = (DefaultStore[0]!='\0');
		ReleaseMutex(Mutex_Stores);
	}

	return Result;
}

LFCORE_API BOOL LFGetDefaultStore(CHAR* StoreID)
{
	if (GetMutex(Mutex_Stores))
	{
		strcpy_s(StoreID, LFKeySize, DefaultStore);
		ReleaseMutex(Mutex_Stores);

		return TRUE;
	}

	StoreID[0] = '\0';
	return FALSE;
}

LFCORE_API void LFGetDefaultStoreName(WCHAR* name, SIZE_T cCount)
{
	LoadString(LFCoreModuleHandle, IDS_DEFAULTSTORE, name, (INT)cCount);
}

UINT MakeDefaultStore(LFStoreDescriptor* s)
{
	assert(s);

	CHAR OldDefaultStore[LFKeySize];
	strcpy_s(OldDefaultStore, LFKeySize, DefaultStore);

	UINT Result = LFRegistryError;

	HKEY k;
	if (RegOpenKeyA(HKEY_CURRENT_USER, LFStoresHive, &k)==ERROR_SUCCESS)
	{
		if (RegSetValueExA(k, "DefaultStore", 0, REG_SZ, (BYTE*)s->StoreID, (DWORD)strlen(s->StoreID))==ERROR_SUCCESS)
		{
			strcpy_s(DefaultStore, LFKeySize, s->StoreID);

			SendShellNotifyMessage(SHCNE_UPDATEITEM, DefaultStore);
			if (OldDefaultStore[0]!='\0')
				SendShellNotifyMessage(SHCNE_UPDATEITEM, OldDefaultStore);

			Result = LFOk;
		}

		RegCloseKey(k);
	}

	return Result;
}

void ChooseNewDefaultStore()
{
	INT no = -1;

	for (UINT a=0; a<StoreCount; a++)
		if (strcmp(DefaultStore, StoreCache[a].StoreID)!=0)
			if ((no==-1) || ((StoreCache[a].Mode & (LFStoreModeBackendMask | LFStoreModeIndexMask))==(LFStoreModeBackendInternal | LFStoreModeIndexInternal)))
				no = a;

	if (no!=-1)
	{
		MakeDefaultStore(&StoreCache[no]);
		return;
	}

	// Default Store löschen
	DefaultStore[0] = '\0';

	HKEY hive;
	if (RegOpenKeyA(HKEY_CURRENT_USER, LFStoresHive, &hive)==ERROR_SUCCESS)
	{
		RegDeleteValue(hive, L"DefaultStore");
		RegCloseKey(hive);
	}
}


// Persistance
//

void AppendGUID(LFStoreDescriptor* s, WCHAR* pPath, WCHAR* pSuffix)
{
	assert(s);
	assert(pPath);
	assert(pSuffix);

	OLECHAR szGUID[MAX_PATH];
	if (StringFromGUID2(s->guid, szGUID, MAX_PATH))
	{
		wcscat_s(pPath, MAX_PATH, szGUID);
		wcscat_s(pPath, MAX_PATH, pSuffix);
	}
}

__forceinline BOOL IsStoreMounted(LFStoreDescriptor* s)
{
	assert(s);

	return s->DatPath[0]!='\0';
}

void GetAutoPath(LFStoreDescriptor* s, WCHAR* pPath)
{
	assert(s);
	assert(pPath);

	SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, L"Stores", pPath);
	wcscat_s(pPath, MAX_PATH, L"\\");
	AppendGUID(s, pPath);
}

UINT GetKeyFileFromStoreDescriptor(LFStoreDescriptor* s, WCHAR* fn)
{
	assert(s);

	if ((s->Mode & LFStoreModeIndexMask)==LFStoreModeIndexInternal)
		return LFIllegalStoreDescriptor;

	if (!IsStoreMounted(s))
		return LFStoreNotMounted;

	wcsncpy_s(fn, MAX_PATH, s->DatPath, 3);		// .store file is always in root directory
	AppendGUID(s, fn, L".store");

	return LFOk;
}

BOOL LoadStoreSettingsFromRegistry(CHAR* StoreID, LFStoreDescriptor* s)
{
	assert(StoreID);
	assert(s);

	BOOL Result = FALSE;
	ZeroMemory(s, sizeof(LFStoreDescriptor));
	strcpy_s(s->StoreID, LFKeySize, StoreID);

	CHAR regkey[256];
	strcpy_s(regkey, 256, LFStoresHive);
	strcat_s(regkey, 256, "\\");
	strcat_s(regkey, 256, StoreID);

	HKEY k;
	if (RegOpenKeyA(HKEY_CURRENT_USER, regkey, &k)==ERROR_SUCCESS)
	{
		Result = TRUE;

		DWORD sz = sizeof(s->StoreName);
		if (RegQueryValueEx(k, L"Name", 0, NULL, (BYTE*)&s->StoreName, &sz)!=ERROR_SUCCESS)
			Result = FALSE;

		sz = sizeof(s->StoreComment);
		RegQueryValueEx(k, L"Comment", 0, NULL, (BYTE*)&s->StoreComment, &sz);

		sz = sizeof(s->Mode);
		if (RegQueryValueEx(k, L"Mode", 0, NULL, (BYTE*)&s->Mode, &sz)!=ERROR_SUCCESS)
			Result = FALSE;

		sz = sizeof(s->guid);
		if (RegQueryValueEx(k, L"GUID", 0, NULL, (BYTE*)&s->guid, &sz)!=ERROR_SUCCESS)
			Result = FALSE;

		sz = sizeof(s->CreationTime);
		RegQueryValueEx(k, L"CreationTime", 0, NULL, (BYTE*)&s->CreationTime, &sz);

		sz = sizeof(s->FileTime);
		RegQueryValueEx(k, L"FileTime", 0, NULL, (BYTE*)&s->FileTime, &sz);

		sz = sizeof(s->MaintenanceTime);
		RegQueryValueEx(k, L"MaintenanceTime", 0, NULL, (BYTE*)&s->MaintenanceTime, &sz);

		sz = sizeof(s->SynchronizeTime);
		RegQueryValueEx(k, L"SynchronizeTime", 0, NULL, (BYTE*)&s->SynchronizeTime, &sz);

		sz = sizeof(s->Flags);
		if (RegQueryValueEx(k, L"AutoLocation", 0, NULL, (BYTE*)&s->Flags, &sz)!=ERROR_SUCCESS)
			Result = FALSE;

		s->Flags |= LFStoreFlagUnchecked;

		sz = sizeof(s->IndexVersion);
		if (RegQueryValueEx(k, L"IndexVersion", 0, NULL, (BYTE*)&s->IndexVersion, &sz)!=ERROR_SUCCESS)
			Result = FALSE;

		switch (s->Mode & LFStoreModeIndexMask)
		{
		case LFStoreModeIndexInternal:
			s->Source = LFTypeSourceInternal;

			sz = sizeof(s->DatPath);
			if (RegQueryValueEx(k, L"Path", 0, NULL, (BYTE*)s->DatPath, &sz)!=ERROR_SUCCESS)
				if (!(s->Flags & LFStoreFlagAutoLocation))
					Result = FALSE;

			break;
		case LFStoreModeIndexHybrid:
			sz = sizeof(s->LastSeen);
			RegQueryValueEx(k, L"LastSeen", 0, NULL, (BYTE*)&s->LastSeen, &sz);
		default:
			sz = sizeof(s->Source);
			RegQueryValueEx(k, L"Source", 0, NULL, (BYTE*)&s->Source, &sz);
		}

		RegCloseKey(k);
	}

	return Result;
}

BOOL LoadStoreSettingsFromFile(WCHAR* fn, LFStoreDescriptor* s)
{
	assert(fn);
	assert(fn[0]!=L'\0');
	assert(s);

	HANDLE hFile = CreateFile(fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return FALSE;

	ZeroMemory(s, sizeof(LFStoreDescriptor));

	DWORD wmRead;
	BOOL Result = (ReadFile(hFile, s, StoreDescriptorFileSize, &wmRead, NULL)==TRUE);
	Result &= (wmRead>=StoreDescriptorRequiredFileSize);
	CloseHandle(hFile);

	return Result;
}

UINT SaveStoreSettingsToRegistry(LFStoreDescriptor* s)
{
	assert(s);

	if ((s->Mode & LFStoreModeIndexMask)==LFStoreModeIndexExternal)
		return LFIllegalStoreDescriptor;

	// Registry-Zugriff
	UINT Result = LFRegistryError;

	CHAR regkey[256];
	strcpy_s(regkey, 256, LFStoresHive);
	strcat_s(regkey, 256, "\\");
	strcat_s(regkey, 256, s->StoreID);

	HKEY k;
	if (RegCreateKeyA(HKEY_CURRENT_USER, regkey, &k)==ERROR_SUCCESS)
	{
		Result = LFOk;

		if (RegSetValueEx(k, L"Name", 0, REG_SZ, (BYTE*)s->StoreName, (DWORD)wcslen(s->StoreName)*sizeof(WCHAR))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(k, L"Comment", 0, REG_SZ, (BYTE*)s->StoreComment, (DWORD)wcslen(s->StoreComment)*sizeof(WCHAR))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(k, L"Mode", 0, REG_DWORD, (BYTE*)&s->Mode, sizeof(UINT))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(k, L"GUID", 0, REG_BINARY, (BYTE*)&s->guid, sizeof(GUID))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(k, L"CreationTime", 0, REG_BINARY, (BYTE*)&s->CreationTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(k, L"FileTime", 0, REG_BINARY, (BYTE*)&s->FileTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(k, L"MaintenanceTime", 0, REG_BINARY, (BYTE*)&s->MaintenanceTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(k, L"SynchronizeTime", 0, REG_BINARY, (BYTE*)&s->SynchronizeTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(k, L"AutoLocation", 0, REG_DWORD, (BYTE*)&s->Flags, sizeof(UINT))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(k, L"IndexVersion", 0, REG_DWORD, (BYTE*)&s->IndexVersion, sizeof(UINT))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		switch (s->Mode & LFStoreModeIndexMask)
		{
		case LFStoreModeIndexInternal:
			if (!(s->Flags & LFStoreFlagAutoLocation))
				if (RegSetValueEx(k, L"Path", 0, REG_SZ, (BYTE*)s->DatPath, (DWORD)wcslen(s->DatPath)*sizeof(WCHAR))!=ERROR_SUCCESS)
					Result = LFRegistryError;
			break;
		case LFStoreModeIndexHybrid:
			if (RegSetValueEx(k, L"LastSeen", 0, REG_SZ, (BYTE*)s->LastSeen, (DWORD)wcslen(s->LastSeen)*sizeof(WCHAR))!=ERROR_SUCCESS)
				Result = LFRegistryError;
		default:
			if (RegSetValueEx(k, L"Source", 0, REG_DWORD, (BYTE*)&s->Source, sizeof(UINT))!=ERROR_SUCCESS)
				Result = LFRegistryError;
		}

		RegCloseKey(k);
	}

	return Result;
}

UINT SaveStoreSettingsToFile(LFStoreDescriptor* s)
{
	assert(s);

	if ((s->Mode & LFStoreModeIndexMask)==LFStoreModeIndexInternal)
		return LFIllegalStoreDescriptor;

	WCHAR fn[MAX_PATH];
	UINT Result = GetKeyFileFromStoreDescriptor(s, fn);
	if (Result!=LFOk)
		return Result;

	HANDLE hFile = CreateFile(fn, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_WRITE_THROUGH, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return (GetLastError()==5) ? LFAccessError : LFDriveNotReady;

	DWORD wmWritten;
	Result = WriteFile(hFile, s, StoreDescriptorFileSize, &wmWritten, NULL) ? LFOk : LFDriveNotReady;
	if (wmWritten!=StoreDescriptorFileSize)
		Result = LFDriveNotReady;

	CloseHandle(hFile);

	return Result;
}

UINT DeleteStoreSettingsFromRegistry(LFStoreDescriptor* s)
{
	assert(s);

	if ((s->Mode & LFStoreModeIndexMask)==LFStoreModeIndexExternal)
		return LFIllegalStoreDescriptor;

	CHAR regkey[256];
	strcpy_s(regkey, 256, LFStoresHive);
	strcat_s(regkey, 256, "\\");
	strcat_s(regkey, 256, s->StoreID);

	LRESULT lres = RegDeleteKeyA(HKEY_CURRENT_USER, regkey);
	return (lres==ERROR_SUCCESS) || (lres==ERROR_FILE_NOT_FOUND) ? LFOk: LFRegistryError;
}

UINT DeleteStoreSettingsFromFile(LFStoreDescriptor* s)
{
	assert(s);

	if ((s->Mode & LFStoreModeIndexMask)==LFStoreModeIndexInternal)
		return LFIllegalStoreDescriptor;

	WCHAR fn[MAX_PATH];
	UINT Result = GetKeyFileFromStoreDescriptor(s, fn);
	if (Result!=LFOk)
		return Result;

	return DeleteFile(fn) ? LFOk : (GetLastError()==5) ? LFAccessError : LFDriveNotReady;
}

void CreateNewStoreID(CHAR* StoreID)
{
	assert(StoreID);

	BOOL unique;

	RANDOMIZE();

	do
	{
		for (UINT a=0; a<LFKeySize-1; a++)
			StoreID[a] = RAND_CHAR();
		StoreID[LFKeySize-1] = 0;

		unique = TRUE;
		for (UINT a=0; a<StoreCount; a++)
			if (strcmp(StoreCache[a].StoreID, StoreID)==0)
			{
				unique = FALSE;
				break;
			}
	}
	while (!unique);
}

void SetStoreAttributes(LFStoreDescriptor* s)
{
	assert(s);
	assert(((s->Mode & LFStoreModeIndexMask)>=LFStoreModeIndexInternal) && ((s->Mode & LFStoreModeIndexMask)<=LFStoreModeIndexExternal));

	// Source
	s->Source = ((s->Mode & LFStoreModeBackendMask)==LFStoreModeBackendInternal) ? LFTypeSourceInternal : (s->Mode & LFStoreModeBackendMask)>>LFStoreModeBackendShift;

	// Store name and source of mounted volume
	if ((s->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
		if (IsStoreMounted(s))
		{
			WCHAR szVolumeRoot[] = L" :\\";
			szVolumeRoot[0] = s->DatPath[0];

			SHFILEINFO sfi;
			if (SHGetFileInfo(szVolumeRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME))
				wcscpy_s(s->LastSeen, 256, sfi.szDisplayName);

			if ((s->Mode & LFStoreModeBackendMask)<=LFStoreModeBackendNTFS)
				s->Source = LFGetSourceForVolume(s->DatPath[0] & 0xFF);
		}

	// Get automatic data path
	if (((s->Mode & LFStoreModeIndexMask)==LFStoreModeIndexInternal) && (s->Flags & LFStoreFlagAutoLocation))
		GetAutoPath(s, s->DatPath);

	// Main index is always subdir of store
	if (((s->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexHybrid) || (IsStoreMounted(s)))
	{
		wcscpy_s(s->IdxPathMain, MAX_PATH, s->DatPath);
		wcscat_s(s->IdxPathMain, MAX_PATH, L"INDEX\\");
	}
	else
	{
		s->IdxPathMain[0] = L'\0';
	}

	// Set aux index for local hybrid stores
	if ((s->Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid)
	{
		GetAutoPath(s, s->IdxPathAux);
		wcscat_s(s->IdxPathAux, MAX_PATH, L"INDEX\\");
	}
	else
	{
		s->IdxPathAux[0] = L'\0';
	}
}


// Initializing
//

__forceinline void LoadRegistry()
{
	// Stores aus der Registry laden
	HKEY hive;
	if (RegOpenKeyA(HKEY_CURRENT_USER, LFStoresHive, &hive)!=ERROR_SUCCESS)
		return;

	DWORD Subkeys;
	if (RegQueryInfoKey(hive, NULL, 0, NULL, &Subkeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL)!=ERROR_SUCCESS)
		Subkeys = 0;

	for (DWORD a=0; a<Subkeys; a++)
	{
		// Noch Platz im Cache?
		if (StoreCount>=MaxStores)
			break;

		// Nächsten Key finden
		CHAR key[256];
		DWORD l = 255;
		if (RegEnumKeyA(hive, a, key, l)!=ERROR_SUCCESS)
		{
			RegCloseKey(hive);
			return;
		}

		// Store laden
		if (LoadStoreSettingsFromRegistry(key, &StoreCache[StoreCount]))
		{
			SetStoreAttributes(&StoreCache[StoreCount]);
			StoreCache[StoreCount++].Flags |= LFStoreFlagUnchecked;
		}
	}

	RegCloseKey(hive);
}

__forceinline void MountExternalVolumes()
{
	DWORD VolumesOnSystem = LFGetLogicalVolumes(LFGLV_EXTERNAL);
	WCHAR szVolumeRoot[4] = L" :\\";

	for (CHAR cVolume='A'; cVolume<='Z'; cVolume++, VolumesOnSystem>>=1)
	{
		if ((VolumesOnSystem & 1)==0)
			continue;

		szVolumeRoot[0] = cVolume;
		SHFILEINFO sfi;
		if (SHGetFileInfo(szVolumeRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ATTRIBUTES))
			if (sfi.dwAttributes)
				MountVolume(cVolume, TRUE);
	}
}

void InitStoreCache()
{
	if (GetMutex(Mutex_Stores))
	{
		if (!Initialized)
		{
			Initialized = TRUE;

			// Load default store
			HKEY k;
			if (RegOpenKeyA(HKEY_CURRENT_USER, LFStoresHive, &k)==ERROR_SUCCESS)
			{
				DWORD type;
				DWORD sz = LFKeySize;
				RegQueryValueExA(k, "DefaultStore", NULL, &type, (BYTE*)DefaultStore, &sz);
				RegCloseKey(k);
			}

			// Stores from registry
			StoreCount = 0;
			LoadRegistry();

			// Mount external volumes
			MountExternalVolumes();

			// Run maintenance, set default store
			BOOL DefaultStoreOk = FALSE;

			for (UINT a=0; a<StoreCount; a++)
			{
				// Run non-scheduled maintenance
				RunMaintenance(&StoreCache[a], FALSE);

				if (strcmp(DefaultStore, StoreCache[a].StoreID)==0)
					DefaultStoreOk = TRUE;
			}

			// Choose new default store if neccessary
			if ((!DefaultStoreOk) && (StoreCount))
				ChooseNewDefaultStore();
		}

		ReleaseMutex(Mutex_Stores);
	}
}


// Cache access
//

void AddStoresToSearchResult(LFSearchResult* sr)
{
	assert(sr);

	for (UINT a=0; a<StoreCount; a++)
		sr->AddStoreDescriptor(&StoreCache[a]);
}

LFStoreDescriptor* FindStore(CHAR* StoreID, HANDLE* lock)
{
	assert(StoreID);

	for (UINT a=0; a<StoreCount; a++)
		if (strcmp(StoreCache[a].StoreID, StoreID)==0)
		{
			if (lock)
				GetMutexForStore(&StoreCache[a], lock);

			return &StoreCache[a];
		}

	return NULL;
}

LFStoreDescriptor* FindStore(GUID guid, HANDLE* lock)
{
	for (UINT a=0; a<StoreCount; a++)
		if (StoreCache[a].guid==guid)
		{
			if (lock)
				GetMutexForStore(&StoreCache[a], lock);
			return &StoreCache[a];
		}

	return NULL;
}

LFStoreDescriptor* FindStore(WCHAR* DatPath, HANDLE* lock)
{
	assert(DatPath);

	for (UINT a=0; a<StoreCount; a++)
		if (wcscmp(StoreCache[a].DatPath, DatPath)==0)
		{
			if (lock)
				GetMutexForStore(&StoreCache[a], lock);
			return &StoreCache[a];
		}

	return NULL;
}

UINT FindStores(CHAR** IDs)
{
	assert(IDs);

	if (StoreCount)
	{
		*IDs = (CHAR*)malloc(LFKeySize*StoreCount);
		CHAR* ptr = *IDs;

		for (UINT a=0; a<StoreCount; a++)
		{
			strcpy_s(ptr, LFKeySize, StoreCache[a].StoreID);
			ptr += LFKeySize;
		}
	}

	return StoreCount;
}

LFCORE_API UINT LFGetStores(CHAR** StoreIDs, UINT* count)
{
	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	*count = FindStores(StoreIDs);
	ReleaseMutex(Mutex_Stores);

	return LFOk;
}

UINT UpdateStore(LFStoreDescriptor* s, BOOL UpdateTime, BOOL MakeDefault)
{
	assert(s);

	// Set file time
	if (UpdateTime)
		GetSystemTimeAsFileTime(&s->FileTime);

	// Find in cache
	LFStoreDescriptor* slot = FindStore(s->StoreID);
	if (!slot)
		if (StoreCount==MaxStores)
			return LFTooManyStores;

	UINT Result = LFOk;
	if ((s->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal)
		Result = SaveStoreSettingsToRegistry(s);
	if ((s->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
		if ((Result==LFOk) && (IsStoreMounted(s)))
			Result = SaveStoreSettingsToFile(s);

	if (Result==LFOk)
	{
		// Update cache
		if (slot)
		{
			*slot = *s;
		}
		else
		{
			StoreCache[StoreCount++] = *s;
		}

		// Make default store if s is the sole store
		if ((MakeDefault) || (DefaultStore[0]=='\0'))
			Result = MakeDefaultStore(s);
	}

	return Result;
}

UINT DeleteStore(LFStoreDescriptor* s)
{
	LFStoreDescriptor victim = *s;

	// Remove from cache
	for (UINT a=0; a<StoreCount; a++)
		if (strcmp(StoreCache[a].StoreID, s->StoreID)==0)
		{
			if (a<StoreCount-1)
			{
				HANDLE MoveLock;
				if (!GetMutexForStore(&StoreCache[StoreCount-1], &MoveLock))
					return LFMutexError;

				StoreCache[a] = StoreCache[--StoreCount];
				ReleaseMutexForStore(MoveLock);
			}
			else
			{
				StoreCount--;
			}
			break;
		}

	// Choose new default store
	if (strcmp(victim.StoreID, DefaultStore)==0)
		ChooseNewDefaultStore();

	// Remove persistent records
	UINT Result = LFOk;
	if ((victim.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal)
		Result = DeleteStoreSettingsFromRegistry(&victim);
	if ((victim.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
		if ((Result==LFOk) && (IsStoreMounted(&victim)))
			Result = DeleteStoreSettingsFromFile(&victim);

	return Result;
}

LFCORE_API UINT LFGetStoreCount()
{
	UINT Result = 0;

	if (GetMutex(Mutex_Stores))
	{
		Result = StoreCount;
		ReleaseMutex(Mutex_Stores);
	}

	return Result;
}


// Volume handling
//

LFCORE_API BOOL LFStoresOnVolume(CHAR cVolume)
{
	BOOL Result = FALSE;

	if (GetMutex(Mutex_Stores))
	{
		for (UINT a=0; a<StoreCount; a++)
			if (IsStoreMounted(&StoreCache[a]))
				if (StoreCache[a].DatPath[0]==cVolume)
				{
					Result = TRUE;
					break;
				}

		ReleaseMutex(Mutex_Stores);
	}

	return Result;
}

UINT MountVolume(CHAR cVolume, BOOL InternalCall)
{
	assert(cVolume>='A');
	assert(cVolume<='Z');

	WCHAR Mask[] = L" :\\*.store";
	Mask[0] = cVolume;
	BOOL ChangeOccured = FALSE;
	UINT Result = LFOk;

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(Mask, &ffd);

	if (hFind!=INVALID_HANDLE_VALUE)
		do
		{
			// Vollständigen Dateinamen zusammensetzen
			WCHAR f[MAX_PATH] = L" :\\";
			f[0] = cVolume;
			wcscat_s(f, MAX_PATH, ffd.cFileName);

			LFStoreDescriptor s;
			if (LoadStoreSettingsFromFile(f, &s)==TRUE)
			{
				// Korrekter Mode?
				if ((s.Mode & LFStoreModeIndexMask)==LFStoreModeIndexInternal)
					continue;

				if (!GetMutex(Mutex_Stores))
				{
					Result = LFMutexError;
					continue;
				}

				// Lokal gültigen Schlüssel eintragen
				CreateNewStoreID(s.StoreID);

				// Store mit der selben GUID suchen
				LFStoreDescriptor* slot = FindStore(s.guid);
				if (slot)
				{
					// Name, Kommentar und Dateizeit aktualisieren
					wcscpy_s(slot->StoreName, 256, s.StoreName);
					wcscpy_s(slot->StoreComment, 256, s.StoreComment);
					slot->FileTime = s.FileTime;
				}
				else
				{
					// Nicht gefunden: der Store wird hier als externer Store behandelt
					s.Mode = (s.Mode & ~LFStoreModeIndexMask) | LFStoreModeIndexExternal;

					// Zum Cache hinzufügen
					if (StoreCount<MaxStores)
					{
						StoreCache[StoreCount] = s;
						slot = &StoreCache[StoreCount++];		// Slot zeigt auf Eintrag
					}
					else
					{
						Result = LFTooManyStores;
					}
				}

				if (slot)
				{
					wcscpy_s(slot->DatPath, MAX_PATH, s.DatPath);
					slot->DatPath[0] = cVolume;

					SetStoreAttributes(slot);

					slot->Flags |= LFStoreFlagUnchecked;

					if (!InternalCall)
						RunMaintenance(slot, FALSE);

					ChangeOccured = TRUE;

					if ((slot->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexHybrid)
						goto Finish;

					// Hybrid-Stores in der Registry abspeichern, damit LastSeen aktualisiert wird
					SaveStoreSettingsToRegistry(slot);

					HANDLE StoreLock;
					if (!GetMutexForStore(slot, &StoreLock))
						goto Finish;

					ReleaseMutex(Mutex_Stores);
					Result = CopyDir(slot->IdxPathMain, slot->IdxPathAux);

					if (!InternalCall)
						SendShellNotifyMessage(SHCNE_UPDATEITEM, slot->StoreID);

					ReleaseMutexForStore(StoreLock);
					continue;
				}

Finish:
				ReleaseMutex(Mutex_Stores);
			}
		}
		while (FindNextFile(hFind, &ffd));

	FindClose(hFind);

	if (!InternalCall)
		if (ChangeOccured)
		{
			SendLFNotifyMessage(LFMessages.StoresChanged);
			SendShellNotifyMessage(SHCNE_UPDATEDIR);
		}
		else
		{
			SendLFNotifyMessage(LFMessages.VolumesChanged);
		}

	return Result;
}

UINT UnmountVolume(CHAR cVolume, BOOL InternalCall)
{
	assert(cVolume>='A');
	assert(cVolume<='Z');

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	UINT Result = LFOk;

	BOOL ChangeOccured = FALSE;
	BOOL RemovedDefaultStore = FALSE;
	VolumeTypes[cVolume-'A'] = DRIVE_UNKNOWN;

	CHAR NotifyIDs[MaxStores][LFKeySize];
	BOOL NotifyTypes[MaxStores];
	UINT NotifyCount = 0;

	for (UINT a=0; a<StoreCount; a++)
		if (IsStoreMounted(&StoreCache[a]))
			if ((StoreCache[a].DatPath[0]==cVolume) && ((StoreCache[a].Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal))
			{
				HANDLE StoreLock;
				if (!GetMutexForStore(&StoreCache[a], &StoreLock))
				{
					Result = LFMutexError;
					continue;
				}

				switch (StoreCache[a].Mode & LFStoreModeIndexMask)
				{
				case LFStoreModeIndexHybrid:
					StoreCache[a].DatPath[0] = StoreCache[a].IdxPathMain[0] = L'\0';
					strcpy_s(NotifyIDs[NotifyCount], LFKeySize, StoreCache[a].StoreID);
					NotifyTypes[NotifyCount++] = FALSE;
					ChangeOccured = TRUE;
					break;
				case LFStoreModeIndexExternal:
					RemovedDefaultStore |= (strcmp(StoreCache[a].StoreID, DefaultStore)==0);
					strcpy_s(NotifyIDs[NotifyCount], LFKeySize, StoreCache[a].StoreID);
					NotifyTypes[NotifyCount++] = TRUE;
					if (a<StoreCount-1)
					{
						HANDLE MoveLock;
						if (!GetMutexForStore(&StoreCache[StoreCount-1], &MoveLock))
						{
							ReleaseMutexForStore(StoreLock);
							Result = LFMutexError;
							continue;
						}

						StoreCache[a--] = StoreCache[--StoreCount];
						ReleaseMutexForStore(MoveLock);
					}
					else
					{
						StoreCount--;
					}
					ChangeOccured = TRUE;
					break;
				}

				ReleaseMutexForStore(StoreLock);
			}

	// Ggf. anderen Store als neuen Default Store
	if (RemovedDefaultStore)
		ChooseNewDefaultStore();
	
	ReleaseMutex(Mutex_Stores);

	if (!InternalCall)
		if (ChangeOccured)
		{
			SendLFNotifyMessage(LFMessages.StoresChanged);

			for (UINT a=0; a<NotifyCount; a++)
				SendShellNotifyMessage(NotifyTypes[a] ? SHCNE_RMDIR : SHCNE_UPDATEITEM, NotifyIDs[a]);
			SendShellNotifyMessage(SHCNE_UPDATEDIR);
		}
		else
		{
			SendLFNotifyMessage(LFMessages.VolumesChanged);
		}

	return Result;
}
