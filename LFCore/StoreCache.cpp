
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

bool Initialized = false;
char KeyChars[38] = { LFKeyChars };

#pragma data_seg()


#pragma bss_seg(".stores")

char DefaultStore[LFKeySize];
unsigned int StoreCount;
LFStoreDescriptor StoreCache[MaxStores];

#pragma data_seg()
#pragma comment(linker, "/SECTION:.stores,RWS")


extern HANDLE Mutex_Stores;
extern HMODULE LFCoreModuleHandle;
extern LFMessageIDs LFMessages;
extern unsigned int VolumeTypes[26];


#define StoreDescriptorFileSize sizeof(LFStoreDescriptor)- \
	sizeof(LFStoreDescriptor().IdxPathMain)-sizeof(LFStoreDescriptor().IdxPathAux)- \
	sizeof(LFStoreDescriptor().Source)- \
	sizeof(LFStoreDescriptor().FileCount)-sizeof(LFStoreDescriptor().FileSize)
#define StoreDescriptorRequiredFileSize StoreDescriptorFileSize- \
	sizeof(LFStoreDescriptor().SynchronizeTime)


// Default store handling
//

LFCore_API bool LFDefaultStoreAvailable()
{
	bool res = false;

	if (GetMutex(Mutex_Stores))
	{
		res = (DefaultStore[0]!='\0');
		ReleaseMutex(Mutex_Stores);
	}

	return res;
}

LFCore_API bool LFGetDefaultStore(char* StoreID)
{
	if (GetMutex(Mutex_Stores))
	{
		strcpy_s(StoreID, LFKeySize, DefaultStore);
		ReleaseMutex(Mutex_Stores);

		return true;
	}

	StoreID[0] = '\0';
	return false;
}

LFCore_API void LFGetDefaultStoreName(wchar_t* name, size_t cCount)
{
	LoadString(LFCoreModuleHandle, IDS_DefaultStore, name, (int)cCount);
}

unsigned int MakeDefaultStore(LFStoreDescriptor* s)
{
	assert(s);

	char OldDefaultStore[LFKeySize];
	strcpy_s(OldDefaultStore, LFKeySize, DefaultStore);

	unsigned int res = LFRegistryError;

	HKEY k;
	if (RegOpenKeyA(HKEY_CURRENT_USER, LFStoresHive, &k)==ERROR_SUCCESS)
	{
		if (RegSetValueExA(k, "DefaultStore", 0, REG_SZ, (BYTE*)s->StoreID, (DWORD)strlen(s->StoreID))==ERROR_SUCCESS)
		{
			strcpy_s(DefaultStore, LFKeySize, s->StoreID);

			SendShellNotifyMessage(SHCNE_UPDATEITEM, DefaultStore);
			if (OldDefaultStore[0]!='\0')
				SendShellNotifyMessage(SHCNE_UPDATEITEM, OldDefaultStore);

			res = LFOk;
		}

		RegCloseKey(k);
	}

	return res;
}

void ChooseNewDefaultStore()
{
	int no = -1;

	for (unsigned int a=0; a<StoreCount; a++)
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

void AppendGUID(LFStoreDescriptor* s, wchar_t* pPath, wchar_t* pSuffix)
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

bool IsStoreMounted(LFStoreDescriptor* s)
{
	assert(s);

	return s->DatPath[0]!='\0';
}

void GetAutoPath(LFStoreDescriptor* s, wchar_t* pPath)
{
	assert(s);
	assert(pPath);

	SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, L"Stores", pPath);
	wcscat_s(pPath, MAX_PATH, L"\\");
	AppendGUID(s, pPath);
}

unsigned int GetKeyFileFromStoreDescriptor(LFStoreDescriptor* s, wchar_t* fn)
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

