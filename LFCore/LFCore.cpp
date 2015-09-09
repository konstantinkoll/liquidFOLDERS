
// LFCore.cpp : Definiert die exportierten Funktionen f�r die DLL-Anwendung.
//

#include "stdafx.h"
#include "IATA.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "LFVariantData.h"
#include "Mutex.h"
#include "resource.h"
#include "ShellProperties.h"
#include "StoreCache.h"
#include "Watchdog.h"
#include <assert.h>
#include <shlwapi.h>
#include <winioctl.h>


HMODULE LFCoreModuleHandle;
OSVERSIONINFO osInfo;
extern LFShellProperty AttrProperties[];


#pragma data_seg(".shared")

LFMessageIDs LFMessages;
UINT VolumeTypes[26] = { DRIVE_UNKNOWN };

#pragma data_seg()
#pragma comment(linker, "/SECTION:.shared,RWS")


#pragma comment(lib, "shlwapi.lib")


LFCORE_API void LFInitialize()
{
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
	InitStoreCache();
	InitWatchdog();
}

LFCORE_API BOOL LFGetApplicationPath(WCHAR* pStr, SIZE_T cCount)
{
	assert(cCount>=MAX_PATH);

	// Registry
	HKEY hKey;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, L"Software\\liquidFOLDERS\\", &hKey)==ERROR_SUCCESS)
	{
		DWORD Size = (DWORD)(cCount*sizeof(WCHAR));
		LSTATUS Result = RegQueryValueEx(hKey, L"InstallLocation", 0, NULL, (BYTE*)pStr, &Size);

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
		WCHAR* Ptr = wcsrchr(pStr, L'\\');
		if (Ptr)
			*(Ptr+1) = L'\0';

		wcscat_s(pStr, cCount, L"liquidFOLDERS.exe");

		if (_waccess(pStr, 0)==0)
			return TRUE;
	}

	return FALSE;
}

LFCORE_API LFMessageIDs* LFGetMessageIDs()
{
	return &LFMessages;
}


// Output handling
//

LFCORE_API void LFCombineFileCountSize(UINT Count, INT64 Size, WCHAR* pStr, SIZE_T cCount)
{
	assert(pStr);

	WCHAR tmpStr[256];
	StrFormatByteSize(Size, tmpStr, 256);

	WCHAR tmpMask[256];
	LoadString(LFCoreModuleHandle, Count==1 ? IDS_FILECOUNT_SINGULAR : IDS_FILECOUNT_PLURAL, tmpMask, 256);

	swprintf_s(pStr, cCount, tmpMask, Count, tmpStr);
}


// Registry settings
//

LFCORE_API BOOL LFHideFileExt()
{
	DWORD HideFileExt = 0;

	HKEY hKey;
	if (RegOpenKeyA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", &hKey)==ERROR_SUCCESS)
	{
		DWORD sz = sizeof(HideFileExt);
		RegQueryValueEx(hKey, L"HideFileExt", 0, NULL, (LPBYTE)&HideFileExt, &sz);

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
		DWORD sz = sizeof(HideVolumesWithNoMedia);
		RegQueryValueEx(hKey, L"HideVolumesWithNoMedia", 0, NULL, (LPBYTE)&HideVolumesWithNoMedia, &sz);

		RegCloseKey(hKey);
	}

	return (HideVolumesWithNoMedia!=0);
}


// Resources
//

void LoadStringEnglish(UINT ID, WCHAR* lpBuffer, INT cchBufferMax)
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

		DWORD Ptr = 0;
		while (Ptr<dwSize)
		{
			if (nStr==nItemID)
			{
				if (lpStr[Ptr])
				{
					wcsncpy_s(lpBuffer, cchBufferMax, &lpStr[Ptr+1], lpStr[Ptr]);
					lpBuffer[lpStr[Ptr]] = L'\0';
				}
				else
				{
					lpBuffer[0] = L'\0';
				}

				break;
			}

			Ptr += lpStr[Ptr]+1;
			nStr++;
		}

		UnlockResource(lpStr);
		FreeResource(hGlobal);
	}
}

