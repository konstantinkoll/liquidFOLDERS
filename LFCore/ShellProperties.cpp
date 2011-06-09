
#include "stdafx.h"
#include "IdxTables.h"
#include "PIDL.h"
#include "ShellProperties.h"
#include <assert.h>
#include <shlobj.h>
#include <shlwapi.h>


extern unsigned char AttrTypes[];


static const GUID PropertyStorage =
	{ 0xB725f130, 0x47Ef, 0x101A, { 0xA5, 0xF1, 0x02, 0x60, 0x8C, 0x9E, 0xEB, 0xAC } };
static const GUID PropertyQuery =
	{ 0x49691c90, 0x7E17, 0x101A, { 0xA9, 0x1C, 0x08, 0x00, 0x2B, 0x2E, 0xCD, 0xA9 } };
static const GUID PropertySummary =
	{ 0xF29F85E0, 0x4FF9, 0x1068, { 0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9 } };
static const GUID PropertyDocuments =
	{ 0xD5CDD502, 0x2e9c, 0x101B, { 0x93, 0x97, 0x08, 0x00, 0x2B, 0x2C, 0xF9, 0xAE } };
static const GUID PropertyImage =
	{ 0x6444048F, 0x4c8b, 0x11D1, { 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03 } };
static const GUID PropertyAudio =
	{ 0x64440490, 0x4C8B, 0x11D1, { 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03 } };
static const GUID PropertyVideo =
	{ 0x64440491, 0x4C8B, 0x11D1, { 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03 } };
static const GUID PropertyMedia =
	{ 0x64440492, 0x4C8B, 0x11D1, { 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03 } };
static const GUID PropertyPhoto =
	{ 0x14B81DA1, 0x0135, 0x4D31, { 0x96, 0xD9, 0x6C, 0xBF, 0xC9, 0x67, 0x1A, 0x99 } };
static const GUID PropertyMusic =
	{ 0x56A3372E, 0xCE9C, 0x11D2, { 0x9F, 0x0E, 0x00, 0x60, 0x97, 0xC6, 0x86, 0xF6 } };
static const GUID PropertyVersion =
	{ 0x0CEF7D53, 0xFA64, 0x11D1, { 0xA2, 0x03, 0x00, 0x00, 0xF8, 0x1F, 0xED, 0xEE } };
static const GUID PropertyUnnamed1 =
	{ 0x3F8472B5, 0xE0AF, 0x4DB2, { 0x80, 0x71, 0xC5, 0x3F, 0xE7, 0x6A, 0xE7, 0xCE } };
static const GUID PropertyUnnamed2 =
	{ 0x72FAB781, 0xACDA, 0x43E5, { 0xB1, 0x55, 0xB2, 0x43, 0x4F, 0x85, 0xE6, 0x78 } };
static const GUID PropertyUnnamed3 =
	{ 0xE3E0584C, 0xB788, 0x4A5A, { 0xBB, 0x20, 0x7F, 0x5A, 0x44, 0xC9, 0xAC, 0xDD } };
static const GUID PropertyUnnamed4 =
	{ 0x2E4B640D, 0x5019, 0x46D8, { 0x88, 0x81, 0x55, 0x41, 0x4C, 0xC5, 0xCA, 0xA0 } };
static const GUID PropertyUnnamed5 =
	{ 0x2CBAA8F5, 0xD81F, 0x47CA, { 0xB1, 0x7A, 0xF8, 0xD8, 0x22, 0x30, 0x01, 0x31 } };
static const GUID PropertyUnnamed6 =
	{ 0x43F8D7B7, 0xA444, 0x4F87, { 0x93, 0x83, 0x52, 0x27, 0x1C, 0x9B, 0x91, 0x5C } };
static const GUID PropertyUnnamed7 =
	{ 0x276D7BB0, 0x5B34, 0x4FB0, { 0xAA, 0x4B, 0x15, 0x8E, 0xD1, 0x2A, 0x18, 0x09 } };


// Der Inhalt dieses Segments wird über alle Instanzen von LFCore geteilt.
// Der Zugriff muss daher über Mutex-Objekte serialisiert/synchronisiert werden.
// Alle Variablen im Segment müssen initalisiert werden !

#pragma data_seg("common_shprop")

