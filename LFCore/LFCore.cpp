// LFCore.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//

#include "stdafx.h"
#include "LFCore.h"
#include "liquidFOLDERS.h"
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
#include <winioctl.h>


HMODULE LFCoreModuleHandle;
LFMessageIDs LFMessages;
OSVERSIONINFO osInfo;
extern const unsigned char AttrTypes[];
extern LFShellProperty AttrProperties[];


#pragma data_seg(".shared")

bool VolumeMounted[26] = { false };
unsigned int VolumeTypes[26] = { DRIVE_UNKNOWN };

#pragma data_seg()
#pragma comment(linker, "/SECTION:.shared,RWS")


LFCore_API void LFInitialize()
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


LFCore_API bool LFHideFileExt()
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

LFCore_API bool LFHideDrivesWithNoMedia()
{
	DWORD HideDrivesWithNoMedia = (osInfo.dwMajorVersion<6) ? 0 : 1;

	HKEY k;
	if (RegOpenKeyA(HKEY_CURRENT_USER, "Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced", &k)==ERROR_SUCCESS)
	{
		DWORD sz = sizeof(HideDrivesWithNoMedia);
		RegQueryValueEx(k, L"HideDrivesWithNoMedia", 0, NULL, (BYTE*)&HideDrivesWithNoMedia, &sz);

		RegCloseKey(k);
	}

	return (HideDrivesWithNoMedia!=0);
}


unsigned int GetDriveBus(char cDrive)
{
	unsigned int res = BusTypeMaxReserved;

	char szBuf[MAX_PATH] = "\\\\?\\ :";
	szBuf[4] = cDrive;
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
			res = pDevDesc->BusType;

		delete[] pDevDesc;
		CloseHandle(hDevice);
	}

	return res;
}

LFCore_API unsigned int LFGetSourceForDrive(char cDrive)
{
	if ((cDrive>='A') && (cDrive<='Z'))
		switch (GetDriveBus(cDrive))
		{
		case BusType1394:
			return LFTypeSource1394;
		case BusTypeUsb:
			return LFTypeSourceUSB;
		}

	return LFTypeSourceUnknown;
}

LFCore_API unsigned int LFGetLogicalDrives(unsigned int mask)
{
	DWORD DrivesOnSystem = GetLogicalDrives();
	if ((mask & LFGLD_IncludeFloppies)==0)
		DrivesOnSystem &= ~3;

	DWORD Index = 1;
	char szDriveRoot[] = " :\\";

	for (char cDrive='A'; cDrive<='Z'; cDrive++, Index<<=1)
	{
		if ((DrivesOnSystem & Index)==0)
		{
			VolumeTypes[cDrive-'A'] = DRIVE_UNKNOWN;
			continue;
		}

		unsigned int uDriveType = VolumeTypes[cDrive-'A'];
		if (uDriveType==DRIVE_UNKNOWN)
		{
			szDriveRoot[0] = cDrive;
			uDriveType = GetDriveTypeA(szDriveRoot);

			if (uDriveType==DRIVE_FIXED)
				switch (GetDriveBus(cDrive))
				{
				case BusType1394:
				case BusTypeUsb:
					uDriveType = DRIVE_REMOVABLE;
					break;
				}

			VolumeTypes[cDrive-'A'] = uDriveType;
		}

		switch (uDriveType)
		{
		case DRIVE_FIXED:
			if (!(mask & LFGLD_Internal))
				DrivesOnSystem &= ~Index;
			break;
		case DRIVE_REMOVABLE:
			if (!(mask & LFGLD_External))
				DrivesOnSystem &= ~Index;
			break;
		case DRIVE_REMOTE:
			if (!(mask & LFGLD_Network))
				DrivesOnSystem &= ~Index;
			break;
		default:
			DrivesOnSystem &= ~Index;
		}
	}

	return DrivesOnSystem;
}

LFCore_API LFMessageIDs* LFGetMessageIDs()
{
	return &LFMessages;
}

LFCore_API void LFCreateSendTo(bool force)
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

LFCore_API HICON LFGetIcon(unsigned int ResID, int cx, int cy)
{
	return (HICON)LoadImage(LFCoreModuleHandle, MAKEINTRESOURCE(ResID), IMAGE_ICON, cx, cy, LR_DEFAULTCOLOR);
}

LFCore_API void LFInitProgress(LFProgress* pProgress, HWND hWnd, unsigned int MajorCount)
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

