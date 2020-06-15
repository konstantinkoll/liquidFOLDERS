
#pragma once
#include "LF.h"


// Core-ID: 0

#define IDXTABLE_MASTER     0
#define IDXTABLECOUNT       6

#define CURIDXVERSION       6


// Core attributes
#define IDXATTRS_CORE           ((1ull<<LFAttrFileName) | (1ull<<LFAttrComments) | (1ull<<LFAttrCreationTime) | \
								(1ull<<LFAttrFileTime) | (1ull<<LFAttrAddTime) | (1ull<<LFAttrDoneTime) | \
								(1ull<<LFAttrFileFormat) | (1ull<<LFAttrFileSize) | (1ull<<LFAttrHashtags) | \
								(1ull<<LFAttrRating) | (1ull<<LFAttrLocationName) | (1ull<<LFAttrLocationIATA) | \
								(1ull<<LFAttrLocationGPS) | (1ull<<LFAttrURL) | (1ull<<LFAttrColor))

// All attributes
#define IDXATTRS_ALL            ((((UINT64)-1)<<(LFLastCoreAttribute+1)) | IDXATTRS_CORE)

// Minimal advertised properties
#define ADVATTRS_MINIMAL        (1ull<<LFAttrFileName)

// Advertised properties for media
#define ADVATTRS_MEDIA          (ADVATTRS_MINIMAL | (1ull<<LFAttrRating) | (1ull<<LFAttrColor) | \
									(1ull<<LFAttrCreator) | (1ull<<LFAttrMediaCollection) | (1ull<<LFAttrTitle))
#define ADVATTRS_OWNMEDIA       ((ADVATTRS_MEDIA & ~(1ull<<LFAttrCreator)) | (1ull<<LFAttrRecordingTime) | \
									(1ull<<LFAttrRecordingEquipment) | (1ull<<LFAttrLocationName) | (1ull<<LFAttrLocationIATA) | \
									(1ull<<LFAttrFileTime) | (1ull<<LFAttrComments))
#define ADVATTRS_DEFAULT        (ADVATTRS_MEDIA | ADVATTRS_OWNMEDIA)


// Slave-ID: 1

#define IDXTABLE_DOCUMENTS     1

#define IDXATTRS_BOOKS         ((1ull<<LFAttrCreator) | (1ull<<LFAttrMediaCollection) | (1ull<<LFAttrTitle) | \
								(1ull<<LFAttrReleased) | (1ull<<LFAttrLanguage) | (1ull<<LFAttrCopyright) | \
								(1ull<<LFAttrISBN) | (1ull<<LFAttrPages) | (1ull<<LFAttrSignature))
#define IDXATTRS_DOCUMENTS     ((1ull<<LFAttrCreator) | (1ull<<LFAttrTitle) | (1ull<<LFAttrLanguage) | \
								(1ull<<LFAttrCopyright) | (1ull<<LFAttrPages) | (1ull<<LFAttrResponsible) | \
								(1ull<<LFAttrCustomer))
#define IDXATTRS_COLORTABLES   ((1ull<<LFAttrCreator) | (1ull<<LFAttrMediaCollection) | (1ull<<LFAttrTitle) | \
								(1ull<<LFAttrApplication) | (1ull<<LFAttrCopyright) | (1ull<<LFAttrResponsible) | \
								(1ull<<LFAttrCustomer))

#define ADVATTRS_BOOKS         (ADVATTRS_MEDIA & (IDXATTRS_CORE | IDXATTRS_BOOKS))
#define ADVATTRS_DOCUMENTS     ((ADVATTRS_OWNMEDIA & (IDXATTRS_CORE | IDXATTRS_DOCUMENTS)) | \
								(1ull<<LFAttrCreationTime) | (1ull<<LFAttrCreator) | (1ull<<LFAttrCustomer))

struct DocumentAttributes
{
	WCHAR Creator[256];
	WCHAR Copyright[256];
	WCHAR Title[256];
	WCHAR Responsible[256];
	BYTE Application;
	BYTE FREE[15];
	CHAR Signature[32];
	CHAR ISBN[32];
	UINT Pages;
	CHAR Language[3];
	WCHAR Customer[256];
	UINT PublishedYear;
	WCHAR MediaCollection[256];
};


