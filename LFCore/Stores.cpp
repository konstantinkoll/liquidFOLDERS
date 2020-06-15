
#include "stdafx.h"
#include "CStoreInternal.h"
#include "CStoreWindows.h"
#include "FileSystem.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "Progress.h"
#include "Stores.h"


extern HMODULE LFCoreModuleHandle;
extern LFMessageIDs LFMessages;
extern LFVolumeDescriptor Volumes[26];


#define LFSTORESHIVE     "Software\\liquidFOLDERS\\Stores"
#define MAXSTORES        100


#pragma data_seg(".shared")

BOOL Initialized = FALSE;
CHAR KeyChars[38] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '_', '-' };

#pragma data_seg()


#pragma bss_seg(".stores")

ABSOLUTESTOREID DefaultStore;
UINT StoreCount;
LFStoreDescriptor StoreCache[MAXSTORES];

#pragma data_seg()
#pragma comment(linker, "/SECTION:.stores,RWS")


#define STOREDESCRIPTORFILESIZE             offsetof(LFStoreDescriptor, IdxPathMain)
#define STOREDESCRIPTORREQUIREDFILESIZE     offsetof(LFStoreDescriptor, SynchronizeTime)

#define ResetStoreFlags(pStoreDescriptor)     pStoreDescriptor->Flags &= (LFStoreFlagsAutoLocation | LFStoreFlagsError);


// Persistence in registry
//

void GetRegistryKey(const ABSOLUTESTOREID& StoreID, LPSTR pKey)
{
	assert(pKey);

	strcpy_s(pKey, 256, LFSTORESHIVE);
	strcat_s(pKey, 256, "\\");
	strcat_s(pKey, 256, StoreID);
}

