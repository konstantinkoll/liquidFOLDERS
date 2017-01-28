
#include "stdafx.h"
#include "CStoreInternal.h"
#include "CStoreWindows.h"
#include "FileSystem.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "Stores.h"
#include <assert.h>


extern HMODULE LFCoreModuleHandle;
extern LFMessageIDs LFMessages;
extern LFVolumeDescriptor Volumes[26];


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

LFStoreDescriptor* FindStore(const CHAR* pStoreID, HMUTEX* phMutex)
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

LFStoreDescriptor* FindStore(const GUID UniqueID, HMUTEX* phMutex)
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

LFStoreDescriptor* FindStore(const WCHAR* pDatPath, HMUTEX* phMutex)
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


// Persistence
//

void CompleteStoreSettings(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);
	assert(((pStoreDescriptor->Mode & LFStoreModeIndexMask)>=LFStoreModeIndexInternal) && ((pStoreDescriptor->Mode & LFStoreModeIndexMask)<=LFStoreModeIndexExternal));

	// Source: only set if unknown to retain USB/IEEE1394 icons when unmounted
	if (pStoreDescriptor->Source==LFTypeSourceUnknown)
		pStoreDescriptor->Source = max(LFTypeSourceInternal, (pStoreDescriptor->Mode & LFStoreModeBackendMask)>>LFStoreModeBackendShift);

	// Store name and source of mounted volume
	if ((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
		if (LFIsStoreMounted(pStoreDescriptor))
		{
			WCHAR szVolumeRoot[] = L" :\\";
			szVolumeRoot[0] = pStoreDescriptor->DatPath[0];

			SHFILEINFO sfi;
			if (SHGetFileInfo(szVolumeRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME))
				wcscpy_s(pStoreDescriptor->LastSeen, 256, sfi.szDisplayName);

			if ((pStoreDescriptor->Mode & LFStoreModeBackendMask)<=LFStoreModeBackendWindows)
			{
				UINT Source = LFGetSourceForVolume((CHAR)pStoreDescriptor->DatPath[0]);

				if (Source>LFTypeSourceInternal)
					pStoreDescriptor->Source = Source;
			}
		}

	// Paths
	if ((pStoreDescriptor->Mode & LFStoreModeBackendMask)==LFStoreModeBackendWindows)
	{
		if (LFIsStoreMounted(pStoreDescriptor))
		{
			if (LFGetSourceForVolume((CHAR)pStoreDescriptor->DatPath[0])>LFTypeSourceInternal)
			{
				wcsncpy_s(pStoreDescriptor->IdxPathMain, MAX_PATH, pStoreDescriptor->DatPath, 3);
				AppendGUID(pStoreDescriptor->IdxPathMain, pStoreDescriptor);
			}
			else
			{
				GetAutoPath(pStoreDescriptor, pStoreDescriptor->IdxPathMain);
			}

			// Dropbox
			WCHAR szPath[MAX_PATH];
			wcscpy_s(szPath, MAX_PATH, pStoreDescriptor->DatPath);
			wcscat_s(szPath, MAX_PATH, L".dropbox");

			if (FileExists(szPath))
				pStoreDescriptor->Source = LFTypeSourceDropbox;

			// iCloud
			if (LFGetICloudPath(szPath))
			{
				wcscat_s(szPath, MAX_PATH, L"\\");

				if (wcsncmp(szPath, pStoreDescriptor->DatPath, wcslen(szPath))==0)
					pStoreDescriptor->Source = LFTypeSourceICloud;
			}

			// OneDrive
			LFOneDrivePaths OneDrivePaths;
			if (LFGetOneDrivePaths(OneDrivePaths))
			{
				// Root path
				wcscat_s(OneDrivePaths.OneDrive, MAX_PATH, L"\\");
				if (wcsncmp(OneDrivePaths.OneDrive, pStoreDescriptor->DatPath, wcslen(OneDrivePaths.OneDrive))==0)
					pStoreDescriptor->Source = LFTypeSourceOneDrive;

				// Camera roll
				if (OneDrivePaths.CameraRoll[0]!=L'\0')
				{
					wcscat_s(OneDrivePaths.CameraRoll, MAX_PATH, L"\\");

					if (wcsncmp(OneDrivePaths.CameraRoll, pStoreDescriptor->DatPath, wcslen(OneDrivePaths.CameraRoll))==0)
						pStoreDescriptor->Source = LFTypeSourceOneDrive;
				}

				// Documents
				if (OneDrivePaths.Documents[0]!=L'\0')
				{
					wcscat_s(OneDrivePaths.Documents, MAX_PATH, L"\\");

					if (wcsncmp(OneDrivePaths.Documents, pStoreDescriptor->DatPath, wcslen(OneDrivePaths.Documents))==0)
						pStoreDescriptor->Source = LFTypeSourceOneDrive;
				}

				// Pictures
				if (OneDrivePaths.Pictures[0]!=L'\0')
				{
					wcscat_s(OneDrivePaths.Pictures, MAX_PATH, L"\\");

					if (wcsncmp(OneDrivePaths.Pictures, pStoreDescriptor->DatPath, wcslen(OneDrivePaths.Pictures))==0)
						pStoreDescriptor->Source = LFTypeSourceOneDrive;
				}
			}
		}
		else
		{
			pStoreDescriptor->IdxPathMain[0] = L'\0';
		}
	}
	else
	{
		// Get automatic data path
		if (((pStoreDescriptor->Mode & LFStoreModeIndexMask)==LFStoreModeIndexInternal) && (pStoreDescriptor->Flags & LFStoreFlagsAutoLocation))
			GetAutoPath(pStoreDescriptor, pStoreDescriptor->DatPath);

		// Main index
		if (((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexHybrid) || (LFIsStoreMounted(pStoreDescriptor)))
		{
			wcscpy_s(pStoreDescriptor->IdxPathMain, MAX_PATH, pStoreDescriptor->DatPath);
			wcscat_s(pStoreDescriptor->IdxPathMain, MAX_PATH, L"INDEX\\");
		}
		else
		{
			pStoreDescriptor->IdxPathMain[0] = L'\0';
		}
	}

	// Set aux index for local hybrid stores
	if ((pStoreDescriptor->Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid)
	{
		GetAutoPath(pStoreDescriptor, pStoreDescriptor->IdxPathAux);
		wcscat_s(pStoreDescriptor->IdxPathAux, MAX_PATH, L"INDEX\\");
	}
	else
	{
		pStoreDescriptor->IdxPathAux[0] = L'\0';
	}
}

void GetRegistryKey(CHAR* pStoreID, CHAR* pKey)
{
	assert(pStoreID);
	assert(pKey);

	strcpy_s(pKey, 256, LFSTORESHIVE);
	strcat_s(pKey, 256, "\\");
	strcat_s(pKey, 256, pStoreID);
}

BOOL LoadStoreSettingsFromRegistry(CHAR* pStoreID, LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreID);
	assert(pStoreDescriptor);

	BOOL Result = FALSE;

	CHAR Key[256];
	GetRegistryKey(pStoreID, Key);

	HKEY hKey;
	if (RegOpenKeyA(HKEY_CURRENT_USER, Key, &hKey)==ERROR_SUCCESS)
	{
		ZeroMemory(pStoreDescriptor, sizeof(LFStoreDescriptor));
		strcpy_s(pStoreDescriptor->StoreID, LFKeySize, pStoreID);

		Result = TRUE;

		DWORD Size = sizeof(pStoreDescriptor->StoreName);
		if (RegQueryValueEx(hKey, L"Name", 0, NULL, (BYTE*)&pStoreDescriptor->StoreName, &Size)!=ERROR_SUCCESS)
			Result = FALSE;

		Size = sizeof(pStoreDescriptor->Comments);
		RegQueryValueEx(hKey, L"Comment", 0, NULL, (BYTE*)&pStoreDescriptor->Comments, &Size);

		Size = sizeof(pStoreDescriptor->Mode);
		if (RegQueryValueEx(hKey, L"Mode", 0, NULL, (BYTE*)&pStoreDescriptor->Mode, &Size)!=ERROR_SUCCESS)
			Result = FALSE;

		Size = sizeof(pStoreDescriptor->UniqueID);
		if (RegQueryValueEx(hKey, L"GUID", 0, NULL, (BYTE*)&pStoreDescriptor->UniqueID, &Size)!=ERROR_SUCCESS)
			Result = FALSE;

		Size = sizeof(pStoreDescriptor->CreationTime);
		RegQueryValueEx(hKey, L"CreationTime", 0, NULL, (BYTE*)&pStoreDescriptor->CreationTime, &Size);

		Size = sizeof(pStoreDescriptor->FileTime);
		RegQueryValueEx(hKey, L"FileTime", 0, NULL, (BYTE*)&pStoreDescriptor->FileTime, &Size);

		Size = sizeof(pStoreDescriptor->MaintenanceTime);
		RegQueryValueEx(hKey, L"MaintenanceTime", 0, NULL, (BYTE*)&pStoreDescriptor->MaintenanceTime, &Size);

		Size = sizeof(pStoreDescriptor->SynchronizeTime);
		RegQueryValueEx(hKey, L"SynchronizeTime", 0, NULL, (BYTE*)&pStoreDescriptor->SynchronizeTime, &Size);

		Size = sizeof(pStoreDescriptor->Flags);
		if (RegQueryValueEx(hKey, L"AutoLocation", 0, NULL, (BYTE*)&pStoreDescriptor->Flags, &Size)!=ERROR_SUCCESS)
			Result = FALSE;

		pStoreDescriptor->Flags &= LFStoreFlagsAutoLocation;

		Size = sizeof(pStoreDescriptor->IndexVersion);
		if (RegQueryValueEx(hKey, L"IndexVersion", 0, NULL, (BYTE*)&pStoreDescriptor->IndexVersion, &Size)!=ERROR_SUCCESS)
			Result = FALSE;

		switch (pStoreDescriptor->Mode & LFStoreModeIndexMask)
		{
		case LFStoreModeIndexInternal:
			Size = sizeof(pStoreDescriptor->DatPath);
			if (RegQueryValueEx(hKey, L"Path", 0, NULL, (BYTE*)pStoreDescriptor->DatPath, &Size)!=ERROR_SUCCESS)
				Result &= ((pStoreDescriptor->Flags & LFStoreFlagsAutoLocation)!=0);

			break;

		case LFStoreModeIndexHybrid:
			Size = sizeof(pStoreDescriptor->LastSeen);
			RegQueryValueEx(hKey, L"LastSeen", 0, NULL, (BYTE*)&pStoreDescriptor->LastSeen, &Size);

		default:
			Size = sizeof(pStoreDescriptor->Source);
			RegQueryValueEx(hKey, L"Source", 0, NULL, (BYTE*)&pStoreDescriptor->Source, &Size);
		}

		RegCloseKey(hKey);
	}

	return Result;
}

UINT SaveStoreSettingsToRegistry(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);
	assert((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal);

	UINT Result = LFRegistryError;

	CHAR Key[256];
	GetRegistryKey(pStoreDescriptor->StoreID, Key);

	HKEY hKey;
	if (RegCreateKeyA(HKEY_CURRENT_USER, Key, &hKey)==ERROR_SUCCESS)
	{
		Result = LFOk;

		if (RegSetValueEx(hKey, L"Name", 0, REG_SZ, (BYTE*)pStoreDescriptor->StoreName, (DWORD)wcslen(pStoreDescriptor->StoreName)*sizeof(WCHAR))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"Comment", 0, REG_SZ, (BYTE*)pStoreDescriptor->Comments, (DWORD)wcslen(pStoreDescriptor->Comments)*sizeof(WCHAR))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"Mode", 0, REG_DWORD, (BYTE*)&pStoreDescriptor->Mode, sizeof(UINT))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"GUID", 0, REG_BINARY, (BYTE*)&pStoreDescriptor->UniqueID, sizeof(GUID))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"CreationTime", 0, REG_BINARY, (BYTE*)&pStoreDescriptor->CreationTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"FileTime", 0, REG_BINARY, (BYTE*)&pStoreDescriptor->FileTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"MaintenanceTime", 0, REG_BINARY, (BYTE*)&pStoreDescriptor->MaintenanceTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"SynchronizeTime", 0, REG_BINARY, (BYTE*)&pStoreDescriptor->SynchronizeTime, sizeof(FILETIME))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"AutoLocation", 0, REG_DWORD, (BYTE*)&pStoreDescriptor->Flags, sizeof(UINT))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		if (RegSetValueEx(hKey, L"IndexVersion", 0, REG_DWORD, (BYTE*)&pStoreDescriptor->IndexVersion, sizeof(UINT))!=ERROR_SUCCESS)
			Result = LFRegistryError;

		switch(pStoreDescriptor->Mode & LFStoreModeIndexMask)
		{
		case LFStoreModeIndexInternal:
			if ((pStoreDescriptor->Flags & LFStoreFlagsAutoLocation)==0)
				if (RegSetValueEx(hKey, L"Path", 0, REG_SZ, (BYTE*)pStoreDescriptor->DatPath, (DWORD)wcslen(pStoreDescriptor->DatPath)*sizeof(WCHAR))!=ERROR_SUCCESS)
					Result = LFRegistryError;

			break;

		case LFStoreModeIndexHybrid:
			if (RegSetValueEx(hKey, L"LastSeen", 0, REG_SZ, (BYTE*)pStoreDescriptor->LastSeen, (DWORD)wcslen(pStoreDescriptor->LastSeen)*sizeof(WCHAR))!=ERROR_SUCCESS)
				Result = LFRegistryError;

		default:
			if (RegSetValueEx(hKey, L"Source", 0, REG_DWORD, (BYTE*)&pStoreDescriptor->Source, sizeof(UINT))!=ERROR_SUCCESS)
				Result = LFRegistryError;
		}

		RegCloseKey(hKey);
	}

	return Result;
}

