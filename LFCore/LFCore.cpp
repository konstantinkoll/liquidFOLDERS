// LFCore.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//

#include "stdafx.h"
#include "..\\include\\LFCore.h"
#include "liquidFOLDERS.h"
#include "resource.h"
#include "LFItemDescriptor.h"
#include "LFVariantData.h"
#include "Domains.h"
#include "License.h"
#include "IATA.h"
#include "Stores.h"
#include "License.h"
#include <iostream>
#include <winioctl.h>


HMODULE LFCoreModuleHandle;
LFMessageIDs LFMessages;
extern unsigned char AttrTypes[];


// Der Inhalt dieses Segments wird über alle Instanzen von LFCore geteilt.
// Der Zugriff muss daher über Mutex-Objekte serialisiert/synchronisiert werden.
// Alle Variablen im Segment müssen initalisiert werden !

#pragma data_seg("common_drives")

unsigned int DriveTypes[26] = { DRIVE_UNKNOWN };

#pragma data_seg()
#pragma comment(linker, "/SECTION:common_drives,RWS")


unsigned int GetDriveBus(char d)
{
	unsigned int res = BusTypeMaxReserved;

	char szBuf[MAX_PATH] = "\\\\?\\ :";
	szBuf[4] = d;
	HANDLE hDevice = CreateFileA(szBuf, GENERIC_READ,
		FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, NULL, NULL);

	if (hDevice!=INVALID_HANDLE_VALUE)
	{
		STORAGE_DEVICE_DESCRIPTOR* pDevDesc = (STORAGE_DEVICE_DESCRIPTOR*)new BYTE[sizeof(STORAGE_DEVICE_DESCRIPTOR) + 512 - 1];
		pDevDesc->Size = sizeof(STORAGE_DEVICE_DESCRIPTOR)+512-1;

		STORAGE_PROPERTY_QUERY Query;
		Query.PropertyId = StorageDeviceProperty;
		Query.QueryType = PropertyStandardQuery;

		DWORD dwOutBytes;
		if (DeviceIoControl(hDevice, IOCTL_STORAGE_QUERY_PROPERTY,
			&Query, sizeof(STORAGE_PROPERTY_QUERY), pDevDesc, pDevDesc->Size,
			&dwOutBytes, NULL))
			res = pDevDesc->BusType;

		delete pDevDesc;
		CloseHandle(hDevice);
	}

	return res;
}