BOOL LoadStoreSettingsFromRegistry(const ABSOLUTESTOREID& StoreID, LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);

	BOOL Result = FALSE;

	CHAR Key[256];
	GetRegistryKey(StoreID, Key);

	HKEY hKey;
	if (RegOpenKeyA(HKEY_CURRENT_USER, Key, &hKey)==ERROR_SUCCESS)
	{
		ZeroMemory(pStoreDescriptor, sizeof(LFStoreDescriptor));
		pStoreDescriptor->StoreID = StoreID;

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

		// Reset flags
		ResetStoreFlags(pStoreDescriptor);

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

		switch (pStoreDescriptor->Mode & LFStoreModeIndexMask)
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

UINT DeleteStoreSettingsFromRegistry(const LFStoreDescriptor& Victim)
{
	assert((Victim.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal);

	CHAR Key[256];
	GetRegistryKey(Victim.StoreID, Key);

	LSTATUS lStatus = RegDeleteKeyA(HKEY_CURRENT_USER, Key);

	return (lStatus==ERROR_SUCCESS) || (lStatus==ERROR_FILE_NOT_FOUND) ? LFOk: LFRegistryError;
}


// Persistence in .store file
//

UINT GetKeyFileFromStoreDescriptor(const LFStoreDescriptor* pStoreDescriptor, LPWSTR pPath)
{
	assert(pStoreDescriptor);
	assert((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal);
	assert(pPath);

	if (!LFIsStoreMounted(pStoreDescriptor))
		return LFStoreNotMounted;

	wcsncpy_s(pPath, MAX_PATH, pStoreDescriptor->DatPath, 3);		// .store file is always in root directory
	AppendGUID(pPath, *pStoreDescriptor, L".store");

	return LFOk;
}

BOOL LoadStoreSettingsFromFile(LPCWSTR pPath, LFStoreDescriptor* pStoreDescriptor)
{
	assert(pPath);
	assert(pStoreDescriptor);

	HANDLE hFile;
	if (CreateFileConcurrent(pPath, FALSE, OPEN_EXISTING, hFile)!=FileOk)
		return FALSE;

	ZeroMemory(pStoreDescriptor, sizeof(LFStoreDescriptor));

	DWORD wmRead;
	BOOL Result = ReadFile(hFile, pStoreDescriptor, STOREDESCRIPTORFILESIZE, &wmRead, NULL);
	Result &= (wmRead>=STOREDESCRIPTORREQUIREDFILESIZE);

	CloseHandle(hFile);

	// Reset flags
	ResetStoreFlags(pStoreDescriptor);

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

	HANDLE hFile;
	if ((Result=CreateFileConcurrent(Path, TRUE, OPEN_ALWAYS, hFile, TRUE))!=FileOk)
		return (Result==FileSharingViolation) ? LFSharingViolation1 : (GetLastError()==ERROR_ACCESS_DENIED) ? LFNoAccessError : (GetLastError()==ERROR_WRITE_PROTECT) ? LFDriveWriteProtected : LFDriveNotReady;

	DWORD wmWritten;
	Result = WriteFile(hFile, pStoreDescriptor, STOREDESCRIPTORFILESIZE, &wmWritten, NULL) ? LFOk : LFDriveNotReady;
	if (wmWritten!=STOREDESCRIPTORFILESIZE)
		Result = (GetLastError()==ERROR_WRITE_PROTECT) ? LFDriveWriteProtected : LFDriveNotReady;

	CloseHandle(hFile);

	return Result;
}

UINT DeleteStoreSettingsFromFile(const LFStoreDescriptor& Victim)
{
	assert((Victim.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal);

	UINT Result;
	WCHAR Path[MAX_PATH];
	if ((Result=GetKeyFileFromStoreDescriptor(&Victim, Path))!=LFOk)
		return Result;

	return DeleteFile(Path) ? LFOk : (GetLastError()==ERROR_ACCESS_DENIED) ? LFNoAccessError : (GetLastError()==ERROR_WRITE_PROTECT) ? LFDriveWriteProtected : LFDriveNotReady;
}


// General persistence
//

void TestCloudPath(LFStoreDescriptor* pStoreDescriptor, LPWSTR pPath, UINT Source)
{
	assert(pPath);

	if (pPath[0]!=L'\0')
	{
		if (pPath[wcslen(pPath)-1]!=L'\\')
			wcscat_s(pPath, MAX_PATH, L"\\");

		if (wcsncmp(pPath, pStoreDescriptor->DatPath, wcslen(pPath))==0)
			pStoreDescriptor->Source = Source;
	}
}

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

			SHFILEINFO ShellFileInfo;
			if (SHGetFileInfo(szVolumeRoot, 0, &ShellFileInfo, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME))
				wcscpy_s(pStoreDescriptor->LastSeen, 256, ShellFileInfo.szDisplayName);

			if ((pStoreDescriptor->Mode & LFStoreModeBackendMask)<=LFStoreModeBackendWindows)
			{
				const UINT Source = LFGetSourceForVolume((CHAR)pStoreDescriptor->DatPath[0]);

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
				AppendGUID(pStoreDescriptor->IdxPathMain, *pStoreDescriptor);
			}
			else
			{
				GetAutoPath(*pStoreDescriptor, pStoreDescriptor->IdxPathMain);
			}

			// Cloud storage
			WCHAR Path[MAX_PATH];

			// Box
			if (LFGetBoxPath(Path))
				TestCloudPath(pStoreDescriptor, Path, LFTypeSourceBox);

			// Dropbox
			wcscpy_s(Path, MAX_PATH, pStoreDescriptor->DatPath);
			wcscat_s(Path, MAX_PATH, L".dropbox");

			if (FileExists(Path))
				pStoreDescriptor->Source = LFTypeSourceDropbox;

			// Google Drive
			if (LFGetGoogleDrivePath(Path))
				TestCloudPath(pStoreDescriptor, Path, LFTypeSourceGoogleDrive);

			// iCloud
			LFICloudPaths iCloudPaths;
			if (LFGetICloudPaths(iCloudPaths))
			{
				TestCloudPath(pStoreDescriptor, iCloudPaths.Drive, LFTypeSourceICloudDrive);
				TestCloudPath(pStoreDescriptor, iCloudPaths.PhotoLibrary, LFTypeSourceICloudPhotos);
			}

			// OneDrive
			LFOneDrivePaths OneDrivePaths;
			if (LFGetOneDrivePaths(OneDrivePaths))
			{
				TestCloudPath(pStoreDescriptor, OneDrivePaths.OneDrive, LFTypeSourceOneDrive);
				TestCloudPath(pStoreDescriptor, OneDrivePaths.CameraRoll, LFTypeSourceOneDrive);
				TestCloudPath(pStoreDescriptor, OneDrivePaths.Documents, LFTypeSourceOneDrive);
				TestCloudPath(pStoreDescriptor, OneDrivePaths.Pictures, LFTypeSourceOneDrive);
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
			GetAutoPath(*pStoreDescriptor, pStoreDescriptor->DatPath);

		// Main index
		if (((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexHybrid) || LFIsStoreMounted(pStoreDescriptor))
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
		GetAutoPath(*pStoreDescriptor, pStoreDescriptor->IdxPathAux);
		wcscat_s(pStoreDescriptor->IdxPathAux, MAX_PATH, L"INDEX\\");
	}
	else
	{
		pStoreDescriptor->IdxPathAux[0] = L'\0';
	}
}

UINT SaveStoreSettings(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);

	UINT Result;

	if ((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal)
		if ((Result=SaveStoreSettingsToRegistry(pStoreDescriptor))!=LFOk)
			return Result;

	if ((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
		if (LFIsStoreMounted(pStoreDescriptor))
			if ((Result=SaveStoreSettingsToFile(pStoreDescriptor))!=LFOk)
				return Result;

	return LFOk;
}


// Cache access
//

void CreateNewStoreID(ABSOLUTESTOREID& StoreID)
{
	StoreID[LFKeySize-1] = 0;
	BOOL IsUnique;

	do
	{
		RANDOMIZE();

		for (UINT a=0; a<LFKeySize-1; a++)
			StoreID[a] = RAND_CHAR();

		IsUnique = TRUE;

		for (UINT a=0; a<StoreCount; a++)
			if (StoreCache[a].StoreID==StoreID)
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

LFStoreDescriptor* FindStore(const ABSOLUTESTOREID& StoreID)
{
	for (UINT a=0; a<StoreCount; a++)
		if (StoreCache[a].StoreID==StoreID)
			return &StoreCache[a];

	return NULL;
}

LFStoreDescriptor* FindStore(const GUID& UniqueID)
{
	for (UINT a=0; a<StoreCount; a++)
		if (StoreCache[a].UniqueID==UniqueID)
			return &StoreCache[a];

	return NULL;
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
		if (MakeDefault || LFIsDefaultStoreID(DefaultStore))
			Result = MakeDefaultStore(pStoreDescriptor);
	}

	return Result;
}

UINT DeleteStoreFromCache(LFStoreDescriptor& Victim)
{
	// Remove persistent records
	UINT Result;

	if ((Victim.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal)
		if ((Result=DeleteStoreSettingsFromRegistry(Victim))!=LFOk)
			return Result;

	if (((Victim.Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal) && (LFIsStoreMounted(&Victim)))
		if ((Result=DeleteStoreSettingsFromFile(Victim))!=LFOk)
			return Result;

	// Remove from cache
	for (UINT a=0; a<StoreCount; a++)
		if (StoreCache[a].StoreID==Victim.StoreID)
		{
			if (a<StoreCount-1)
			{
				// Swap with last in cache (store needs to be locked first)
				HMUTEX hMutex;
				if (!GetMutexForStore(&StoreCache[StoreCount-1], hMutex))
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
	if (Victim.StoreID==DefaultStore)
		ChooseNewDefaultStore();

	return LFOk;
}


// Default store
//

UINT MakeDefaultStore(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);

	UINT Result = LFRegistryError;

	HKEY hKey;
	if (RegCreateKeyA(HKEY_CURRENT_USER, LFSTORESHIVE, &hKey)==ERROR_SUCCESS)
	{
		if (RegSetValueExA(hKey, "DefaultStore", 0, REG_SZ, (LPBYTE)&pStoreDescriptor->StoreID, LFKeySize-1)==ERROR_SUCCESS)
		{
			DefaultStore = pStoreDescriptor->StoreID;
			Result = LFOk;
		}

		RegCloseKey(hKey);
	}

	return Result;
}

void ChooseNewDefaultStore(BOOL OnInitialize)
{
	INT Slot = -1;

	for (UINT a=0; a<StoreCount; a++)
		if (StoreCache[a].StoreID!=DefaultStore) 
			if(OnInitialize || ((StoreCache[a].Mode & LFStoreModeIndexMask)==LFStoreModeIndexInternal))
		{
			Slot = a;

			break;
		}

	if (Slot!=-1)
	{
		MakeDefaultStore(&StoreCache[Slot]);
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

LFCORE_API UINT LFGetDefaultStore(STOREID& StoreID)
{
	if (GetMutexForStores())
	{
		StoreID = DefaultStore;

		ReleaseMutexForStores();

		return StoreID[0] ? LFOk : LFNoDefaultStore;
	}

	return LFMutexError;
}

LFCORE_API UINT LFSetDefaultStore(const ABSOLUTESTOREID& StoreID)
{
	if (!GetMutexForStores())
		return LFMutexError;

	UINT Result = LFOk;

	if (StoreID!=DefaultStore)
	{
		LFStoreDescriptor* pStoreDescriptor;
		Result = (pStoreDescriptor=FindStore(StoreID))!=NULL ? MakeDefaultStore(pStoreDescriptor) : LFIllegalID;

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

UINT StoreFlagsToType(const LFStoreDescriptor* pStoreDescriptor, UINT ItemType)
{
	assert(pStoreDescriptor);

	UINT Type = pStoreDescriptor->Source | ItemType | (pStoreDescriptor->Flags & (LFStoreFlagsManageable | LFStoreFlagsWriteable)) |
		(LFIsStoreMounted(pStoreDescriptor) ? LFTypeMounted : LFTypeGhosted);

	// Add capability flags
	if ((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal)
		Type |= LFTypeShortcutAllowed;

	if ((pStoreDescriptor->Mode & LFStoreModeBackendMask)!=LFStoreModeBackendInternal)
		Type |= (LFTypeSynchronizeAllowed | LFTypeExplorerAllowed);

	return Type;
}

void GetDiskFreeSpaceForStore(LFStoreDescriptor& StoreDescriptor)
{
	if (LFIsStoreMounted(&StoreDescriptor))
		if (GetDiskFreeSpaceEx(StoreDescriptor.DatPath, &StoreDescriptor.FreeBytesAvailable, &StoreDescriptor.TotalNumberOfBytes, &StoreDescriptor.TotalNumberOfBytesFree))
			return;

	StoreDescriptor.FreeBytesAvailable.QuadPart = StoreDescriptor.TotalNumberOfBytes.QuadPart = StoreDescriptor.TotalNumberOfBytesFree.QuadPart = 0;
}

UINT GetStore(LFStoreDescriptor* pStoreDescriptor, CStore*& pStore)
{
	pStore = NULL;

	if (!pStoreDescriptor)
		return LFIllegalID;

	HMUTEX hMutex;
	if (!GetMutexForStore(pStoreDescriptor, hMutex))
		return LFMutexError;

	// Create matching CStore object
	switch (pStoreDescriptor->Mode & LFStoreModeBackendMask)
	{
	case LFStoreModeBackendInternal:
		pStore = new CStoreInternal(pStoreDescriptor, hMutex);
		break;

	case LFStoreModeBackendWindows:
		pStore = new CStoreWindows(pStoreDescriptor, hMutex);
		break;

	default:
		ReleaseMutexForStore(hMutex);
		return LFIllegalStoreDescriptor;
	}

	// Done
	return LFOk;
}

UINT GetStore(const ABSOLUTESTOREID& StoreID, CStore*& pStore)
{
	pStore = NULL;

	if (!GetMutexForStores())
		return LFMutexError;

	UINT Result = GetStore(FindStore(StoreID), pStore);

	ReleaseMutexForStores();

	return Result;
}

UINT OpenStore(const ABSOLUTESTOREID& StoreID, CStore*& pStore, BOOL WriteAccess)
{
	UINT Result;
	if ((Result=GetStore(StoreID, pStore))==LFOk)
		if ((Result=pStore->Open(WriteAccess))!=LFOk)
		{
			delete pStore;
			pStore = NULL;
		}

	return Result;
}

UINT OpenStore(const STOREID& StoreID, CStore*& pStore, BOOL WriteAccess)
{
	UINT Result;
	ABSOLUTESTOREID AbsoluteStoreID;

	return ((Result=LFResolveStoreIDEx(AbsoluteStoreID, StoreID))!=LFOk) ? Result : OpenStore(AbsoluteStoreID, pStore, WriteAccess);
}

UINT CommitInitializeStore(LFStoreDescriptor* pStoreDescriptor, LFProgress* pProgress=NULL)
{
	assert(pStoreDescriptor);

	CompleteStoreSettings(pStoreDescriptor);

	// The store starts empty
	pStoreDescriptor->Flags |= LFStoreFlagsMaintained;

	// Writeable?
	if (VolumeWriteable((CHAR)pStoreDescriptor->IdxPathMain[0]))
		pStoreDescriptor->Flags |= LFStoreFlagsManageable | LFStoreFlagsWriteable;
	
	// Create index
	UINT Result = UpdateStoreInCache(pStoreDescriptor);
	if (Result==LFOk)
	{
		CStore* pStore;
		if ((Result=GetStore(pStoreDescriptor->StoreID, pStore))==LFOk)
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

LFCORE_API UINT LFResolveStoreIDEx(ABSOLUTESTOREID& AbsoluteStoreID, const STOREID& StoreID)
{
	if (LFIsDefaultStoreID(StoreID))
		return LFGetDefaultStore(AbsoluteStoreID);

	AbsoluteStoreID = MAKEABSOLUTESTOREID(StoreID);

	return LFOk;
}

LFCORE_API UINT LFGetAllStores(LPCABSOLUTESTOREID& lpcStoreIDs, UINT& Count)
{
	lpcStoreIDs = NULL;
	Count = 0;

	if (!GetMutexForStores())
		return LFMutexError;

	if ((Count=StoreCount)>0)
	{
		lpcStoreIDs = (LPABSOLUTESTOREID)malloc(LFKeySize*StoreCount);

		for (UINT a=0; a<StoreCount; a++)
			((LPSTOREID)lpcStoreIDs)[a] = StoreCache[a].StoreID;
	}

	ReleaseMutexForStores();

	return LFOk;
}

LFCORE_API UINT LFGetStoreSettings(const STOREID& StoreID, LFStoreDescriptor& StoreDescriptor, BOOL DiskFreeSpace)
{
	if (!GetMutexForStores())
		return LFMutexError;

	UINT Result;
	ABSOLUTESTOREID AbsoluteStoreID;

	if ((Result=LFResolveStoreIDEx(AbsoluteStoreID, StoreID))!=LFOk)
		return Result;

	LFStoreDescriptor* pSlot = FindStore(AbsoluteStoreID);
	if (pSlot)
	{
		StoreDescriptor = *pSlot;

		// Only get free disk space when requested by caller
		if (DiskFreeSpace)
			GetDiskFreeSpaceForStore(StoreDescriptor);
	}

	ReleaseMutexForStores();

	return pSlot ? LFOk : LFIllegalID;
}

LFCORE_API UINT LFGetStoreSettingsEx(GUID UniqueID, LFStoreDescriptor& StoreDescriptor)
{
	if (!GetMutexForStores())
		return LFMutexError;

	LFStoreDescriptor* pSlot = FindStore(UniqueID);
	if (pSlot)
	{
		StoreDescriptor = *pSlot;

		// Always get free disk space
		GetDiskFreeSpaceForStore(StoreDescriptor);
	}

	ReleaseMutexForStores();

	return pSlot ? LFOk : LFIllegalID;
}

LFCORE_API BOOL LFStoresOnVolume(CHAR cVolume)
{
	BOOL Result = FALSE;

	if (GetMutexForStores())
	{
		for (UINT a=0; a<StoreCount; a++)
			if (LFIsStoreMounted(&StoreCache[a]) && (StoreCache[a].DatPath[0]==cVolume))
			{
				Result = TRUE;
				break;
			}

		ReleaseMutexForStores();
	}

	return Result;
}

LFCORE_API UINT LFGetStoreIcon(const LFStoreDescriptor* pStoreDescriptor, UINT* pType)
{
	assert(pStoreDescriptor);

	if (pType)
	{
		// Basic, Mounted?
		*pType = StoreFlagsToType(pStoreDescriptor, LFTypeStore);

		// Empty?
		if (pStoreDescriptor->Flags & LFStoreFlagsMaintained)
		{
			*pType |= LFTypeMaintained;

			if (!pStoreDescriptor->Statistics.FileCount[LFContextAllFiles] && !pStoreDescriptor->Statistics.FileCount[LFContextArchive])
				*pType = (*pType & ~LFTypeBadgeMask) | LFTypeBadgeEmpty;
		}
		else
		{
			*pType |= LFTypeGhosted;
		}

		// New?
		FILETIME CurrentTime;
		GetSystemTimeAsFileTime(&CurrentTime);

		ULARGE_INTEGER ULI1;
		ULARGE_INTEGER ULI2;

		ULI1.LowPart = CurrentTime.dwLowDateTime;
		ULI1.HighPart = CurrentTime.dwHighDateTime;
		ULI2.LowPart = pStoreDescriptor->CreationTime.dwLowDateTime;
		ULI2.HighPart = pStoreDescriptor->CreationTime.dwHighDateTime;

		if (ULI1.QuadPart<ULI2.QuadPart+864000000000ull)
			*pType = (*pType & ~LFTypeBadgeMask) | LFTypeBadgeNew;

		// Default store?
		if (GetMutexForStores())
		{
			if (pStoreDescriptor->StoreID==DefaultStore)
				*pType = (*pType & ~LFTypeBadgeMask) | LFTypeBadgeDefault | LFTypeDefault;

			ReleaseMutexForStores();
		}

		// Error?
		if (pStoreDescriptor->Flags & LFStoreFlagsError)
			*pType = (*pType & ~LFTypeBadgeMask) | LFTypeBadgeError;
	}

	return max(LFTypeSourceInternal, pStoreDescriptor->Source);
}

LFCORE_API UINT LFCreateStoreLiquidfolders(LPWSTR pStoreName, LPCWSTR pComments, CHAR cVolume, BOOL MakeSearchable)
{
	if (!GetMutexForStores())
		return LFMutexError;

	LFStoreDescriptor StoreDescriptor;
	GetDescriptorForNewStore(&StoreDescriptor);

#if (LFStoreModeBackendInternal!=0)
	Store.Mode = LFStoreModeBackendInternal;
#endif

	// Set data
	if (pComments)
		wcscpy_s(StoreDescriptor.Comments, 256, pComments);

	if (cVolume)
	{
		swprintf_s(StoreDescriptor.DatPath, MAX_PATH, L"%c:\\", cVolume);
		AppendGUID(StoreDescriptor.DatPath, StoreDescriptor);

		StoreDescriptor.Mode |= (LFGetSourceForVolume(cVolume)==LFTypeSourceInternal) ? LFStoreModeIndexInternal : MakeSearchable ? LFStoreModeIndexHybrid : LFStoreModeIndexExternal;
	}
	else
	{
		StoreDescriptor.Flags |= LFStoreFlagsAutoLocation;
	}

	// Retrieve or save store name
	if (pStoreName)
	{
		if (*pStoreName)
			wcscpy_s(StoreDescriptor.StoreName, 256, pStoreName);

		// Copy name back for display purposes
		wcscpy_s(pStoreName, 256, StoreDescriptor.StoreName);
	}

	return CommitInitializeStore(&StoreDescriptor);
}

LFCORE_API UINT LFCreateStoreWindows(LPCWSTR pPath, LPWSTR pStoreName, LFProgress* pProgress)
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

	// Set store name
	if (pStoreName && *pStoreName)
	{
		// Given store name
		wcscpy_s(Store.StoreName, 256, pStoreName);
	}
	else
	{
		// Create store name
		wcscpy_s(Store.StoreName, 256, pPath);

		if (TrailingBackslash)
			Store.StoreName[wcslen(Store.StoreName)-1] = L'\0';

		LPCWSTR pChar = wcsrchr(Store.StoreName, L'\\');
		if (pChar)
		{
			// Valid subfolder path
			wcscpy_s(Store.StoreName, 256, pChar+1);
		}
		else
		{
			// Get volume name
			WCHAR szVolumeRoot[] = L" :\\";
			szVolumeRoot[0] = Store.DatPath[0];

			SHFILEINFO ShellFileInfo;
			if (SHGetFileInfo(szVolumeRoot, 0, &ShellFileInfo, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME))
				wcscpy_s(Store.StoreName, 256, ShellFileInfo.szDisplayName);
		}

		// Copy back created store name to caller
		if (pStoreName)
			wcscpy_s(pStoreName, 256, Store.StoreName);
	}

	return CommitInitializeStore(&Store, pProgress);
}

LFCORE_API UINT LFMakeStoreSearchable(const ABSOLUTESTOREID& StoreID, BOOL Searchable)
{
	if (!GetMutexForStores())
		return LFMutexError;

	UINT Result;
	CStore* pStore;
	if ((Result=GetStore(StoreID, pStore))==LFOk)
	{
		LFStoreDescriptor* pStoreDescriptor = pStore->p_StoreDescriptor;
		LFStoreDescriptor Victim = *pStoreDescriptor;

		switch (pStoreDescriptor->Mode & LFStoreModeIndexMask)
		{
		case LFStoreModeIndexHybrid:
			if (!Searchable)
				if (LFIsStoreMounted(pStoreDescriptor))
				{
					if ((Result=DeleteStoreSettingsFromRegistry(Victim))==LFOk)
					{
						// Convert to external index
						pStoreDescriptor->Mode = (pStoreDescriptor->Mode & ~LFStoreModeIndexMask) | LFStoreModeIndexExternal;
						CompleteStoreSettings(pStoreDescriptor);

						if ((Result=UpdateStoreInCache(pStoreDescriptor))==LFOk)
						{
							// Delete aux index
							WCHAR Path[MAX_PATH];
							GetAutoPath(Victim, Path);
							DeleteDirectory(Path);
						}
					}
				}
				else
				{
					// Store is not mounted, so delete local copy
					if ((Result=pStore->PrepareDelete())==LFOk)
						if ((Result=pStore->CommitDelete())==LFOk)
							Result = DeleteStoreFromCache(Victim);

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

LFCORE_API UINT LFDeleteStore(const ABSOLUTESTOREID& StoreID, LFProgress* pProgress)
{
	if (!GetMutexForStores())
		return LFMutexError;

	UINT Result;

	LFStoreDescriptor* pStoreDescriptor = FindStore(StoreID);
	if (pStoreDescriptor)
	{
		// Progress
		if (ProgressMinorStart(pProgress, 2, pStoreDescriptor->StoreName, TRUE))
		{
			ReleaseMutexForStores();

			return LFCancel;
		}

		// !!!WARNING!!!
		// The store object must be created on a copy of the store descriptor,
		// otherwise removing the store before actual deletion may move another
		// store descriptor to the same address!
		LFStoreDescriptor Victim = *pStoreDescriptor;

		// Get CStore object
		CStore* pStore;
		if ((Result=GetStore(&Victim, pStore))==LFOk)
		{
			if ((Result=pStore->PrepareDelete())==LFOk)
				if ((Result=DeleteStoreFromCache(Victim))==LFOk)
				{
					// Early release of mutex for broadcast message and before physically deleting files
					ReleaseMutexForStores();

					SendLFNotifyMessage(LFMessages.StoresChanged);

					// Progress
					ProgressMinorNext(pProgress);

					Result = pStore->CommitDelete();
				}
				else
				{
					// Progress
					ProgressMinorNext(pProgress, TRUE);
				}

			delete pStore;
		}

		// Progress
		ProgressMinorNext(pProgress);
	}
	else
	{
		Result = LFIllegalID;
	}

	ReleaseMutexForStores();

	return Result;
}

LFCORE_API UINT LFSetStoreAttributes(const ABSOLUTESTOREID& StoreID, LPCWSTR pName, LPCWSTR pComment)
{
	if (!pName && !pComment)
		return LFOk;

	if (pName && (pName[0]==L'\0'))
		return LFIllegalValue;

	if (!GetMutexForStores())
		return LFMutexError;

	UINT Result;
	LFStoreDescriptor* pStoreDescriptor = FindStore(StoreID);
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

LFCORE_API UINT LFSynchronizeStores(const STOREID& StoreID, LFProgress* pProgress)
{
	UINT Result;

	if (LFIsDefaultStoreID(StoreID))
	{
		// Iterate all stores
		LPCABSOLUTESTOREID lpcStoreIDs;
		UINT Count;
		if ((Result=LFGetAllStores(lpcStoreIDs, Count))==LFOk)
		{
			// Progress
			ProgressMajorStart(pProgress, Count);

			for (UINT a=0; a<Count; a++)
			{
				CStore* pStore;
				if ((Result=OpenStore(lpcStoreIDs[a], pStore))==LFOk)
				{
					Result = pStore->Synchronize(pProgress);
					delete pStore;
				}

				// Stores which are not mounted do not present an error
				if (Result==LFStoreNotMounted)
					Result = LFOk;

				// Progress
				if (ProgressMajorNext(pProgress))
					break;
			}

			free(lpcStoreIDs);

			SendLFNotifyMessage(LFMessages.StoreAttributesChanged);
			SendLFNotifyMessage(LFMessages.StatisticsChanged);
		}
	}
	else
	{
		// Single store
		CStore* pStore;
		if ((Result=OpenStore(StoreID, pStore))==LFOk)
		{
			Result = pStore->Synchronize(pProgress);
			delete pStore;

			if (Result==LFOk)
				SendLFNotifyMessage(LFMessages.StoreAttributesChanged);

			SendLFNotifyMessage(LFMessages.StatisticsChanged);
		}
	}

	return Result;
}

LFCORE_API LFMaintenanceList* LFScheduledMaintenance(LFProgress* pProgress)
{
	LFMaintenanceList* pMaintenanceList = new LFMaintenanceList();

	LPCABSOLUTESTOREID lpcStoreIDs;
	UINT Count;
	if ((pMaintenanceList->m_LastError=LFGetAllStores(lpcStoreIDs, Count))==LFOk)
	{
		// Progress
		ProgressMajorStart(pProgress, Count);

		for (UINT a=0; a<Count; a++)
		{
			CStore* pStore;
			if ((pMaintenanceList->m_LastError=GetStore(lpcStoreIDs[a], pStore))!=LFOk)
				break;

			pStore->ScheduledMaintenance(pMaintenanceList, pProgress);
			delete pStore;

			// Progress
			if (ProgressMajorNext(pProgress))
				break;
		}

		free(lpcStoreIDs);

		SendLFNotifyMessage(LFMessages.StoreAttributesChanged);
	}

	// Sort
	pMaintenanceList->SortItems();

	return pMaintenanceList;
}

LFCORE_API UINT LFGetFileLocation(LFItemDescriptor* pItemDescriptor, LPWSTR pPath, SIZE_T cCount, BOOL RemoveNew)
{
	assert(pItemDescriptor);
	assert(pPath);
	assert(cCount>=MAX_PATH);

	// For files only
	if (!LFIsFile(pItemDescriptor))
		return LFIllegalItemType;

	// Get file location from backend
	UINT Result;
	CStore* pStore;
	if ((Result=GetStore(pItemDescriptor->StoreID, pStore))==LFOk)
	{
		// For optimal performance, get the file location without index query
		WCHAR Path[2*MAX_PATH];
		if ((Result=pStore->GetFilePath(pItemDescriptor, Path, 2*MAX_PATH))==LFOk)
		{
			const BOOL Exists = FileExists(Path);
			if (!Exists)
				Result = LFNoFileBody;

			// Do we have to open the index?
			if ((Exists!=((pItemDescriptor->CoreAttributes.Flags & LFFlagMissing)==0)) || (RemoveNew && (pItemDescriptor->CoreAttributes.Flags & LFFlagNew)))
				if ((Result=pStore->Open(TRUE))==LFOk)
				{
					Result = pStore->GetFileLocation(pItemDescriptor, Path, 2*MAX_PATH, RemoveNew);
				}
				else
				{
					// Ignore a write protected store - open the file regardless!
					if (Result==LFDriveWriteProtected)
						Result = LFOk;
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

	// Stores
	if (GetMutexForStores())
	{
		for (UINT a=0; a<StoreCount; a++)
			pSearchResult->AddStoreDescriptor(StoreCache[a]);

		ReleaseMutexForStores();
	}
	else
	{
		pSearchResult->m_LastError = LFMutexError;
	}
}

LFCORE_API UINT LFQueryStatistics(LFStatistics& Statistics, const STOREID& StoreID, UINT64* pGlobalContextSet)
{
	ZeroMemory(&Statistics, sizeof(LFStatistics));

	if (pGlobalContextSet)
		*pGlobalContextSet = 0;

	if (!GetMutexForStores())
		return LFMutexError;

	if (LFIsDefaultStoreID(StoreID))
	{
		// Iterate all stores
		for (UINT a=0; a<StoreCount; a++)
		{
			for (ITEMCONTEXT Context=0; Context<=LFLastQueryContext; Context++)
			{
				Statistics.FileCount[Context] += StoreCache[a].Statistics.FileCount[Context];
				Statistics.FileSize[Context] += StoreCache[a].Statistics.FileSize[Context];
			}

			for (UINT Priority=0; Priority<=LFMaxRating; Priority++)
				Statistics.TaskCount[Priority] += StoreCache[a].Statistics.TaskCount[Priority];
		}

		// Global context set
		if (pGlobalContextSet)
			for (ITEMCONTEXT Context=0; Context<=LFLastQueryContext; Context++)
				if (Statistics.FileCount[Context])
					*pGlobalContextSet |= (1ull<<Context);
	}
	else
	{
		// Single store
		LFStoreDescriptor* pStoreDescriptor = FindStore(MAKEABSOLUTESTOREID(StoreID));

		if (pStoreDescriptor)
			Statistics = pStoreDescriptor->Statistics;

		// Global context set
		if (pGlobalContextSet)
			for (ITEMCONTEXT Context=0; Context<=LFLastQueryContext; Context++)
				for (UINT a=0; a<StoreCount; a++)
					if (StoreCache[a].Statistics.FileCount[Context])
					{
						*pGlobalContextSet |= (1ull<<Context);
						break;
					}
	}

	ReleaseMutexForStores();

	return LFOk;
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
				CHAR ID[LFKeySize];
				if (RegEnumKeyA(hKey, a, ID, LFKeySize)==ERROR_SUCCESS)
					if (LoadStoreSettingsFromRegistry(MAKEABSOLUTESTOREID(ID), &StoreCache[StoreCount]))
						CompleteStoreSettings(&StoreCache[StoreCount++]);
			}
	}

	RegCloseKey(hKey);
}

void MountVolumes(UINT Mask, BOOL OnInitialize)
{
	DWORD VolumesOnSystem = LFGetLogicalVolumes(Mask);
	WCHAR szVolumeRoot[4] = L" :\\";

	for (CHAR cVolume='A'; cVolume<='Z'; cVolume++, VolumesOnSystem>>=1)
	{
		if ((VolumesOnSystem & 1)==0)
			continue;

		szVolumeRoot[0] = cVolume;

		SHFILEINFO ShellFileInfo;
		if (SHGetFileInfo(szVolumeRoot, 0, &ShellFileInfo, sizeof(SHFILEINFO), SHGFI_ATTRIBUTES))
			if (ShellFileInfo.dwAttributes)
				MountVolume(cVolume, TRUE, OnInitialize);
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
				RegQueryValueExA(hKey, "DefaultStore", NULL, &Type, (LPBYTE)&DefaultStore, &Size);
				RegCloseKey(hKey);
			}

			// Stores from registry
			StoreCount = 0;
			LoadRegistry();

			// Mount external volumes
			MountVolumes(LFGLV_EXTERNAL | LFGLV_NETWORK, TRUE);

			// Run non-scheduled maintenance, set default store
			BOOL DefaultStoreOk = FALSE;

			for (UINT a=0; a<StoreCount; a++)
			{
				// Maintenance
				if ((StoreCache[a].Flags & LFStoreFlagsMaintained)==0)
				{
					CStore* pStore;
					if (GetStore(&StoreCache[a], pStore)==LFOk)
					{
						pStore->MaintenanceAndStatistics();
						delete pStore;
					}
				}

				// Default store
				if (StoreCache[a].StoreID==DefaultStore)
					DefaultStoreOk = TRUE;
			}

			// Choose new default store if neccessary
			if (!DefaultStoreOk)
				ChooseNewDefaultStore(TRUE);
		}

		ReleaseMutexForStores();
	}
}


// Volume mount/unmount
//

UINT MountVolume(CHAR cVolume, BOOL Mount, BOOL OnInitialize)
{
	assert(cVolume>='A');
	assert(cVolume<='Z');

	if (!GetMutexForStores())
		return LFMutexError;

	UINT Result = LFOk;
	const UINT Source = LFGetSourceForVolume(cVolume);
	UINT ChangeOccured = 0;		// 1: updated statistics changed, 2: stores changed

	// Mark all stores on that volume as victim
	for (UINT a=0; a<StoreCount; a++)
		if (StoreCache[a].DatPath[0]==cVolume)
			StoreCache[a].Flags |= LFStoreFlagsVictim;

	// Mount volume
	if (Mount)
	{
		WCHAR Mask[] = L" :\\*.store";
		Mask[0] = cVolume;

		WIN32_FIND_DATA FindData;
		HANDLE hFind = FindFirstFile(Mask, &FindData);
		if (hFind!=INVALID_HANDLE_VALUE)
			do
			{
				// Construct name of .store file
				WCHAR Path[MAX_PATH] = L" :\\";
				Path[0] = cVolume;
				wcscat_s(Path, MAX_PATH, FindData.cFileName);

				LFStoreDescriptor Store;
				if (LoadStoreSettingsFromFile(Path, &Store))
				{
					UINT UpdateStore = 0;

					// Is there a store with the same GUID?
					LFStoreDescriptor* pSlot = FindStore(Store.UniqueID);
					if (pSlot)
					{
						// Keep this store
						pSlot->Flags &= ~LFStoreFlagsVictim;

						// Is the .store file newer than the last mount time?
						if ((CompareFileTime(&FindData.ftLastWriteTime, &pSlot->MountTime)>0) || ((Source!=LFTypeSourceNethood) && ((pSlot->Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid)))
						{
							// Yes, but just update attributes
							wcscpy_s(pSlot->StoreName, 256, Store.StoreName);
							wcscpy_s(pSlot->Comments, 256, Store.Comments);
							pSlot->FileTime = Store.FileTime;
							pSlot->MaintenanceTime = Store.MaintenanceTime;
							pSlot->SynchronizeTime = Store.SynchronizeTime;

							UpdateStore = (pSlot->DatPath[0]==L'\0') ? 2 : 1;

							// Do NOT set the store index mode to hybrid to avoid errors when mounting a drive twice!
							// This can occur when the a network volume is periodically remounted.
							// When the store had hybrid indexing before, this setting is maintained as only relevant
							// attribute data is copied from the .store file on disc.
						}
					}
					else
					{
						// Set store index mode to external - this is the only option when the store wasn't known before
						Store.Mode = (Store.Mode & ~LFStoreModeIndexMask) | LFStoreModeIndexExternal;

						// Create locally unique key
						CreateNewStoreID(Store.StoreID);

						// Add to cache
						if (StoreCount<MAXSTORES)
						{
							StoreCache[StoreCount] = Store;
							pSlot = &StoreCache[StoreCount++];		// Slot zeigt auf Eintrag

							UpdateStore = 2;
						}
						else
						{
							Result = LFTooManyStores;
						}
					}

					// Store needs to be updated
					if (UpdateStore)
					{
						// The store must have a cache slot by now - be ultra-safe!
						assert(pSlot);

						pSlot->MountTime = FindData.ftLastWriteTime;
						ChangeOccured |= 2;

						// Set data path
						wcscpy_s(pSlot->DatPath, MAX_PATH, Store.DatPath);
						pSlot->DatPath[0] = cVolume;

						CompleteStoreSettings(pSlot);
					}

					// Run non-scheduled maintenance, either when neccessary or on a network volume
					if (pSlot)
					{
						if (!OnInitialize && ((pSlot->Source==LFTypeSourceNethood) || ((pSlot->Flags & LFStoreFlagsMaintained)==0)))
						{
							CStore* pStore;
							if ((Result=GetStore(pSlot, pStore))==LFOk)
							{
								if ((pSlot->Flags & LFStoreFlagsMaintained)==0)
								{
									Result = pStore->MaintenanceAndStatistics();
								}
								else
									if ((Result=pStore->Open(FALSE))==LFOk)
									{
										Result = pStore->UpdateStatistics();
									}
								
								delete pStore;

								ChangeOccured |= 1;
							}
						}

						// Update "Last seen" in registry for hybrid stores when the store is mounted for the first time
						if ((UpdateStore==2) && ((pSlot->Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid))
							SaveStoreSettingsToRegistry(pSlot);
					}
				}
			}
			while (FindNextFile(hFind, &FindData));

		FindClose(hFind);
	}
	else
	{
		Volumes[cVolume-'A'].Mounted = FALSE;
	}

	// Remove remaining victims
	BOOL RemovedDefaultStore = FALSE;

	for (UINT a=0; a<StoreCount; a++)
		if (StoreCache[a].Flags & LFStoreFlagsVictim)
		{
			HANDLE hMutex;
			if (!GetMutexForStore(&StoreCache[a], hMutex))
			{
				Result = LFMutexError;
				continue;
			}

			// New default store required
			RemovedDefaultStore |= (StoreCache[a].StoreID==DefaultStore);
			ChangeOccured |= 2;

			if ((StoreCache[a].Mode & LFStoreModeIndexMask)==LFStoreModeIndexHybrid)
			{
				// Unmount
				StoreCache[a].DatPath[0] = StoreCache[a].IdxPathMain[0] = L'\0';
			}
			else
			{
				// Remove from cache
				if (a<StoreCount-1)
				{
					HANDLE hMutexMove;
					if (!GetMutexForStore(&StoreCache[StoreCount-1], hMutexMove))
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

	// Choose new default store if neccessary
	if (RemovedDefaultStore)
		ChooseNewDefaultStore();

	ReleaseMutexForStores();

	// Send notification
	if (!OnInitialize && ChangeOccured)
		SendLFNotifyMessage((ChangeOccured & 2) ? LFMessages.StoresChanged : LFMessages.StatisticsChanged);

	return Result;
}
