
// LFCore.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//

#include "stdafx.h"
#include "AttributeTables.h"
#include "FileSystem.h"
#include "IATA.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "LFVariantData.h"
#include "resource.h"
#include "ShellProperties.h"
#include "Stores.h"
#include "Watchdog.h"
#include <assert.h>
#include <shlwapi.h>
#include <winioctl.h>


HMODULE LFCoreModuleHandle;
OSVERSIONINFO osInfo;


#pragma data_seg(".shared")

LFMessageIDs LFMessages;
LFVolumeDescriptor Volumes[26];

#pragma data_seg()
#pragma comment(linker, "/SECTION:.shared,RWS")


#pragma comment(lib, "shlwapi.lib")


LFCORE_API void LFInitialize()
{
	ZeroMemory(&Volumes, sizeof(Volumes));

	ZeroMemory(&osInfo, sizeof(OSVERSIONINFO));
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osInfo);

	LFMessages.UpdateProgress = RegisterWindowMessageA("liquidFOLDERS.UpdateProgress");
	LFMessages.ItemsDropped = RegisterWindowMessageA("liquidFOLDERS.ItemsDropped");
	LFMessages.StoresChanged = RegisterWindowMessageA("liquidFOLDERS.StoresChanged");
	LFMessages.StoreAttributesChanged = RegisterWindowMessageA("liquidFOLDERS.StoreAttributesChanged");
	LFMessages.DefaultStoreChanged = RegisterWindowMessageA("liquidFOLDERS.DefaultStoreChanged");
	LFMessages.StatisticsChanged = RegisterWindowMessageA("liquidFOLDERS.StatisticsChanged");

	InitMutex();
	InitAirportDatabase();
	InitStores();
	InitWatchdog();
}

LFCORE_API BOOL LFGetApplicationPath(LPWSTR pStr, SIZE_T cCount)
{
	assert(cCount>=MAX_PATH);

	// Registry
	HKEY hKey;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, L"Software\\liquidFOLDERS\\", &hKey)==ERROR_SUCCESS)
	{
		DWORD dwSize = (DWORD)(cCount*sizeof(WCHAR));
		LSTATUS Result = RegQueryValueEx(hKey, L"InstallLocation", 0, NULL, (BYTE*)pStr, &dwSize);

		RegCloseKey(hKey);

		if (Result==ERROR_SUCCESS)
			return TRUE;
	}

	// "Programme"-Verzeichnis
	if (SHGetSpecialFolderPath(NULL, pStr, CSIDL_PROGRAM_FILES, FALSE))
	{
		wcscat_s(pStr, cCount, L"\\liquidFOLDERS\\liquidFOLDERS.exe");

		if (_waccess(pStr, 0)==0)
			return TRUE;
	}

	// Selbes Verzeichnis wie die DLL
	GetModuleFileName(LFCoreModuleHandle, pStr, (DWORD)cCount);
	if (GetLastError()==ERROR_SUCCESS)
	{
		WCHAR* pChar = wcsrchr(pStr, L'\\');
		if (pChar)
			*(pChar+1) = L'\0';

		wcscat_s(pStr, cCount, L"liquidFOLDERS.exe");

		if (_waccess(pStr, 0)==0)
			return TRUE;
	}

	return FALSE;
}

LFCORE_API const LFMessageIDs* LFGetMessageIDs()
{
	return &LFMessages;
}


// Output handling
//

LFCORE_API void __stdcall LFGetFileSummary(UINT Count, INT64 Size, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	WCHAR tmpStr[256];
	StrFormatByteSize(Size, tmpStr, 256);

	WCHAR tmpMask[256];
	LoadString(LFCoreModuleHandle, Count==1 ? IDS_FILESUMMARY_SINGULAR : IDS_FILESUMMARY_PLURAL, tmpMask, 256);

	swprintf_s(pStr, cCount, tmpMask, Count, tmpStr);

	// Axe file size if 0
	if (!Size)
	{
		WCHAR* pChar = wcsstr(pStr, L" (");

		if (pChar)
			*pChar = L'\0';
	}
}

