
#include "stdafx.h"
#include "CStoreInternal.h"
#include "CStoreWindows.h"
#include "FileSystem.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "PIDL.h"
#include "Stores.h"
#include <assert.h>


extern HMODULE LFCoreModuleHandle;
extern LFMessageIDs LFMessages;
extern UINT VolumeTypes[26];


#define LFSTORESHIVE     "Software\\liquidFOLDERS\\Stores"
#define MAXSTORES        100


#pragma data_seg(".shared")

BOOL Initialized = FALSE;
CHAR KeyChars[38] = { LFKeyChars };

#pragma data_seg()


#pragma bss_seg(".stores")

CHAR DefaultStore[LFKeySize];
UINT StoreCount;
LFStoreDescriptor StoreCache[MAXSTORES];

#pragma data_seg()
#pragma comment(linker, "/SECTION:.stores,RWS")


#define STOREDESCRIPTORFILESIZE             offsetof(LFStoreDescriptor, IdxPathMain)
#define STOREDESCRIPTORREQUIREDFILESIZE     offsetof(LFStoreDescriptor, SynchronizeTime)


// Persistance
//

UINT GetKeyFileFromStoreDescriptor(LFStoreDescriptor* s, WCHAR* fn)
{
	assert(s);

	if ((s->Mode & LFStoreModeIndexMask)==LFStoreModeIndexInternal)
		return LFIllegalStoreDescriptor;

	if (!LFIsStoreMounted(s))
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
	strcpy_s(regkey, 256, LFSTORESHIVE);
	strcat_s(regkey, 256, "\\");
	strcat_s(regkey, 256, StoreID);

	HKEY hKey;
	if (RegOpenKeyA(HKEY_CURRENT_USER, regkey, &hKey)==ERROR_SUCCESS)
	{
		Result = TRUE;

		DWORD sz = sizeof(s->StoreName);
		if (RegQueryValueEx(hKey, L"Name", 0, NULL, (BYTE*)&s->StoreName, &sz)!=ERROR_SUCCESS)
			Result = FALSE;

		sz = sizeof(s->Comments);
		RegQueryValueEx(hKey, L"Comment", 0, NULL, (BYTE*)&s->Comments, &sz);

		sz = sizeof(s->Mode);
		if (RegQueryValueEx(hKey, L"Mode", 0, NULL, (BYTE*)&s->Mode, &sz)!=ERROR_SUCCESS)
			Result = FALSE;

		sz = sizeof(s->UniqueID);
		if (RegQueryValueEx(hKey, L"GUID", 0, NULL, (BYTE*)&s->UniqueID, &sz)!=ERROR_SUCCESS)
			Result = FALSE;

		sz = sizeof(s->CreationTime);
		RegQueryValueEx(hKey, L"CreationTime", 0, NULL, (BYTE*)&s->CreationTime, &sz);

		sz = sizeof(s->FileTime);
		RegQueryValueEx(hKey, L"FileTime", 0, NULL, (BYTE*)&s->FileTime, &sz);

		sz = sizeof(s->MaintenanceTime);
		RegQueryValueEx(hKey, L"MaintenanceTime", 0, NULL, (BYTE*)&s->MaintenanceTime, &sz);

		sz = sizeof(s->SynchronizeTime);
		RegQueryValueEx(hKey, L"SynchronizeTime", 0, NULL, (BYTE*)&s->SynchronizeTime, &sz);

		sz = sizeof(s->AutoLocation);
		if (RegQueryValueEx(hKey, L"AutoLocation", 0, NULL, (BYTE*)&s->AutoLocation, &sz)!=ERROR_SUCCESS)
			Result = FALSE;

		sz = sizeof(s->IndexVersion);
		if (RegQueryValueEx(hKey, L"IndexVersion", 0, NULL, (BYTE*)&s->IndexVersion, &sz)!=ERROR_SUCCESS)
			Result = FALSE;

		s->Source = s->Mode >> LFStoreModeBackendShift;

		switch(s->Mode & LFStoreModeIndexMask)
		{
		case LFStoreModeIndexInternal:
			sz = sizeof(s->DatPath);
			if (RegQueryValueEx(hKey, L"Path", 0, NULL, (BYTE*)s->DatPath, &sz)!=ERROR_SUCCESS)
				if (!(s->AutoLocation))
					Result = FALSE;

			break;
		case LFStoreModeIndexHybrid:
			sz = sizeof(s->LastSeen);
			RegQueryValueEx(hKey, L"LastSeen", 0, NULL, (BYTE*)&s->LastSeen, &sz);
		default:
			sz = sizeof(s->Source);
			RegQueryValueEx(hKey, L"Source", 0, NULL, (BYTE*)&s->Source, &sz);
		}

		RegCloseKey(hKey);
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
	BOOL Result = (ReadFile(hFile, s, STOREDESCRIPTORFILESIZE, &wmRead, NULL)==TRUE);
	Result &= (wmRead>=STOREDESCRIPTORREQUIREDFILESIZE);
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
	strcpy_s(regkey, 256, LFSTORESHIVE);
	strcat_s(regkey, 256, "\\");
	strcat_s(regkey, 256, s->StoreID);

	HKEY hKey;
	if (RegCreateKeyA(HKEY_CURRENT_USER, regkey, &hKey)==ERROR_SUCCESS)
	{
		Result = LFOk;

		if (RegSetValueEx(hKey, L"Name", 0, REG_SZ, (BYTE*)s->StoreName, (DWORD)wcslen(s->StoreName)*sizeof(WCHAR))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"Comment", 0, REG_SZ, (BYTE*)s->Comments, (DWORD)wcslen(s->Comments)*sizeof(WCHAR))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"Mode", 0, REG_DWORD, (BYTE*)&s->Mode, sizeof(UINT))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"GUID", 0, REG_BINARY, (BYTE*)&s->UniqueID, sizeof(GUID))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"CreationTime", 0, REG_BINARY, (BYTE*)&s->CreationTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"FileTime", 0, REG_BINARY, (BYTE*)&s->FileTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"MaintenanceTime", 0, REG_BINARY, (BYTE*)&s->MaintenanceTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"SynchronizeTime", 0, REG_BINARY, (BYTE*)&s->SynchronizeTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"AutoLocation", 0, REG_DWORD, (BYTE*)&s->AutoLocation, sizeof(UINT))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"IndexVersion", 0, REG_DWORD, (BYTE*)&s->IndexVersion, sizeof(UINT))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		switch(s->Mode & LFStoreModeIndexMask)
		{
		case LFStoreModeIndexInternal:
			if (!s->AutoLocation)
				if (RegSetValueEx(hKey, L"Path", 0, REG_SZ, (BYTE*)s->DatPath, (DWORD)wcslen(s->DatPath)*sizeof(WCHAR))!=ERROR_SUCCESS)
					Result = LFRegistryError;
			break;
		case LFStoreModeIndexHybrid:
			if (RegSetValueEx(hKey, L"LastSeen", 0, REG_SZ, (BYTE*)s->LastSeen, (DWORD)wcslen(s->LastSeen)*sizeof(WCHAR))!=ERROR_SUCCESS)
				Result = LFRegistryError;
		default:
			if (RegSetValueEx(hKey, L"Source", 0, REG_DWORD, (BYTE*)&s->Source, sizeof(UINT))!=ERROR_SUCCESS)
				Result = LFRegistryError;
		}

		RegCloseKey(hKey);
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
	Result = WriteFile(hFile, s, STOREDESCRIPTORFILESIZE, &wmWritten, NULL) ? LFOk : LFDriveNotReady;
	if (wmWritten!=STOREDESCRIPTORFILESIZE)
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
	strcpy_s(regkey, 256, LFSTORESHIVE);
	strcat_s(regkey, 256, "\\");
	strcat_s(regkey, 256, s->StoreID);

	LSTATUS lres = RegDeleteKeyA(HKEY_CURRENT_USER, regkey);
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