bool LoadStoreSettingsFromRegistry(char* StoreID, LFStoreDescriptor* s)
{
	assert(StoreID);
	assert(s);

	bool res = false;
	ZeroMemory(s, sizeof(LFStoreDescriptor));
	strcpy_s(s->StoreID, LFKeySize, StoreID);

	char regkey[256];
	strcpy_s(regkey, 256, LFStoresHive);
	strcat_s(regkey, 256, "\\");
	strcat_s(regkey, 256, StoreID);

	HKEY k;
	if (RegOpenKeyA(HKEY_CURRENT_USER, regkey, &k)==ERROR_SUCCESS)
	{
		res = true;

		DWORD sz = sizeof(s->StoreName);
		if (RegQueryValueEx(k, L"Name", 0, NULL, (BYTE*)&s->StoreName, &sz)!=ERROR_SUCCESS)
			res = false;

		sz = sizeof(s->StoreComment);
		RegQueryValueEx(k, L"Comment", 0, NULL, (BYTE*)&s->StoreComment, &sz);

		sz = sizeof(s->Mode);
		if (RegQueryValueEx(k, L"Mode", 0, NULL, (BYTE*)&s->Mode, &sz)!=ERROR_SUCCESS)
			res = false;

		sz = sizeof(s->guid);
		if (RegQueryValueEx(k, L"GUID", 0, NULL, (BYTE*)&s->guid, &sz)!=ERROR_SUCCESS)
			res = false;

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
			res = false;

		s->Flags |= LFStoreFlagUnchecked;

		sz = sizeof(s->IndexVersion);
		if (RegQueryValueEx(k, L"IndexVersion", 0, NULL, (BYTE*)&s->IndexVersion, &sz)!=ERROR_SUCCESS)
			res = false;

		switch (s->Mode & LFStoreModeIndexMask)
		{
		case LFStoreModeIndexInternal:
			s->Source = LFTypeSourceInternal;

			sz = sizeof(s->DatPath);
			if (RegQueryValueEx(k, L"Path", 0, NULL, (BYTE*)s->DatPath, &sz)!=ERROR_SUCCESS)
				if (!(s->Flags & LFStoreFlagAutoLocation))
					res = false;

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

	return res;
}

bool LoadStoreSettingsFromFile(wchar_t* fn, LFStoreDescriptor* s)
{
	assert(fn);
	assert(fn[0]!=L'\0');
	assert(s);

	HANDLE hFile = CreateFile(fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return false;

	ZeroMemory(s, sizeof(LFStoreDescriptor));

	DWORD wmRead;
	bool res = (ReadFile(hFile, s, StoreDescriptorFileSize, &wmRead, NULL)==TRUE);
	res &= (wmRead>=StoreDescriptorRequiredFileSize);
	CloseHandle(hFile);

	return res;
}

unsigned int SaveStoreSettingsToRegistry(LFStoreDescriptor* s)
{
	assert(s);

	if ((s->Mode & LFStoreModeIndexMask)==LFStoreModeIndexExternal)
		return LFIllegalStoreDescriptor;

	// Registry-Zugriff
	unsigned int res = LFRegistryError;

	char regkey[256];
	strcpy_s(regkey, 256, LFStoresHive);
	strcat_s(regkey, 256, "\\");
	strcat_s(regkey, 256, s->StoreID);

	HKEY k;
	if (RegCreateKeyA(HKEY_CURRENT_USER, regkey, &k)==ERROR_SUCCESS)
	{
		res = LFOk;

		if (RegSetValueEx(k, L"Name", 0, REG_SZ, (BYTE*)s->StoreName, (DWORD)wcslen(s->StoreName)*sizeof(wchar_t))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"Comment", 0, REG_SZ, (BYTE*)s->StoreComment, (DWORD)wcslen(s->StoreComment)*sizeof(wchar_t))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"Mode", 0, REG_DWORD, (BYTE*)&s->Mode, sizeof(unsigned int))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"GUID", 0, REG_BINARY, (BYTE*)&s->guid, sizeof(GUID))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"CreationTime", 0, REG_BINARY, (BYTE*)&s->CreationTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"FileTime", 0, REG_BINARY, (BYTE*)&s->FileTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"MaintenanceTime", 0, REG_BINARY, (BYTE*)&s->MaintenanceTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"SynchronizeTime", 0, REG_BINARY, (BYTE*)&s->SynchronizeTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"AutoLocation", 0, REG_DWORD, (BYTE*)&s->Flags, sizeof(unsigned int))!=ERROR_SUCCESS)
			res = LFRegistryError;

		if (RegSetValueEx(k, L"IndexVersion", 0, REG_DWORD, (BYTE*)&s->IndexVersion, sizeof(unsigned int))!=ERROR_SUCCESS)
			res = LFRegistryError;

		switch (s->Mode & LFStoreModeIndexMask)
		{
		case LFStoreModeIndexInternal:
			if (!(s->Flags & LFStoreFlagAutoLocation))
				if (RegSetValueEx(k, L"Path", 0, REG_SZ, (BYTE*)s->DatPath, (DWORD)wcslen(s->DatPath)*sizeof(wchar_t))!=ERROR_SUCCESS)
					res = LFRegistryError;
			break;
		case LFStoreModeIndexHybrid:
			if (RegSetValueEx(k, L"LastSeen", 0, REG_SZ, (BYTE*)s->LastSeen, (DWORD)wcslen(s->LastSeen)*sizeof(wchar_t))!=ERROR_SUCCESS)
				res = LFRegistryError;
		default:
			if (RegSetValueEx(k, L"Source", 0, REG_DWORD, (BYTE*)&s->Source, sizeof(unsigned int))!=ERROR_SUCCESS)
				res = LFRegistryError;
		}

		RegCloseKey(k);
	}

	return res;
}