LFCORE_API void __stdcall LFGetFileSummaryEx(const LFFileSummary& FileSummary, LPWSTR pStr, SIZE_T cCount)
{
	assert(pStr);

	if (FileSummary.Duration && FileSummary.OnlyMediaFiles)
	{
		UINT nID = IDS_MINUTE;

		UINT64 Duration = max(1, (FileSummary.Duration+30000)/60000);
		if (Duration>=60)
		{
			nID = IDS_HOUR;
			Duration /= 60;
		}

		WCHAR tmpMask[256];
		LoadString(LFCoreModuleHandle, Duration==1 ? nID : nID+1, tmpMask, 256);

		WCHAR tmpStr[256];
		swprintf_s(tmpStr, 256, tmpMask, max(1, Duration));

		LoadString(LFCoreModuleHandle, FileSummary.FileCount==1 ? IDS_MEDIASUMMARY_SINGULAR : IDS_MEDIASUMMARY_PLURAL, tmpMask, 256);
		swprintf_s(pStr, cCount, tmpMask, FileSummary.FileCount, tmpStr);
	}
	else
	{
		LFGetFileSummary(FileSummary.FileCount, FileSummary.FileSize, pStr, cCount);
	}
}


// Registry settings
//

LFCORE_API BOOL LFHideFileExt()
{
	DWORD HideFileExt = 0;

	HKEY hKey;
	if (RegOpenKeyA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", &hKey)==ERROR_SUCCESS)
	{
		DWORD dwSize = sizeof(HideFileExt);
		RegQueryValueEx(hKey, L"HideFileExt", 0, NULL, (LPBYTE)&HideFileExt, &dwSize);

		RegCloseKey(hKey);
	}

	return (HideFileExt!=0);
}

LFCORE_API BOOL LFHideVolumesWithNoMedia()
{
	DWORD HideVolumesWithNoMedia = (osInfo.dwMajorVersion<6) ? 0 : 1;

	HKEY hKey;
	if (RegOpenKeyA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", &hKey)==ERROR_SUCCESS)
	{
		DWORD dwSize = sizeof(HideVolumesWithNoMedia);
		RegQueryValueEx(hKey, L"HideVolumesWithNoMedia", 0, NULL, (LPBYTE)&HideVolumesWithNoMedia, &dwSize);

		RegCloseKey(hKey);
	}

	return (HideVolumesWithNoMedia!=0);
}


// Resources
//

void LoadStringEnglish(UINT ID, LPWSTR lpBuffer, SIZE_T cchBufferMax)
{
	DWORD nID = (ID>>4)+1;
	DWORD nItemID = ID & 0x000F;

	HRSRC hResource = FindResourceEx(LFCoreModuleHandle, RT_STRING, MAKEINTRESOURCE(nID), MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL));
	if (hResource)
	{
		HGLOBAL hGlobal = LoadResource(LFCoreModuleHandle, hResource);
		LPCWSTR lpStr = (LPCWSTR)LockResource(hGlobal);

		UINT nStr = 0;
		DWORD dwSize = SizeofResource(LFCoreModuleHandle, hResource);

		DWORD pChar = 0;
		while (pChar<dwSize)
		{
			if (nStr==nItemID)
			{
				if (lpStr[pChar])
				{
					wcsncpy_s(lpBuffer, cchBufferMax, &lpStr[pChar+1], lpStr[pChar]);
					lpBuffer[lpStr[pChar]] = L'\0';
				}
				else
				{
					lpBuffer[0] = L'\0';
				}

				break;
			}

			pChar += lpStr[pChar]+1;
			nStr++;
		}
	}
}

void LoadTwoStrings(HINSTANCE hInstance, UINT ID, LPWSTR lpBuffer1, SIZE_T cchBufferMax1, LPWSTR lpBuffer2, SIZE_T cchBufferMax2)
{
	assert(lpBuffer1);
	assert(lpBuffer2);

	WCHAR tmpStr[256];
	LoadString(hInstance, ID, tmpStr, 256);

	WCHAR* pChar = wcschr(tmpStr, L'\n');
	if (pChar)
	{
		wcscpy_s(lpBuffer2, cchBufferMax2, pChar+1);
		*pChar = L'\0';
	}
	else
	{
		*lpBuffer2 = L'\0';
	}

	wcscpy_s(lpBuffer1, cchBufferMax1, tmpStr);
}


// Volumes
//

BYTE GetVolumeBus(CHAR cVolume)
{
	assert(cVolume>='A');
	assert(cVolume<='Z');

	BYTE VolumeBus = BusTypeMaxReserved;

	WCHAR szDevice[7] = L"\\\\?\\ :";
	szDevice[4] = cVolume;

	HANDLE hDevice = CreateFile(szDevice, 0, 0, NULL, OPEN_EXISTING, NULL, NULL);
	if (hDevice!=INVALID_HANDLE_VALUE)
	{
		STORAGE_ADAPTER_DESCRIPTOR OutBuffer;
		OutBuffer.Size = sizeof(STORAGE_ADAPTER_DESCRIPTOR);

		STORAGE_PROPERTY_QUERY Query;
		Query.PropertyId = StorageAdapterProperty;
		Query.QueryType = PropertyStandardQuery;

		DWORD dwOutBytes;
		if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
			&Query, sizeof(STORAGE_PROPERTY_QUERY), &OutBuffer, sizeof(STORAGE_ADAPTER_DESCRIPTOR),
			&dwOutBytes, NULL))
			VolumeBus = OutBuffer.BusType;

		CloseHandle(hDevice);
	}

	return VolumeBus;
}

