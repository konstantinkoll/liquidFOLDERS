
// LFCore.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//

#include "stdafx.h"
#include "LFCore.h"
#include "resource.h"
#include "IATA.h"
#include "LFItemDescriptor.h"
#include "LFVariantData.h"
#include "License.h"
#include "Mutex.h"
#include "ShellProperties.h"
#include "StoreCache.h"
#include "Watchdog.h"
#include <assert.h>
#include <iostream>
#include <shlobj.h>
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


LFCORE_API void LFInitialize()
{
	ZeroMemory(&osInfo, sizeof(OSVERSIONINFO));
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osInfo);

	LFMessages.ItemsDropped = RegisterWindowMessageA("liquidFOLDERS.ItemsDropped");
	LFMessages.StoresChanged = RegisterWindowMessageA("liquidFOLDERS.StoresChanged");
	LFMessages.StoreAttributesChanged = RegisterWindowMessageA("liquidFOLDERS.StoreAttributesChanged");
	LFMessages.DefaultStoreChanged = RegisterWindowMessageA("liquidFOLDERS.DefaultStoreChanged");
	LFMessages.VolumesChanged = RegisterWindowMessageA("liquidFOLDERS.VolumesChanged");
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
	HKEY k;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, L"Software\\liquidFOLDERS\\", &k)==ERROR_SUCCESS)
	{
		DWORD Size = (DWORD)(cCount*sizeof(WCHAR));
		LSTATUS Result = RegQueryValueEx(k, L"InstallLocation", 0, NULL, (BYTE*)pStr, &Size);

		RegCloseKey(k);

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
	GetModuleFileName(LFCoreModuleHandle, pStr, cCount);
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

void LoadStringEnglish(HINSTANCE hInstance, UINT uID, WCHAR* lpBuffer, INT cchBufferMax)
{
	DWORD nID = (uID>>4)+1;
	DWORD nItemID = uID & 0x000F;

	HRSRC hRes = FindResourceEx(hInstance, RT_STRING, MAKEINTRESOURCE(nID), MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL));
	if (hRes)
	{
		HGLOBAL hGlobal = LoadResource(hInstance, hRes);
		LPCWSTR lpStr = (LPCWSTR)LockResource(hGlobal);

		UINT nStr = 0;
		DWORD dwSize = SizeofResource(hInstance, hRes);
		DWORD ptr = 0;
		while (ptr<dwSize)
		{
			if (nStr==nItemID)
			{
				if (lpStr[ptr])
				{
					wcsncpy_s(lpBuffer, cchBufferMax, &lpStr[ptr+1], lpStr[ptr]);
					lpBuffer[lpStr[ptr]] = L'\0';
				}
				else
				{
					lpBuffer[0] = L'\0';
				}

				break;
			}

			ptr += lpStr[ptr]+1;
			nStr++;
		}

		UnlockResource(lpStr);
		FreeResource(hGlobal);
	}
}

void LoadTwoStrings(HINSTANCE hInstance, UINT uID, WCHAR* lpBuffer1, INT cchBufferMax1, WCHAR* lpBuffer2, INT cchBufferMax2)
{
	assert(lpBuffer1);
	assert(lpBuffer2);

	WCHAR tmpStr[256];
	LoadString(hInstance, uID, tmpStr, 256);

	WCHAR* brk = wcschr(tmpStr, L'\n');
	if (brk)
	{
		wcscpy_s(lpBuffer2, cchBufferMax2, brk+1);
		*brk = L'\0';
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
		switch (GetVolumeBus(cVolume))
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
				switch (GetVolumeBus(cVolume))
				{
				case BusType1394:
				case BusTypeUsb:
					uVolumeType = DRIVE_REMOVABLE;
					break;
				}

			VolumeTypes[cVolume-'A'] = uVolumeType;
		}

		switch (uVolumeType)
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
	LoadString(LFCoreModuleHandle, ID+IDS_ERR_FIRST, pStr, (INT)cCount);
}








