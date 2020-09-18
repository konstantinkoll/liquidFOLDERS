
// LFCore.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//

#include "stdafx.h"
#include "FileProperties.h"
#include "FileSystem.h"
#include "IATA.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "LFMemorySort.h"
#include "LFVariantData.h"
#include "resource.h"
#include "Stores.h"
#include "TableAttributes.h"
#include "Thumbnails.h"
#include "Volumes.h"
#include "Watchdog.h"
#include <shlwapi.h>
#include <winioctl.h>


HMODULE LFCoreModuleHandle;
OSVERSIONINFO osInfo;


#pragma data_seg(".shared")

LFMessageIDs LFMessages;

const COLORREF ItemColorFade[LFItemColorFadeCount] = { 0x00000000, 0x808080, 0xC0C0C0, 0xE0E0E0 };
const COLORREF ItemColors[LFItemColorCount] = {
	LFItemColorDefault, LFItemColorRed, LFItemColorOrange, LFItemColorYellow,
	LFItemColorGreen, LFItemColorBlue, LFItemColorPurple, LFItemColorGray
};

WCHAR ItemColorNames[LFItemColorCount-1][256];

#pragma data_seg()
#pragma comment(linker, "/SECTION:.shared,RWS")


#pragma comment(lib, "shlwapi.lib")


LFCORE_API void LFInitialize()
{
	// OS version
	ZeroMemory(&osInfo, sizeof(OSVERSIONINFO));
	osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osInfo);

	// Messages
	LFMessages.UpdateProgress = RegisterWindowMessageA("liquidFOLDERS.UpdateProgress");
	LFMessages.ItemsDropped = RegisterWindowMessageA("liquidFOLDERS.ItemsDropped");
	LFMessages.StoresChanged = RegisterWindowMessageA("liquidFOLDERS.StoresChanged");
	LFMessages.StoreAttributesChanged = RegisterWindowMessageA("liquidFOLDERS.StoreAttributesChanged");
	LFMessages.DefaultStoreChanged = RegisterWindowMessageA("liquidFOLDERS.DefaultStoreChanged");
	LFMessages.StatisticsChanged = RegisterWindowMessageA("liquidFOLDERS.StatisticsChanged");

	// Color names
	for (UINT a=0; a<LFItemColorCount-1; a++)
		LoadString(LFCoreModuleHandle, IDS_COLOR1+a, ItemColorNames[a], 256);

	// Other modules
	InitMutex();
	InitAirportDatabase();
	InitThumbnails();
	InitVolumes();
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

LFCORE_API void LFGetFileSummary(LPWSTR pStr, SIZE_T cCount, UINT Count, INT64 Size)
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

LFCORE_API void LFGetFileSummaryEx(LPWSTR pStr, SIZE_T cCount, const LFFileSummary& FileSummary)
{
	assert(pStr);

	if (FileSummary.Duration && FileSummary.OnlyTimebasedMediaFiles)
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
		LFGetFileSummary(pStr, cCount, FileSummary.FileCount, FileSummary.FileSize);
	}
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


// Threading
//

LFCORE_API void LFInitProgress(LFProgress& Progress, HWND hWnd, UINT MajorCount)
{
	ZeroMemory(&Progress, sizeof(LFProgress));

	Progress.hWnd = hWnd;
	Progress.ProgressState = LFProgressWorking;
	Progress.MajorCount = MajorCount;
}


// Error handling
//

LFCORE_API void LFGetErrorText(LPWSTR pStr, SIZE_T cCount, UINT ID)
{
	assert(pStr);

	LoadString(LFCoreModuleHandle, IDS_ERR_FIRST+ID, pStr, (INT)cCount);
}


// Descriptors
//

LFCORE_API void LFGetAttrCategoryName(LPWSTR pStr, SIZE_T cCount, UINT ID)
{
	assert(pStr);

	LoadString(LFCoreModuleHandle, IDS_ATTRCATEGORY_FIRST+ID, pStr, (INT)cCount);
}