unsigned char DomainSlaves[LFDomainCount] = {
	IDMaster,							// LFDomainAllFiles
	IDMaster,							// LFDomainAllMediaFiles
	IDMaster,							// LFDomainFavorites
	IDMaster,							// LFDomainTrash
	IDMaster,							// LFDomainUnknown
	IDMaster,							// LFDomainFilters
	IDSlaveAudio,						// LFDomainAudio
	IDSlavePictures,					// LFDomainPhotos
	IDSlavePictures,					// LFDomainPictures
	IDSlaveVideos,						// LFDomainVideos
	IDMaster,							// LFDomainArchives
	IDMaster,							// LFDomainContacts
	IDSlaveDocuments,					// LFDomainDocuments
	IDMaster,							// LFDomainEvents
	IDMaster,							// LFDomainFonts
	IDMaster,							// LFDomainGeodata
	IDSlaveMails,						// LFDomainMessages
	IDSlaveDocuments,					// LFDomainPresentations
	IDSlaveDocuments,					// LFDomainSpreadsheets
	IDSlaveDocuments					// LFDomainWeb
};

#include "DomainTable.h"

LFShellProperty AttrProperties[LFAttributeCount] = {
	{ PropertyStorage, 10 },		// LFAttrFileName
	{ 0, 0 },						// LFAttrStoreID
	{ PropertyStorage, 8 },			// LFAttrFileID
	{ PropertySummary, 6 },			// LFAttrComments
	{ PropertyVersion, 3 },			// LFAttrDescription
	{ PropertyStorage, 15 },		// LFAttrCreationTime
	{ PropertyStorage, 14 },		// LFAttrFileTime
	{ PropertyUnnamed5, 100 },		// LFAttrAddTime
	{ 0, 0 },						// LFAttrDeleteTime
	{ PropertyUnnamed6, 100 },		// LFAttrArchiveTime
	{ PropertyStorage, 4 },			// LFAttrFileFormat
	{ 0, 0 },						// LFAttrFileCount
	{ PropertyStorage, 12 },		// LFAttrFileSize
	{ 0, 0 },						// LFAttrFlags
	{ PropertyQuery, 9 },			// LFAttrURL
	{ PropertySummary, 5 },			// LFAttrTags
	{ PropertyMedia, 9 },			// LFAttrRating
	{ PropertyUnnamed3, 11 },		// LFAttrPriority
	{ 0, 0 },						// LFAttrLocationName
	{ 0, 0 },						// LFAttrLocationIATA
	{ 0, 0 },						// LFAttrLocationGPS

	{ PropertyImage, 3 },			// LFAttrWidth
	{ PropertyImage, 4 },			// LFAttrHeight
	{ 0, 0 },						// LFAttrDimension
	{ 0, 0 },						// LFAttrAspectRatio
	{ PropertyVideo, 44 },			// LFAttrVideoCodec
	{ PropertyPhoto, 18248 },		// LFAttrRoll

	{ 0, 0 },						// LFAttrExposure
	{ 0, 0 },						// LFAttrFocus
	{ 0, 0 },						// LFAttrAperture
	{ 0, 0 },						// LFAttrChip

	{ PropertyMusic, 4 },			// LFAttrAlbum
	{ PropertyMedia, 7 },			// LFAttrChannels
	{ PropertyMedia, 5 },			// LFAttrSamplerate
	{ 0, 0 },						// LFAttrAudioCodec

	{ PropertyAudio, 3 },			// LFAttrDuration
	{ PropertyMedia, 4 },			// LFAttrBitrate

	{ PropertySummary, 4 },			// LFAttrArtist
	{ PropertySummary, 2 },			// LFAttrTitle
	{ PropertyMedia, 11 },			// LFAttrCopyright
	{ 0, 0 },						// LFAttrISBN
	{ 0, 0 },						// LFAttrLanguage
	{ PropertyDocuments, 14 },		// LFAttrPages
	{ PropertyUnnamed4, 100 },		// LFAttrRecordingTime
	{ PropertyPhoto, 272 },			// LFAttrRecordingEquipment
	{ 0, 0 },						// LFAttrSignature

	{ 0, 0 },						// LFAttrFrom
	{ 0, 0 },						// LFAttrTo
	{ 0, 0 },						// LFAttrResponsible
	{ PropertyUnnamed1, 100 },		// LFAttrDueTime
	{ PropertyUnnamed2, 100 },		// LFAttrDoneTime
	{ PropertyUnnamed7, 100 }		// LFAttrClient
};

#pragma data_seg()
#pragma comment(linker, "/SECTION:common_shprop,RWS")