LFCORE_API void LFCreateSendTo(BOOL force)
{
	HKEY k;
	if (RegCreateKeyA(HKEY_CURRENT_USER, "Software\\liquidFOLDERS", &k)==ERROR_SUCCESS)
	{
		BOOL created = FALSE;

		DWORD type;
		DWORD sz = sizeof(created);
		if (RegQueryValueExA(k, "SendToCreated", NULL, &type, (BYTE*)created, &sz)==ERROR_SUCCESS)
		{
			force |= (created==FALSE);
		}
		else
		{
			force = TRUE;
		}

		created = TRUE;
		RegSetValueExA(k, "SendToCreated", 0, REG_DWORD, (BYTE*)&created, sizeof(created));
		RegCloseKey(k);
	}

	if (force)
	{
		WCHAR Path[MAX_PATH];
		if (SHGetSpecialFolderPath(NULL, Path, CSIDL_SENDTO, TRUE))
		{
			WCHAR Name[256];
			LFGetDefaultStoreName(Name, 256);

			wcscat_s(Path, MAX_PATH, L"\\");
			wcscat_s(Path, MAX_PATH, Name);
			wcscat_s(Path, MAX_PATH, L".LFSendTo");

			HANDLE hFile = CreateFile(Path, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			if (hFile!=INVALID_HANDLE_VALUE)
				CloseHandle(hFile);
		}
		}
}



__forceinline void SetRange(BYTE &var, UINT ID, UINT lo, UINT hi, BYTE val)
{
	if ((ID>=lo) && (ID<=hi))
		var = val;
}

LFCORE_API void LFGetAttributeInfo(LFAttributeDescriptor& Attr, UINT ID)
{
	ZeroMemory(&Attr, sizeof(LFAttributeDescriptor));
	if (ID>=LFAttributeCount)
		return;

	LoadString(LFCoreModuleHandle, ID+IDS_ATTR_FIRST, Attr.Name, 256);
	Attr.AlwaysVisible = (ID==LFAttrFileName);
	Attr.Type = AttrTypes[ID];

	WCHAR tmpBuf[256];
	WCHAR* ptrSrc = tmpBuf;
	WCHAR* ptrDst = Attr.XMLID;
	LoadStringEnglish(LFCoreModuleHandle, ID+IDS_ATTR_FIRST, tmpBuf, 256);

	do
	{
		WCHAR ch = (WCHAR)towlower(*ptrSrc);
		if ((ch>=L'a') && (ch<=L'z'))
			*(ptrDst++) = ch;
	}
	while (*(ptrSrc++)!=L'\0');
	*ptrDst = L'\0';

	// Type and character count (where appropriate)
	switch (Attr.Type)
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
	case LFTypeAnsiString:
		Attr.cCharacters = (UINT)GetAttributeMaxCharacterCount(ID);
		break;
	case LFTypeFourCC:
		Attr.cCharacters = 4;
		break;
	}

	// Recommended width
	static const UINT rWidths[LFTypeCount] = { 200, 200, 200, 100, 100, 100, 120, 100, 100, 100, 150, 140, 100 };
	Attr.RecommendedWidth = (ID==LFAttrComments) ? 350 : rWidths[Attr.Type];

	// Category
	if (ID<=LFAttrRating)
	{
		Attr.Category = ((ID==LFAttrStoreID) || (ID==LFAttrFileID) || (ID==LFAttrAddTime) || (ID==LFAttrDeleteTime) || (ID==LFAttrFileFormat) || (ID==LFAttrFlags)) ? LFAttrCategoryInternal : LFAttrCategoryBasic;
	}
	else
	{
		SetRange(Attr.Category, ID, LFAttrLocationName, LFAttrLocationGPS, LFAttrCategoryGeotags);
		SetRange(Attr.Category, ID, LFAttrWidth, LFAttrRoll, LFAttrCategoryVisual);
		SetRange(Attr.Category, ID, LFAttrExposure, LFAttrChip, LFAttrCategoryPhotographic);
		SetRange(Attr.Category, ID, LFAttrAlbum, LFAttrAudioCodec, LFAttrCategoryAudio);
		SetRange(Attr.Category, ID, LFAttrDuration, LFAttrBitrate, LFAttrCategoryTimebased);
		SetRange(Attr.Category, ID, LFAttrArtist, LFAttrSignature, LFAttrCategoryBibliographic);
		SetRange(Attr.Category, ID, LFAttrFrom, LFAttrCustomer, LFAttrCategoryWorkflow);
		SetRange(Attr.Category, ID, LFAttrPriority, LFAttrPriority, LFAttrCategoryWorkflow);
	}

	// Sorting
	Attr.Sortable = (Attr.Type!=LFTypeFlags);
	Attr.PreferDescendingSort = (Attr.Type==LFTypeRating) || (Attr.Type==LFTypeTime) || (Attr.Type==LFTypeMegapixel);

	// ReadOnly
	switch (ID)
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
		Attr.ReadOnly = TRUE;
		break;
	default:
		Attr.ReadOnly = (Attr.Category==LFAttrCategoryInternal);
	}

	// Format
	Attr.FormatRight = (((Attr.Type>=LFTypeUINT) && (Attr.Type!=LFTypeTime)) || (ID==LFAttrStoreID) || (ID==LFAttrFileID));

	// Shell property
	Attr.ShPropertyMapping = AttrProperties[ID];
	if (!Attr.ShPropertyMapping.ID)
	{
		Attr.ShPropertyMapping.Schema = PropertyLiquidFolders;
		Attr.ShPropertyMapping.ID = ID;
	}
}