LFCORE_API void LFGetAttributeInfo(LFAttributeDescriptor& AttributeDescriptor, ATTRIBUTE ID)
{
	assert(ID<LFAttributeCount);
	assert(LFItemColorCount<=8);

	ZeroMemory(&AttributeDescriptor, sizeof(LFAttributeDescriptor));

	// Alternate names
	UINT cAttributeContextRecords = 1;
	UINT64 ContextSet = ALLCONTEXTSSET;

	for (UINT a=0; a<SPECIALATTRIBUTENAMESCOUNT; a++)
		if (SpecialAttributeNames[a].Attr==ID)
		{
			assert((SpecialAttributeNames[a].ContextSet & (1ull<<LFContextAllFiles))==0);
			assert(cAttributeContextRecords<LFAttributeContextRecordCount);

			ContextSet &= ~SpecialAttributeNames[a].ContextSet;

			AttributeDescriptor.ContextRecords[cAttributeContextRecords].IconID = SpecialAttributeNames[a].IconID;
			AttributeDescriptor.ContextRecords[cAttributeContextRecords].SortDescending = SpecialAttributeNames[a].SortDescending;
			AttributeDescriptor.ContextRecords[cAttributeContextRecords].ContextSet = SpecialAttributeNames[a].ContextSet;
			LoadString(LFCoreModuleHandle, SpecialAttributeNames[a].nID, AttributeDescriptor.ContextRecords[cAttributeContextRecords++].Name, LFAttributeNameSize);
		}

	// Default name
	AttributeDescriptor.ContextRecords[0].IconID = AttrProperties[ID].DefaultIconID;
	AttributeDescriptor.ContextRecords[0].SortDescending = (TypeProperties[AttrProperties[ID].Type].DataFlags & LFDataSortDescending);
	AttributeDescriptor.ContextRecords[0].ContextSet = ContextSet;
	LoadString(LFCoreModuleHandle, IDS_ATTR_FIRST+ID, AttributeDescriptor.ContextRecords[0].Name, LFAttributeNameSize);

	// XML ID
	WCHAR Str[LFAttributeNameSize];
	LoadStringEnglish(ID+IDS_ATTR_FIRST, Str, LFAttributeNameSize);

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
	assert(TypeProperties[AttrProperties[ID].Type].AllowedViews & (1<<AttrProperties[ID].DefaultView));
	assert(!(AttrProperties[ID].DataFlags & LFDataAlwaysVisible) || TypeProperties[AttrProperties[ID].Type].DefaultColumnWidth);
	assert((AttrProperties[ID].Type!=LFTypeUnicodeArray) || !AttrProperties[ID].DefaultIconID);
	assert(AttrProperties[ID].DefaultPriority<=LFMinAttributePriority);

	assert(!(AttrProperties[ID].DataFlags & LFDataTaxonomy) || !(TypeProperties[AttrProperties[ID].Type].DataFlags & LFDataTaxonomy));
	assert(!(AttrProperties[ID].DataFlags & LFDataBucket) || !(TypeProperties[AttrProperties[ID].Type].DataFlags & LFDataBucket));
	assert((AttrProperties[ID].DataFlags & LFDataBucket) || !(AttrProperties[ID].DataFlags & LFDataTaxonomy));
	assert((TypeProperties[AttrProperties[ID].Type].DataFlags & LFDataBucket) || !(TypeProperties[AttrProperties[ID].Type].DataFlags & LFDataTaxonomy));
	assert(!(AttrProperties[ID].DataFlags & LFDataTaxonomy) || (AttrProperties[ID].DataFlags & LFDataEditable));
	assert((AttrProperties[ID].DataFlags & LFDataTaxonomy) || (TypeProperties[AttrProperties[ID].Type].DataFlags & LFDataTaxonomy) || !(AttrProperties[ID].DataFlags & LFDataTaxonomyPickGlobally));

	assert((ID==LFAttrColor) || (AttrProperties[ID].Type!=LFTypeColor));

	assert(LFAttrFileName==0);
	assert(AttrProperties[LFAttrFileName].DefaultPriority==0);
	assert(AttrProperties[LFAttrComments].DefaultPriority==LFMinAttributePriority);
	assert(AttrProperties[LFAttrFileFormat].DefaultPriority==LFMinAttributePriority);

#ifdef _DEBUG
	// The alternate sort attribute has to resolve to LFAttrFileName eventually
	ATTRIBUTE Attr = ID;
	do
	{
		Attr = AttrProperties[Attr].AlternateSort;
	}
	while (Attr!=LFAttrFileName);
#endif

	// Properties
	AttributeDescriptor.AttrProperties = AttrProperties[ID];
	AttributeDescriptor.TypeProperties = TypeProperties[AttrProperties[ID].Type];
}

