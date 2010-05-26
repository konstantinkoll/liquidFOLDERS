#include "StdAfx.h"
#include "IdxTables.h"
#include "ShellProperties.h"

static const GUID PropertyStorage =
	{ 0xb725f130, 0x47ef, 0x101a, { 0xa5, 0xf1, 0x02, 0x60, 0x8c, 0x9e, 0xeb, 0xac } };
static const GUID PropertyQuery =
	{ 0x49691c90, 0x7e17, 0x101a, { 0xa9, 0x1c, 0x08, 0x00, 0x2b, 0x2e, 0xcd, 0xa9 } };
static const GUID PropertySummary =
	{ 0xf29f85e0, 0x4ff9, 0x1068, { 0xab, 0x91, 0x08, 0x00, 0x2b, 0x27, 0xb3, 0xd9 } };

// Der Inhalt dieses Segments wird über alle Instanzen von LFCore geteilt.
// Der Zugriff muss daher über Mutex-Objekte serialisiert/synchronisiert werden.
// Alle Variablen im Segment müssen initalisiert werden !

#pragma data_seg("common_shprop")

unsigned char DomainSlaves[LFDomainCount] = {
	IDMaster,							// LFDomainAllFiles
	IDMaster,							// LFDomainAllMediaFiles
	IDMaster,							// LFDomainFavorites
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
	IDSlaveDocuments,					// LFDomainWeb
	IDMaster,							// LFDomainTrash
	IDMaster							// LFDomainUnknown
};

#include "Registry.h"

LFShellProperty AttrProperties[LFAttributeCount] = {
	{ PropertyStorage, 10 },		// LFAttrFileName
	{ 0, 0 },						// LFAttrStoreID
	{ 0, 0 },						// LFAttrFileID
	{ PropertySummary, 6 },			// LFAttrComment
	{ 0, 0 },						// LFAttrHint
	{ PropertyStorage, 15 },		// LFAttrCreationTime
	{ PropertyStorage, 14 },		// LFAttrFileTime
	{ 0, 0 },						// LFAttrFileFormat
	{ PropertyStorage, 12 },		// LFAttrFileSize
	{ 0, 0 },						// LFAttrFlags
	{ PropertyQuery, 9 },			// LFAttrURL
	{ 0, 0 },						// LFAttrTags
	{ 0, 0 },						// LFAttrRating
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
	{ 0, 0 },						// LFAttrDoneTime
};

#pragma data_seg()
#pragma comment(linker, "/SECTION:common_shprop,RWS")


LFItemDescriptor* GetItemDescriptorForFile(wchar_t* fn, LFItemDescriptor* i)
{
	if (!i)
		i = LFAllocItemDescriptor();
	i->Type = (i->Type & (!LFTypeMask)) | LFTypeFile;

	// Name
	wchar_t* LastBackslash = wcsrchr(fn, '\\');
	if (*LastBackslash=='\0')
	{
		LastBackslash = fn;
	}
	else
	{
		LastBackslash++;
	}

	wchar_t Name[256];
	wcscpy_s(Name, 256, LastBackslash);

	// Erweiterung
	wchar_t* LastExt = wcsrchr(Name, '.');
	if (*LastExt!='\0')
	{
		char Ext[16] = { 0 };

		wchar_t* Ptr = LastExt+1;
		unsigned int cCount = 0;
		while ((*Ptr!='\0') && (cCount<16))
		{
			Ext[cCount++] = (*Ptr<255) ? tolower(*Ptr) & 0xFF : '_';
			*Ptr++;
		}

		SetAttribute(i, LFAttrFileFormat, Ext);
		*LastExt = '\0';
	}

	SetAttribute(i, LFAttrFileName, Name);

	// Attribute des Dateisystems
	WIN32_FIND_DATAW ffd;
	HANDLE hFind = FindFirstFileW(fn, &ffd);

	if (hFind!=INVALID_HANDLE_VALUE)
	{
		SetAttribute(i, LFAttrCreationTime, &ffd.ftCreationTime);
		SetAttribute(i, LFAttrFileTime, &ffd.ftLastWriteTime);
		__int64 size = (((__int64)ffd.nFileSizeHigh) << 32)+ffd.nFileSizeLow;
		SetAttribute(i, LFAttrFileSize, &size);
	}

	FindClose(hFind);

	return i;
}
