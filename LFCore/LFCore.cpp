
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





UINT GetVolumeBus(CHAR cVolume)
{
	UINT Result = BusTypeMaxReserved;

	CHAR szBuf[MAX_PATH] = "\\\\?\\ :";
	szBuf[4] = cVolume;
	HANDLE hDevice = CreateFileA(szBuf, 0, 0, NULL, OPEN_EXISTING, NULL, NULL);

	if (hDevice!=INVALID_HANDLE_VALUE)
	{
		STORAGE_ADAPTER_DESCRIPTOR* pDevDesc = (STORAGE_ADAPTER_DESCRIPTOR*)new BYTE[sizeof(STORAGE_ADAPTER_DESCRIPTOR)];
		pDevDesc->Size = sizeof(STORAGE_ADAPTER_DESCRIPTOR);

		STORAGE_PROPERTY_QUERY Query;
		Query.PropertyId = StorageAdapterProperty;
		Query.QueryType = PropertyStandardQuery;

		DWORD dwOutBytes;
		if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
			&Query, sizeof(STORAGE_PROPERTY_QUERY), pDevDesc, pDevDesc->Size,
			&dwOutBytes, NULL))
			Result = pDevDesc->BusType;

		delete[] pDevDesc;
		CloseHandle(hDevice);
	}

	return Result;
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
	if ((Mask & LFGLV_IncludeFloppies)==0)
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
			if (!(Mask & LFGLV_Internal))
				VolumesOnSystem &= ~Index;
			break;
		case DRIVE_REMOVABLE:
			if (!(Mask & LFGLV_External))
				VolumesOnSystem &= ~Index;
			break;
		case DRIVE_REMOTE:
			if (!(Mask & LFGLV_Network))
				VolumesOnSystem &= ~Index;
			break;
		default:
			VolumesOnSystem &= ~Index;
		}
	}

	return VolumesOnSystem;
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

