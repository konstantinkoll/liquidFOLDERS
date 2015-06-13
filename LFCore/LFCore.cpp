// LFCore.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//

#include "stdafx.h"
#include "LFCore.h"
#include "LF.h"
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
LFMessageIDs LFMessages;
OSVERSIONINFO osInfo;
extern const unsigned char AttrTypes[];
extern LFShellProperty AttrProperties[];


#pragma data_seg(".shared")

unsigned int VolumeTypes[26] = { DRIVE_UNKNOWN };

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


LFCORE_API void LFCombineFileCountSize(unsigned int count, __int64 size, wchar_t* str, size_t cCount)
{
	wchar_t tmpStr[256];
	StrFormatByteSize(size, tmpStr, 256);

	wchar_t tmpMask[256];
	LoadString(LFCoreModuleHandle, count==1 ? IDS_FILECOUNT_SINGULAR : IDS_FILECOUNT_PLURAL, tmpMask, 256);

	swprintf_s(str, cCount, tmpMask, count, tmpStr);
}


void LoadStringEnglish(HINSTANCE hInstance, unsigned int uID, wchar_t* lpBuffer, int cchBufferMax)
{
	DWORD nID = (uID>>4)+1;
	DWORD nItemID = uID & 0x000F;

	HRSRC hRes = FindResourceEx(hInstance, RT_STRING, MAKEINTRESOURCE(nID), MAKELANGID(LANG_ENGLISH, SUBLANG_NEUTRAL));
	if (hRes)
	{
		HGLOBAL hGlobal = LoadResource(hInstance, hRes);
		LPCWSTR lpStr = (LPCWSTR)LockResource(hGlobal);

		unsigned int nStr = 0;
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

void LoadTwoStrings(HINSTANCE hInstance, unsigned int uID, wchar_t* lpBuffer1, int cchBufferMax1, wchar_t* lpBuffer2, int cchBufferMax2)
{
	assert(lpBuffer1);
	assert(lpBuffer2);

	wchar_t tmpStr[256];
	LoadString(hInstance, uID, tmpStr, 256);

	wchar_t* brk = wcschr(tmpStr, L'\n');
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


LFCORE_API bool LFHideFileExt()
{
	DWORD HideFileExt = 0;

	HKEY k;
	if (RegOpenKeyA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", &k)==ERROR_SUCCESS)
	{
		DWORD sz = sizeof(HideFileExt);
		RegQueryValueEx(k, L"HideFileExt", 0, NULL, (BYTE*)&HideFileExt, &sz);

		RegCloseKey(k);
	}

	return (HideFileExt!=0);
}

LFCORE_API bool LFHideVolumesWithNoMedia()
{
	DWORD HideVolumesWithNoMedia = (osInfo.dwMajorVersion<6) ? 0 : 1;

	HKEY k;
	if (RegOpenKeyA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", &k)==ERROR_SUCCESS)
	{
		DWORD sz = sizeof(HideVolumesWithNoMedia);
		RegQueryValueEx(k, L"HideVolumesWithNoMedia", 0, NULL, (BYTE*)&HideVolumesWithNoMedia, &sz);

		RegCloseKey(k);
	}

	return (HideVolumesWithNoMedia!=0);
}


unsigned int GetVolumeBus(char cVolume)
{
	unsigned int Result = BusTypeMaxReserved;

	char szBuf[MAX_PATH] = "\\\\?\\ :";
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

LFCORE_API unsigned int LFGetSourceForVolume(char cVolume)
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

LFCORE_API unsigned int LFGetLogicalVolumes(unsigned int Mask)
{
	DWORD VolumesOnSystem = GetLogicalDrives();
	if ((Mask & LFGLV_IncludeFloppies)==0)
		VolumesOnSystem &= ~3;

	DWORD Index = 1;
	char szVolumeRoot[] = " :\\";

	for (char cVolume='A'; cVolume<='Z'; cVolume++, Index<<=1)
	{
		if ((VolumesOnSystem & Index)==0)
		{
			VolumeTypes[cVolume-'A'] = DRIVE_UNKNOWN;
			continue;
		}

		unsigned int uVolumeType = VolumeTypes[cVolume-'A'];
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

LFCORE_API LFMessageIDs* LFGetMessageIDs()
{
	return &LFMessages;
}

LFCORE_API void LFCreateSendTo(bool force)
{
	HKEY k;
	if (RegCreateKeyA(HKEY_CURRENT_USER, "Software\\liquidFOLDERS", &k)==ERROR_SUCCESS)
	{
		BOOL created = false;

		DWORD type;
		DWORD sz = sizeof(created);
		if (RegQueryValueExA(k, "SendToCreated", NULL, &type, (BYTE*)created, &sz)==ERROR_SUCCESS)
		{
			force |= (created==FALSE);
		}
		else
		{
			force = true;
		}

		created = TRUE;
		RegSetValueExA(k, "SendToCreated", 0, REG_DWORD, (BYTE*)&created, sizeof(created));
		RegCloseKey(k);
	}

	if (force)
	{
		wchar_t Path[MAX_PATH];
		if (SHGetSpecialFolderPath(NULL, Path, CSIDL_SENDTO, TRUE))
		{
			wchar_t Name[256];
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

LFCORE_API void LFInitProgress(LFProgress* pProgress, HWND hWnd, unsigned int MajorCount)
{
	assert(pProgress);

	ZeroMemory(pProgress, sizeof(LFProgress));
	pProgress->hWnd = hWnd;
	pProgress->ProgressState = LFProgressWorking;
	pProgress->MajorCount = MajorCount;
}


__forceinline void SetRange(unsigned char &var, unsigned int ID, unsigned int lo, unsigned int hi, unsigned char val)
{
	if ((ID>=lo) && (ID<=hi))
		var = val;
}

LFCORE_API void LFGetAttributeInfo(LFAttributeDescriptor& attr, unsigned int ID)
{
	ZeroMemory(&attr, sizeof(LFAttributeDescriptor));
	if (ID>=LFAttributeCount)
		return;

	LoadString(LFCoreModuleHandle, ID+IDS_ATTR_FIRST, attr.Name, 256);
	attr.AlwaysVisible = (ID==LFAttrFileName);
	attr.Type = AttrTypes[ID];

	wchar_t tmpBuf[256];
	wchar_t* ptrSrc = tmpBuf;
	wchar_t* ptrDst = attr.XMLID;
	LoadStringEnglish(LFCoreModuleHandle, ID+IDS_ATTR_FIRST, tmpBuf, 256);

	do
	{
		wchar_t ch = (wchar_t)towlower(*ptrSrc);
		if ((ch>=L'a') && (ch<=L'z'))
			*(ptrDst++) = ch;
	}
	while (*(ptrSrc++)!=L'\0');
	*ptrDst = L'\0';

	// Type and character count (where appropriate)
	switch (attr.Type)
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
	case LFTypeAnsiString:
		attr.cCharacters = (unsigned int)GetAttributeMaxCharacterCount(ID);
		break;
	case LFTypeFourCC:
		attr.cCharacters = 4;
		break;
	}

	// Recommended width
	const unsigned int rWidths[LFTypeCount] = { 200, 200, 200, 100, 100, 100, 120, 100, 100, 100, 150, 140, 100 };
	attr.RecommendedWidth = (ID==LFAttrComments) ? 350 : rWidths[attr.Type];

	// Category
	if (ID<=LFAttrRating)
	{
		attr.Category = ((ID==LFAttrStoreID) || (ID==LFAttrFileID) || (ID==LFAttrAddTime) || (ID==LFAttrDeleteTime) || (ID==LFAttrFileFormat) || (ID==LFAttrFlags)) ? LFAttrCategoryInternal : LFAttrCategoryBasic;
	}
	else
	{
		SetRange(attr.Category, ID, LFAttrLocationName, LFAttrLocationGPS, LFAttrCategoryGeotags);
		SetRange(attr.Category, ID, LFAttrWidth, LFAttrRoll, LFAttrCategoryVisual);
		SetRange(attr.Category, ID, LFAttrExposure, LFAttrChip, LFAttrCategoryPhotographic);
		SetRange(attr.Category, ID, LFAttrAlbum, LFAttrAudioCodec, LFAttrCategoryAudio);
		SetRange(attr.Category, ID, LFAttrDuration, LFAttrBitrate, LFAttrCategoryTimebased);
		SetRange(attr.Category, ID, LFAttrArtist, LFAttrSignature, LFAttrCategoryBibliographic);
		SetRange(attr.Category, ID, LFAttrFrom, LFAttrCustomer, LFAttrCategoryWorkflow);
		SetRange(attr.Category, ID, LFAttrPriority, LFAttrPriority, LFAttrCategoryWorkflow);
	}

	// Sorting
	attr.Sortable = (attr.Type!=LFTypeFlags);
	attr.PreferDescendingSort = (attr.Type==LFTypeRating) || (attr.Type==LFTypeTime) || (attr.Type==LFTypeMegapixel);

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
	case LFAttrRecordingEquipment:
	case LFAttrFrom:
	case LFAttrTo:
	case LFAttrLikeCount:
		attr.ReadOnly = true;
		break;
	default:
		attr.ReadOnly = (attr.Category==LFAttrCategoryInternal);
	}

	// Format
	attr.FormatRight = (((attr.Type>=LFTypeUINT) && (attr.Type!=LFTypeTime)) || (ID==LFAttrStoreID) || (ID==LFAttrFileID));

	// Shell property
	attr.ShPropertyMapping = AttrProperties[ID];
	if (!attr.ShPropertyMapping.ID)
	{
		attr.ShPropertyMapping.Schema = PropertyLF;
		attr.ShPropertyMapping.ID = ID;
	}
}


LFCORE_API void LFGetContextInfo(LFContextDescriptor& ctx, unsigned int ID)
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

		for (unsigned int a=0; a<LFAttributeCount; a++)
			if ((((a!=LFAttrDescription) && (a!=LFAttrFileCount)) || (ID<LFContextSubfolderDefault)) && (a!=LFAttrDescription) && (a!=LFAttrArchiveTime) && (a!=LFAttrDeleteTime))
				AllowAttribute(a);
	}
}


LFCORE_API void LFGetItemCategoryInfo(LFItemCategoryDescriptor& cat, unsigned int ID)
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
			LFFilterCondition* item = LFAllocFilterCondition();
			*item = *c;
			item->Next = fn->ConditionList;
			fn->ConditionList = item;

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

LFCORE_API LFFilterCondition* LFAllocFilterCondition()
{
	LFFilterCondition* c = new LFFilterCondition();
	ZeroMemory(c, sizeof(LFFilterCondition));

	return c;
}

LFCORE_API void LFFreeFilterCondition(LFFilterCondition* c)
{
	if (c)
		delete c;
}


LFCORE_API LFSearchResult* LFAllocSearchResult(int ctx, LFSearchResult* Result)
{
	return Result ? new LFSearchResult(Result) : new LFSearchResult(ctx);
}

LFCORE_API void LFFreeSearchResult(LFSearchResult* Result)
{
	if (Result)
		delete Result;
}

LFCORE_API bool LFAddItemDescriptor(LFSearchResult* Result, LFItemDescriptor* i)
{
	return Result->AddItemDescriptor(i);
}

LFCORE_API void LFRemoveItemDescriptor(LFSearchResult* Result, unsigned int idx)
{
	Result->RemoveItemDescriptor(idx);
}

LFCORE_API void LFRemoveFlaggedItemDescriptors(LFSearchResult* Result)
{
	Result->RemoveFlaggedItemDescriptors();
}

LFCORE_API void LFSortSearchResult(LFSearchResult* Result, unsigned int attr, bool descending)
{
	Result->Sort(attr, descending);
}

LFCORE_API LFSearchResult* LFGroupSearchResult(LFSearchResult* Result, unsigned int attr, bool descending, bool groupone, LFFilter* f)
{
	assert(f);

	if (f->Options.IsSubfolder)
	{
		Result->Sort(attr, descending);
		return Result;
	}

	// Special treatment for string arrays
	if (AttrTypes[attr]==LFTypeUnicodeArray)
	{
		LFSearchResult* cooked = new LFSearchResult(Result);
		cooked->GroupArray(attr, f);
		cooked->Sort(attr, descending);
		return cooked;
	}

	// Special treatment for missing GPS location
	if (attr==LFAttrLocationGPS)
		for (unsigned int a=0; a<Result->m_ItemCount; a++)
			if (IsNullValue(LFAttrLocationGPS, Result->m_Items[a]->AttributeValues[LFAttrLocationGPS]))
			{
				LFAirport* airport;
				if (LFIATAGetAirportByCode((char*)Result->m_Items[a]->AttributeValues[LFAttrLocationIATA], &airport))
					Result->m_Items[a]->AttributeValues[LFAttrLocationGPS] = &airport->Location;
			}

	Result->Sort(attr, descending);
	LFSearchResult* cooked = new LFSearchResult(Result);
	cooked->Group(attr, groupone, f);

	// Revert to old GPS location
	if (attr==LFAttrLocationGPS)
		for (unsigned int a=0; a<Result->m_ItemCount; a++)
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
			unsigned int cFiles = pFiles->cFiles;
			char* ptr = (char*)(((unsigned char*)pFiles)+sizeof(LIQUIDFILES));

			for (unsigned int a=0; a<cFiles; a++)
			{
				char StoreID[LFKeySize];
				strcpy_s(StoreID, LFKeySize, ptr);
				ptr += LFKeySize;

				char FileID[LFKeySize];
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
	if (il)
		delete il;
}

LFCORE_API bool LFAddFileID(LFFileIDList* il, char* StoreID, char* FileID, void* UserData)
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
		unsigned int NumFiles = DragQueryFile(hDrop, (UINT)-1, NULL, 0);
		wchar_t FileName[2*MAX_PATH];

		for (unsigned int a=0; a<NumFiles; a++)
			if (DragQueryFile(hDrop, a, FileName, 2*MAX_PATH))
				il->AddPath(FileName);
	}

	return il;
}

LFCORE_API void LFFreeFileImportList(LFFileImportList* il)
{
	if (il)
		delete il;
}

LFCORE_API bool LFAddImportPath(LFFileImportList* il, wchar_t* path)
{
	return il->AddPath(path);
}


LFCORE_API LFMaintenanceList* LFAllocMaintenanceList()
{
	return new LFMaintenanceList();
}

LFCORE_API void LFFreeMaintenanceList(LFMaintenanceList* ml)
{
	if (ml)
		delete ml;
}


LFCORE_API LFTransactionList* LFAllocTransactionList()
{
	return new LFTransactionList();
}

LFCORE_API void LFFreeTransactionList(LFTransactionList* tl)
{
	if (tl)
		delete tl;
}

LFCORE_API bool LFAddItemDescriptor(LFTransactionList* tl, LFItemDescriptor* i, unsigned int UserData)
{
	return tl->AddItemDescriptor(i, UserData);
}

LFCORE_API LPITEMIDLIST LFDetachPIDL(LFTransactionList* tl, unsigned int idx)
{
	return tl->DetachPIDL(idx);
}

LFCORE_API HGLOBAL LFCreateDropFiles(LFTransactionList* tl)
{
	return tl->CreateDropFiles();
}

LFCORE_API HGLOBAL LFCreateLiquidFiles(LFTransactionList* tl)
{
	return tl->CreateLiquidFiles();
}


LFCORE_API void LFGetAttrCategoryName(wchar_t* pStr, unsigned int ID)
{
	LoadString(LFCoreModuleHandle, ID+IDS_ATTRCATEGORY_FIRST, pStr, 256);
}

LFCORE_API void LFGetSourceName(wchar_t* pStr, unsigned int ID, bool qualified)
{
	LoadString(LFCoreModuleHandle, ID+(qualified ? IDS_QSRC_FIRST : IDS_SRC_FIRST), pStr, 256);
}

LFCORE_API void LFGetErrorText(wchar_t* pStr, unsigned int ID)
{
	LoadString(LFCoreModuleHandle, ID+IDS_ERR_FIRST, pStr, 256);
}

LFCORE_API void LFErrorBox(unsigned int ID, HWND hWnd)
{
	if (ID>LFCancel)
	{
		wchar_t Caption[256];
		wchar_t msg[256];
		LoadString(LFCoreModuleHandle, IDS_ERRORCAPTION, Caption, 256);
		LFGetErrorText(msg, ID);

		MessageBox(hWnd, msg, Caption, MB_OK | MB_ICONERROR);
	}
}
