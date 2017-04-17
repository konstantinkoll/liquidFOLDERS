
#pragma once
#include "LF.h"


// Core-ID: 0

#define IDXTABLE_MASTER     0
#define IDXTABLECOUNT       6

#define CURIDXVERSION       3


#define IDXATTRS_GROUP        (1ull<<LFAttrHashtags)
#define IDXATTRS_CORE         ((1ull<<LFAttrFileName) | (1ull<<LFAttrComments) | (1ull<<LFAttrCreationTime) | \
								(1ull<<LFAttrFileTime) | (1ull<<LFAttrAddTime) | (1ull<<LFAttrFileFormat) | \
								(1ull<<LFAttrFileSize) | (1ull<<LFAttrURL) | (1ull<<LFAttrRating) | \
								(1ull<<LFAttrPriority) | (1ull<<LFAttrLocationName) | (1ull<<LFAttrLocationIATA) | \
								(1ull<<LFAttrLocationGPS))
#define IDXATTRS_ALL          ((((UINT64)-1)<<LFLastCoreAttribute) | IDXATTRS_CORE)
#define IDXATTRS_DETAILS      ((1ull<<LFAttrFileName) | (1ull<<LFAttrComments) | (1ull<<LFAttrCreationTime) | \
								(1ull<<LFAttrFileTime) | (1ull<<LFAttrRating) | (1ull<<LFAttrPriority) | \
								(1ull<<LFAttrArtist) | (1ull<<LFAttrTitle) | (1ull<<LFAttrAlbum) | \
								(1ull<<LFAttrDuration) | (1ull<<LFAttrRecordingTime) | (1ull<<LFAttrRoll) | \
								(1ull<<LFAttrCustomer) | (1ull<<LFAttrPages) | (1ull<<LFAttrEquipment))
#define IDXATTRS_LOCATION     ((1ull<<LFAttrLocationName) | (1ull<<LFAttrLocationIATA))
#define IDXATTRS_VISUAL       ((1ull<<LFAttrFileName) | (1ull<<LFAttrComments) | (1ull<<LFAttrHashtags) | \
								(1ull<<LFAttrFileTime) | (1ull<<LFAttrRating) | (1ull<<LFAttrTitle) | \
								(1ull<<LFAttrRoll) | IDXATTRS_LOCATION)



// Slave-ID: 1

#define IDXTABLE_DOCUMENTS     1
#define IDXATTRS_DOCUMENTS     ((1ull<<LFAttrAuthor) | (1ull<<LFAttrCopyright) | (1ull<<LFAttrTitle) | \
								(1ull<<LFAttrResponsible) | (1ull<<LFAttrDueTime) | (1ull<<LFAttrDoneTime) | \
								(1ull<<LFAttrSignature) | (1ull<<LFAttrISBN) | (1ull<<LFAttrPages) | \
								(1ull<<LFAttrLanguage) | (1ull<<LFAttrCustomer))

struct DocumentAttributes
{
	WCHAR Author[256];
	WCHAR Copyright[256];
	WCHAR Title[256];
	WCHAR Responsible[256];
	FILETIME DueTime;
	FILETIME DoneTime;
	CHAR Signature[32];
	CHAR ISBN[32];
	UINT Pages;
	CHAR Language[3];
	WCHAR Customer[256];
};


// Slave-ID: 2

#define IDXTABLE_MESSAGES     2
#define IDXATTRS_MESSAGES     ((1ull<<LFAttrFrom) | (1ull<<LFAttrTo) | (1ull<<LFAttrTitle) | \
								(1ull<<LFAttrLanguage) | (1ull<<LFAttrResponsible) | (1ull<<LFAttrDueTime) | \
								(1ull<<LFAttrDoneTime))

struct MessageAttributes
{
	CHAR From[256];
	CHAR To[256];
	WCHAR Title[256];
	CHAR Language[3];
	WCHAR Responsible[256];
	FILETIME DueTime;
	FILETIME DoneTime;
};


// Slave-ID: 3

#define IDXTABLE_AUDIO     3
#define IDXATTRS_AUDIO     ((1ull<<LFAttrArtist) | (1ull<<LFAttrCopyright) | (1ull<<LFAttrTitle) | \
								(1ull<<LFAttrAlbum) | (1ull<<LFAttrAudioCodec) | (1ull<<LFAttrChannels) | \
								(1ull<<LFAttrSamplerate) | (1ull<<LFAttrDuration) | (1ull<<LFAttrBitrate) | \
								(1ull<<LFAttrRecordingTime) | (1ull<<LFAttrLanguage) | (1ull<<LFAttrGenre))

struct AudioAttributes
{
	WCHAR Artist[256];
	WCHAR Copyright[256];
	WCHAR Title[256];
	WCHAR Album[256];
	UINT AudioCodec;
	UINT Channels;
	UINT Samplerate;
	UINT Duration;
	UINT Bitrate;
	FILETIME RecordingTime;
	CHAR Language[3];
	UINT Genre;
};


// Slave-ID: 4

#define IDXTABLE_PICTURES     4
#define IDXATTRS_PICTURES     ((1ull<<LFAttrArtist) | (1ull<<LFAttrCopyright) | (1ull<<LFAttrTitle) | \
								(1ull<<LFAttrEquipment) | (1ull<<LFAttrRoll) | (1ull<<LFAttrExposure) | \
								(1ull<<LFAttrHeight) | (1ull<<LFAttrWidth) | (1ull<<LFAttrDimension) | \
								(1ull<<LFAttrAspectRatio) | (1ull<<LFAttrAperture) | (1ull<<LFAttrFocus) | \
								(1ull<<LFAttrChip) | (1ull<<LFAttrRecordingTime) | (1ull<<LFAttrLanguage) | \
								(1ull<<LFAttrCustomer))

struct PictureAttributes
{
	WCHAR Artist[256];
	WCHAR Copyright[256];
	WCHAR Title[256];
	WCHAR Equipment[256];
	WCHAR Roll[256];
	WCHAR Exposure[32];
	UINT Height;
	UINT Width;
	LFFraction Aperture;
	LFFraction Focus;
	WCHAR Chip[32];
	FILETIME RecordingTime;
	CHAR Language[3];
	WCHAR Customer[256];
};


// Slave-ID: 5

#define IDXTABLE_VIDEOS     5
#define IDXATTRS_VIDEOS     ((1ull<<LFAttrArtist) | (1ull<<LFAttrCopyright) | (1ull<<LFAttrTitle) | \
								(1ull<<LFAttrEquipment) | (1ull<<LFAttrRoll) | (1ull<<LFAttrHeight) | \
								(1ull<<LFAttrWidth) | (1ull<<LFAttrDimension) | (1ull<<LFAttrAspectRatio) | \
								(1ull<<LFAttrAudioCodec) | (1ull<<LFAttrVideoCodec) | (1ull<<LFAttrChannels) | \
								(1ull<<LFAttrSamplerate) | (1ull<<LFAttrDuration) | (1ull<<LFAttrBitrate) | \
								(1ull<<LFAttrRecordingTime) | (1ull<<LFAttrLanguage) | (1ull<<LFAttrCustomer))

struct VideoAttributes
{
	WCHAR Artist[256];
	WCHAR Copyright[256];
	WCHAR Title[256];
	WCHAR Equipment[256];
	WCHAR Roll[256];
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
	WCHAR Customer[256];
};


// Data structures

struct IdxTableEntry
{
	UINT Attr;
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