LFCORE_API void LFInitProgress(LFProgress* pProgress, HWND hWnd, UINT MajorCount)
{
	assert(pProgress);

	ZeroMemory(pProgress, sizeof(LFProgress));
	pProgress->hWnd = hWnd;
	pProgress->ProgressState = LFProgressWorking;
	pProgress->MajorCount = MajorCount;
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
	Attr.Sortable = (Attr.Type!=LFTypeFlags) && (Attr.Type!=LFTypeGeoCoordinates);
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
		Attr.ShPropertyMapping.Schema = PropertyLF;
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


LFCORE_API LFFilter* LFAllocFilter(LFFilter* f)
{
	LFFilter* fn = new LFFilter;
	if (f)
	{
		*fn = *f;
		fn->ConditionList = NULL;

		LFFilterCondition* c = f->ConditionList;
		while (c)
		{
			fn->ConditionList = LFAllocFilterCondition(c->Compare, c->AttrData, fn->ConditionList);

			c = c->Next;
		}
	}
	else
	{
		ZeroMemory(fn, sizeof(LFFilter));
		fn->Mode = LFFilterModeStores;
	}

	return fn;
}

LFCORE_API void LFFreeFilter(LFFilter* f)
{
	if (f)
	{
		LFFilterCondition* c = f->ConditionList;
		while (c)
		{
			LFFilterCondition* victim = c;
			c = c->Next;
			LFFreeFilterCondition(victim);
		}

		delete f;
	}
}


LFCORE_API LFFilterCondition* LFAllocFilterCondition(BYTE Compare, LFVariantData& v, LFFilterCondition* pNext)
{
	LFFilterCondition* c = new LFFilterCondition();

	c->Next = pNext;
	c->AttrData = v;
	c->Compare = Compare;

	return c;
}

LFCORE_API LFFilterCondition* LFAllocFilterConditionEx(BYTE Compare, UINT Attr, LFFilterCondition* pNext)
{
	LFFilterCondition* c = new LFFilterCondition();

	c->Next = pNext;
	c->AttrData.Attr = Attr;
	c->AttrData.Type = (Attr<LFAttributeCount) ? AttrTypes[Attr] : LFTypeUnicodeString;
	c->AttrData.IsNull = FALSE;
	c->Compare = Compare;

	return c;
}


LFCORE_API LFSearchResult* LFAllocSearchResult(INT ctx)
{
	return new LFSearchResult(ctx);
}

LFCORE_API void LFFreeSearchResult(LFSearchResult* Result)
{
	delete Result;
}

LFCORE_API BOOL LFAddItemDescriptor(LFSearchResult* Result, LFItemDescriptor* i)
{
	return Result->AddItemDescriptor(i);
}

LFCORE_API void LFRemoveFlaggedItemDescriptors(LFSearchResult* Result)
{
	Result->RemoveFlaggedItemDescriptors();
}

LFCORE_API void LFSortSearchResult(LFSearchResult* Result, UINT Attr, BOOL descending)
{
	Result->Sort(Attr, descending);
}

LFCORE_API LFSearchResult* LFGroupSearchResult(LFSearchResult* Result, UINT Attr, BOOL descending, BOOL groupone, LFFilter* f)
{
	assert(f);

	if (f->Options.IsSubfolder)
	{
		Result->Sort(Attr, descending);
		return Result;
	}

	// Special treatment for string arrays
	if (AttrTypes[Attr]==LFTypeUnicodeArray)
	{
		LFSearchResult* cooked = new LFSearchResult(Result);
		cooked->GroupArray(Attr, f);
		cooked->Sort(Attr, descending);
		return cooked;
	}

	// Special treatment for missing GPS location
	if (Attr==LFAttrLocationGPS)
		for (UINT a=0; a<Result->m_ItemCount; a++)
			if (IsNullValue(AttrTypes[LFAttrLocationGPS], Result->m_Items[a]->AttributeValues[LFAttrLocationGPS]))
			{
				LFAirport* airport;
				if (LFIATAGetAirportByCode((CHAR*)Result->m_Items[a]->AttributeValues[LFAttrLocationIATA], &airport))
					Result->m_Items[a]->AttributeValues[LFAttrLocationGPS] = &airport->Location;
			}

	Result->Sort(Attr, descending);
	LFSearchResult* cooked = new LFSearchResult(Result);
	cooked->Group(Attr, groupone, f);

	// Revert to old GPS location
	if (Attr==LFAttrLocationGPS)
		for (UINT a=0; a<Result->m_ItemCount; a++)
			Result->m_Items[a]->AttributeValues[LFAttrLocationGPS] = &Result->m_Items[a]->CoreAttributes.LocationGPS;

	return cooked;
}


LFCORE_API LFFileIDList* LFAllocFileIDList()
{
	return new LFFileIDList();
}

LFCORE_API LFFileIDList* LFAllocFileIDList(HLIQUID hLiquid)
{
	LFFileIDList* il = new LFFileIDList();

	if (hLiquid)
	{
		LIQUIDFILES* pFiles = (LIQUIDFILES*)GlobalLock(hLiquid);
		if (pFiles)
		{
			UINT cFiles = pFiles->cFiles;
			CHAR* ptr = (CHAR*)(((BYTE*)pFiles)+sizeof(LIQUIDFILES));

			for (UINT a=0; a<cFiles; a++)
			{
				CHAR StoreID[LFKeySize];
				strcpy_s(StoreID, LFKeySize, ptr);
				ptr += LFKeySize;

				CHAR FileID[LFKeySize];
				strcpy_s(FileID, LFKeySize, ptr);
				ptr += LFKeySize;

				il->AddFileID(StoreID, FileID);
			}
		}

		GlobalUnlock(hLiquid);
	}

	return il;
}

LFCORE_API void LFFreeFileIDList(LFFileIDList* il)
{
	delete il;
}

LFCORE_API BOOL LFAddFileID(LFFileIDList* il, CHAR* StoreID, CHAR* FileID, void* UserData)
{
	return il->AddFileID(StoreID, FileID, UserData);
}

LFCORE_API HGLOBAL LFCreateLiquidFiles(LFFileIDList* il)
{
	return il->CreateLiquidFiles();
}


LFCORE_API LFFileImportList* LFAllocFileImportList()
{
	return new LFFileImportList();
}

LFCORE_API LFFileImportList* LFAllocFileImportList(HDROP hDrop)
{
	LFFileImportList* il = new LFFileImportList();

	if (hDrop)
	{
		UINT NumFiles = DragQueryFile(hDrop, (UINT32)-1, NULL, 0);
		WCHAR FileName[2*MAX_PATH];

		for (UINT a=0; a<NumFiles; a++)
			if (DragQueryFile(hDrop, a, FileName, 2*MAX_PATH))
				il->AddPath(FileName);
	}

	return il;
}

LFCORE_API void LFFreeFileImportList(LFFileImportList* il)
{
	delete il;
}

LFCORE_API BOOL LFAddImportPath(LFFileImportList* il, WCHAR* path)
{
	return il->AddPath(path);
}


LFCORE_API void LFFreeMaintenanceList(LFMaintenanceList* ml)
{
	delete ml;
}


LFCORE_API LFTransactionList* LFAllocTransactionList()
{
	return new LFTransactionList();
}

LFCORE_API void LFFreeTransactionList(LFTransactionList* tl)
{
	delete tl;
}

LFCORE_API BOOL LFAddItemDescriptor(LFTransactionList* tl, LFItemDescriptor* i, UINT UserData)
{
	return tl->AddItemDescriptor(i, UserData);
}

LFCORE_API HGLOBAL LFCreateDropFiles(LFTransactionList* tl)
{
	return tl->CreateDropFiles();
}

LFCORE_API HGLOBAL LFCreateLiquidFiles(LFTransactionList* tl)
{
	return tl->CreateLiquidFiles();
}


LFCORE_API void LFGetAttrCategoryName(WCHAR* pStr, UINT ID)
{
	LoadString(LFCoreModuleHandle, ID+IDS_ATTRCATEGORY_FIRST, pStr, 256);
}

LFCORE_API void LFGetSourceName(WCHAR* pStr, UINT ID, BOOL qualified)
{
	LoadString(LFCoreModuleHandle, ID+(qualified ? IDS_QSRC_FIRST : IDS_SRC_FIRST), pStr, 256);
}

LFCORE_API void LFGetErrorText(WCHAR* pStr, UINT ID)
{
	LoadString(LFCoreModuleHandle, ID+IDS_ERR_FIRST, pStr, 256);
}

LFCORE_API void LFErrorBox(UINT ID, HWND hWnd)
{
	if (ID>LFCancel)
	{
		WCHAR Caption[256];
		WCHAR msg[256];
		LoadString(LFCoreModuleHandle, IDS_ERRORCAPTION, Caption, 256);
		LFGetErrorText(msg, ID);

		MessageBox(hWnd, msg, Caption, MB_OK | MB_ICONERROR);
	}
}