void GetVolumeInformation(CHAR cVolume, BOOL ForceAvailable=FALSE)
{
	assert(cVolume>='A');
	assert(cVolume<='Z');

	const UINT VolumeID = cVolume-'A';

	// Already mounted?
	if (Volumes[VolumeID].Mounted)
		return;

	// Check if volume exists
	if (!ForceAvailable)
		if ((GetLogicalDrives() & (1<<VolumeID))==0)
			return;

	Volumes[VolumeID].Mounted = TRUE;
	Volumes[VolumeID].LogicalVolumeType = LFGLV_INTERNAL;
	Volumes[VolumeID].Source = LFTypeSourceInternal;

	// Detect drive type and volume source
	CHAR szRoot[] = " :\\";
	szRoot[0] = cVolume;

	switch (GetDriveTypeA(szRoot))
	{
	case DRIVE_REMOVABLE:
		Volumes[VolumeID].LogicalVolumeType = LFGLV_EXTERNAL;

	case DRIVE_FIXED:
		switch (GetVolumeBus(cVolume))
		{
		case BusType1394:
			Volumes[VolumeID].LogicalVolumeType = LFGLV_EXTERNAL;
			Volumes[VolumeID].Source = LFTypeSource1394;
			break;

		case BusTypeUsb:
			Volumes[VolumeID].LogicalVolumeType = LFGLV_EXTERNAL;
			Volumes[VolumeID].Source = LFTypeSourceUSB;
			break;
		}

		break;

	case DRIVE_REMOTE:
		Volumes[VolumeID].LogicalVolumeType = LFGLV_NETWORK;
		Volumes[VolumeID].Source = LFTypeSourceNethood;
		break;

	default:
		Volumes[VolumeID].LogicalVolumeType = 0;
	}
}

LFCORE_API UINT LFGetSourceForVolume(CHAR cVolume)
{
	assert(cVolume>='A');
	assert(cVolume<='Z');

	GetVolumeInformation(cVolume);

	const UINT VolumeID = cVolume-'A';

	return Volumes[VolumeID].Mounted ? Volumes[VolumeID].Source : LFTypeSourceUnknown;
}