void SetStoreAttributes(LFStoreDescriptor* s)
{
	assert(s);
	assert(((s->Mode & LFStoreModeIndexMask)>=LFStoreModeIndexInternal) && ((s->Mode & LFStoreModeIndexMask)<=LFStoreModeIndexExternal));

	// Source
	if (s->Source==LFTypeSourceUnknown)
		s->Source = ((s->Mode & LFStoreModeBackendMask)==LFStoreModeBackendInternal) ? LFTypeSourceInternal : (s->Mode & LFStoreModeBackendMask)>>LFStoreModeBackendShift;

	// Store name and source of mounted volume
	if ((s->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
		if (LFIsStoreMounted(s))
		{
			WCHAR szVolumeRoot[] = L" :\\";
			szVolumeRoot[0] = s->DatPath[0];

			SHFILEINFO sfi;
			if (SHGetFileInfo(szVolumeRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME))
				wcscpy_s(s->LastSeen, 256, sfi.szDisplayName);

			if ((s->Mode & LFStoreModeBackendMask)<=LFStoreModeBackendWindows)
				s->Source = LFGetSourceForVolume(s->DatPath[0] & 0xFF);
		}

	// Paths
	if ((s->Mode & LFStoreModeBackendMask)==LFStoreModeBackendWindows)
	{
		if (LFIsStoreMounted(s))
		{
			UINT Source = LFGetSourceForVolume((CHAR)s->DatPath[0]);
			if ((Source==LFTypeSourceUSB) || (Source==LFTypeSource1394))
			{
				wcsncpy_s(s->IdxPathMain, MAX_PATH, s->DatPath, 3);
				AppendGUID(s, s->IdxPathMain);
			}
			else
			{
				GetAutoPath(s, s->IdxPathMain);
			}
		}
		else
		{
			s->IdxPathMain[0] = L'\0';
		}
	}
	else
	{
		// Get automatic data path
		if (((s->Mode & LFStoreModeIndexMask)==LFStoreModeIndexInternal) && s->AutoLocation)
			GetAutoPath(s, s->DatPath);

		// Main index
		if (((s->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexHybrid) || (LFIsStoreMounted(s)))
		{
			wcscpy_s(s->IdxPathMain, MAX_PATH, s->DatPath);
			wcscat_s(s->IdxPathMain, MAX_PATH, L"INDEX\\");
		}
		else
		{
			s->IdxPathMain[0] = L'\0';
		}
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

UINT UpdateStoreInCache(LFStoreDescriptor* s, BOOL UpdateFileTime, BOOL MakeDefault)
{
	assert(s);

	// Set file time
	GetSystemTimeAsFileTime(UpdateFileTime ? &s->FileTime : &s->MaintenanceTime);

	// Find in cache
	LFStoreDescriptor* slot = FindStore(s->StoreID);
	if (!slot)
		if (StoreCount==MAXSTORES)
			return LFTooManyStores;

	UINT Result = LFOk;
	if ((s->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal)
		Result = SaveStoreSettingsToRegistry(s);
	if ((s->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
		if ((Result==LFOk) && (LFIsStoreMounted(s)))
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

UINT DeleteStoreFromCache(LFStoreDescriptor* s)
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
		if ((Result==LFOk) && (LFIsStoreMounted(&victim)))
			Result = DeleteStoreSettingsFromFile(&victim);

	return Result;
}



UINT MountVolume(CHAR cVolume, BOOL OnInitialize)
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

			LFStoreDescriptor Store;
			if (LoadStoreSettingsFromFile(f, &Store)==TRUE)
			{
				if (!GetMutexForStores())
				{
					Result = LFMutexError;
					continue;
				}

				// Lokal gültigen Schlüssel eintragen
				CreateNewStoreID(Store.StoreID);

				// Store mit der selben GUID suchen
				LFStoreDescriptor* slot = FindStore(Store.UniqueID);
				if (slot)
				{
					// Name, Kommentar und Dateizeit aktualisieren
					wcscpy_s(slot->StoreName, 256, Store.StoreName);
					wcscpy_s(slot->Comments, 256, Store.Comments);
					slot->FileTime = Store.FileTime;
				}
				else
				{
					// Nicht gefunden: der Store wird hier als externer Store behandelt
					Store.Mode = (Store.Mode & ~LFStoreModeIndexMask) | LFStoreModeIndexExternal;

					// Zum Cache hinzufügen
					if (StoreCount<MAXSTORES)
					{
						StoreCache[StoreCount] = Store;
						slot = &StoreCache[StoreCount++];		// Slot zeigt auf Eintrag
					}
					else
					{
						Result = LFTooManyStores;
					}
				}

				if (slot)
				{
					wcscpy_s(slot->DatPath, MAX_PATH, Store.DatPath);
					slot->DatPath[0] = cVolume;

					SetStoreAttributes(slot);

					if (!OnInitialize)
					{
						CStore* pStore;
						if (GetStore(slot, &pStore)==LFOk)
						{
							Result = pStore->MaintenanceAndStatistics();
							delete pStore;
						}
					}

					ChangeOccured = TRUE;

					if ((slot->Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid)
					{
						// Hybrid-Stores in der Registry abspeichern, damit LastSeen aktualisiert wird
						SaveStoreSettingsToRegistry(slot);

						if (!OnInitialize)
							SendShellNotifyMessage(SHCNE_UPDATEITEM, slot->StoreID);
					}

				}

				ReleaseMutexForStores();
			}
		}
		while (FindNextFile(hFind, &ffd));

	FindClose(hFind);

	if (!OnInitialize)
		if (ChangeOccured)
		{
			SendLFNotifyMessage(LFMessages.StoresChanged);
			SendShellNotifyMessage(SHCNE_UPDATEDIR);
		}

	return Result;
}







// NEU

// Cache access
//

void CreateNewStoreID(CHAR* pStoreID)
{
	assert(pStoreID);

	pStoreID[LFKeySize-1] = 0;
	BOOL IsUnique;

	do
	{
		RANDOMIZE();

		for (UINT a=0; a<LFKeySize-1; a++)
			pStoreID[a] = RAND_CHAR();

		IsUnique = TRUE;

		for (UINT a=0; a<StoreCount; a++)
			if (strcmp(StoreCache[a].StoreID, pStoreID)==0)
			{
				IsUnique = FALSE;
				break;
			}
	}
	while (!IsUnique);
}

void GetDescriptorForNewStore(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);

	ZeroMemory(pStoreDescriptor, sizeof(LFStoreDescriptor));

	// Name
	LoadString(LFCoreModuleHandle, IDS_NEWSTORE, pStoreDescriptor->StoreName, 256);

	// Create uniquie ID
	CoCreateGuid(&pStoreDescriptor->UniqueID);

	// CreateTime and MaintenanceTime
	GetSystemTimeAsFileTime(&pStoreDescriptor->CreationTime);
	pStoreDescriptor->MaintenanceTime = pStoreDescriptor->CreationTime;

	// Create ID
	CreateNewStoreID(pStoreDescriptor->StoreID);

	// Index version
	pStoreDescriptor->IndexVersion = CURIDXVERSION;
}

LFStoreDescriptor* FindStore(CHAR* pStoreID, HMUTEX* phMutex)
{
	assert(pStoreID);

	for (UINT a=0; a<StoreCount; a++)
		if (strcmp(StoreCache[a].StoreID, pStoreID)==0)
		{
			if (phMutex)
				GetMutexForStore(&StoreCache[a], phMutex);

			return &StoreCache[a];
		}

	return NULL;
}

LFStoreDescriptor* FindStore(GUID UniqueID, HMUTEX* phMutex)
{
	for (UINT a=0; a<StoreCount; a++)
		if (StoreCache[a].UniqueID==UniqueID)
		{
			if (phMutex)
				GetMutexForStore(&StoreCache[a], phMutex);

			return &StoreCache[a];
		}

	return NULL;
}

LFStoreDescriptor* FindStore(WCHAR* pDatPath, HMUTEX* phMutex)
{
	assert(pDatPath);

	for (UINT a=0; a<StoreCount; a++)
		if (wcscmp(StoreCache[a].DatPath, pDatPath)==0)
		{
			if (phMutex)
				GetMutexForStore(&StoreCache[a], phMutex);

			return &StoreCache[a];
		}

	return NULL;
}


// Default store handling
//

UINT MakeDefaultStore(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);

	UINT Result = LFRegistryError;

	HKEY hKey;
	if (RegOpenKeyA(HKEY_CURRENT_USER, LFSTORESHIVE, &hKey)==ERROR_SUCCESS)
	{
		if (RegSetValueExA(hKey, "DefaultStore", 0, REG_SZ, (BYTE*)pStoreDescriptor->StoreID, LFKeySize-1)==ERROR_SUCCESS)
		{
			CHAR OldDefaultStore[LFKeySize];
			strcpy_s(OldDefaultStore, LFKeySize, DefaultStore);

			strcpy_s(DefaultStore, LFKeySize, pStoreDescriptor->StoreID);

			Result = LFOk;

			// Notifications
			SendShellNotifyMessage(SHCNE_UPDATEITEM, DefaultStore);

			if (OldDefaultStore[0]!='\0')
				SendShellNotifyMessage(SHCNE_UPDATEITEM, OldDefaultStore);
		}

		RegCloseKey(hKey);
	}

	return Result;
}

void ChooseNewDefaultStore()
{
	INT No = -1;

	for (UINT a=0; a<StoreCount; a++)
		if (strcmp(DefaultStore, StoreCache[a].StoreID)!=0)
			if ((No==-1) || ((StoreCache[a].Mode & (LFStoreModeBackendMask | LFStoreModeIndexMask))==(LFStoreModeBackendInternal | LFStoreModeIndexInternal)))
				No = a;

	if (No!=-1)
	{
		MakeDefaultStore(&StoreCache[No]);
	}
	else
	{
		// Remove default store
		DefaultStore[0] = '\0';

		HKEY hKey;
		if (RegOpenKeyA(HKEY_CURRENT_USER, LFSTORESHIVE, &hKey)==ERROR_SUCCESS)
		{
			RegDeleteValue(hKey, L"DefaultStore");
			RegCloseKey(hKey);
		}
	}
}

LFCORE_API UINT LFGetDefaultStore(CHAR* pStoreID)
{
	if (GetMutexForStores())
	{
		UINT Result = DefaultStore[0] ? LFOk : LFNoDefaultStore;

		if (pStoreID)
			strcpy_s(pStoreID, LFKeySize, DefaultStore);

		ReleaseMutexForStores();

		return Result;
	}

	return LFMutexError;
}

LFCORE_API UINT LFSetDefaultStore(CHAR* pStoreID)
{
	assert(pStoreID);

	if (*pStoreID==L'\0')
		return LFIllegalValue;

	if (!GetMutexForStores())
		return LFMutexError;

	UINT Result = LFOk;

	if (strcmp(pStoreID, DefaultStore)!=0)
	{
		LFStoreDescriptor* pStoreDescriptor;
		Result = (pStoreDescriptor=FindStore(pStoreID))!=NULL ? MakeDefaultStore(pStoreDescriptor) : LFIllegalID;

		ReleaseMutexForStores();

		if (Result==LFOk)
			SendLFNotifyMessage(LFMessages.DefaultStoreChanged);
	}

	return Result;
}


// Stores
//

UINT GetStore(LFStoreDescriptor* pStoreDescriptor, CStore** ppStore, HMUTEX hMutex)
{
	assert(ppStore);

	*ppStore = NULL;

	if (!pStoreDescriptor)
		return LFIllegalID;

	if (!hMutex)
		if (!GetMutexForStore(pStoreDescriptor, &hMutex))
			return LFMutexError;

	// Create matching CStore object
	switch (pStoreDescriptor->Mode & LFStoreModeBackendMask)
	{
	case LFStoreModeBackendInternal:
		*ppStore = new CStoreInternal(pStoreDescriptor, hMutex);
		break;

	case LFStoreModeBackendWindows:
		*ppStore = new CStoreWindows(pStoreDescriptor, hMutex);
		break;

	default:
		ReleaseMutexForStore(hMutex);
		return LFIllegalStoreDescriptor;
	}

	// Done
	return LFOk;
}

UINT GetStore(CHAR* StoreID, CStore** ppStore)
{
	assert(ppStore);

	*ppStore = NULL;

	if (!GetMutexForStores())
		return LFMutexError;

	HMUTEX hMutex;
	LFStoreDescriptor* pStoreDescriptor = FindStore(StoreID, &hMutex);
	UINT Result = GetStore(pStoreDescriptor, ppStore, hMutex);

	ReleaseMutexForStores();

	return Result;
}

UINT OpenStore(CHAR* StoreID, BOOL WriteAccess, CStore** ppStore)
{
	UINT Result = GetStore(StoreID, ppStore);

	if (Result==LFOk)
		if ((Result=(*ppStore)->Open(WriteAccess))!=LFOk)
		{
			delete *ppStore;
			*ppStore = NULL;
		}

	return Result;
}

UINT CommitInitializeStore(LFStoreDescriptor* pStoreDescriptor, LFProgress* pProgress=NULL)
{
	assert(pStoreDescriptor);

	SetStoreAttributes(pStoreDescriptor);

	UINT Result = UpdateStoreInCache(pStoreDescriptor);
	if (Result==LFOk)
	{
		CStore* pStore;
		if ((Result=GetStore(pStoreDescriptor->StoreID, &pStore))==LFOk)
		{
			Result = pStore->Initialize(pProgress);
			delete pStore;
		}

		ReleaseMutexForStores();

		SendLFNotifyMessage(LFMessages.StoresChanged);
		SendShellNotifyMessage(SHCNE_UPDATEDIR);
	}
	else
	{
		ReleaseMutexForStores();
	}

	return Result;
}


// Operations
//

LFCORE_API UINT LFGetStoreCount()
{
	if (GetMutexForStores())
	{
		UINT Result = StoreCount;

		ReleaseMutexForStores();

		return Result;
	}

	return 0;
}

LFCORE_API UINT LFGetAllStores(CHAR** ppStoreIDs, UINT* pCount)
{
	assert(ppStoreIDs);
	assert(pCount);

	*ppStoreIDs = NULL;
	*pCount = NULL;

	if (!GetMutexForStores())
		return LFMutexError;

	if (StoreCount)
	{
		*ppStoreIDs = (CHAR*)malloc(LFKeySize*StoreCount);
		CHAR* Ptr = *ppStoreIDs;

		for (UINT a=0; a<StoreCount; a++)
		{
			strcpy_s(Ptr, LFKeySize, StoreCache[a].StoreID);

			Ptr += LFKeySize;
		}

		*pCount = StoreCount;
	}

	ReleaseMutexForStores();

	return LFOk;
}

LFCORE_API UINT LFGetStoreSettings(CHAR* pStoreID, LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreID);
	assert(pStoreDescriptor);

	UINT Result;

	// Store
	CHAR StoreID[LFKeySize];
	strcpy_s(StoreID, LFKeySize, pStoreID);

	if (StoreID[0]=='\0')
		if ((Result=LFGetDefaultStore(StoreID))!=LFOk)
			return Result;

	if (!GetMutexForStores())
		return LFMutexError;

	LFStoreDescriptor* pSlot = FindStore(StoreID);
	if (pSlot)
		*pStoreDescriptor = *pSlot;

	ReleaseMutexForStores();

	return (pSlot ? LFOk : LFIllegalID);
}

LFCORE_API UINT LFGetStoreSettingsEx(GUID UniqueID, LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);

	if (!GetMutexForStores())
		return LFMutexError;

	LFStoreDescriptor* pSlot = FindStore(UniqueID);
	if (pSlot)
		*pStoreDescriptor = *pSlot;

	ReleaseMutexForStores();

	return (pSlot ? LFOk : LFIllegalID);
}

LFCORE_API BOOL LFStoresOnVolume(CHAR cVolume)
{
	BOOL Result = FALSE;

	if (GetMutexForStores())
	{
		for (UINT a=0; a<StoreCount; a++)
			if (LFIsStoreMounted(&StoreCache[a]))
				if (StoreCache[a].DatPath[0]==cVolume)
				{
					Result = TRUE;
					break;
				}

		ReleaseMutexForStores();
	}

	return Result;
}

LFCORE_API UINT LFGetStoreIcon(LFStoreDescriptor* pStoreDescriptor, UINT* pType)
{
	assert(pStoreDescriptor);

	if (pType)
	{
		*pType = LFTypeStore | pStoreDescriptor->Source;

		// Empty?
		if (!pStoreDescriptor->FileCount[LFContextAllFiles])
			*pType = (*pType & ~LFTypeBadgeMask) | LFTypeBadgeEmpty;

		// New?
		FILETIME CurrentTime;
		GetSystemTimeAsFileTime(&CurrentTime);

		ULARGE_INTEGER ULI1;
		ULARGE_INTEGER ULI2;

		ULI1.LowPart = CurrentTime.dwLowDateTime;
		ULI1.HighPart = CurrentTime.dwHighDateTime;
		ULI2.LowPart = pStoreDescriptor->CreationTime.dwLowDateTime;
		ULI2.HighPart = pStoreDescriptor->CreationTime.dwHighDateTime;

		if (ULI1.QuadPart<ULI2.QuadPart+(ULONGLONG)86400*10*1000*1000)
			*pType = (*pType & ~LFTypeBadgeMask) | LFTypeBadgeNew;

		// Default store?
		if (GetMutexForStores())
		{
			if (strcmp(pStoreDescriptor->StoreID, DefaultStore)==0)
				*pType = (*pType & ~LFTypeBadgeMask) | LFTypeBadgeDefault | LFTypeDefault;

			ReleaseMutexForStores();
		}

		// Wrong index version?
		if (pStoreDescriptor->IndexVersion<CURIDXVERSION)
			*pType = (*pType & ~LFTypeBadgeMask) | LFTypeBadgeError;

		// Mounted?
		if (!LFIsStoreMounted(pStoreDescriptor))
			*pType |= LFTypeNotMounted | LFTypeGhosted;

		// Capabilities
		if ((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal)
			*pType |= LFTypeShortcutAllowed;

		if ((pStoreDescriptor->Mode & LFStoreModeBackendMask)!=LFStoreModeBackendInternal)
			*pType |= LFTypeSynchronizeAllowed;
	}

	return pStoreDescriptor->Source;
}

LFCORE_API UINT LFCreateStoreLiquidfolders(WCHAR* pStoreName, WCHAR* pComments, CHAR cVolume, BOOL MakeSearchable)
{
	if (!GetMutexForStores())
		return LFMutexError;

	LFStoreDescriptor Store;
	GetDescriptorForNewStore(&Store);

#if (LFStoreModeBackendInternal!=0)
	Store.Mode = LFStoreModeBackendInternal;
#endif

	// Set data
	if (pStoreName)
		wcscpy_s(Store.StoreName, 256, pStoreName);

	if (pComments)
		wcscpy_s(Store.Comments, 256, pComments);

	if (cVolume)
	{
		swprintf_s(Store.DatPath, MAX_PATH, L"%c:\\", cVolume);
		AppendGUID(&Store, Store.DatPath);

		Store.Mode |= (LFGetSourceForVolume(cVolume)==LFTypeSourceUnknown) ? LFStoreModeIndexInternal : MakeSearchable ? LFStoreModeIndexHybrid : LFStoreModeIndexExternal;
	}
	else
	{
		Store.AutoLocation = TRUE;
	}

	return CommitInitializeStore(&Store);
}

LFCORE_API UINT LFCreateStoreWindows(WCHAR* pPath, LFProgress* pProgress)
{
	assert(pPath);

	LFStoreDescriptor Store;
	GetDescriptorForNewStore(&Store);

	UINT Source = LFGetSourceForVolume((CHAR)*pPath);
	Store.Mode = LFStoreModeBackendWindows | ((Source==LFTypeSourceUSB) || (Source==LFTypeSource1394) ? LFStoreModeIndexExternal : LFStoreModeIndexInternal);

	// Set data
	BOOL TrailingBackslash = (pPath[wcslen(pPath)-1]==L'\\');

	wcscpy_s(Store.DatPath, 256, pPath);
	if (!TrailingBackslash)
		wcscat_s(Store.DatPath, MAX_PATH, L"\\");

	wcscpy_s(Store.StoreName, 256, pPath);

	if (TrailingBackslash)
		Store.StoreName[wcslen(Store.StoreName)-1] = L'\0';

	WCHAR* Ptr = wcsrchr(Store.StoreName, L'\\');
	if (Ptr)
		wcscpy_s(Store.StoreName, 256, Ptr+1);

	return CommitInitializeStore(&Store, pProgress);
}

LFCORE_API UINT LFMakeStoreSearchable(CHAR* pStoreID, BOOL Searchable)
{
	assert(pStoreID);

	if (!GetMutexForStores())
		return LFMutexError;

	UINT Result;
	CStore* pStore;
	if ((Result=GetStore(pStoreID, &pStore))==LFOk)
	{
		LFStoreDescriptor* pStoreDescriptor = pStore->p_StoreDescriptor;
		LFStoreDescriptor Victim = *pStoreDescriptor;

		switch (pStoreDescriptor->Mode & LFStoreModeIndexMask)
		{
		case LFStoreModeIndexHybrid:
			if (!Searchable)
				if (LFIsStoreMounted(pStoreDescriptor))
				{
					if ((Result=DeleteStoreSettingsFromRegistry(pStoreDescriptor))==LFOk)
					{
						// Convert to external index
						pStoreDescriptor->Mode = (pStoreDescriptor->Mode & ~LFStoreModeIndexMask) | LFStoreModeIndexExternal;
						SetStoreAttributes(pStoreDescriptor);

						if ((Result=UpdateStoreInCache(pStoreDescriptor))==LFOk)
						{
							// Delete aux index
							WCHAR Path[MAX_PATH];
							GetAutoPath(&Victim, Path);
							DeleteDirectory(Path);
						}
					}
				}
				else
				{
					// Store is not mounted, so delete local copy
					if ((Result=pStore->PrepareDelete())==LFOk)
						if ((Result=pStore->CommitDelete())==LFOk)
							Result = DeleteStoreFromCache(&Victim);

					pStoreDescriptor = NULL;
				}

			break;

		case LFStoreModeIndexExternal:
			if (Searchable)
			{
				assert(LFIsStoreMounted(pStoreDescriptor));

				// Convert to hybrid index
				pStoreDescriptor->Mode = (pStoreDescriptor->Mode & ~LFStoreModeIndexMask) | LFStoreModeIndexHybrid;
				SetStoreAttributes(pStoreDescriptor);

				if ((Result=UpdateStoreInCache(pStoreDescriptor))==LFOk)
					if ((Result=pStore->CreateDirectories())==LFOk)
						Result = CopyDirectory(pStoreDescriptor->IdxPathMain, pStoreDescriptor->IdxPathAux);
			}

			break;
		}

		delete pStore;

		// Notifications
		if (Result==LFOk)
		{
			SendLFNotifyMessage(LFMessages.StoreAttributesChanged);
			SendShellNotifyMessage(pStoreDescriptor ? SHCNE_UPDATEITEM : SHCNE_RMDIR, pStoreID);
			SendShellNotifyMessage(SHCNE_UPDATEDIR);
		}
	}

	return Result;
}

LFCORE_API UINT LFDeleteStore(CHAR* pStoreID, LFProgress* pProgress)
{
	assert(pStoreID);

	if (!GetMutexForStores())
		return LFMutexError;

	UINT Result;
	HMUTEX hMutex;

	LFStoreDescriptor* pStoreDescriptor = FindStore(pStoreID, &hMutex);
	if (pStoreDescriptor)
	{
		// Progress
		if (pProgress)
		{
			pProgress->MinorCount = 2;
			pProgress->MinorCurrent = 0;
			pProgress->NoMinorCounter = TRUE;
			wcscpy_s(pProgress->Object, 256, pStoreDescriptor->StoreName);

			if (UpdateProgress(pProgress))
			{
				ReleaseMutexForStores();
				ReleaseMutexForStore(hMutex);

				return LFCancel;
			}
		}

		// !!!WARNING!!!
		// The store object must be created on a copy of the store descriptor,
		// otherwise removing the store before actual deletion may move another
		// store descriptor to the same address!
		LFStoreDescriptor Victim = *pStoreDescriptor;

		// Get CStore object
		CStore* pStore;
		if ((Result=GetStore(&Victim, &pStore, hMutex))==LFOk)
		{
			if ((Result=pStore->PrepareDelete())==LFOk)
				if ((Result=DeleteStoreFromCache(pStoreDescriptor))==LFOk)
				{
					ReleaseMutexForStores();

					SendLFNotifyMessage(LFMessages.StoresChanged);
					SendShellNotifyMessage(SHCNE_RMDIR, Victim.StoreID);
					SendShellNotifyMessage(SHCNE_UPDATEDIR);

					// Progress
					if (pProgress)
					{
						pProgress->MinorCurrent++;
						UpdateProgress(pProgress);
					}

					Result = pStore->CommitDelete();
				}
				else
				{
					if (pProgress)
						pProgress->MinorCount++;
				}

			delete pStore;
		}

		// Progress
		if (pProgress)
		{
			pProgress->MinorCurrent++;
			UpdateProgress(pProgress);
		}
	}
	else
	{
		ReleaseMutexForStores();

		Result = LFIllegalID;
	}

	return Result;
}

LFCORE_API UINT LFSetStoreAttributes(CHAR* pStoreID, WCHAR* pName, WCHAR* pComment)
{
	assert(pStoreID);

	if ((!pName) && (!pComment))
		return LFOk;

	if (pName)
		if (pName[0]==L'\0')
			return LFIllegalValue;

	if (!GetMutexForStores())
		return LFMutexError;

	LPITEMIDLIST pidlOld;
	LPITEMIDLIST pidlOldDelegate;
	GetPIDLsForStore(pStoreID, &pidlOld, &pidlOldDelegate);

	UINT Result;
	LFStoreDescriptor* pStoreDescriptor = FindStore(pStoreID);
	if (pStoreDescriptor)
	{
		if (pName)
			wcscpy_s(pStoreDescriptor->StoreName, 256, pName);

		if (pComment)
			wcscpy_s(pStoreDescriptor->Comments, 256, pComment);

		Result = UpdateStoreInCache(pStoreDescriptor);
	}
	else
	{
		Result = LFIllegalID;
	}

	ReleaseMutexForStores();

	if (Result==LFOk)
	{
		SendLFNotifyMessage(LFMessages.StoreAttributesChanged);

		if (pComment)
			SendShellNotifyMessage(SHCNE_UPDATEITEM, pStoreID);

		if (pName)
		{
			SendShellNotifyMessage(SHCNE_RENAMEFOLDER, pStoreID, pidlOld);
			SendShellNotifyMessage(SHCNE_RENAMEFOLDER, pStoreID, pidlOldDelegate);
			SendShellNotifyMessage(SHCNE_UPDATEDIR);
		}
	}

	CoTaskMemFree(pidlOld);
	CoTaskMemFree(pidlOldDelegate);

	return Result;
}

LFCORE_API UINT LFSynchronizeStore(CHAR* pStoreID, LFProgress* pProgress)
{
	assert(pStoreID);

	CStore* pStore;
	UINT Result;
	if ((Result=OpenStore(pStoreID, TRUE, &pStore))==LFOk)
	{
		Result = pStore->Synchronize(FALSE, pProgress);
		delete pStore;
	}

	return Result;
}

LFCORE_API LFMaintenanceList* LFScheduledMaintenance(LFProgress* pProgress)
{
	LFMaintenanceList* pMaintenanceList = new LFMaintenanceList();

	CHAR* pStoreIDs;
	UINT Count;
	if ((pMaintenanceList->m_LastError=LFGetAllStores(&pStoreIDs, &Count))!=LFOk)
		return pMaintenanceList;

	if (Count)
	{
		if (pProgress)
			pProgress->MajorCount = Count;

		CHAR* Ptr = pStoreIDs;
		for (UINT a=0; a<Count; a++)
		{
			CStore* pStore;
			if ((pMaintenanceList->m_LastError=GetStore(Ptr, &pStore))!=LFOk)
			{
				free(pStoreIDs);
				return pMaintenanceList;
			}

			pStore->ScheduledMaintenance(pMaintenanceList, pProgress);
			delete pStore;

			Ptr += LFKeySize;

			if (pProgress)
			{
				if (pProgress->UserAbort)
					break;

				pProgress->MajorCurrent++;
			}
		}

		free(pStoreIDs);
	}

	SendLFNotifyMessage(LFMessages.StoreAttributesChanged);

	return pMaintenanceList;
}

LFCORE_API UINT LFGetFileLocation(LFItemDescriptor* pItemDescriptor, WCHAR* pPath, SIZE_T cCount, BOOL RemoveNew, BOOL CheckExists)
{
	assert(pItemDescriptor);
	assert(pPath);
	assert(cCount>=MAX_PATH);

	// For files only
	if ((pItemDescriptor->Type & LFTypeMask)!=LFTypeFile)
		return LFIllegalItemType;

	// Linked files are not locally stored
	if (pItemDescriptor->CoreAttributes.Flags & LFFlagLink)
	{
		*pPath = L'\0';
		return LFOk;
	}

	// Get file location from backend
	CStore* pStore;
	UINT Result;
	if ((Result=GetStore(pItemDescriptor->StoreID, &pStore))==LFOk)
	{
		WCHAR Path[2*MAX_PATH];
		if ((Result=pStore->GetFileLocation(pItemDescriptor, Path, 2*MAX_PATH))==LFOk)
		{
			if (CheckExists || (RemoveNew && (pItemDescriptor->CoreAttributes.Flags & LFFlagNew)))
			{
				BOOL Exists = FileExists(Path);

				if ((Exists!=((pItemDescriptor->CoreAttributes.Flags & LFFlagMissing)==0)) || (RemoveNew && (pItemDescriptor->CoreAttributes.Flags & LFFlagNew)))
					if ((Result=pStore->Open(TRUE))==LFOk)
					{
						pStore->UpdateMissingFlag(pItemDescriptor, Exists, RemoveNew);

						if (RemoveNew)
							SendLFNotifyMessage(LFMessages.StatisticsChanged);
					}

				if (CheckExists && !Exists)
					Result = LFNoFileBody;
			}
			if (Result==LFOk)
				if (wcslen(&Path[4])<=MAX_PATH)
				{
					wcscpy_s(pPath, cCount, &Path[4]);
				}
				else
				{
					GetShortPathName(&Path[4], pPath, (DWORD)cCount);
				}
		}

		delete pStore;
	}

	return Result;
}


// Queries
//

void QueryStores(LFSearchResult* pSearchResult)
{
	assert(pSearchResult);

	pSearchResult->m_HasCategories = TRUE;

	// Stores
	if (GetMutexForStores())
	{
		for (UINT a=0; a<StoreCount; a++)
			pSearchResult->AddStoreDescriptor(&StoreCache[a]);

		ReleaseMutexForStores();
	}
	else
	{
		pSearchResult->m_LastError = LFMutexError;
	}
}

LFCORE_API LFStatistics* LFQueryStatistics(CHAR* StoreID)
{
	LFStatistics* pStatistics = new LFStatistics;
	ZeroMemory(pStatistics, sizeof(LFStatistics));

	if (GetMutexForStores())
	{
		if ((!StoreID) || (*StoreID==L'\0'))
		{
			// All stores
			for (UINT a=0; a<StoreCount; a++)
				for (UINT Context=0; Context<=min(LFLastQueryContext, 31); Context++)
				{
					pStatistics->FileCount[Context] += StoreCache[a].FileCount[Context];
					pStatistics->FileSize[Context] += StoreCache[a].FileSize[Context];
				}
		}
		else
		{
			// Single store
			HANDLE StoreLock = NULL;
			LFStoreDescriptor* pStoreDescriptor = FindStore(StoreID, &StoreLock);

			if (pStoreDescriptor)
			{
				for (UINT Context=0; Context<=min(LFLastQueryContext, 31); Context++)
				{
					pStatistics->FileCount[Context] += pStoreDescriptor->FileCount[Context];
					pStatistics->FileSize[Context] += pStoreDescriptor->FileSize[Context];
				}

				ReleaseMutexForStore(StoreLock);
			}
		}

		ReleaseMutexForStores();
	}
	else
	{
		pStatistics->LastError = LFMutexError;
	}

	return pStatistics;
}


// Initializing
//

__forceinline void LoadRegistry()
{
	HKEY hKey;
	if (RegOpenKeyA(HKEY_CURRENT_USER, LFSTORESHIVE, &hKey)==ERROR_SUCCESS)
	{
		DWORD Subkeys;
		if (RegQueryInfoKey(hKey, NULL, 0, NULL, &Subkeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL)==ERROR_SUCCESS)
			for (DWORD a=0; a<Subkeys; a++)
			{
				// Slot available?
				if (StoreCount>=MAXSTORES)
					break;

				// Enum next ID
				CHAR ID[256];
				DWORD Size = 255;
				if (RegEnumKeyA(hKey, a, ID, Size)==ERROR_SUCCESS)
					if (LoadStoreSettingsFromRegistry(ID, &StoreCache[StoreCount]))
						SetStoreAttributes(&StoreCache[StoreCount++]);
			}
	}

	RegCloseKey(hKey);
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

void InitStores()
{
	if (GetMutexForStores())
	{
		if (!Initialized)
		{
			Initialized = TRUE;

			// Load default store
			ZeroMemory(DefaultStore, sizeof(DefaultStore));

			HKEY hKey;
			if (RegOpenKeyA(HKEY_CURRENT_USER, LFSTORESHIVE, &hKey)==ERROR_SUCCESS)
			{
				DWORD Type;
				DWORD Size = LFKeySize;
				RegQueryValueExA(hKey, "DefaultStore", NULL, &Type, (BYTE*)DefaultStore, &Size);
				RegCloseKey(hKey);
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
				// Maintenance
				CStore* pStore;
				if (GetStore(&StoreCache[a], &pStore)==LFOk)
				{
					pStore->MaintenanceAndStatistics();
					delete pStore;
				}

				// Default store
				if (strcmp(DefaultStore, StoreCache[a].StoreID)==0)
					DefaultStoreOk = TRUE;
			}

			// Choose new default store if neccessary
			if ((!DefaultStoreOk) && (StoreCount))
				ChooseNewDefaultStore();
		}

		ReleaseMutexForStores();
	}
}


// Volume handling
//

UINT UnmountVolume(CHAR cVolume)
{
	assert(cVolume>='A');
	assert(cVolume<='Z');

	if (!GetMutexForStores())
		return LFMutexError;

	UINT Result = LFOk;

	BOOL ChangeOccured = FALSE;
	BOOL RemovedDefaultStore = FALSE;
	VolumeTypes[cVolume-'A'] = DRIVE_UNKNOWN;

	CHAR NotifyIDs[MAXSTORES][LFKeySize];
	BOOL NotifyTypes[MAXSTORES];
	UINT NotifyCount = 0;

	for (UINT a=0; a<StoreCount; a++)
		if (StoreCache[a].DatPath[0]==cVolume)
		{
			HANDLE hMutex;
			if (!GetMutexForStore(&StoreCache[a], &hMutex))
			{
				Result = LFMutexError;
				continue;
			}

			// New default store required
			RemovedDefaultStore |= (strcmp(StoreCache[a].StoreID, DefaultStore)==0);
			ChangeOccured = TRUE;

			if ((StoreCache[a].Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid)
			{
				// Notification
				strcpy_s(NotifyIDs[NotifyCount], LFKeySize, StoreCache[a].StoreID);
				NotifyTypes[NotifyCount++] = FALSE;

				// Unmount
				StoreCache[a].DatPath[0] = StoreCache[a].IdxPathMain[0] = L'\0';
			}
			else
			{
				// Notification
				strcpy_s(NotifyIDs[NotifyCount], LFKeySize, StoreCache[a].StoreID);
				NotifyTypes[NotifyCount++] = TRUE;

				// Unmount
				if (a<StoreCount-1)
				{
					HANDLE hMutexMove;
					if (!GetMutexForStore(&StoreCache[StoreCount-1], &hMutexMove))
					{
						ReleaseMutexForStore(hMutex);

						Result = LFMutexError;
						continue;
					}

					StoreCache[a--] = StoreCache[--StoreCount];
					ReleaseMutexForStore(hMutexMove);
				}
				else
				{
					StoreCount--;
				}
			}

			ReleaseMutexForStore(hMutex);
		}

	// Ggf. anderen Store als neuen Default Store
	if (RemovedDefaultStore)
		ChooseNewDefaultStore();
	
	ReleaseMutexForStores();

	if (ChangeOccured)
	{
		SendLFNotifyMessage(LFMessages.StoresChanged);

		for (UINT a=0; a<NotifyCount; a++)
			SendShellNotifyMessage(NotifyTypes[a] ? SHCNE_RMDIR : SHCNE_UPDATEITEM, NotifyIDs[a]);
		SendShellNotifyMessage(SHCNE_UPDATEDIR);
	}

	return Result;
}