void LoadTwoStrings(HINSTANCE hInstance, UINT ID, WCHAR* lpBuffer1, INT cchBufferMax1, WCHAR* lpBuffer2, INT cchBufferMax2)
{
	assert(lpBuffer1);
	assert(lpBuffer2);

	WCHAR tmpStr[256];
	LoadString(hInstance, ID, tmpStr, 256);

	WCHAR* Ptr = wcschr(tmpStr, L'\n');
	if (Ptr)
	{
		wcscpy_s(lpBuffer2, cchBufferMax2, Ptr+1);
		*Ptr = L'\0';
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
	BYTE VolumeBus = BusTypeMaxReserved;

	CHAR szBuf[MAX_PATH] = "\\\\?\\ :";
	szBuf[4] = cVolume;

	HANDLE hDevice = CreateFileA(szBuf, 0, 0, NULL, OPEN_EXISTING, NULL, NULL);
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

LFCORE_API UINT LFGetSourceForVolume(CHAR cVolume)
{
	if ((cVolume>='A') && (cVolume<='Z'))
		switch(GetVolumeBus(cVolume))
		{
		case BusType1394:
			return LFTypeSource1394;

		case BusTypeUsb:
			return LFTypeSourceUSB;

		}

	return LFTypeSourceUnknown;
}

LFCORE_API UINT LFGetLogicalVolumes(UINT Mask)
{
	DWORD VolumesOnSystem = GetLogicalDrives();
	if ((Mask & LFGLV_INCLUDEFLOPPIES)==0)
		VolumesOnSystem &= ~3;

	DWORD Index = 1;
	CHAR szVolumeRoot[] = " :\\";

	for (CHAR cVolume='A'; cVolume<='Z'; cVolume++, Index<<=1)
	{
		if ((VolumesOnSystem & Index)==0)
		{
			VolumeTypes[cVolume-'A'] = DRIVE_UNKNOWN;
			continue;
		}

		UINT uVolumeType = VolumeTypes[cVolume-'A'];
		if (uVolumeType==DRIVE_UNKNOWN)
		{
			szVolumeRoot[0] = cVolume;
			uVolumeType = GetDriveTypeA(szVolumeRoot);

			if (uVolumeType==DRIVE_FIXED)
				switch(GetVolumeBus(cVolume))
				{
				case BusType1394:
				case BusTypeUsb:
					uVolumeType = DRIVE_REMOVABLE;
					break;
				}

			VolumeTypes[cVolume-'A'] = uVolumeType;
		}

		switch(uVolumeType)
		{
		case DRIVE_FIXED:
			if (!(Mask & LFGLV_INTERNAL))
				VolumesOnSystem &= ~Index;

			break;

		case DRIVE_REMOVABLE:
			if (!(Mask & LFGLV_EXTERNAL))
				VolumesOnSystem &= ~Index;

			break;

		case DRIVE_REMOTE:
			if (!(Mask & LFGLV_NETWORK))
				VolumesOnSystem &= ~Index;

			break;

		default:
			VolumesOnSystem &= ~Index;
		}
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

LFCORE_API void LFGetErrorText(WCHAR* pStr, SIZE_T cCount, UINT ID)
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

#define SETRANGE(Var, ID, Lo, Hi, Val) if ((ID>=Lo) && (ID<=Hi)) Var = Val;
#define ALLOWATTRIBUTE(Attr) ContextDescriptor.AllowedAttributes[Attr>>5] |= 1<<(Attr & 0x1F);

LFCORE_API void LFGetAttrCategoryName(WCHAR* pStr, SIZE_T cCount, UINT ID)
{
	assert(pStr);

	LoadString(LFCoreModuleHandle, IDS_ATTRCATEGORY_FIRST+ID, pStr, cCount);
}

LFCORE_API void LFGetAttributeInfo(LFAttributeDescriptor& AttributeDescriptor, UINT ID)
{
	ZeroMemory(&AttributeDescriptor, sizeof(LFAttributeDescriptor));

	if (ID>=LFAttributeCount)
		return;

	LoadString(LFCoreModuleHandle, IDS_ATTR_FIRST+ID, AttributeDescriptor.Name, 256);

	AttributeDescriptor.AlwaysVisible = (ID==LFAttrFileName);
	AttributeDescriptor.Type = AttrTypes[ID];

	WCHAR Str[256];
	LoadStringEnglish(ID+IDS_ATTR_FIRST, Str, 256);

	WCHAR* PtrSrc = Str;
	WCHAR* PtrDst = AttributeDescriptor.XMLID;

	do
	{
		WCHAR Ch = (WCHAR)towlower(*PtrSrc);

		if ((Ch>=L'a') && (Ch<=L'z'))
			*(PtrDst++) = Ch;
	}
	while (*(PtrSrc++)!=L'\0');

	*PtrDst = L'\0';

	// Type and character count (where appropriate)
	if (AttributeDescriptor.Type<=LFTypeAnsiString)
		AttributeDescriptor.cCharacters = (UINT)GetAttributeMaxCharacterCount(ID);

	// Recommended width
	static const UINT rWidths[LFTypeCount] = { 200, 200, 200, 100, 100, 100, 120, 100, 100, 100, 150, 140, 100 };
	AttributeDescriptor.RecommendedWidth = (ID==LFAttrComments) ? 350 : rWidths[AttributeDescriptor.Type];

	// Category
	if (ID<=LFAttrRating)
	{
		AttributeDescriptor.Category = ((ID==LFAttrStoreID) || (ID==LFAttrFileID) || (ID==LFAttrAddTime) || (ID==LFAttrDeleteTime) || (ID==LFAttrFileFormat) || (ID==LFAttrFlags)) ? LFAttrCategoryInternal : LFAttrCategoryBasic;
	}
	else
	{
		SETRANGE(AttributeDescriptor.Category, ID, LFAttrLocationName, LFAttrLocationGPS, LFAttrCategoryGeotags);
		SETRANGE(AttributeDescriptor.Category, ID, LFAttrWidth, LFAttrRoll, LFAttrCategoryVisual);
		SETRANGE(AttributeDescriptor.Category, ID, LFAttrExposure, LFAttrChip, LFAttrCategoryPhotographic);
		SETRANGE(AttributeDescriptor.Category, ID, LFAttrAlbum, LFAttrAudioCodec, LFAttrCategoryAudio);
		SETRANGE(AttributeDescriptor.Category, ID, LFAttrDuration, LFAttrBitrate, LFAttrCategoryTimebased);
		SETRANGE(AttributeDescriptor.Category, ID, LFAttrArtist, LFAttrSignature, LFAttrCategoryBibliographic);
		SETRANGE(AttributeDescriptor.Category, ID, LFAttrFrom, LFAttrCustomer, LFAttrCategoryWorkflow);
		SETRANGE(AttributeDescriptor.Category, ID, LFAttrPriority, LFAttrPriority, LFAttrCategoryWorkflow);
	}

	// Sorting
	AttributeDescriptor.Sortable = (AttributeDescriptor.Type!=LFTypeFlags);
	AttributeDescriptor.PreferDescendingSort = (AttributeDescriptor.Type==LFTypeRating) || (AttributeDescriptor.Type==LFTypeTime) || (AttributeDescriptor.Type==LFTypeMegapixel);

	// Read only
	switch(ID)
	{
	case LFAttrDescription:
	case LFAttrCreationTime:
	case LFAttrFileTime:
	case LFAttrArchiveTime:
	case LFAttrFileCount:
	case LFAttrFileSize:
	case LFAttrHeight:
	case LFAttrWidth:
	case LFAttrDimension:
	case LFAttrAspectRatio:
	case LFAttrVideoCodec:
	case LFAttrExposure:
	case LFAttrFocus:
	case LFAttrAperture:
	case LFAttrChip:
	case LFAttrChannels:
	case LFAttrSamplerate:
	case LFAttrAudioCodec:
	case LFAttrDuration:
	case LFAttrBitrate:
	case LFAttrPages:
	case LFAttrEquipment:
	case LFAttrFrom:
	case LFAttrTo:
	case LFAttrLikeCount:
		AttributeDescriptor.ReadOnly = TRUE;
		break;

	default:
		AttributeDescriptor.ReadOnly = (AttributeDescriptor.Category==LFAttrCategoryInternal);
	}

	// Format
	AttributeDescriptor.FormatRight = (((AttributeDescriptor.Type>=LFTypeUINT) && (AttributeDescriptor.Type!=LFTypeTime)) || (ID==LFAttrStoreID) || (ID==LFAttrFileID));

	// Shell property
	if (AttrProperties[ID].ID)
	{
		AttributeDescriptor.ShPropertyMapping = AttrProperties[ID];
	}
	else
	{
		AttributeDescriptor.ShPropertyMapping.Schema = PropertyLiquidFolders;
		AttributeDescriptor.ShPropertyMapping.ID = ID;
	}
}

LFCORE_API void LFGetSourceName(WCHAR* pStr, SIZE_T cCount, UINT ID, BOOL Qualified)
{
	LoadString(LFCoreModuleHandle, (Qualified ? IDS_QSRC_FIRST : IDS_SRC_FIRST)+ID, pStr, cCount);
}

LFCORE_API void LFGetItemCategoryInfo(LFItemCategoryDescriptor& ItemCategoryDescriptor, UINT ID)
{
	ZeroMemory(&ItemCategoryDescriptor, sizeof(LFItemCategoryDescriptor));

	if (ID>=LFItemCategoryCount)
		return;

	LoadTwoStrings(LFCoreModuleHandle, IDS_ITEMCATEGORY_FIRST+ID, ItemCategoryDescriptor.Caption, 256, ItemCategoryDescriptor.Hint, 256);
}

LFCORE_API void LFGetContextInfo(LFContextDescriptor& ContextDescriptor, UINT ID)
{
	ZeroMemory(&ContextDescriptor, sizeof(LFContextDescriptor));

	if (ID>=LFContextCount)
		return;

	LoadTwoStrings(LFCoreModuleHandle, IDS_CONTEXT_FIRST+ID, ContextDescriptor.Name, 256, ContextDescriptor.Comment, 256);

	ContextDescriptor.AllowGroups = (ID<=LFLastGroupContext) || (ID==LFContextSearch);

	ALLOWATTRIBUTE(LFAttrFileName);
	ALLOWATTRIBUTE(LFAttrStoreID);
	ALLOWATTRIBUTE(LFAttrComments);

	switch(ID)
	{
	case LFContextStores:
		ALLOWATTRIBUTE(LFAttrDescription);
		ALLOWATTRIBUTE(LFAttrCreationTime);
		ALLOWATTRIBUTE(LFAttrFileTime);
		ALLOWATTRIBUTE(LFAttrFileCount);
		ALLOWATTRIBUTE(LFAttrFileSize);

		break;

	case LFContextFilters:
		ALLOWATTRIBUTE(LFAttrFileID);
		ALLOWATTRIBUTE(LFAttrCreationTime);
		ALLOWATTRIBUTE(LFAttrFileTime);
		ALLOWATTRIBUTE(LFAttrAddTime);
		ALLOWATTRIBUTE(LFAttrFileSize);

		break;

	default:
		if (ID==LFContextArchive)
			ALLOWATTRIBUTE(LFAttrArchiveTime);

		if (ID==LFContextTrash)
			ALLOWATTRIBUTE(LFAttrDeleteTime);

		for (UINT a=0; a<LFAttributeCount; a++)
			if (((a!=LFAttrFileCount) || (ID<LFContextSubfolderDefault)) && (a!=LFAttrDescription) && (a!=LFAttrArchiveTime) && (a!=LFAttrDeleteTime))
				ALLOWATTRIBUTE(a);
	}
}