unsigned int SaveStoreSettingsToFile(LFStoreDescriptor* s)
{
	assert(s);

	if ((s->Mode & LFStoreModeIndexMask)==LFStoreModeIndexInternal)
		return LFIllegalStoreDescriptor;

	wchar_t fn[MAX_PATH];
	unsigned int res = GetKeyFileFromStoreDescriptor(s, fn);
	if (res!=LFOk)
		return res;

	HANDLE hFile = CreateFile(fn, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_WRITE_THROUGH, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return LFDriveNotReady;

	DWORD wmWritten;
	res = WriteFile(hFile, s, StoreDescriptorFileSize, &wmWritten, NULL) ? LFOk : LFDriveNotReady;
	if (wmWritten!=StoreDescriptorFileSize)
		res = LFDriveNotReady;
	CloseHandle(hFile);

	return res;
}

unsigned int DeleteStoreSettingsFromRegistry(LFStoreDescriptor* s)
{
	assert(s);

	if ((s->Mode & LFStoreModeIndexMask)==LFStoreModeIndexExternal)
		return LFIllegalStoreDescriptor;

	char regkey[256];
	strcpy_s(regkey, 256, LFStoresHive);
	strcat_s(regkey, 256, "\\");
	strcat_s(regkey, 256, s->StoreID);

	LRESULT lres = RegDeleteKeyA(HKEY_CURRENT_USER, regkey);
	return (lres==ERROR_SUCCESS) || (lres==ERROR_FILE_NOT_FOUND) ? LFOk: LFRegistryError;
}

unsigned int DeleteStoreSettingsFromFile(LFStoreDescriptor* s)
{
	assert(s);

	if ((s->Mode & LFStoreModeIndexMask)==LFStoreModeIndexInternal)
		return LFIllegalStoreDescriptor;

	wchar_t fn[MAX_PATH];
	unsigned int res = GetKeyFileFromStoreDescriptor(s, fn);
	if (res!=LFOk)
		return res;

	return DeleteFile(fn) ? LFOk : LFDriveNotReady;
}

void CreateNewStoreID(char* StoreID)
{
	assert(StoreID);

	bool unique;

	RANDOMIZE();

	do
	{
		for (unsigned int a=0; a<LFKeySize-1; a++)
			StoreID[a] = RAND_CHAR();
		StoreID[LFKeySize-1] = 0;

		unique = true;
		for (unsigned int a=0; a<StoreCount; a++)
			if (strcmp(StoreCache[a].StoreID, StoreID)==0)
			{
				unique = false;
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
			wchar_t szDriveRoot[] = L" :\\";
			szDriveRoot[0] = s->DatPath[0];

			SHFILEINFO sfi;
			if (SHGetFileInfo(szDriveRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME))
				wcscpy_s(s->LastSeen, 256, sfi.szDisplayName);

			if ((s->Mode & LFStoreModeBackendMask)<=LFStoreModeBackendNTFS)
				s->Source = LFGetSourceForDrive(s->DatPath[0] & 0xFF);
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
		char key[256];
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

__forceinline void MountExternalDrives()
{
	DWORD DrivesOnSystem = LFGetLogicalDrives(LFGLD_External);
	wchar_t szDriveRoot[4] = L" :\\";

	for (char cDrive='A'; cDrive<='Z'; cDrive++, DrivesOnSystem>>=1)
	{
		if ((DrivesOnSystem & 1)==0)
			continue;

		szDriveRoot[0] = cDrive;
		SHFILEINFO sfi;
		if (SHGetFileInfo(szDriveRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_ATTRIBUTES))
			if (sfi.dwAttributes)
				MountDrive(cDrive, true);
	}
}

void InitStoreCache()
{
	if (GetMutex(Mutex_Stores))
	{
		if (!Initialized)
		{
			Initialized = true;

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

			// Mount external drives
			MountExternalDrives();

			// Run maintenance, set default store
			bool DefaultStoreOk = false;

			for (unsigned int a=0; a<StoreCount; a++)
			{
				// Run non-scheduled maintenance
				RunMaintenance(&StoreCache[a], false);

				if (strcmp(DefaultStore, StoreCache[a].StoreID)==0)
					DefaultStoreOk = true;
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

	for (unsigned int a=0; a<StoreCount; a++)
		sr->AddStoreDescriptor(&StoreCache[a]);
}

LFStoreDescriptor* FindStore(char* StoreID, HANDLE* lock)
{
	assert(StoreID);

	for (unsigned int a=0; a<StoreCount; a++)
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
	for (unsigned int a=0; a<StoreCount; a++)
		if (StoreCache[a].guid==guid)
		{
			if (lock)
				GetMutexForStore(&StoreCache[a], lock);
			return &StoreCache[a];
		}

	return NULL;
}

LFStoreDescriptor* FindStore(wchar_t* DatPath, HANDLE* lock)
{
	assert(DatPath);

	for (unsigned int a=0; a<StoreCount; a++)
		if (wcscmp(StoreCache[a].DatPath, DatPath)==0)
		{
			if (lock)
				GetMutexForStore(&StoreCache[a], lock);
			return &StoreCache[a];
		}

	return NULL;
}

unsigned int FindStores(char** IDs)
{
	assert(IDs);

	if (StoreCount)
	{
		*IDs = (char*)malloc(LFKeySize*StoreCount);
		char* ptr = *IDs;

		for (unsigned int a=0; a<StoreCount; a++)
		{
			strcpy_s(ptr, LFKeySize, StoreCache[a].StoreID);
			ptr += LFKeySize;
		}
	}

	return StoreCount;
}

LFCore_API unsigned int LFGetStores(char** StoreIDs, unsigned int* count)
{
	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	*count = FindStores(StoreIDs);
	ReleaseMutex(Mutex_Stores);

	return LFOk;
}

unsigned int UpdateStore(LFStoreDescriptor* s, bool UpdateTime, bool MakeDefault)
{
	assert(s);

	// Set file time
	if (UpdateTime)
		GetSystemTimeAsFileTime(&s->FileTime);

	// Update cache
	LFStoreDescriptor* slot = FindStore(s->StoreID);
	if (!slot)
	{
		if (StoreCount==MaxStores)
			return LFTooManyStores;

		slot = &StoreCache[StoreCount];
		StoreCache[StoreCount++] = *s;
	}
	else
	{
		*slot = *s;
	}

	unsigned int res = LFOk;
	if ((s->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal)
		res = SaveStoreSettingsToRegistry(s);
	if ((s->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
		if ((res==LFOk) && (IsStoreMounted(s)))
			res = SaveStoreSettingsToFile(s);

	// Make default store if s is the sole store
	if (res==LFOk)
		if ((MakeDefault) || (DefaultStore[0]=='\0'))
			res = MakeDefaultStore(s);

	return res;
}

unsigned int DeleteStore(LFStoreDescriptor* s)
{
	LFStoreDescriptor victim = *s;

	// Remove from cache
	for (unsigned int a=0; a<StoreCount; a++)
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
	unsigned int res = LFOk;
	if ((victim.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal)
		res = DeleteStoreSettingsFromRegistry(&victim);
	if ((victim.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
		if ((res==LFOk) && (IsStoreMounted(&victim)))
			res = DeleteStoreSettingsFromFile(&victim);

	return res;
}

LFCore_API unsigned int LFGetStoreCount()
{
	unsigned int res = 0;

	if (GetMutex(Mutex_Stores))
	{
		res = StoreCount;
		ReleaseMutex(Mutex_Stores);
	}

	return res;
}


// Drive handling
//

LFCore_API bool LFStoresOnDrive(char cDrive)
{
	bool res = false;

	if (GetMutex(Mutex_Stores))
	{
		for (unsigned int a=0; a<StoreCount; a++)
			if (IsStoreMounted(&StoreCache[a]))
				if (StoreCache[a].DatPath[0]==cDrive)
				{
					res = true;
					break;
				}

		ReleaseMutex(Mutex_Stores);
	}

	return res;
}

unsigned int MountDrive(char cDrive, bool InternalCall)
{
	wchar_t mask[] = L" :\\*.store";
	mask[0] = cDrive;
	bool ChangeOccured = false;
	unsigned int res = LFOk;

	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(mask, &ffd);

	if (hFind!=INVALID_HANDLE_VALUE)
		do
		{
			// Vollständigen Dateinamen zusammensetzen
			wchar_t f[MAX_PATH] = L" :\\";
			f[0] = cDrive;
			wcscat_s(f, MAX_PATH, ffd.cFileName);

			LFStoreDescriptor s;
			if (LoadStoreSettingsFromFile(f, &s)==true)
			{
				// Korrekter Mode?
				if ((s.Mode & LFStoreModeIndexMask)==LFStoreModeIndexInternal)
					continue;

				if (!GetMutex(Mutex_Stores))
				{
					res = LFMutexError;
					continue;
				}

				// Lokal gültigen Schlüssel eintragen
				CreateNewStoreID(s.StoreID);

				// Store mit der selben GUID suchen
				LFStoreDescriptor* slot = FindStore(s.guid);
				if (!slot)
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
						res = LFTooManyStores;
					}
				}
				else
				{
					// Wenn der Store kein Hybrid-Store ist, würde er doppelt gemountet. Überspringen!
					if ((slot->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexHybrid)
					{
						slot = NULL;
					}
					else
					{
						// Name, Kommentar und Dateizeit aktualisieren
						wcscpy_s(slot->StoreName, 256, s.StoreName);
						wcscpy_s(slot->StoreComment, 256, s.StoreComment);
						slot->FileTime = s.FileTime;
					}
				}

				if (slot)
				{
					wcscpy_s(slot->DatPath, MAX_PATH, s.DatPath);
					slot->DatPath[0] = cDrive;

					SetStoreAttributes(slot);

					slot->Flags |= LFStoreFlagUnchecked;

					if (!InternalCall)
						RunMaintenance(slot, false);

					ChangeOccured = true;

					if ((slot->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexHybrid)
						goto Finish;

					// Hybrid-Stores in der Registry abspeichern, damit LastSeen aktualisiert wird
					SaveStoreSettingsToRegistry(slot);

					HANDLE StoreLock;
					if (!GetMutexForStore(slot, &StoreLock))
						goto Finish;

					ReleaseMutex(Mutex_Stores);
					res = CopyDir(slot->IdxPathMain, slot->IdxPathAux);

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
	{
		SendLFNotifyMessage(ChangeOccured ? LFMessages.StoresChanged : LFMessages.VolumesChanged, NULL);
		SendShellNotifyMessage(SHCNE_UPDATEDIR);
	}

	return res;
}

unsigned int UnmountDrive(char cDrive, bool InternalCall)
{
	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	bool ChangeOccured = false;
	bool RemovedDefaultStore = false;
	VolumeTypes[cDrive-'A'] = DRIVE_UNKNOWN;
	unsigned int res = LFOk;

	char NotifyIDs[MaxStores][LFKeySize];
	unsigned int NotifyCount = 0;

	for (unsigned int a=0; a<StoreCount; a++)
		if (IsStoreMounted(&StoreCache[a]))
			if ((StoreCache[a].DatPath[0]==cDrive) && ((StoreCache[a].Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal))
			{
				HANDLE StoreLock;
				if (!GetMutexForStore(&StoreCache[a], &StoreLock))
				{
					res = LFMutexError;
					continue;
				}

				switch (StoreCache[a].Mode & LFStoreModeIndexMask)
				{
				case LFStoreModeIndexHybrid:
					StoreCache[a].DatPath[0] = StoreCache[a].IdxPathMain[0] = '\0';
					strcpy_s(NotifyIDs[NotifyCount++], LFKeySize, StoreCache[a].StoreID);
					ChangeOccured = true;
					break;
				case LFStoreModeIndexExternal:
					RemovedDefaultStore |= (strcmp(StoreCache[a].StoreID, DefaultStore)==0);
					if (a<StoreCount-1)
					{
						HANDLE MoveLock;
						if (!GetMutexForStore(&StoreCache[StoreCount-1], &MoveLock))
						{
							ReleaseMutexForStore(StoreLock);
							res = LFMutexError;
							continue;
						}

						StoreCache[a--] = StoreCache[--StoreCount];
						ReleaseMutexForStore(MoveLock);
					}
					else
					{
						StoreCount--;
					}
					ChangeOccured = true;
					break;
				}

				ReleaseMutexForStore(StoreLock);
			}

	// Ggf. anderen Store als neuen Default Store
	if (RemovedDefaultStore)
		ChooseNewDefaultStore();

	ReleaseMutex(Mutex_Stores);

	if (!InternalCall)
	{
		SendLFNotifyMessage(ChangeOccured ? LFMessages.StoresChanged : LFMessages.VolumesChanged, NULL);
		for (unsigned int a=0; a<NotifyCount; a++)
			SendShellNotifyMessage(SHCNE_UPDATEITEM, NotifyIDs[a]);
		SendShellNotifyMessage(SHCNE_UPDATEDIR);
	}

	return res;
}