// Slave-ID: 2

#define IDXTABLE_MESSAGES     2

#define IDXATTRS_MESSAGES     ((1ull<<LFAttrFrom) | (1ull<<LFAttrTo) | (1ull<<LFAttrTitle) | \
								(1ull<<LFAttrLanguage) | (1ull<<LFAttrResponsible))

struct MessageAttributes
{
	CHAR From[256];
	CHAR To[256];
	WCHAR Title[256];
	CHAR Language[3];
	WCHAR Responsible[256];
};


// Slave-ID: 3

#define IDXTABLE_AUDIO     3

#define IDXATTRS_AUDIO     ((1ull<<LFAttrLength) | (1ull<<LFAttrBitrate) | (1ull<<LFAttrLanguage) | \
								(1ull<<LFAttrRecordingTime) | (1ull<<LFAttrCopyright) | (1ull<<LFAttrChannels) | \
								(1ull<<LFAttrSamplerate))
#define IDXATTRS_MUSIC     ((1ull<<LFAttrCreator) | (1ull<<LFAttrMediaCollection) | (1ull<<LFAttrSequenceInCollection) | \
								(1ull<<LFAttrTitle) | (1ull<<LFAttrReleased) | (1ull<<LFAttrLength) | \
								(1ull<<LFAttrBitrate) | (1ull<<LFAttrLanguage) | (1ull<<LFAttrCopyright) | \
								(1ull<<LFAttrGenre) | (1ull<<LFAttrChannels) | (1ull<<LFAttrSamplerate))

#define ADVATTRS_AUDIO         (ADVATTRS_OWNMEDIA & (IDXATTRS_CORE | IDXATTRS_AUDIO))
#define ADVATTRS_MUSIC         ((ADVATTRS_MEDIA & (IDXATTRS_CORE | IDXATTRS_MUSIC)) | \
								(1ull<<LFAttrLength) | (1ull<<LFAttrSequenceInCollection) | (1ull<<LFAttrGenre))

struct AudioAttributes
{
	WCHAR Creator[256];
	WCHAR Copyright[256];
	WCHAR Title[256];
	WCHAR MediaCollection[256];
	UINT AudioCodec;
	UINT Channels;
	UINT Samplerate;
	UINT Duration;
	UINT Bitrate;
	FILETIME RecordingTime;
	CHAR Language[3];
	UINT Genre;
	UINT SequenceInCollection;
	UINT PublishedYear;
};


// Slave-ID: 4

#define IDXTABLE_PICTURES     4

#define IDXATTRS_PICTURES     ((1ull<<LFAttrCreator) | (1ull<<LFAttrMediaCollection) | (1ull<<LFAttrTitle) | \
								(1ull<<LFAttrLanguage) | (1ull<<LFAttrRecordingTime) | (1ull<<LFAttrRecordingEquipment) | \
								(1ull<<LFAttrCopyright) | (1ull<<LFAttrWidth) | (1ull<<LFAttrHeight) | \
								(1ull<<LFAttrDimension) | (1ull<<LFAttrAspectRatio) | (1ull<<LFAttrApplication) | \
								(1ull<<LFAttrExposure) | (1ull<<LFAttrFocus) | (1ull<<LFAttrAperture) | \
								(1ull<<LFAttrChip) | (1ull<<LFAttrCustomer))

#define ADVATTRS_PICTURES     ((ADVATTRS_OWNMEDIA & (IDXATTRS_CORE | IDXATTRS_PICTURES)) | \
								(1ull<<LFAttrHashtags) | (1ull<<LFAttrApplication))

struct PictureAttributes
{
	WCHAR Creator[256];
	WCHAR Copyright[256];
	WCHAR Title[256];
	WCHAR RecordingEquipment[256];
	WCHAR MediaCollection[256];
	WCHAR Exposure[32];
	UINT Height;
	UINT Width;
	LFFraction Aperture;
	LFFraction Focus;
	WCHAR Chip[32];
	FILETIME RecordingTime;
	CHAR Language[3];
	BYTE Application;
	WCHAR Customer[256];
};


// Slave-ID: 5

#define IDXTABLE_VIDEOS     5