LFCORE_API UINT LFGetLogicalVolumes(UINT Mask)
{
	DWORD VolumesOnSystem = GetLogicalDrives();
	if ((Mask & LFGLV_FLOPPIES)==0)
		VolumesOnSystem &= ~3;

	DWORD Index = 4;

	for (CHAR cVolume='C'; cVolume<='Z'; cVolume++, Index<<=1)
	{
		const UINT VolumeID = cVolume-'A';

		if ((VolumesOnSystem & Index)==0)
		{
			Volumes[VolumeID].Mounted = FALSE;
			continue;
		}

		GetVolumeInformation(cVolume, TRUE);

		if ((Mask & Volumes[VolumeID].LogicalVolumeType)==0)
			VolumesOnSystem &= ~Index;
	}

	return VolumesOnSystem;
}


// Threading
//

LFCORE_API void LFInitProgress(LFProgress* pProgress, HWND hWnd, UINT MajorCount)
{
	assert(pProgress);

	ZeroMemory(pProgress, sizeof(LFProgress));

	pProgress->hWnd = hWnd;
	pProgress->ProgressState = LFProgressWorking;
	pProgress->MajorCount = MajorCount;
}


// Error handling
//

LFCORE_API void LFGetErrorText(LPWSTR pStr, SIZE_T cCount, UINT ID)
{
	assert(pStr);

	LoadString(LFCoreModuleHandle, IDS_ERR_FIRST+ID, pStr, (INT)cCount);
}

LFCORE_API void LFCoreErrorBox(UINT ID, HWND hWnd)
{
	if (ID>LFCancel)
	{
		WCHAR Caption[256];
		LoadString(LFCoreModuleHandle, IDS_ERRORCAPTION, Caption, 256);

		WCHAR Message[256];
		LFGetErrorText(Message, 256, ID);

		if (!hWnd)
			hWnd = GetForegroundWindow();

		MessageBox(hWnd, Message, Caption, MB_OK | MB_ICONERROR);
	}
}


// Descriptors
//

LFCORE_API void LFGetAttrCategoryName(LPWSTR pStr, SIZE_T cCount, UINT ID)
{
	assert(pStr);

	LoadString(LFCoreModuleHandle, IDS_ATTRCATEGORY_FIRST+ID, pStr, (INT)cCount);
}

LFCORE_API void LFGetAttributeInfo(LFAttributeDescriptor& AttributeDescriptor, UINT ID)
{
	ZeroMemory(&AttributeDescriptor, sizeof(LFAttributeDescriptor));

	if (ID>=LFAttributeCount)
		return;

	// Name
	LoadString(LFCoreModuleHandle, IDS_ATTR_FIRST+ID, AttributeDescriptor.Name, 256);

	// XML ID
	WCHAR Str[256];
	LoadStringEnglish(ID+IDS_ATTR_FIRST, Str, 256);

	WCHAR* pCharSrc = Str;
	WCHAR* pCharDst = AttributeDescriptor.XMLID;

	do
	{
		WCHAR Ch = (WCHAR)towlower(*pCharSrc);

		if ((Ch>=L'a') && (Ch<=L'z'))
			*(pCharDst++) = Ch;
	}
	while (*(pCharSrc++)!=L'\0');

	*pCharDst = L'\0';

	// Check consistency
	assert(AttrProperties[ID].Type<LFTypeCount);
	assert((AttrProperties[ID].Type!=LFAttrFlags) || AttrProperties[ID].ReadOnly);
	assert((AttrProperties[ID].DefaultView==(UINT)-1) || (TypeProperties[AttrProperties[ID].Type].AllowedViews & (1<<AttrProperties[ID].DefaultView)));
	assert(AttrProperties[ID].DefaultPriority<=LFMaxAttributePriority);

	assert(LFAttrFileName==0);
	assert(AttrProperties[LFAttrFileName].DefaultPriority==0);
	assert(AttrProperties[LFAttrComments].DefaultPriority==LFMaxAttributePriority);
	assert(AttrProperties[LFAttrFileFormat].DefaultPriority==LFMaxAttributePriority);

	// Properties
	AttributeDescriptor.AttrProperties = AttrProperties[ID];
	AttributeDescriptor.TypeProperties = TypeProperties[AttrProperties[ID].Type];
}

