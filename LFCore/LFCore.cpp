// LFCore.cpp : Definiert die exportierten Funktionen für die DLL-Anwendung.
//

#include "stdafx.h"
#include "..\\include\\LFCore.h"
#include "liquidFOLDERS.h"
#include "resource.h"
#include "IATA.h"
#include "LFItemDescriptor.h"
#include "LFVariantData.h"
#include "License.h"
#include "ShellProperties.h"
#include <assert.h>
#include <iostream>
#include <shlobj.h>
#include <winioctl.h>


HMODULE LFCoreModuleHandle;
LFMessageIDs LFMessages;
extern unsigned char AttrTypes[];
extern LFShellProperty AttrProperties[];


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
					uDriveType = DRIVE_EXTHD;
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
		case DRIVE_EXTHD:
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

LFCore_API unsigned int LFGetDriveIcon(char Drv, bool IsMounted)
{
	unsigned int ic = IDI_DRV_Default;

	if ((Drv>='A') && (Drv<='Z'))
		switch (DriveTypes[Drv-'A'])
		{
		case DRIVE_FIXED:
		case DRIVE_REMOVABLE:
			ic = IsMounted ? IDI_DRV_Default : IDI_DRV_Empty;
			break;
		case DRIVE_EXTHD:
			ic = IDI_DRV_ExtHD;
			break;
		case DRIVE_REMOTE:
			ic = IsMounted ? IDI_DRV_Connected : IDI_DRV_Disconnected;
		}

	return ic;
}

LFCore_API LFMessageIDs* LFGetMessageIDs()
{
	return &LFMessages;
}