unsigned char GetHardcodedDomain(char* ext)
{
	unsigned int left = 0;
	unsigned int right = sizeof(Registry)/sizeof(RegisteredFile);

	while (left<right)
	{
		unsigned int mid = left+(right-left)/2;

		switch (strcmp(ext, Registry[mid].Format))
		{
		case 0:
			return Registry[mid].DomainID;
		case 1:
			left = mid+1;
			break;
		case -1:
			right = mid;
			break;
		}
	}

	return 0;
}

unsigned char GetPerceivedDomain(char* ext)
{
	wchar_t ExtW[17];
	ExtW[0] = '.';
	MultiByteToWideChar(CP_ACP, 0, ext, -1, &ExtW[1], 16);

	PERCEIVED Type;
	PERCEIVEDFLAG Flag;
	if (AssocGetPerceivedType(ExtW, &Type, &Flag, NULL)==S_OK)
		switch (Type)
		{
		case PERCEIVED_TYPE_IMAGE:
			return LFDomainPictures;
		case PERCEIVED_TYPE_AUDIO:
			return LFDomainAudio;
		case PERCEIVED_TYPE_VIDEO:
			return LFDomainVideos;
		case PERCEIVED_TYPE_COMPRESSED:
			return LFDomainArchives;
		case PERCEIVED_TYPE_CONTACTS:
			return LFDomainContacts;
		}

	return 0;
}

void SetFileDomainAndSlave(LFItemDescriptor* i)
{
	assert(i);

	#ifdef _DEBUG
	// Test: ist die Domain-Liste korrekt sortiert?
	for (unsigned int a=0; a<(sizeof(Registry)/sizeof(RegisteredFile))-2; a++)
		if (strcmp(Registry[a].Format, Registry[a+1].Format)>-1)
			MessageBoxA(NULL, Registry[a].Format, "Registry sort error", 0);
	#endif

	// Domain
	if (!i->CoreAttributes.DomainID)
		i->CoreAttributes.DomainID = GetHardcodedDomain(i->CoreAttributes.FileFormat);

	// TODO: Benutzer-Einstellungen abfragen

	if (!i->CoreAttributes.DomainID)
		i->CoreAttributes.DomainID = GetPerceivedDomain(i->CoreAttributes.FileFormat);

	// Slave
	assert(i->CoreAttributes.DomainID<LFDomainCount);
	i->CoreAttributes.SlaveID = DomainSlaves[i->CoreAttributes.DomainID];
}

bool GetShellProperty(IShellFolder2* pParentFolder, LPCITEMIDLIST pidlRel, GUID Schema, UINT ID, LFItemDescriptor*i, UINT attr)
{
	SHCOLUMNID column = { Schema, ID };
	VARIANT value = { 0 };
	
	if (FAILED(pParentFolder->GetDetailsEx(pidlRel, &column, &value)))
		return false;

	switch (value.vt)
	{
	case VT_BSTR:
		if ((AttrTypes[attr]==LFTypeUnicodeString) || (AttrTypes[attr]==LFTypeUnicodeArray))
			SetAttribute(i, attr, value.pbstrVal);
		break;
	case VT_I4:
		if (((AttrTypes[attr]==LFTypeUINT) || (AttrTypes[attr]==LFTypeBitrate)) && (value.intVal>=0))
			SetAttribute(i, attr, &value.intVal);
		break;
	case VT_UI4:
		switch (AttrTypes[attr])
		{
		case LFTypeUINT:
		case LFTypeFourCC:
			SetAttribute(i, attr, &value.uintVal);
			break;
		}
		break;
	case VT_I8:
	case VT_UI8:
		switch (AttrTypes[attr])
		{
		case LFTypeINT64:
			SetAttribute(i, attr, &value.ullVal);
			break;
		case LFTypeDuration:
			value.ullVal /= 10000;
			UINT Duration = (value.ullVal>0xFFFFFFFF) ? 0xFFFFFFFF : (UINT)value.ullVal;
			SetAttribute(i, attr, &Duration);
			break;
		}
		break;
	case VT_R8:
		if (AttrTypes[attr]==LFTypeDouble)
			SetAttribute(i, attr, &value.dblVal);
		break;
	case VT_DATE:
		if (AttrTypes[attr]==LFTypeTime)
		{
			SYSTEMTIME st;
			FILETIME ft;
			VariantTimeToSystemTime(value.date, &st);
			SystemTimeToFileTime(&st, &ft);
			SetAttribute(i, attr, &ft);
		}
		break;
	}
	
	VariantClear(&value);
	return (value.vt!=0);
}