LFCORE_API void LFGetContextInfo(LFContextDescriptor& ctx, UINT ID)
{
	ZeroMemory(&ctx, sizeof(LFContextDescriptor));
	if (ID>=LFContextCount)
		return;

#define AllowAttribute(Attr) ctx.AllowedAttributes[Attr>>5] |= 1<<(Attr & 0x1F);

	LoadTwoStrings(LFCoreModuleHandle, IDS_CONTEXT_FIRST+ID, ctx.Name, 256, ctx.Comment, 256);

	ctx.AllowGroups = (ID<=LFLastGroupContext) || (ID==LFContextSearch);

	AllowAttribute(LFAttrFileName);
	AllowAttribute(LFAttrStoreID);
	AllowAttribute(LFAttrComments);

	switch (ID)
	{
	case LFContextStores:
		AllowAttribute(LFAttrDescription);
		AllowAttribute(LFAttrCreationTime);
		AllowAttribute(LFAttrFileTime);
		AllowAttribute(LFAttrFileCount);
		AllowAttribute(LFAttrFileSize);
		break;
	case LFContextFilters:
		AllowAttribute(LFAttrFileID);
		AllowAttribute(LFAttrCreationTime);
		AllowAttribute(LFAttrFileTime);
		AllowAttribute(LFAttrAddTime);
		AllowAttribute(LFAttrFileSize);
		break;
	default:
		if (ID==LFContextArchive)
			AllowAttribute(LFAttrArchiveTime);
		if (ID==LFContextTrash)
			AllowAttribute(LFAttrDeleteTime);

		for (UINT a=0; a<LFAttributeCount; a++)
			if ((((a!=LFAttrDescription) && (a!=LFAttrFileCount)) || (ID<LFContextSubfolderDefault)) && (a!=LFAttrDescription) && (a!=LFAttrArchiveTime) && (a!=LFAttrDeleteTime))
				AllowAttribute(a);
	}
}


LFCORE_API void LFGetItemCategoryInfo(LFItemCategoryDescriptor& cat, UINT ID)
{
	ZeroMemory(&cat, sizeof(LFItemCategoryDescriptor));
	if (ID>=LFItemCategoryCount)
		return;

	LoadTwoStrings(LFCoreModuleHandle, IDS_ITEMCATEGORY_FIRST+ID, cat.Caption, 256, cat.Hint, 256);
}












LFCORE_API void LFGetAttrCategoryName(WCHAR* pStr, UINT ID)
{
	LoadString(LFCoreModuleHandle, ID+IDS_ATTRCATEGORY_FIRST, pStr, 256);
}

LFCORE_API void LFGetSourceName(WCHAR* pStr, UINT ID, BOOL qualified)
{
	LoadString(LFCoreModuleHandle, ID+(qualified ? IDS_QSRC_FIRST : IDS_SRC_FIRST), pStr, 256);
}

LFCORE_API void LFErrorBox(UINT ID, HWND hWnd)
{
	if (ID>LFCancel)
	{
		WCHAR Caption[256];
		WCHAR msg[256];
		LoadString(LFCoreModuleHandle, IDS_ERRORCAPTION, Caption, 256);
		LFGetErrorText(msg, 256, ID);

		MessageBox(hWnd, msg, Caption, MB_OK | MB_ICONERROR);
	}
}