LFCore_API unsigned int LFGetLogicalDrives(unsigned int mask)
{
	DWORD DrivesOnSystem = GetLogicalDrives() & ~3;
	DWORD Index = 1;
	char szDriveRoot[] = " :\\";

	for (char cDrive='A'; cDrive<='Z'; cDrive++, Index<<=1)
	{
		if (!(DrivesOnSystem & Index))
		{
			DriveTypes[cDrive-'A'] = DRIVE_UNKNOWN;
			continue;
		}

		unsigned int uDriveType = DriveTypes[cDrive-'A'];
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

			DriveTypes[cDrive-'A'] = uDriveType;
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

LFCore_API bool LFIsLicensed()
{
	return IsLicensed();
}


LFCore_API LFAttributeDescriptor* LFAllocAttributeDescriptor()
{
	LFAttributeDescriptor* a = new LFAttributeDescriptor;
	ZeroMemory(a, sizeof(LFAttributeDescriptor));
	return a;
}

inline void SetRange(unsigned char &var, unsigned int ID, unsigned int lo, unsigned int hi, unsigned char val)
{
	if ((ID>=lo) && (ID<=hi))
		var = val;
}

LFCore_API LFAttributeDescriptor* LFGetAttributeInfo(unsigned int ID)
{
	if (ID>=LFAttributeCount)
		return NULL;

	LFAttributeDescriptor* a = LFAllocAttributeDescriptor();
	LoadString(LFCoreModuleHandle, ID+IDS_FirstAttribute, a->Name, 64);
	a->AlwaysVisible = (ID==LFAttrFileName);
	a->Type = AttrTypes[ID];

	// Type and character count (where appropriate)
	if ((a->Type==LFTypeUnicodeString) || (a->Type==LFTypeAnsiString))
		a->cCharacters = GetAttributeMaxCharacterCount(ID);

	// Recommended width
	const unsigned int rWidths[] = { 200, 200, 100, 100, 100, 120, 100, 100, 100, 150, 140, 100 };
	a->RecommendedWidth = (ID==LFAttrHint) ? 350 : rWidths[a->Type];

	// Category
	if (ID<=LFAttrRating)
	{
		a->Category = ((ID==LFAttrStoreID) || (ID==LFAttrFileID) || (ID==LFAttrFileFormat) || (ID==LFAttrFlags)) ? LFAttrCategoryInternal : LFAttrCategoryBasic;
	}
	else
	{
		SetRange(a->Category, ID, LFAttrLocationName, LFAttrLocationGPS, LFAttrCategoryGeotags);
		SetRange(a->Category, ID, LFAttrHeight, LFAttrRoll, LFAttrCategoryVisual);
		SetRange(a->Category, ID, LFAttrExposure, LFAttrChip, LFAttrCategoryPhotographic);
		SetRange(a->Category, ID, LFAttrAlbum, LFAttrAudioCodec, LFAttrCategoryAudio);
		SetRange(a->Category, ID, LFAttrDuration, LFAttrBitrate, LFAttrCategoryTimebased);
		SetRange(a->Category, ID, LFAttrArtist, LFAttrSignature, LFAttrCategoryBibliographic);
		SetRange(a->Category, ID, LFAttrFrom, LFAttrDoneTime, LFAttrCategoryWorkflow);
		SetRange(a->Category, ID, LFAttrPriority, LFAttrPriority, LFAttrCategoryWorkflow);
	}

	// Sortable
	a->Sortable = (a->Type!=LFTypeFlags);

	// ReadOnly
	switch (ID)
	{
	case LFAttrHint:
	case LFAttrCreationTime:
	case LFAttrFileTime:
	case LFAttrFileSize:
	case LFAttrHeight:
	case LFAttrWidth:
	case LFAttrResolution:
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
		a->ReadOnly = true;
		break;
	default:
		a->ReadOnly = (a->Category==LFAttrCategoryInternal);
	}

	return a;
}

LFCore_API void LFFreeAttributeDescriptor(LFAttributeDescriptor* a)
{
	if (a)
		delete a;
}


LFCore_API LFContextDescriptor* LFAllocContextDescriptor()
{
	LFContextDescriptor* c = new LFContextDescriptor;
	ZeroMemory(c, sizeof(LFContextDescriptor));
	return c;
}

LFCore_API LFContextDescriptor* LFGetContextInfo(unsigned int ID)
{
	if (ID>=LFContextCount)
		return NULL;

	LFContextDescriptor* c = LFAllocContextDescriptor();
	LoadString(LFCoreModuleHandle, ID+IDS_FirstContext, c->Name, 64);
	c->AllowExtendedViews = (ID>LFContextClipboard);
	c->AllowGroups = (ID>LFContextStoreHome) && (ID!=LFContextClipboard);

	c->AllowedAttributes = new LFBitArray(LFAttributeCount);
	(*c->AllowedAttributes) += LFAttrFileName;
	(*c->AllowedAttributes) += LFAttrStoreID;
	(*c->AllowedAttributes) += LFAttrFileID;
	(*c->AllowedAttributes) += LFAttrHint;

	switch (ID)
	{
	case LFContextStores:
		(*c->AllowedAttributes) += LFAttrComment;
		(*c->AllowedAttributes) += LFAttrCreationTime;
		(*c->AllowedAttributes) += LFAttrFileTime;
		break;
	case LFContextStoreHome:
		break;
	default:
		for (unsigned int a=0; a<LFAttributeCount; a++)
			(*c->AllowedAttributes) += a;
	}

	return c;
}

LFCore_API void LFFreeContextDescriptor(LFContextDescriptor* c)
{
	if (c)
	{
		if (c->AllowedAttributes)
			delete c->AllowedAttributes;
		delete c;
	}
}


LFCore_API LFItemCategoryDescriptor* LFAllocItemCategoryDescriptor()
{
	LFItemCategoryDescriptor* c = new LFItemCategoryDescriptor;
	ZeroMemory(c, sizeof(LFItemCategoryDescriptor));
	return c;
}

LFCore_API LFItemCategoryDescriptor* LFGetItemCategoryInfo(unsigned int ID)
{
	if (ID>=LFItemCategoryCount)
		return NULL;

	wchar_t tmpStr[256+64+1];
	LoadString(LFCoreModuleHandle, ID+IDS_FirstItemCategory, tmpStr, 256+64+1);
	size_t sz = wcscspn(tmpStr, L"\n");

	LFItemCategoryDescriptor* c = LFAllocItemCategoryDescriptor();
	wcsncpy_s(c->Name, 64, tmpStr, sz);
	if (sz<wcslen(tmpStr))
		wcscpy_s(c->Hint, 256, &tmpStr[sz+1]);

	return c;
}

LFCore_API void LFFreeItemCategoryDescriptor(LFItemCategoryDescriptor* c)
{
	if (c)
		delete c;
}

LFCore_API LFDomainDescriptor* LFAllocDomainDescriptor()
{
	LFDomainDescriptor* d = new LFDomainDescriptor;
	ZeroMemory(d, sizeof(LFDomainDescriptor));
	d->ImportantAttributes = new LFBitArray(LFAttributeCount);
	return d;
}

LFCore_API LFDomainDescriptor* LFGetDomainInfo(unsigned int ID)
{
	if (ID>=LFDomainCount)
		return NULL;

	LFDomainDescriptor* d = LFAllocDomainDescriptor();

	LoadString(LFCoreModuleHandle, IDS_FirstDomain+ID, d->DomainName, 256);
	d->Hint[0] = '\0';

	for (unsigned int a=0; a<wcslen(d->DomainName); a++)
		if (d->DomainName[a]=='\n')
		{
			wcsncpy_s(d->Hint, 256, &d->DomainName[a+1], wcslen(d->DomainName)-a-1);
			d->DomainName[a] = '\0';
			break;
		}

	*(d->ImportantAttributes) += LFAttrFileName;
	*(d->ImportantAttributes) += LFAttrHint;
	*(d->ImportantAttributes) += LFAttrCreationTime;
	*(d->ImportantAttributes) += LFAttrFileTime;
	*(d->ImportantAttributes) += LFAttrRoll;
	*(d->ImportantAttributes) += LFAttrComment;
	*(d->ImportantAttributes) += LFAttrRating;
	*(d->ImportantAttributes) += LFAttrPriority;

	if (ID<=LFDomainFilters)
	{
		d->CategoryID = LFCategoryStore;
	}
	else
		if (ID<=LFDomainVideos)
		{
			d->CategoryID = LFCategoryMultimediaTypes;
		}
		else
			if ((ID==LFDomainTrash) || (ID==LFDomainUnknown))
			{
				d->CategoryID = LFCategoryHousekeeping;
			}
			else
			{
				d->CategoryID = LFCategoryOtherTypes;
			}

	const unsigned int Icons[LFDomainCount] = { IDI_FLD_All, IDI_FLD_Favorites, IDI_FLD_System, IDI_FLD_All,
		IDI_FLD_Audio, IDI_FLD_Pictures, IDI_FLD_Photos, IDI_FLD_Video, IDI_FLD_Archive, IDI_FLD_Contacts,
		IDI_FLD_Documents, IDI_FLD_Calendar, IDI_FLD_Fonts, IDI_FLD_Location, IDI_FLD_Mail, IDI_FLD_Presentations,
		IDI_FLD_Spreadsheets, IDI_FLD_Web, IDI_FLD_Trash, IDI_FLD_Default };
	d->IconID = Icons[ID];

	switch (ID)
	{
	case LFDomainAllFiles:
	case LFDomainContacts:
	case LFDomainEvents:
		*(d->ImportantAttributes) += LFAttrLocationName;
		*(d->ImportantAttributes) += LFAttrLocationIATA;
		*(d->ImportantAttributes) += LFAttrResponsible;
		*(d->ImportantAttributes) += LFAttrDueTime;
		*(d->ImportantAttributes) += LFAttrDoneTime;
		break;
	case LFDomainAllMultimediaFiles:
		*(d->ImportantAttributes) += LFAttrLocationName;
		*(d->ImportantAttributes) += LFAttrLocationIATA;
		*(d->ImportantAttributes) += LFAttrArtist;
		*(d->ImportantAttributes) += LFAttrTitle;
		*(d->ImportantAttributes) += LFAttrAlbum;
		*(d->ImportantAttributes) += LFAttrDuration;
		*(d->ImportantAttributes) += LFAttrBitrate;
		*(d->ImportantAttributes) += LFAttrRecordingTime;
		*(d->ImportantAttributes) += LFAttrLanguage;
		break;
	case LFDomainAudio:
		*(d->ImportantAttributes) += LFAttrArtist;
		*(d->ImportantAttributes) += LFAttrTitle;
		*(d->ImportantAttributes) += LFAttrAlbum;
		*(d->ImportantAttributes) += LFAttrDuration;
		*(d->ImportantAttributes) += LFAttrBitrate;
		*(d->ImportantAttributes) += LFAttrRecordingTime;
		break;
	case LFDomainPictures:
	case LFDomainPhotos:
		*(d->ImportantAttributes) += LFAttrLocationName;
		*(d->ImportantAttributes) += LFAttrLocationIATA;
		*(d->ImportantAttributes) += LFAttrArtist;
		*(d->ImportantAttributes) += LFAttrTitle;
		*(d->ImportantAttributes) += LFAttrRecordingTime;
		*(d->ImportantAttributes) += LFAttrLanguage;
		break;
	case LFDomainVideos:
		*(d->ImportantAttributes) += LFAttrLocationName;
		*(d->ImportantAttributes) += LFAttrLocationIATA;
		*(d->ImportantAttributes) += LFAttrArtist;
		*(d->ImportantAttributes) += LFAttrTitle;
		*(d->ImportantAttributes) += LFAttrDuration;
		*(d->ImportantAttributes) += LFAttrRecordingTime;
		*(d->ImportantAttributes) += LFAttrLanguage;
		break;
	case LFDomainDocuments:
	case LFDomainPresentations:
	case LFDomainSpreadsheets:
		*(d->ImportantAttributes) += LFAttrLocationName;
		*(d->ImportantAttributes) += LFAttrLocationIATA;
		*(d->ImportantAttributes) += LFAttrArtist;
		*(d->ImportantAttributes) += LFAttrCopyright;
		*(d->ImportantAttributes) += LFAttrResponsible;
		*(d->ImportantAttributes) += LFAttrTitle;
		*(d->ImportantAttributes) += LFAttrSignature;
		*(d->ImportantAttributes) += LFAttrDueTime;
		*(d->ImportantAttributes) += LFAttrDoneTime;
		*(d->ImportantAttributes) += LFAttrLanguage;
		break;
	case LFDomainGeodata:
		*(d->ImportantAttributes) += LFAttrLocationName;
		*(d->ImportantAttributes) += LFAttrLocationIATA;
		break;
	}

	return d;
}

LFCore_API void LFFreeDomainDescriptor(LFDomainDescriptor* d)
{
	if (d)
	{
		if (d->ImportantAttributes)
			delete d->ImportantAttributes;
		delete d;
	}
}


LFCore_API LFFilter* LFAllocFilter(LFFilter* f)
{
	LFFilter* filter = new LFFilter;
	if (f)
	{
		memcpy(filter, f, sizeof(LFFilter));
	}
	else
	{
		ZeroMemory(filter, sizeof(LFFilter));
		filter->Mode = LFFilterModeStores;
		filter->Result.FilterType = LFFilterTypeDefault;
	}
	return filter;
}

LFCore_API void LFFreeFilter(LFFilter* f)
{
	if (f)
		delete f;
}


LFCore_API LFSearchResult* LFAllocSearchResult(int ctx, LFSearchResult* res, bool AllowEmptyDrives)
{
	return (res==NULL) ? new LFSearchResult(ctx) : new LFSearchResult(ctx, res, AllowEmptyDrives);
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

LFCore_API void LFRemoveEntry(LFTransactionList* tl, unsigned idx)
{
	tl->RemoveEntry(idx);
}

LFCore_API void LFRemoveFlaggedEntries(LFTransactionList* tl)
{
	tl->RemoveFlaggedEntries();
}

LFCore_API void LFRemoveErrorEntries(LFTransactionList* tl)
{
	tl->RemoveErrorEntries();
}


LFCore_API LFStoreDescriptor* LFAllocStoreDescriptor()
{
	LFStoreDescriptor* s = new LFStoreDescriptor;
	ZeroMemory(s, sizeof(LFStoreDescriptor));
	return s;
}

LFCore_API void LFFreeStoreDescriptor(LFStoreDescriptor* s)
{
	if (s)
		delete s;
}


wchar_t* LoadResourceString(unsigned int ID, unsigned int length)
{
	wchar_t* str = new wchar_t[length];
	LoadString(LFCoreModuleHandle, ID, str, length);
	return str;
}

LFCore_API wchar_t* LFGetAttrCategoryName(unsigned int ID)
{
	return LoadResourceString(ID+IDS_FirstAttrCategory, 64);
}

LFCore_API wchar_t* LFGetErrorText(unsigned int ID)
{
	return LoadResourceString(ID+IDS_FirstError, 256);
}

LFCore_API void LFErrorBox(unsigned int ID, HWND hWnd)
{
	if (ID>LFCancel)
	{
		wchar_t* msg = LFGetErrorText(ID);
		wchar_t caption[256];
		LoadString(LFCoreModuleHandle, IDS_ErrorCaption, caption, 256);

		MessageBox(hWnd, msg, caption, MB_OK | MB_ICONERROR);

		free(msg);
	}
}


LFCore_API bool LFAttributeSortableInView(unsigned int Attr, unsigned int ViewMode)
{
	bool b = (Attr!=LFAttrLocationGPS);
	switch (ViewMode)
	{
	case LFViewAutomatic:
		b = true;
		break;
	case LFViewCalendarYear:
	case LFViewCalendarWeek:
	case LFViewCalendarDay:
	case LFViewTimeline:
		b = ((Attr==LFAttrCreationTime) || (Attr==LFAttrFileTime) || (Attr==LFAttrRecordingTime) || (Attr==LFAttrDueTime) || (Attr==LFAttrDoneTime));
		break;
	case LFViewGlobe:
		b = ((Attr==LFAttrLocationIATA) || (Attr==LFAttrLocationGPS));
		break;
	case LFViewTagcloud:
		b &= (Attr!=LFAttrRating);
		break;
	}
	return b;
}



// TEMP
//

#include <io.h>
#include <malloc.h>
#include <objbase.h>
#include <shellapi.h>
#include <shlobj.h>
#include <sys/types.h>
#include <sys/stat.h>
void AddLocation(LFSearchResult* res, char* loc)
{
	char tmpStr[8];
	strcpy_s(tmpStr, 8, loc);

	LFAirport* airport;
	if (LFIATAGetAirportByCode(tmpStr, &airport)==true)
	{
		LFItemDescriptor* i = LFIATACreateFolderForAirport(airport);
		sprintf_s(tmpStr, 8, "%d", res->m_Count+1);
		SetAttribute(i, LFAttrFileID, tmpStr);
		wchar_t hint[256];
		swprintf_s(hint, 256 , L"%d files", 10+(rand()%1000));
		SetAttribute(i, LFAttrHint, hint);
		unsigned char Rating = rand()%(LFMaxRating+1);
		SetAttribute(i, LFAttrRating, &Rating);
		res->AddItemDescriptor(i);
	}
}

//
// TEMP




LFCore_API LFSearchResult* LFQuery(LFFilter* filter)
{
	LFSearchResult* res = NULL;
	DWORD start = GetTickCount();

	if (!filter)
	{
		res = QueryStores();
	}
	else
	{
		// Ggf. Default Store einsetzen
		if ((strcmp(filter->StoreID, "")==0) &&
			((filter->Mode==LFFilterModeStoreHome) || (filter->Mode==LFFilterModeSearchInStore)))
			if (LFDefaultStoreAvailable())
			{
				char* ds = LFGetDefaultStore();
				strcpy_s(filter->StoreID, LFKeySize, ds);
				free(ds);
			}
			else
			{
				res = new LFSearchResult(LFContextDefault);
				res->m_LastError = LFNoDefaultStore;
			}

		// Query
		if (!res)
			switch (filter->Mode)
			{
			case LFFilterModeStores:
				res = QueryStores(filter);
				break;
			case LFFilterModeStoreHome:
				res = QueryDomains(filter);
				break;
			default:
/*				res = LFAllocSearchResult(LFContextDefault);
				res->m_RecommendedView = LFViewLargeIcons;
				res->m_LastError = LFIllegalQuery;
				if (wcscmp(filter->Name, L"")==0)
					wcscpy_s(filter->Name, 256, L"?");
				filter->Result.FilterType = LFFilterTypeIllegalRequest;*/



				// TEMP
				//
				res = LFAllocSearchResult(LFContextDefault);
				char loc[4];
				for (unsigned int a=0; a<100; a++)
				{
					loc[0] = char(65+rand()%26);
					loc[1] = char(65+rand()%26);
					loc[2] = char(65+rand()%26);
					loc[3] = '\0';
					AddLocation(res, loc);
				}

				if (wcscmp(filter->Name, L"Trash")==0)
				{
					filter->Result.FilterType = LFFilterTypeTrash;
					res->m_Context = LFContextTrash;
				}
				//
				// TEMP



			}

		// Statistik
		if ((wcscmp(filter->Name, L"")==0) && (res->m_Context!=LFContextDefault))
			LoadString(LFCoreModuleHandle, res->m_Context+IDS_FirstContext, filter->Name, 256);
		GetLocalTime(&filter->Result.Time);
		filter->Result.ItemCount = res->m_Count;
	}

	res->m_QueryTime = GetTickCount()-start;
	return res;
}