#define IDXATTRS_MOVIES     ((1ull<<LFAttrCreator) | (1ull<<LFAttrTitle) | (1ull<<LFAttrReleased) | \
								(1ull<<LFAttrLength) | (1ull<<LFAttrBitrate) | (1ull<<LFAttrLanguage) | \
								(1ull<<LFAttrCopyright) | (1ull<<LFAttrWidth) | (1ull<<LFAttrHeight) | \
								(1ull<<LFAttrDimension) | (1ull<<LFAttrAspectRatio) | (1ull<<LFAttrFramerate) | \
								(1ull<<LFAttrChannels) | (1ull<<LFAttrSamplerate))
#define IDXATTRS_TVSHOWS    ((1ull<<LFAttrCreator) | (1ull<<LFAttrMediaCollection) | (1ull<<LFAttrSequenceInCollection) | \
								(1ull<<LFAttrTitle) | (1ull<<LFAttrReleased) | (1ull<<LFAttrLength) | \
								(1ull<<LFAttrBitrate) | (1ull<<LFAttrLanguage) | (1ull<<LFAttrCopyright) | \
								(1ull<<LFAttrWidth) | (1ull<<LFAttrHeight) | (1ull<<LFAttrDimension) | \
								(1ull<<LFAttrAspectRatio) | (1ull<<LFAttrFramerate) | (1ull<<LFAttrChannels) | \
								(1ull<<LFAttrSamplerate))
#define IDXATTRS_VIDEOS     ((1ull<<LFAttrCreator) | (1ull<<LFAttrMediaCollection) | (1ull<<LFAttrTitle) | \
								(1ull<<LFAttrLength) | (1ull<<LFAttrBitrate) | (1ull<<LFAttrLanguage) | \
								(1ull<<LFAttrRecordingTime) | (1ull<<LFAttrRecordingEquipment) | (1ull<<LFAttrCopyright) | \
								(1ull<<LFAttrWidth) | (1ull<<LFAttrHeight) | (1ull<<LFAttrDimension) | \
								(1ull<<LFAttrAspectRatio) | (1ull<<LFAttrFramerate) | (1ull<<LFAttrApplication) | \
								(1ull<<LFAttrVideoCodec) | (1ull<<LFAttrChannels) | (1ull<<LFAttrSamplerate) | \
								(1ull<<LFAttrAudioCodec) | (1ull<<LFAttrCustomer))

#define ADVATTRS_MOVIES       ((ADVATTRS_MEDIA & (IDXATTRS_CORE | IDXATTRS_MOVIES)) | \
								(1ull<<LFAttrLength))
#define ADVATTRS_TVSHOWS      ((ADVATTRS_MEDIA & (IDXATTRS_CORE | IDXATTRS_TVSHOWS) & ~(1ull<<LFAttrCreator)) | \
								(1ull<<LFAttrLength) | (1ull<<LFAttrSequenceInCollection))
#define ADVATTRS_VIDEOS       ((ADVATTRS_OWNMEDIA & (IDXATTRS_CORE | IDXATTRS_VIDEOS) & ~(1ull<<LFAttrCreator)) | \
								(1ull<<LFAttrHashtags) | (1ull<<LFAttrApplication) | (1ull<<LFAttrLength))

struct VideoAttributes
{
	WCHAR Creator[256];
	WCHAR Copyright[256];
	WCHAR Title[256];
	WCHAR RecordingEquipment[256];
	WCHAR MediaCollection[256];
	UINT Height;
	UINT Width;
	UINT AudioCodec;
	UINT VideoCodec;
	UINT Channels;
	UINT Samplerate;
	UINT Duration;
	UINT Bitrate;
	FILETIME RecordingTime;
	CHAR Language[3];
	BYTE Application;
	WCHAR Customer[256];
	UINT SequenceInCollection;
	UINT PublishedYear;
	UINT Framerate;
};


// Data structures

struct IdxTableEntry
{
	ATTRIBUTE Attr;
	INT_PTR Offset;
};

struct IdxTable
{
	WCHAR FileName[13];
	UINT Size;
	UINT cTableEntries;
	const IdxTableEntry* pTableEntries;
};

extern const IdxTable IndexTables[];
extern const IdxTableEntry CoreAttributeEntries[];
