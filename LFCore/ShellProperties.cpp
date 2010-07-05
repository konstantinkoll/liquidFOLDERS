#include "StdAfx.h"
#include "IdxTables.h"
#include "ShellProperties.h"
#include <assert.h>
#include <shlwapi.h>

static const GUID PropertyStorage =
	{ 0xb725f130, 0x47ef, 0x101a, { 0xa5, 0xf1, 0x02, 0x60, 0x8c, 0x9e, 0xeb, 0xac } };
static const GUID PropertyQuery =
	{ 0x49691c90, 0x7e17, 0x101a, { 0xa9, 0x1c, 0x08, 0x00, 0x2b, 0x2e, 0xcd, 0xa9 } };
static const GUID PropertySummary =
	{ 0xf29f85e0, 0x4ff9, 0x1068, { 0xab, 0x91, 0x08, 0x00, 0x2b, 0x27, 0xb3, 0xd9 } };
static const GUID PropertyMedia =
	{ 0x64440492, 0x4c8b, 0x11d1, { 0x8b, 0x70, 0x08, 0x00, 0x36, 0xb1, 0x1a, 0x03} };
static const GUID PropertyVersion =
	{ 0x0CEF7D53, 0xFA64, 0x11D1, { 0xA2, 0x03, 0x00, 0x00, 0xF8, 0x1F, 0xED, 0xEE } };


// Der Inhalt dieses Segments wird �ber alle Instanzen von LFCore geteilt.
// Der Zugriff muss daher �ber Mutex-Objekte serialisiert/synchronisiert werden.
// Alle Variablen im Segment m�ssen initalisiert werden !

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
	{ PropertySummary, 6 },			// LFAttrComment
	{ PropertyVersion, 3 },			// LFAttrDescription
	{ PropertyStorage, 15 },		// LFAttrCreationTime
	{ PropertyStorage, 14 },		// LFAttrFileTime
	{ 0, 0 },						// LFAttrDeleteTime
	{ PropertyStorage, 4 },			// LFAttrFileFormat
	{ PropertyStorage, 12 },		// LFAttrFileSize
	{ 0, 0 },						// LFAttrFlags
	{ PropertyQuery, 9 },			// LFAttrURL
	{ PropertySummary, 5 },			// LFAttrTags
	{ PropertyMedia, 9 },			// LFAttrRating
	{ 0, 0 },						// LFAttrPriority
	{ 0, 0 },						// LFAttrLocationName
	{ 0, 0 },						// LFAttrLocationIATA
	{ 0, 0 },						// LFAttrLocationGPS

	{ 0, 0 },						// LFAttrHeight
	{ 0, 0 },						// LFAttrWidth
	{ 0, 0 },						// LFAttrResolution
	{ 0, 0 },						// LFAttrAspectRatio
	{ 0, 0 },						// LFAttrVideoCodec
	{ 0, 0 },						// LFAttrRoll

	{ 0, 0 },						// LFAttrExposure
	{ 0, 0 },						// LFAttrFocus
	{ 0, 0 },						// LFAttrAperture
	{ 0, 0 },						// LFAttrChip

	{ 0, 0 },						// LFAttrAlbum
	{ 0, 0 },						// LFAttrChannels
	{ 0, 0 },						// LFAttrSamplerate
	{ 0, 0 },						// LFAttrAudioCodec

	{ 0, 0 },						// LFAttrDuration
	{ 0, 0 },						// LFAttrBitrate

	{ PropertySummary, 4 },			// LFAttrArtist
	{ PropertySummary, 2 },			// LFAttrTitle
	{ 0, 0 },						// LFAttrCopyright
	{ 0, 0 },						// LFAttrISBN
	{ 0, 0 },						// LFAttrLanguage
	{ 0, 0 },						// LFAttrPages
	{ 0, 0 },						// LFAttrRecordingTime
	{ 0, 0 },						// LFAttrRecordingEquipment
	{ 0, 0 },						// LFAttrSignature

	{ 0, 0 },						// LFAttrFrom
	{ 0, 0 },						// LFAttrTo
	{ 0, 0 },						// LFAttrResponsible
	{ 0, 0 },						// LFAttrDueTime
	{ 0, 0 }						// LFAttrDoneTime
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
	size_t sz = strlen(ext)+1;
	ExtW[0] = '.';
	MultiByteToWideChar(CP_ACP, 0, ext, (int)sz, &ExtW[1], (int)sz);

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

void SetAttributesFromFile(LFItemDescriptor* i, wchar_t* fn)
{
	// Attribute des Dateisystems
	WIN32_FIND_DATAW ffd;
	HANDLE hFind = FindFirstFileW(fn, &ffd);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
		SYSTEMTIME ust;
		SYSTEMTIME lst;
		FILETIME lft;
		FileTimeToSystemTime(&ffd.ftCreationTime, &ust);
		SystemTimeToTzSpecificLocalTime(NULL, &ust, &lst);
		SystemTimeToFileTime(&lst, &lft);
		FileTimeToLocalFileTime(&ffd.ftCreationTime, &lft);
		SetAttribute(i, LFAttrCreationTime, &lft);
		FileTimeToSystemTime(&ffd.ftLastWriteTime, &ust);
		SystemTimeToTzSpecificLocalTime(NULL, &ust, &lst);
		SystemTimeToFileTime(&lst, &lft);
		SetAttribute(i, LFAttrFileTime, &lft);
		__int64 size = (((__int64)ffd.nFileSizeHigh) << 32)+ffd.nFileSizeLow;
		SetAttribute(i, LFAttrFileSize, &size);
	}

	FindClose(hFind);

	// Domain und Slave
	SetFileDomainAndSlave(i);

	// TODO: weitere Attribute
}

void SetNameExtFromFile(LFItemDescriptor* i, wchar_t* fn)
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
				Ext[cCount++] = (*Ptr<255) ? tolower(*Ptr) & 0xFF : L'_';
				*Ptr++;
			}

			SetAttribute(i, LFAttrFileFormat, Ext);
			*LastExt = L'\0';

			// Bei versteckten Unix-Dateien Erweiterung als Name einsetzen
			if (Name[0]==L'\0')
				wcscpy_s(Name, 256, LastExt+1);
		}

	SetAttribute(i, LFAttrFileName, Name);
}