void SetAttributesFromFile(LFItemDescriptor* i, wchar_t* fn, bool metadata)
{
	// Attribute des Dateisystems
	WIN32_FIND_DATA ffd;
	HANDLE hFind = FindFirstFile(fn, &ffd);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
		__int64 size = (((__int64)ffd.nFileSizeHigh) << 32)+ffd.nFileSizeLow;
		SetAttribute(i, LFAttrFileSize, &size);
		SetAttribute(i, LFAttrCreationTime, &ffd.ftCreationTime);
		SetAttribute(i, LFAttrFileTime, &ffd.ftLastWriteTime);
	}

	FindClose(hFind);

	// Domain und Slave
	SetFileDomainAndSlave(i);

	if (!metadata)
		return;

	// Shell properties
	IShellFolder* pDesktop;
	if (SUCCEEDED(SHGetDesktopFolder(&pDesktop)))
	{
		LPITEMIDLIST pidlFQ;
		if (SUCCEEDED(pDesktop->ParseDisplayName(NULL, NULL, fn, NULL, &pidlFQ, NULL)))
		{
			IShellFolder2* pParentFolder = NULL;
			LPCITEMIDLIST pidlRel = NULL;
			if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder2, (void**)&pParentFolder, &pidlRel)))
			{
				for (unsigned int a=0; a<LFAttributeCount; a++)
					if ((AttrProperties[a].ID) && (a!=LFAttrFileName) && (a!=LFAttrFileSize) && (a!=LFAttrFileFormat) && (a!=LFAttrCreationTime) && (a!=LFAttrFileTime))
						GetShellProperty(pParentFolder, pidlRel, AttrProperties[a].Schema, AttrProperties[a].ID, i, a);

				// Besondere Eigenschaften
				GetShellProperty(pParentFolder, pidlRel, PropertyAudio, 4, i, LFAttrBitrate);
				GetShellProperty(pParentFolder, pidlRel, PropertyAudio, 5, i, LFAttrSamplerate);
				GetShellProperty(pParentFolder, pidlRel, PropertyAudio, 7, i, LFAttrChannels);
				GetShellProperty(pParentFolder, pidlRel, PropertyAudio, 10, i, LFAttrAudioCodec);

				GetShellProperty(pParentFolder, pidlRel, PropertyPhoto, 36867, i, LFAttrRecordingTime);

				GetShellProperty(pParentFolder, pidlRel, PropertyVideo, 3, i, LFAttrWidth);
				GetShellProperty(pParentFolder, pidlRel, PropertyVideo, 4, i, LFAttrHeight);
				GetShellProperty(pParentFolder, pidlRel, PropertyVideo, 8, i, LFAttrBitrate);

				pParentFolder->Release();
			}

			FreePIDL(pidlFQ);
		}

		pDesktop->Release();
	}

	// TODO: weitere Attribute
}

void SetNameExtAddFromFile(LFItemDescriptor* i, wchar_t* fn)
{
	i->Type = (i->Type & !LFTypeMask) | LFTypeFile;

	// Name
	wchar_t Name[256];
	wchar_t* LastBackslash = wcsrchr(fn, L'\\');
	wcscpy_s(Name, 256, (!LastBackslash) ? fn : (*LastBackslash==L'\0') ? fn : LastBackslash+1);

	// Erweiterung
	wchar_t* LastExt = wcsrchr(Name, L'.');
	if (LastExt)
		if (*LastExt!='\0')
		{
			char Ext[LFExtSize] = { 0 };

			wchar_t* Ptr = LastExt+1;
			unsigned int cCount = 0;
			while ((*Ptr!=L'\0') && (cCount<LFExtSize-1))
			{
				Ext[cCount++] = (*Ptr<=255) ? tolower(*Ptr) & 0xFF : L'_';
				*Ptr++;
			}

			SetAttribute(i, LFAttrFileFormat, Ext);
			*LastExt = L'\0';

			// Bei versteckten Unix-Dateien Erweiterung als Name einsetzen
			if (Name[0]==L'\0')
				wcscpy_s(Name, 256, LastExt+1);
		}

	SetAttribute(i, LFAttrFileName, Name);

	// Added
	FILETIME ft;
	GetSystemTimeAsFileTime(&ft);
	SetAttribute(i, LFAttrAddTime, &ft);
}