LFCore_API bool LFIsLicensed(LFLicense* License, bool Reload)
{
	return IsLicensed(License, Reload);
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
	if ((a->Type==LFTypeUnicodeString) || (a->Type==LFTypeUnicodeArray) || (a->Type==LFTypeAnsiString))
		a->cCharacters = (unsigned int)GetAttributeMaxCharacterCount(ID);

	// Recommended width
	const unsigned int rWidths[LFTypeCount] = { 200, 200, 200, 100, 100, 100, 120, 100, 100, 100, 150, 140, 100 };
	switch (ID)
	{
	case LFAttrComment:
		a->RecommendedWidth = 350;
		break;
	case LFAttrDescription:
		a->RecommendedWidth = 100;
		break;
	default:
		a->RecommendedWidth = rWidths[a->Type];
	}

	// Category
	if (ID<=LFAttrRating)
	{
		a->Category = ((ID==LFAttrStoreID) || (ID==LFAttrFileID) || (ID==LFAttrDeleteTime) || (ID==LFAttrFileCount) || (ID==LFAttrFileFormat) || (ID==LFAttrFlags)) ? LFAttrCategoryInternal : LFAttrCategoryBasic;
	}
	else
	{
		SetRange(a->Category, ID, LFAttrLocationName, LFAttrLocationGPS, LFAttrCategoryGeotags);
		SetRange(a->Category, ID, LFAttrWidth, LFAttrRoll, LFAttrCategoryVisual);
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
	case LFAttrDescription:
	case LFAttrCreationTime:
	case LFAttrAddTime:
	case LFAttrFileTime:
	case LFAttrDeleteTime:
	case LFAttrArchiveTime:
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
		a->ReadOnly = true;
		break;
	default:
		a->ReadOnly = (a->Category==LFAttrCategoryInternal);
	}

	// Format
	a->FormatRight = (((a->Type>=LFTypeUINT) && (a->Type!=LFTypeTime)) || (ID==LFAttrStoreID) || (ID==LFAttrFileID) || (ID==LFAttrFileCount));

	// Shell property
	a->ShPropertyMapping = AttrProperties[ID];
	if (!a->ShPropertyMapping.ID)
	{
		a->ShPropertyMapping.Schema = PropertyLF;
		a->ShPropertyMapping.ID = ID;
	}

	// Icon

	switch (ID)
	{
	case LFAttrCreationTime:
	case LFAttrAddTime:
	case LFAttrFileTime:
	case LFAttrDuration:
	case LFAttrArchiveTime:
	case LFAttrDueTime:
	case LFAttrDoneTime:
	case LFAttrRecordingTime:
		a->IconID = IDI_FLD_Calendar;
		break;
	case LFAttrRating:
		a->IconID = IDI_FLD_Favorites;
		break;
	case LFAttrRoll:
		a->IconID = IDI_FLD_Roll;
		break;
	case LFAttrLocationName:
	case LFAttrLocationIATA:
	case LFAttrLocationGPS:
		a->IconID = IDI_FLD_Location;
		break;
	case LFAttrArtist:
	case LFAttrResponsible:
		a->IconID = IDI_FLD_Contacts;
		break;
	case LFAttrLanguage:
		a->IconID = IDI_FLD_Fonts;
		break;
	default:
		a->IconID = (a->Category==LFAttrCategoryInternal) ? IDI_FLD_System : IDI_FLD_Default;
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
	LoadString(LFCoreModuleHandle, ID+IDS_FirstContext, c->Name, 256);
	c->AllowGroups = (ID>LFContextClipboard) && (ID<LFContextHousekeeping);

	c->AllowedAttributes = new LFBitArray(LFAttributeCount);
	(*c->AllowedAttributes) += LFAttrFileName;
	(*c->AllowedAttributes) += LFAttrStoreID;
	(*c->AllowedAttributes) += LFAttrFileID;
	(*c->AllowedAttributes) += LFAttrDescription;
	(*c->AllowedAttributes) += LFAttrComment;

	switch (ID)
	{
	case LFContextStores:
		(*c->AllowedAttributes) += LFAttrCreationTime;
		(*c->AllowedAttributes) += LFAttrFileTime;
		break;
	case LFContextStoreHome:
		(*c->AllowedAttributes) += LFAttrFileCount;
		(*c->AllowedAttributes) += LFAttrFileSize;
		break;
	case LFContextTrash:
		(*c->AllowedAttributes) += LFAttrDeleteTime;
	default:
		for (unsigned int a=0; a<LFAttributeCount; a++)
			if ((a!=LFAttrDeleteTime) && ((ID<LFContextSubfolderDefault) || (a!=LFAttrFileCount)))
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

	wchar_t tmpStr[256+256+1];
	LoadString(LFCoreModuleHandle, ID+IDS_FirstItemCategory, tmpStr, 256+256+1);
	size_t sz = wcscspn(tmpStr, L"\n");

	LFItemCategoryDescriptor* c = LFAllocItemCategoryDescriptor();
	wcsncpy_s(c->Caption, 256, tmpStr, sz);
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

	wchar_t tmpStr[256];
	LoadString(LFCoreModuleHandle, IDS_FirstDomain+ID, tmpStr, 256);
	wchar_t* brk = wcschr(tmpStr, L'\n');
	if (brk)
	{
		wcscpy_s(d->Comment, 256, brk+1);
		*brk = L'\0';
	}

	wcscpy_s(d->DomainName, 64, tmpStr);

	*(d->ImportantAttributes) += LFAttrFileName;
	*(d->ImportantAttributes) += LFAttrDescription;
	*(d->ImportantAttributes) += LFAttrCreationTime;
	*(d->ImportantAttributes) += LFAttrFileTime;
	*(d->ImportantAttributes) += LFAttrRoll;
	*(d->ImportantAttributes) += LFAttrComment;
	*(d->ImportantAttributes) += LFAttrTags;
	*(d->ImportantAttributes) += LFAttrRating;
	*(d->ImportantAttributes) += LFAttrPriority;

	if ((ID==LFDomainTrash) || (ID==LFDomainUnknown))
	{
		d->CategoryID = LFItemCategoryHousekeeping;
	}
	else
		if ((ID==LFDomainAllMediaFiles) || ((ID>=LFDomainAudio) && (ID<=LFDomainVideos)))
		{
			d->CategoryID = LFItemCategoryMediaTypes;
		}
		else
			if ((ID==LFDomainAllFiles) || (ID==LFDomainFilters) || (ID==LFDomainFavorites))
			{
				d->CategoryID = LFItemCategoryStore;
			}
			else
			{
				d->CategoryID = LFItemCategoryOtherTypes;
			}

	const unsigned int Icons[LFDomainCount] = { IDI_FLD_All, IDI_FLD_All, IDI_FLD_Favorites, IDI_FLD_Trash, IDI_FLD_Default,
		IDI_FLD_System, IDI_FLD_Audio, IDI_FLD_Photos, IDI_FLD_Pictures, IDI_FLD_Video, IDI_FLD_Archive, IDI_FLD_Contacts,
		IDI_FLD_Documents, IDI_FLD_Calendar, IDI_FLD_Fonts, IDI_FLD_Location, IDI_FLD_Mail, IDI_FLD_Presentations,
		IDI_FLD_Spreadsheets, IDI_FLD_Web };
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
	case LFDomainAllMediaFiles:
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
		*(d->ImportantAttributes) += LFAttrArchiveTime;
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
	case LFDomainTrash:
		*(d->ImportantAttributes) += LFAttrDeleteTime;
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
		*filter = *f;
		filter->ConditionList = NULL;

		LFFilterCondition* c = f->ConditionList;
		while (c)
		{
			LFFilterCondition* item = LFAllocFilterCondition();;
			*item = *c;
			item->Next = filter->ConditionList;
			filter->ConditionList = item;

			c = c->Next;
		}
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
	return (res) ? new LFSearchResult(res): new LFSearchResult(ctx);
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

LFCore_API LFSearchResult* LFGroupSearchResult(LFSearchResult* res, unsigned int attr, bool descending, unsigned int icon, bool groupone, LFFilter* f)
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
		cooked->GroupArray(attr, icon, f);
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
	cooked->Group(attr, icon, groupone, f);

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

LFCore_API void LFFreeFileIDList(LFFileIDList* il)
{
	if (il)
		delete il;
}

LFCore_API bool LFAddFileID(LFFileIDList* il, char* StoreID, char* FileID, void* UserData)
{
	return il->AddFileID(StoreID, FileID, UserData);
}


LFCore_API LFFileImportList* LFAllocFileImportList()
{
	return new LFFileImportList();
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


LFCore_API LFMaintenanceList* LFAllocMaintenanceList()
{
	return new LFMaintenanceList();
}

LFCore_API void LFFreeMaintenanceList(LFMaintenanceList* ml)
{
	if (ml)
		delete ml;
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
	*str = L'\0';
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