LFCORE_API void LFGetSourceName(LPWSTR pStr, SIZE_T cCount, UINT ID, BOOL Qualified)
{
	LoadString(LFCoreModuleHandle, (Qualified ? IDS_QSRC_FIRST : IDS_SRC_FIRST)+ID, pStr, (INT)cCount);
}

LFCORE_API void LFGetItemCategoryInfo(LFItemCategoryDescriptor& ItemCategoryDescriptor, UINT ID)
{
	ZeroMemory(&ItemCategoryDescriptor, sizeof(LFItemCategoryDescriptor));

	if (ID>=LFItemCategoryCount)
		return;

	if (ID<LFItemCategory0600)
	{
		// Load strings from resources
		LoadTwoStrings(LFCoreModuleHandle, IDS_ITEMCATEGORY_FIRST+ID, ItemCategoryDescriptor.Caption, 256, ItemCategoryDescriptor.Hint, 256);
	}
	else
	{
		// Create time according to locale
		SYSTEMTIME st;
		ZeroMemory(&st, sizeof(st));

		st.wHour = (WORD)(ID-LFItemCategory0600+6);
		GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, ItemCategoryDescriptor.Caption, 256);
	}
}

LFCORE_API void LFGetContextInfo(LFContextDescriptor& ContextDescriptor, UINT ID)
{
	ZeroMemory(&ContextDescriptor, sizeof(LFContextDescriptor));

	if (ID>=LFContextCount)
		return;

	LoadTwoStrings(LFCoreModuleHandle, IDS_CONTEXT_FIRST+ID, ContextDescriptor.Name, 256, ContextDescriptor.Comment, 256);

	// Check consistency
	assert(CtxProperties[ID].AvailableViews!=0);
	assert(CtxProperties[ID].AvailableViews & (1<<CtxProperties[ID].DefaultView));
	assert(TypeProperties[AttrProperties[CtxProperties[ID].DefaultAttribute].Type].AllowedViews & (1<<CtxProperties[ID].DefaultView));

	assert(CtxProperties[ID].AvailableAttributes & (1ull<<LFAttrFileName));
	assert(CtxProperties[ID].AvailableAttributes & (1ull<<CtxProperties[ID].DefaultAttribute));
	assert(CtxProperties[ID].AdvertisedAttributes!=0);
	assert((CtxProperties[ID].AvailableAttributes | CtxProperties[ID].AdvertisedAttributes)==CtxProperties[ID].AvailableAttributes);

#ifdef _DEBUG
	for (UINT a=0; a<LFAttributeCount; a++)
		if ((CtxProperties[ID].AvailableAttributes>>a) & 1)
		{
			assert(CtxProperties[ID].AvailableViews & TypeProperties[AttrProperties[a].Type].AllowedViews);
			assert(TypeProperties[AttrProperties[a].Type].Sortable);
			assert(AttrProperties[a].DefaultView!=(UINT)-1);
		}
#endif

	// Properties
	ContextDescriptor.CtxProperties = CtxProperties[ID];
}

LFCORE_API void __stdcall LFGetSortedAttributeList(LFAttributeList& AttributeList)
{
	UINT Index = 0;

	for (UINT Priority=0; Priority<=LFMaxAttributePriority; Priority++)
		for (UINT a=0; a<LFAttributeCount; a++)
			if (AttrProperties[a].DefaultPriority==Priority)
				AttributeList[Index++] = a;
}