UINT DeleteStoreSettingsFromRegistry(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);
	assert((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal);

	CHAR Key[256];
	GetRegistryKey(pStoreDescriptor->StoreID, Key);

	LSTATUS lStatus = RegDeleteKeyA(HKEY_CURRENT_USER, Key);

	return (lStatus==ERROR_SUCCESS) || (lStatus==ERROR_FILE_NOT_FOUND) ? LFOk: LFRegistryError;
}


UINT GetKeyFileFromStoreDescriptor(LFStoreDescriptor* pStoreDescriptor, WCHAR* pPath)
{
	assert(pStoreDescriptor);
	assert((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal);
	assert(pPath);

	if (!LFIsStoreMounted(pStoreDescriptor))
		return LFStoreNotMounted;

	wcsncpy_s(pPath, MAX_PATH, pStoreDescriptor->DatPath, 3);		// .store file is always in root directory
	AppendGUID(pPath, pStoreDescriptor, L".store");

	return LFOk;
}

BOOL LoadStoreSettingsFromFile(WCHAR* pPath, LFStoreDescriptor* pStoreDescriptor)
{
	assert(pPath);
	assert(pPath[0]!=L'\0');
	assert(pStoreDescriptor);

	HANDLE hFile = CreateFile(pPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return FALSE;

	ZeroMemory(pStoreDescriptor, sizeof(LFStoreDescriptor));

	DWORD wmRead;
	BOOL Result = ReadFile(hFile, pStoreDescriptor, STOREDESCRIPTORFILESIZE, &wmRead, NULL);
	Result &= (wmRead>=STOREDESCRIPTORREQUIREDFILESIZE);

	CloseHandle(hFile);

	return Result;
}

UINT SaveStoreSettingsToFile(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);
	assert((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal);

	UINT Result;
	WCHAR Path[MAX_PATH];
	if ((Result=GetKeyFileFromStoreDescriptor(pStoreDescriptor, Path))!=LFOk)
		return Result;

	HANDLE hFile = CreateFile(Path, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_WRITE_THROUGH, NULL);
	if (hFile==INVALID_HANDLE_VALUE)
		return (GetLastError()==ERROR_ACCESS_DENIED) ? LFAccessError : LFDriveNotReady;

	DWORD wmWritten;
	Result = WriteFile(hFile, pStoreDescriptor, STOREDESCRIPTORFILESIZE, &wmWritten, NULL) ? LFOk : LFDriveNotReady;
	if (wmWritten!=STOREDESCRIPTORFILESIZE)
		Result = LFDriveNotReady;

	CloseHandle(hFile);

	return Result;
}

UINT DeleteStoreSettingsFromFile(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);
	assert((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal);

	UINT Result;
	WCHAR Path[MAX_PATH];
	if ((Result=GetKeyFileFromStoreDescriptor(pStoreDescriptor, Path))!=LFOk)
		return Result;

	return DeleteFile(Path) ? LFOk : (GetLastError()==ERROR_ACCESS_DENIED) ? LFAccessError : LFDriveNotReady;
}

UINT SaveStoreSettings(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);

	UINT Result = LFOk;

	if ((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal)
		Result = SaveStoreSettingsToRegistry(pStoreDescriptor);

	if ((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
		if ((Result==LFOk) && (LFIsStoreMounted(pStoreDescriptor)))
			Result = SaveStoreSettingsToFile(pStoreDescriptor);

	return Result;
}

UINT UpdateStoreInCache(LFStoreDescriptor* pStoreDescriptor, BOOL UpdateFileTime, BOOL MakeDefault)
{
	assert(pStoreDescriptor);

	// Update time
	GetSystemTimeAsFileTime(UpdateFileTime ? &pStoreDescriptor->FileTime : &pStoreDescriptor->MaintenanceTime);

	// Find in cache
	LFStoreDescriptor* pSlot = FindStore(pStoreDescriptor->StoreID);

	if (!pSlot)
		if (StoreCount==MAXSTORES)
			return LFTooManyStores;

	// Save to registry and/or file
	UINT Result = SaveStoreSettings(pStoreDescriptor);

	// Cache
	if (Result==LFOk)
	{
		// Add or update
		(pSlot ? *pSlot : StoreCache[StoreCount++]) = *pStoreDescriptor;

		// Make default store
		if (MakeDefault || (DefaultStore[0]=='\0'))
			Result = MakeDefaultStore(pStoreDescriptor);
	}

	return Result;
}

UINT DeleteStoreFromCache(LFStoreDescriptor* pStoreDescriptor)
{
	LFStoreDescriptor Victim = *pStoreDescriptor;

	// Remove from cache
	for (UINT a=0; a<StoreCount; a++)
		if (strcmp(StoreCache[a].StoreID, pStoreDescriptor->StoreID)==0)
		{
			if (a<StoreCount-1)
			{
				// Swap with last in cache (store needs to be locked first)
				HMUTEX hMutex;
				if (!GetMutexForStore(&StoreCache[StoreCount-1], &hMutex))
					return LFMutexError;

				StoreCache[a] = StoreCache[--StoreCount];
				ReleaseMutexForStore(hMutex);
			}
			else
			{
				// Victim is the last store in cache
				StoreCount--;
			}

			break;
		}

	// Choose new default store
	if (strcmp(Victim.StoreID, DefaultStore)==0)
		ChooseNewDefaultStore();

	// Remove persistent records
	UINT Result = LFOk;

	if ((Victim.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal)
		Result = DeleteStoreSettingsFromRegistry(&Victim);

	if ((Victim.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
		if ((Result==LFOk) && (LFIsStoreMounted(&Victim)))
			Result = DeleteStoreSettingsFromFile(&Victim);

	return Result;
}


// Default store handling
//

UINT MakeDefaultStore(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);

	UINT Result = LFRegistryError;

	HKEY hKey;
	if (RegCreateKeyA(HKEY_CURRENT_USER, LFSTORESHIVE, &hKey)==ERROR_SUCCESS)
	{
		if (RegSetValueExA(hKey, "DefaultStore", 0, REG_SZ, (BYTE*)pStoreDescriptor->StoreID, LFKeySize-1)==ERROR_SUCCESS)
		{
			CHAR OldDefaultStore[LFKeySize];
			strcpy_s(OldDefaultStore, LFKeySize, DefaultStore);

			strcpy_s(DefaultStore, LFKeySize, pStoreDescriptor->StoreID);

			Result = LFOk;
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
		if (RegCreateKeyA(HKEY_CURRENT_USER, LFSTORESHIVE, &hKey)==ERROR_SUCCESS)
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

LFCORE_API UINT LFSetDefaultStore(const CHAR* pStoreID)
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
	else
	{
		ReleaseMutexForStores();
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

UINT GetStore(const CHAR* pStoreID, CStore** ppStore)
{
	assert(ppStore);

	*ppStore = NULL;

	if (!GetMutexForStores())
		return LFMutexError;

	HMUTEX hMutex;
	LFStoreDescriptor* pStoreDescriptor = FindStore(pStoreID, &hMutex);
	UINT Result = GetStore(pStoreDescriptor, ppStore, hMutex);

	ReleaseMutexForStores();

	return Result;
}

UINT OpenStore(const CHAR* pStoreID, BOOL WriteAccess, CStore** ppStore)
{
	UINT Result = GetStore(pStoreID, ppStore);

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

	CompleteStoreSettings(pStoreDescriptor);

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

LFCORE_API UINT LFGetStoreSettings(const CHAR* pStoreID, LFStoreDescriptor* pStoreDescriptor)
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

		// Error?
		if (pStoreDescriptor->Flags & LFStoreFlagsError)
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

	return max(LFTypeSourceInternal, pStoreDescriptor->Source);
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
		AppendGUID(Store.DatPath, &Store);

		Store.Mode |= (LFGetSourceForVolume(cVolume)==LFTypeSourceUnknown) ? LFStoreModeIndexInternal : MakeSearchable ? LFStoreModeIndexHybrid : LFStoreModeIndexExternal;
	}
	else
	{
		Store.Flags |= LFStoreFlagsAutoLocation;
	}

	return CommitInitializeStore(&Store);
}

LFCORE_API UINT LFCreateStoreWindows(WCHAR* pPath, WCHAR* pStoreName, LFProgress* pProgress)
{
	assert(pPath);

	LFStoreDescriptor Store;
	GetDescriptorForNewStore(&Store);

	Store.Mode = LFStoreModeBackendWindows | (LFGetSourceForVolume((CHAR)*pPath)>LFTypeSourceInternal ? LFStoreModeIndexExternal : LFStoreModeIndexInternal);

	// Set path
	BOOL TrailingBackslash = (pPath[wcslen(pPath)-1]==L'\\');

	wcscpy_s(Store.DatPath, 256, pPath);
	if (!TrailingBackslash)
		wcscat_s(Store.DatPath, MAX_PATH, L"\\");

	// Create name
	wcscpy_s(Store.StoreName, 256, pPath);

	if (TrailingBackslash)
		Store.StoreName[wcslen(Store.StoreName)-1] = L'\0';

	WCHAR* Ptr = wcsrchr(Store.StoreName, L'\\');
	if (Ptr)
		wcscpy_s(Store.StoreName, 256, Ptr+1);

	// Retrieve or store created name
	if (pStoreName)
		if (*pStoreName)
		{
			wcscpy_s(Store.StoreName, 256, pStoreName);
		}
		else
		{
			wcscpy_s(pStoreName, 256, Store.StoreName);
		}

	return CommitInitializeStore(&Store, pProgress);
}

LFCORE_API UINT LFMakeStoreSearchable(const CHAR* pStoreID, BOOL Searchable)
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
						CompleteStoreSettings(pStoreDescriptor);

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
				CompleteStoreSettings(pStoreDescriptor);

				if ((Result=UpdateStoreInCache(pStoreDescriptor))==LFOk)
					if ((Result=pStore->CreateDirectories())==LFOk)
						Result = CopyDirectory(pStoreDescriptor->IdxPathMain, pStoreDescriptor->IdxPathAux);
			}

			break;
		}

		delete pStore;

		// Notifications
		if (Result==LFOk)
			SendLFNotifyMessage(LFMessages.StoreAttributesChanged);
	}

	ReleaseMutexForStores();

	return Result;
}

LFCORE_API UINT LFDeleteStore(const CHAR* pStoreID, LFProgress* pProgress)
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

LFCORE_API UINT LFSetStoreAttributes(const CHAR* pStoreID, WCHAR* pName, WCHAR* pComment)
{
	assert(pStoreID);

	if ((!pName) && (!pComment))
		return LFOk;

	if (pName)
		if (pName[0]==L'\0')
			return LFIllegalValue;

	if (!GetMutexForStores())
		return LFMutexError;

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
		SendLFNotifyMessage(LFMessages.StoreAttributesChanged);

	return Result;
}

LFCORE_API UINT LFSynchronizeStore(const CHAR* pStoreID, LFProgress* pProgress)
{
	assert(pStoreID);

	CStore* pStore;
	UINT Result;
	if ((Result=OpenStore(pStoreID, TRUE, &pStore))==LFOk)
	{
		Result = pStore->Synchronize(FALSE, pProgress);
		delete pStore;

		if (Result==LFOk)
			SendLFNotifyMessage(LFMessages.StoreAttributesChanged);

		SendLFNotifyMessage(LFMessages.StatisticsChanged);
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
					pStatistics->FileCount[Context] = pStoreDescriptor->FileCount[Context];
					pStatistics->FileSize[Context] = pStoreDescriptor->FileSize[Context];
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
	if (RegCreateKeyA(HKEY_CURRENT_USER, LFSTORESHIVE, &hKey)==ERROR_SUCCESS)
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
						CompleteStoreSettings(&StoreCache[StoreCount++]);
			}
	}

	RegCloseKey(hKey);
}

__forceinline void MountExternalVolumes()
{
	DWORD VolumesOnSystem = LFGetLogicalVolumes(LFGLV_EXTERNAL | LFGLV_NETWORK);
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
			if (RegCreateKeyA(HKEY_CURRENT_USER, LFSTORESHIVE, &hKey)==ERROR_SUCCESS)
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

UINT MountVolume(CHAR cVolume, BOOL OnInitialize)
{
	assert(cVolume>='A');
	assert(cVolume<='Z');

	UINT Result = LFOk;
	BOOL ChangeOccured = FALSE;

	WCHAR Mask[] = L" :\\*.store";
	Mask[0] = cVolume;

	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = FindFirstFile(Mask, &FindFileData);

	if (hFind!=INVALID_HANDLE_VALUE)
		do
		{
			// Vollständigen Dateinamen zusammensetzen
			WCHAR Path[MAX_PATH] = L" :\\";
			Path[0] = cVolume;
			wcscat_s(Path, MAX_PATH, FindFileData.cFileName);

			LFStoreDescriptor Store;
			if (LoadStoreSettingsFromFile(Path, &Store))
			{
				if (!GetMutexForStores())
				{
					Result = LFMutexError;
					continue;
				}

				// Lokal gültigen Schlüssel eintragen
				CreateNewStoreID(Store.StoreID);

				// Store mit der selben GUID suchen
				LFStoreDescriptor* pSlot = FindStore(Store.UniqueID);
				if (pSlot)
				{
					// Name, Kommentar und Dateizeit aktualisieren
					wcscpy_s(pSlot->StoreName, 256, Store.StoreName);
					wcscpy_s(pSlot->Comments, 256, Store.Comments);
					pSlot->FileTime = Store.FileTime;
					pSlot->MaintenanceTime = Store.MaintenanceTime;
					pSlot->SynchronizeTime = Store.SynchronizeTime;

					// Do NOT set the store index mode to hybrid to avoid errors when mounting a drive twice!
					// This can occur when the same network volume is mounted as two different drives.
					// When the store had hybrid indexing before, this setting is maintained as only some
					// data is copied from the .store file on disc.
				}
				else
				{
					// Set store index mode to external - this is the only option when the store wasn't known before
					Store.Mode = (Store.Mode & ~LFStoreModeIndexMask) | LFStoreModeIndexExternal;

					// Add to cache
					if (StoreCount<MAXSTORES)
					{
						StoreCache[StoreCount] = Store;
						pSlot = &StoreCache[StoreCount++];		// Slot zeigt auf Eintrag
					}
					else
					{
						Result = LFTooManyStores;
					}
				}

				// Store is in cache
				if (pSlot)
				{
					ChangeOccured = TRUE;

					// Set data path
					wcscpy_s(pSlot->DatPath, MAX_PATH, Store.DatPath);
					pSlot->DatPath[0] = cVolume;

					CompleteStoreSettings(pSlot);

					// Run quick maintenance
					if (!OnInitialize)
					{
						CStore* pStore;
						if (GetStore(pSlot, &pStore)==LFOk)
						{
							Result = pStore->MaintenanceAndStatistics();
							delete pStore;
						}
					}

					// Update "Last seen" in registry for hybrid stores
					if ((pSlot->Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid)
						SaveStoreSettingsToRegistry(pSlot);
				}

				ReleaseMutexForStores();
			}
		}
		while (FindNextFile(hFind, &FindFileData));

	FindClose(hFind);

	if (!OnInitialize && ChangeOccured)
		SendLFNotifyMessage(LFMessages.StoresChanged);

	return Result;
}

UINT UnmountVolume(CHAR cVolume)
{
	assert(cVolume>='A');
	assert(cVolume<='Z');

	if (!GetMutexForStores())
		return LFMutexError;

	UINT Result = LFOk;

	BOOL ChangeOccured = FALSE;
	BOOL RemovedDefaultStore = FALSE;
	Volumes[cVolume-'A'].Mounted = FALSE;

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

			// Unmount
			if ((StoreCache[a].Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid)
			{
				StoreCache[a].DatPath[0] = StoreCache[a].IdxPathMain[0] = L'\0';
			}
			else
			{
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
		SendLFNotifyMessage(LFMessages.StoresChanged);

	return Result;
}