LFCORE_API void LFGetSourceName(LPWSTR pStr, SIZE_T cCount, UINT ID, BOOL Qualified)
{
	assert(ID<LFSourceCount);

	LoadString(LFCoreModuleHandle, (Qualified ? IDS_QSRC_FIRST : IDS_SRC_FIRST)+ID, pStr, (INT)cCount);
}

LFCORE_API void LFGetItemCategoryInfo(LFItemCategoryDescriptor& ItemCategoryDescriptor, UINT ID)
{
	assert(ID<LFItemCategoryCount);

	ZeroMemory(&ItemCategoryDescriptor, sizeof(LFItemCategoryDescriptor));

	LoadTwoStrings(LFCoreModuleHandle, IDS_ITEMCATEGORY_FIRST+ID, ItemCategoryDescriptor.Caption, 256, ItemCategoryDescriptor.Hint, 256);
}

LFCORE_API void LFGetContextInfo(LFContextDescriptor& ContextDescriptor, ITEMCONTEXT ID)
{
	assert(ID<LFContextCount);

	ZeroMemory(&ContextDescriptor, sizeof(LFContextDescriptor));

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
	for (ATTRIBUTE Attr=0; Attr<LFAttributeCount; Attr++)
		if (((CtxProperties[ID].AvailableAttributes>>Attr) & 1) && ((ID<LFContextSubfolderDefault) || (TypeProperties[AttrProperties[Attr].Type].DataFlags & LFDataSortableInSubfolder)))
			assert(CtxProperties[ID].AvailableViews & TypeProperties[AttrProperties[Attr].Type].AllowedViews);
#endif

	assert((ID<=LFLastPersistentContext) || !CtxProperties[ID].SubfolderContext);
	assert(((ID>LFContextFilters) && (ID<=LFLastPersistentContext)) || !CtxProperties[ID].AllowMoveToContext);
	assert((CtxProperties[ID].AllowMoveToContext & (((1ull<<(LFLastPersistentContext+1))-1)-(1ull<<LFContextFilters)))==CtxProperties[ID].AllowMoveToContext);

	// Properties
	ContextDescriptor.CtxProperties = CtxProperties[ID];
}

LFCORE_API void LFGetSortedAttributeList(LFAttributeList& AttributeList)
{
	UINT Index = 0;

	for (UINT Priority=0; Priority<=LFMinAttributePriority; Priority++)
		for (ATTRIBUTE Attr=0; Attr<LFAttributeCount; Attr++)
			if (AttrProperties[Attr].DefaultPriority==Priority)
				AttributeList[Index++] = Attr;
}


LFCORE_API COLORREF LFGetItemColor(UINT ID, UINT Fade)
{
	assert(ID<LFItemColorCount);
	assert(Fade<=LFItemColorFadeLight);

	return (ItemColors[ID]>>Fade) | ItemColorFade[Fade];
}
