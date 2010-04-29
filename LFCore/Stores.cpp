#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFItemDescriptor.h"
#include "Mutex.h"
#include "StoreCache.h"
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

	cbAcl = sizeof(ACL) + ((sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) + GetLengthSid(ptu->User.Sid));
	pAcl = (ACL*)malloc(cbAcl);

	if (!InitializeAcl(pAcl, cbAcl, ACL_REVISION))
	{
		dwError = GetLastError();
		goto Cleanup;
	}

	if (!AddAccessAllowedAce(pAcl,ACL_REVISION,GENERIC_ALL|STANDARD_RIGHTS_ALL|SPECIFIC_RIGHTS_ALL,ptu->User.Sid))
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

unsigned int ValidateStoreDirectories(LFStoreDescriptor* s)
{
	// Store phys. anlegen
	DWORD res = CreateDir(s->DatPath);
		if ((res!=ERROR_SUCCESS) && (res!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;
	res = CreateDir(s->IdxPathMain);
		if ((res!=ERROR_SUCCESS) && (res!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;

	if (s->StoreMode==LFStoreModeHybrid)
	{
		char tmpStr[MAX_PATH];
		GetAutoPath(s, tmpStr);
		res = CreateDir(tmpStr);
		if ((res!=ERROR_SUCCESS) && (res!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;
		res = CreateDir(s->IdxPathAux);
		if ((res!=ERROR_SUCCESS) && (res!=ERROR_ALREADY_EXISTS))
			return LFIllegalPhysicalPath;
	}

	return LFOk;
}

void AddDrives(LFSearchResult* res)
{
	DWORD DrivesOnSystem = LFGetLogicalDrives(LFGLD_External);

	for (char cDrive='A'; cDrive<='Z'; cDrive++, DrivesOnSystem>>=1)
	{
		if (!(DrivesOnSystem & 1))
			continue;

		wchar_t szDriveRoot[] = L" :\\";
		szDriveRoot[0] = cDrive;
		SHFILEINFO sfi;
		if (SHGetFileInfo(szDriveRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_ATTRIBUTES))
		{
			LFItemDescriptor* d = LFAllocItemDescriptor();
			d->Type = LFTypeDrive;
			if (sfi.dwAttributes)
			{
				d->IconID = IDI_DRV_Default;
			}
			else
			{
				d->IconID = IDI_DRV_Empty;
				d->Type |= LFTypeGhosted | LFTypeNotMounted;
			}
			d->CategoryID = LFCategoryDrives;
			SetAttribute(d, LFAttrFileName, sfi.szDisplayName);
			char key[] = " :";
			key[0] = cDrive;
			SetAttribute(d, LFAttrFileID, key);
			SetAttribute(d, LFAttrHint, sfi.szTypeName);
			res->AddItemDescriptor(d);
		}
	}
}

LFSearchResult* QueryStores(LFFilter* filter)
{
	LFSearchResult* res = new LFSearchResult(LFContextStores);
	res->m_RecommendedView = LFViewLargeIcons;
	res->m_LastError = LFOk;
	res->m_HasCategories = true;

	if (!GetMutex(Mutex_Stores))
	{
		res->m_LastError = LFMutexError;
		return res;
	}

	AddStores(res);
	ReleaseMutex(Mutex_Stores);

	if (filter)
	{
		if (!filter->Legacy)
			AddDrives(res);

		filter->Result.FilterType = LFFilterTypeStores;
	}

	return res;
}


LFCore_API unsigned int LFGetStoreSettings(char* key, LFStoreDescriptor* s)
{
	if (!key)
		return LFIllegalKey;
	if (strcmp(key, "")==0)
		return LFIllegalKey;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	LFStoreDescriptor* slot = FindStore(key);
	if (slot)
		*s = *slot;

	ReleaseMutex(Mutex_Stores);
	return (slot ? LFOk : LFIllegalKey);
}

LFCore_API unsigned int LFGetStoreSettings(_GUID guid, LFStoreDescriptor* s)
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
	CoCreateGuid(&s->GUID);

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

	// CreateTime setzen
	SYSTEMTIME st;
	GetLocalTime(&st);
	SystemTimeToFileTime(&st, &s->CreationTime);

	// Key generieren
	CreateStoreKey(s->StoreID);

	// Store speichern
	res = UpdateStore(s, MakeDefault);

	ReleaseMutex(Mutex_Stores);

	if (res==LFOk)
	{
		res = ValidateStoreDirectories(s);
		SendNotifyMessage(HWND_BROADCAST, LFMessages.StoresChanged, s->StoreMode==LFStoreModeInternal ? LFMSGF_IntStores : LFMSGF_ExtHybStores , (LPARAM)hWndSource);
	}

	return res;
}

LFCore_API unsigned int LFMakeDefaultStore(char* key, HWND hWndSource, bool InternalCall)
{
	if (!key)
		return LFIllegalKey;
	if (strcmp(key, "")==0)
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
	if (strcmp(key, "")==0)
		return LFIllegalKey;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	unsigned int res = LFIllegalKey;
	HANDLE StoreLock = NULL;
	LFStoreDescriptor* slot = FindStore(key, &StoreLock);
	if (slot)
	{
		if (slot->StoreMode!=LFStoreModeExternal)
		{
			res = LFIllegalStoreDescriptor;
			goto Cleanup;
		}

		if (!IsStoreMounted(slot))
		{
			res = LFIllegalStoreDescriptor;
			goto Cleanup;
		}

		slot->StoreMode = LFStoreModeHybrid;
		res = ValidateStoreSettings(slot);
		if (res!=LFOk)
			goto Cleanup;

		res = UpdateStore(slot);
		if (res!=LFOk)
			goto Cleanup;

		res = ValidateStoreDirectories(slot);
		if (res!=LFOk)
			goto Cleanup;

		// TODO: Index kopieren
		}

Cleanup:
	ReleaseMutexForStore(StoreLock);
	ReleaseMutex(Mutex_Stores);

	if (res==LFOk)
		SendMessage(HWND_BROADCAST, LFMessages.StoreAttributesChanged, LFMSGF_ExtHybStores, (LPARAM)hWndSource);

	return res;
}

LFCore_API unsigned int LFSetStoreAttributes(char* key, wchar_t* name, wchar_t* comment, HWND hWndSource, bool InternalCall)
{
	if (!key)
		return LFIllegalKey;
	if (strcmp(key, "")==0)
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

	ReleaseMutex(Mutex_Stores);

	if ((res==LFOk) && (!InternalCall))
		SendMessage(HWND_BROADCAST, LFMessages.StoreAttributesChanged, slot->StoreMode==LFStoreModeInternal ? LFMSGF_IntStores : LFMSGF_ExtHybStores, (LPARAM)hWndSource);

	return res;
}

LFCore_API unsigned int LFSetStoreComment(char* key, wchar_t* comment, HWND hWndSource, bool InternalCall)
{
	if (!key)
		return LFIllegalKey;
	if (strcmp(key, "")==0)
		return LFIllegalKey;
	if (!comment)
		return LFIllegalValue;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	unsigned int res = LFIllegalKey;
	LFStoreDescriptor* slot = FindStore(key);
	if (slot)
	{
		wcscpy_s(slot->Comment, 256, comment);
		res = UpdateStore(slot);
	}

	ReleaseMutex(Mutex_Stores);

	if ((res==LFOk) && (!InternalCall))
		SendNotifyMessage(HWND_BROADCAST, LFMessages.StoreAttributesChanged, slot->StoreMode==LFStoreModeInternal ? LFMSGF_IntStores : LFMSGF_ExtHybStores, (LPARAM)hWndSource);

	return res;
}

LFCore_API unsigned int LFDeleteStore(char* key, HWND hWndSource)
{
	if (!key)
		return LFIllegalKey;
	if (strcmp(key, "")==0)
		return LFIllegalKey;

	if (!GetMutex(Mutex_Stores))
		return LFMutexError;

	unsigned int res = LFIllegalKey;
	HANDLE StoreLock = NULL;
	LFStoreDescriptor* slot = FindStore(key, &StoreLock);
	if (slot)
	{
		LFStoreDescriptor victim = *slot;
		res = DeleteStore(slot);
		ReleaseMutexForStore(StoreLock);
		ReleaseMutex(Mutex_Stores);

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
			RemoveDir(victim.DatPath);
	}
	else
	{
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