LFCore_API void LFGetAttributeInfo(LFAttributeDescriptor& attr, unsigned int ID)
{
	ZeroMemory(&attr, sizeof(LFAttributeDescriptor));
	if (ID>=LFAttributeCount)
		return;

	LoadString(LFCoreModuleHandle, ID+IDS_FirstAttribute, attr.Name, 256);
	attr.AlwaysVisible = (ID==LFAttrFileName);
	attr.Type = AttrTypes[ID];

	wchar_t tmpBuf[256];
	wchar_t* ptrSrc = tmpBuf;
	wchar_t* ptrDst = attr.XMLID;
	LoadStringEnglish(LFCoreModuleHandle, ID+IDS_FirstAttribute, tmpBuf, 256);

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


LFCore_API LFContextDescriptor* LFAllocContextDescriptor()
{
	LFContextDescriptor* c = new LFContextDescriptor;
	return c;
}

LFCore_API void LFGetContextInfo(LFContextDescriptor& ctx, unsigned int ID)
{
	ZeroMemory(&ctx, sizeof(LFContextDescriptor)-sizeof(LFAllowedAttributes));
	if (ID>=LFContextCount)
		return;

	LoadTwoStrings(LFCoreModuleHandle, IDS_FirstContext+ID, ctx.Name, 256, ctx.Comment, 256);

	ctx.AllowGroups = (ID<=LFLastGroupContext) || (ID==LFContextSearch);

	ctx.AllowedAttributes += LFAttrFileName;
	ctx.AllowedAttributes += LFAttrStoreID;
	ctx.AllowedAttributes += LFAttrComments;

	switch (ID)
	{
	case LFContextStores:
		ctx.AllowedAttributes += LFAttrDescription;
		ctx.AllowedAttributes += LFAttrCreationTime;
		ctx.AllowedAttributes += LFAttrFileTime;
		ctx.AllowedAttributes += LFAttrFileCount;
		ctx.AllowedAttributes += LFAttrFileSize;
		break;
	case LFContextFilters:
		ctx.AllowedAttributes += LFAttrFileID;
		ctx.AllowedAttributes += LFAttrCreationTime;
		ctx.AllowedAttributes += LFAttrFileTime;
		ctx.AllowedAttributes += LFAttrAddTime;
		ctx.AllowedAttributes += LFAttrFileSize;
		break;
	default:
		if (ID==LFContextArchive)
			ctx.AllowedAttributes += LFAttrArchiveTime;
		if (ID==LFContextTrash)
			ctx.AllowedAttributes += LFAttrDeleteTime;

		for (unsigned int a=0; a<LFAttributeCount; a++)
			if ((((a!=LFAttrDescription) && (a!=LFAttrFileCount)) || (ID<LFContextSubfolderDefault)) && (a!=LFAttrDescription) && (a!=LFAttrArchiveTime) && (a!=LFAttrDeleteTime))
				ctx.AllowedAttributes += a;
	}
}


LFCore_API void LFGetItemCategoryInfo(LFItemCategoryDescriptor& cat, unsigned int ID)
{
	ZeroMemory(&cat, sizeof(LFItemCategoryDescriptor));
	if (ID>=LFItemCategoryCount)
		return;

	LoadTwoStrings(LFCoreModuleHandle, IDS_FirstItemCategory+ID, cat.Caption, 256, cat.Hint, 256);
}


LFCore_API LFFilter* LFAllocFilter(LFFilter* f)
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

LFCore_API void LFFreeFilter(LFFilter* f)
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

LFCore_API LFFilterCondition* LFAllocFilterCondition()
{
	LFFilterCondition* c = new LFFilterCondition();
	ZeroMemory(c, sizeof(LFFilterCondition));

	return c;
}

LFCore_API void LFFreeFilterCondition(LFFilterCondition* c)
{
	if (c)
		delete c;
}


LFCore_API LFSearchResult* LFAllocSearchResult(int ctx, LFSearchResult* res)
{
	return (res) ? new LFSearchResult(res) : new LFSearchResult(ctx);
}

LFCore_API void LFFreeSearchResult(LFSearchResult* res)
{
	if (res)
		delete res;
}

LFCore_API bool LFAddItemDescriptor(LFSearchResult* res, LFItemDescriptor* i)
{
	return res->AddItemDescriptor(i);
}

LFCore_API void LFRemoveItemDescriptor(LFSearchResult* res, unsigned int idx)
{
	res->RemoveItemDescriptor(idx);
}

LFCore_API void LFRemoveFlaggedItemDescriptors(LFSearchResult* res)
{
	res->RemoveFlaggedItemDescriptors();
}

LFCore_API void LFSortSearchResult(LFSearchResult* res, unsigned int attr, bool descending)
{
	res->Sort(attr, descending);
}

LFCore_API LFSearchResult* LFGroupSearchResult(LFSearchResult* res, unsigned int attr, bool descending, bool groupone, LFFilter* f)
{
	assert(f);

	if (f->Options.IsSubfolder)
	{
		res->Sort(attr, descending);
		return res;
	}

	// Special treatment for string arrays
	if (AttrTypes[attr]==LFTypeUnicodeArray)
	{
		LFSearchResult* cooked = new LFSearchResult(res);
		cooked->GroupArray(attr, f);
		cooked->Sort(attr, descending);
		return cooked;
	}

	// Special treatment for missing GPS location
	if (attr==LFAttrLocationGPS)
		for (unsigned int a=0; a<res->m_ItemCount; a++)
			if (IsNullValue(LFAttrLocationGPS, res->m_Items[a]->AttributeValues[LFAttrLocationGPS]))
			{
				LFAirport* airport;
				if (LFIATAGetAirportByCode((char*)res->m_Items[a]->AttributeValues[LFAttrLocationIATA], &airport))
					res->m_Items[a]->AttributeValues[LFAttrLocationGPS] = &airport->Location;
			}

	res->Sort(attr, descending);
	LFSearchResult* cooked = new LFSearchResult(res);
	cooked->Group(attr, groupone, f);

	// Revert to old GPS location
	if (attr==LFAttrLocationGPS)
		for (unsigned int a=0; a<res->m_ItemCount; a++)
			res->m_Items[a]->AttributeValues[LFAttrLocationGPS] = &res->m_Items[a]->CoreAttributes.LocationGPS;

	return cooked;
}


LFCore_API LFFileIDList* LFAllocFileIDList()
{
	return new LFFileIDList();
}

LFCore_API LFFileIDList* LFAllocFileIDList(HLIQUID hLiquid)
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

LFCore_API void LFFreeFileIDList(LFFileIDList* il)
{
	if (il)
		delete il;
}

LFCore_API bool LFAddFileID(LFFileIDList* il, char* StoreID, char* FileID, void* UserData)
{
	return il->AddFileID(StoreID, FileID, UserData);
}

LFCore_API HGLOBAL LFCreateLiquidFiles(LFFileIDList* il)
{
	return il->CreateLiquidFiles();
}


LFCore_API LFFileImportList* LFAllocFileImportList()
{
	return new LFFileImportList();
}

LFCore_API LFFileImportList* LFAllocFileImportList(HDROP hDrop)
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

LFCore_API void LFFreeFileImportList(LFFileImportList* il)
{
	if (il)
		delete il;
}

LFCore_API bool LFAddImportPath(LFFileImportList* il, wchar_t* path)
{
	return il->AddPath(path);
}


LFCore_API LFMaintenanceList* LFAllocMaintenanceList()
{
	return new LFMaintenanceList();
}

LFCore_API void LFFreeMaintenanceList(LFMaintenanceList* ml)
{
	if (ml)
		delete ml;
}


LFCore_API LFTransactionList* LFAllocTransactionList()
{
	return new LFTransactionList();
}

LFCore_API void LFFreeTransactionList(LFTransactionList* tl)
{
	if (tl)
		delete tl;
}

LFCore_API bool LFAddItemDescriptor(LFTransactionList* tl, LFItemDescriptor* i, unsigned int UserData)
{
	return tl->AddItemDescriptor(i, UserData);
}

LFCore_API LPITEMIDLIST LFDetachPIDL(LFTransactionList* tl, unsigned int idx)
{
	return tl->DetachPIDL(idx);
}

LFCore_API HGLOBAL LFCreateDropFiles(LFTransactionList* tl)
{
	return tl->CreateDropFiles();
}

LFCore_API HGLOBAL LFCreateLiquidFiles(LFTransactionList* tl)
{
	return tl->CreateLiquidFiles();
}


LFCore_API void LFGetAttrCategoryName(wchar_t* pStr, unsigned int ID)
{
	LoadString(LFCoreModuleHandle, ID+IDS_FirstAttrCategory, pStr, 256);
}

LFCore_API void LFGetSourceName(wchar_t* pStr, unsigned int ID, bool qualified)
{
	LoadString(LFCoreModuleHandle, ID+(qualified ? IDS_FirstQualifiedSource : IDS_FirstSource), pStr, 256);
}

LFCore_API void LFGetErrorText(wchar_t* pStr, unsigned int ID)
{
	LoadString(LFCoreModuleHandle, ID+IDS_FirstError, pStr, 256);
}

LFCore_API void LFErrorBox(unsigned int ID, HWND hWnd)
{
	if (ID>LFCancel)
	{
		wchar_t caption[256];
		wchar_t msg[256];
		LoadString(LFCoreModuleHandle, IDS_ErrorCaption, caption, 256);
		LFGetErrorText(msg, ID);

		MessageBox(hWnd, msg, caption, MB_OK | MB_ICONERROR);
	}
}
