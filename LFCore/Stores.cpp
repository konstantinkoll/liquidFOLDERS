#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFVariantData.h"
#include "Mutex.h"
#include "StoreCache.h"
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
			SetAttributeUnicodeString(d, LFAttrFileName, sfi.szDisplayName);
			char key[] = " :";
			key[0] = cDrive;
			SetAttributeAnsiString(d, LFAttrFileID, key);
			SetAttributeUnicodeString(d, LFAttrHint, sfi.szTypeName);
			res->AddItemDescriptor(d);
		}
	}
}

LFSearchResult* QueryStores(LFFilter* filter)
{
	LFSearchResult* res = new LFSearchResult(LFContextStores);
	res->m_RecommendedView = LFViewLargeIcons;
	res->m_LastError = LFOk;
	res->m_HasCategories = TRUE;

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

LFCore_API unsigned int LFCreateStore(LFStoreDescriptor* s, BOOL MakeDefault, HWND hWndSource)
{
	// GUID generieren
	CoCreateGuid(&s->GUID);

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
		// Store phys. anlegen
		// TODO

		SendNotifyMessage(HWND_BROADCAST, LFMessages.StoresChanged, s->StoreMode==LFStoreModeInternal ? LFMSGF_IntStores : LFMSGF_ExtHybStores , (LPARAM)hWndSource);
	}

	return res;
}

LFCore_API unsigned int LFMakeDefaultStore(char* key, HWND hWndSource, BOOL InternalCall)
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
		if (slot->StoreMode!=LFStoreModeExternal)
		{
			res = LFIllegalStoreDescriptor;
		}
		else
		{
			char path[MAX_PATH];
			GetAutoPath(slot, path);
			strcat_s(path, MAX_PATH, "INDEX\\");

			// TODO: Index kopieren

			slot->StoreMode = LFStoreModeHybrid;
			strcpy_s(slot->IdxPathAux, MAX_PATH, path);

			res = UpdateStore(slot);
		}

	ReleaseMutexForStore(StoreLock);
	ReleaseMutex(Mutex_Stores);

	if (res==LFOk)
		SendMessage(HWND_BROADCAST, LFMessages.StoreAttributesChanged, LFMSGF_ExtHybStores, (LPARAM)hWndSource);

	return res;
}

LFCore_API unsigned int LFSetStoreAttributes(char* key, wchar_t* name, wchar_t* comment, HWND hWndSource, BOOL InternalCall)
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

LFCore_API unsigned int LFSetStoreComment(char* key, wchar_t* comment, HWND hWndSource, BOOL InternalCall)
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

		if (IsStoreMounted(&victim))
		{
			// TODO: Store phys. löschen (Verzeichnisse, Index), dabei nur auf victim zugreifen
		}
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
