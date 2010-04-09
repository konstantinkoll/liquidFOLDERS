#pragma once


// Resource IDs from LFCore.DLL

#include "..\\LFCore\\resource.h"


// BitArray

#include "LFIndexing.h"


// Messages

struct LFMessageIDs
{
	unsigned int LookChanged;

	unsigned int StoresChanged;
	unsigned int StoreAttributesChanged;
	unsigned int DefaultStoreChanged;
};

#define LFMSGF_IntStores                1
#define LFMSGF_ExtHybStores             2


// Globals

#define LFKeySize                       16
#define LFKeyLength                     LFKeySize-1
#define LFKeyChars                      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '_', '-'


// Mutex objects

#define LFCM_Watchdog                   "Local\\LFWatchdogMutex"
#define LFCM_Stores                     "Local\\LFCoreMutex_Stores"


// Globe textures

#define LFTextureNone                   -1
#define LFTextureAuto                   0
#define LFTexture1024                   1
#define LFTexture2048                   2
#define LFTexture4096                   3
#define LFTexture8192                   4


// IATA database

struct LFCountry
{
	UINT ID;
	char Name[64];
};

struct LFGeoCoordinates
{
	double Latitude;
	double Longitude;
};

struct LFFraction
{
	UINT Num;
	UINT Denum;
};

struct LFAirport
{
	UINT CountryID;
	char Code[4];
	char MetroCode[4];
	char Name[64];
	LFGeoCoordinates Location;
};


// Application view IDs (keep in Sync with command IDs in Resource.h of StoreManager)

#define LFViewAutomatic                 0
#define LFViewLargeIcons                1
#define LFViewSmallIcons                2
#define LFViewList                      3
#define LFViewDetails                   4
#define LFViewTiles                     5
#define LFViewPreview                   6
#define LFViewCalendarYear              7
#define LFViewCalendarWeek              8
#define LFViewCalendarDay               9
#define LFViewGlobe                     10
#define LFViewTagcloud                  11
#define LFViewTimeline                  12


// Item categories

#define LFCategoryInternalStores        0
#define LFCategoryHybridStores          1
#define LFCategoryExternalStores        2
#define LFCategoryDrives                3
#define LFCategoryStore                 4
#define LFCategoryFilter                5
#define LFCategoryMultimediaTypes       6
#define LFCategoryOtherTypes            7
#define LFCategoryHousekeeping          8

#define LFItemCategoryCount             9


// Context descriptor

struct LFItemCategoryDescriptor
{
	wchar_t Name[64];
	wchar_t Hint[256];
};


// Contexts

#define LFContextStores                 0
#define LFContextStoreHome              1
#define LFContextClipboard              2
#define LFContextDefault                3
#define LFContextTrash                  4

#define LFContextCount                  5


// Context descriptor

struct LFContextDescriptor
{
	wchar_t Name[64];
	BOOL AllowExtendedViews;
	BOOL AllowGroups;
	BitArray* AllowedAttributes;
};


// Attributes

#define LFAttrFileName                  0
#define LFAttrStoreID                   1
#define LFAttrFileID                    2
#define LFAttrComment                   3
#define LFAttrHint                      4
#define LFAttrCreationTime              5
#define LFAttrFileTime                  6
#define LFAttrFileFormat                7
#define LFAttrFileSize                  8
#define LFAttrFlags                     9
#define LFAttrURL                      10
#define LFAttrTags                     11
#define LFAttrRating                   12
#define LFAttrPriority                 13
#define LFAttrLocationName             14
#define LFAttrLocationIATA             15
#define LFAttrLocationGPS              16

#define LFAttrHeight                   17
#define LFAttrWidth                    18
#define LFAttrResolution               19
#define LFAttrAspectRatio              20
#define LFAttrVideoCodec               21
#define LFAttrRoll                     22

#define LFAttrExposure                 23
#define LFAttrFocus                    24
#define LFAttrAperture                 25
#define LFAttrChip                     26

#define LFAttrAlbum                    27
#define LFAttrChannels                 28
#define LFAttrSamplerate               29
#define LFAttrAudioCodec               30

#define LFAttrDuration                 31
#define LFAttrBitrate                  32

#define LFAttrArtist                   33
#define LFAttrTitle                    34
#define LFAttrCopyright                35
#define LFAttrISBN                     36
#define LFAttrLanguage                 37
#define LFAttrPages                    38
#define LFAttrRecordingTime            39
#define LFAttrRecordingEquipment       40
#define LFAttrSignature                41

#define LFAttrFrom                     42
#define LFAttrTo                       43
#define LFAttrResponsible              44
#define LFAttrDueTime                  45
#define LFAttrDoneTime                 46

#define LFAttributeCount               47
#define LFLastLocalAttribute           16


// Attribute types

#define LFTypeUnicodeString             0
#define LFTypeAnsiString                1
#define LFTypeFourCC                    2
#define LFTypeRating                    3
#define LFTypeUINT                      4
#define LFTypeINT64                     5
#define LFTypeFraction                  6
#define LFTypeDouble                    7
#define LFTypeFlags                     8
#define LFTypeGeoCoordinates            9
#define LFTypeTime                     10
#define LFTypeDuration                 11

#define LFMaxRating                    10


// Variant attribute data

struct LFVariantData
{
	unsigned int Attr;
	unsigned char Type;
	bool IsNull;
	union
	{
		wchar_t UnicodeString[256];
		char AnsiString[256];
		unsigned int FourCC;
		unsigned char Rating;
		unsigned int UINT;
		__int64 INT64;
		LFFraction Fraction;
		double Double;
		unsigned int Flags;
		LFGeoCoordinates GeoCoordinates;
		FILETIME Time;
		unsigned int Duration;
	};
};


// Attribute categories

#define LFAttrCategoryBasic             0
#define LFAttrCategoryGeotags           1
#define LFAttrCategoryVisual            2
#define LFAttrCategoryPhotographic      3
#define LFAttrCategoryAudio             4
#define LFAttrCategoryTimebased         5
#define LFAttrCategoryBibliographic     6
#define LFAttrCategoryWorkflow          7
#define LFAttrCategoryInternal          8

#define LFAttrCategoryCount             9


// Attribute descriptor

struct LFAttributeDescriptor
{
	wchar_t Name[64];
	bool AlwaysVisible;
	bool Sortable;
	bool ReadOnly;
	unsigned char Type;
	unsigned char Category;
	unsigned int RecommendedWidth;

	unsigned int cCharacters;
};


// Domains

#define LFDomainAllFiles                0
#define LFDomainFavorites               1
#define LFDomainFilters                 2
#define LFDomainAllMultimediaFiles      3
#define LFDomainAudio                   4
#define LFDomainPictures                5
#define LFDomainPhotos                  6
#define LFDomainVideos                  7
#define LFDomainArchives                8
#define LFDomainContacts                9
#define LFDomainDocuments              10
#define LFDomainEvents                 11
#define LFDomainFonts                  12
#define LFDomainGeodata                13
#define LFDomainMessages               14
#define LFDomainPresentations          15
#define LFDomainSpreadsheets           16
#define LFDomainWeb                    17
#define LFDomainTrash                  18
#define LFDomainUnknown                19

#define LFDomainCount                  20


// Domain descriptor

struct LFDomainDescriptor
{
	wchar_t DomainName[64];
	wchar_t Hint[256];
	unsigned int IconID;
	unsigned int CategoryID;
	BitArray* ImportantAttributes;
};


// Search filter

#define LFFilterModeStores              1
#define LFFilterModeStoreHome           2
#define LFFilterModeSearchInStore       3
#define LFFilterModeSearchAllStores     4

#define LFFilterTypeStores              0
#define LFFilterTypeStoreHome           1
#define LFFilterTypeSubfolder           2
#define LFFilterTypeQueryFilter         3
#define LFFilterTypeTrash               4
#define LFFilterTypeIllegalRequest      5
#define LFFilterTypeDefault             -1

struct LFFilterResult
{
	SYSTEMTIME Time;
	UINT ItemCount;
	int FilterType;
};

struct LFFilter
{
	wchar_t Name[256];
	UINT Mode;
	bool Legacy;							// If false, backlink and drives are inserted
	bool AllowSubfolders;					// Ignored by query engine
	LFFilterResult Result;					// Set by the query engine

	char StoreID[LFKeySize];				// Valid in filter modes StoreOverview and SearchInStore
};


// View parameters

struct LFViewParameters
{
	unsigned int Mode;
	unsigned int Background;
	BOOL GrannyMode;
	BOOL ShowCategories;
	BOOL FullRowSelect;
	BOOL AlwaysSave;
	BOOL Changed;
	int ColumnOrder[LFAttributeCount];
	int ColumnWidth[LFAttributeCount];

	unsigned int SortBy;
	BOOL Descending;
	BOOL AutoDirs;

	int GlobeAngleY;
	int GlobeAngleZ;
	int GlobeZoom;
	BOOL GlobeShowBubbles;
	BOOL GlobeShowAirportNames;
	BOOL GlobeShowGPS;
	BOOL GlobeShowHints;

	BOOL TagcloudOmitRare;
	BOOL TagcloudAlphabetic;
	BOOL TagcloudUseSize;
	BOOL TagcloudUseColors;
	BOOL TagcloudUseOpacity;
};


// Attribute structures

struct LFCoreAttributes
{
	wchar_t FileName[256];
	char StoreID[LFKeySize];
	char FileID[LFKeySize];
	wchar_t Comment[256];
	FILETIME CreationTime;
	FILETIME FileTime;
	unsigned int FileFormat;
	__int64 FileSize;
	unsigned int Flags;
	wchar_t Tags[256];
	unsigned char Rating;
	unsigned char Priority;
	char URL[256];
	wchar_t LocationName[256];
	char LocationIATA[4];
	LFGeoCoordinates LocationGPS;
};

struct LFDocumentAttributes
{
	wchar_t Author[256];
	wchar_t Copyright[256];
	wchar_t Title[256];
	wchar_t Responsible[256];
	FILETIME DueTime;
	FILETIME DoneTime;
	char Signature[32];
	char ISBN[32];
	unsigned int Pages;
	char Language[3];
};

struct LFMailAttributes
{
	char From[256];
	char To[256];
	wchar_t Subject[256];
	char Language[3];
};

struct LFAudioAttributes
{
	wchar_t Artist[256];
	wchar_t Copyright[256];
	wchar_t Title[256];
	wchar_t Album[256];
	unsigned int Channels;
	unsigned int Samplerate;
	unsigned int Duration;
	unsigned int Bitrate;
	FILETIME RecordingTime;
	char Language[3];
};

struct LFPictureAttributes
{
	wchar_t Artist[256];
	wchar_t Copyright[256];
	wchar_t Title[256];
	wchar_t Equipment[256];
	wchar_t Roll[256];
	wchar_t Exposure[32];
	unsigned int Height;
	unsigned int Width;
	LFFraction Aperture;
	LFFraction Focus;
	wchar_t Chip[32];
	FILETIME RecordingTime;
	char Language[3];
};

struct LFVideoAttributes
{
	wchar_t Artist[256];
	wchar_t Copyright[256];
	wchar_t Title[256];
	wchar_t Equipment[256];
	wchar_t Roll[256];
	unsigned int Height;
	unsigned int Width;
	unsigned int AudioCodec;
	unsigned int VideoCodec;
	unsigned int Channels;
	unsigned int Samplerate;
	unsigned int Duration;
	unsigned int Bitrate;
	FILETIME RecordingTime;
	char Language[3];
};


// File structure

#define LFTypeDefaultStore              0x0001
#define LFTypeNotMounted                0x0002
#define LFTypeGhosted                   0x0004
#define LFTypeLink                      0x0008
#define LFTypeDrive                     0x0100
#define LFTypeStore                     0x0200
#define LFTypeFile                      0x0400
#define LFTypeVirtual                   0x0800
#define LFTypeMask                      0xFF00

#define LFFlagTrash                     0x0001
#define LFFlagNew                       0x0002
#define LFFlagLink                      0x0004
#define LFLastFlagBit                   2

struct LFItemDescriptor
{
	LFFilter* NextFilter;
	unsigned int IconID;
	unsigned int CategoryID;
	unsigned int Type;

	LFCoreAttributes CoreAttributes;
	wchar_t Hint[256];

	void* AttributeValues[LFAttributeCount];
	wchar_t* AttributeStrings[LFAttributeCount];

	int Position;
	bool DeleteFlag;
	unsigned int RefCount;
};


// Store structure

#define LFStoreModeInternal             0
#define LFStoreModeHybrid               1
#define LFStoreModeExternal             2

struct LFStoreDescriptor
{
	char StoreID[LFKeySize];
	wchar_t StoreName[256];
	wchar_t LastSeen[256];
	wchar_t Comment[256];
	int StoreMode;
	_GUID GUID;
	BOOL AutoLocation;
	FILETIME CreationTime;
	FILETIME FileTime;
	char DatPath[MAX_PATH];
	char IdxPathMain[MAX_PATH];					// Volatile
	char IdxPathAux[MAX_PATH];					// Volatile
};


// Error codes

#define LFOk                            0
#define LFCancel                        1
#define LFMemoryError                   2
#define LFIllegalQuery                  3
#define LFIllegalStoreDescriptor        4
#define LFStoreNotFound                 5
#define LFDriveNotReady                 6
#define LFIllegalPhysicalPath           7
#define LFRegistryError                 8
#define LFIllegalKey                    9
#define LFNoDefaultStore                10
#define LFTooManyStores                 11
#define LFStoreNotMounted               12
#define LFMutexError                    13
#define LFIllegalAttribute              14
#define LFIllegalItemType               15
#define LFIllegalValue                  16


// Structures and classes from LFCore.DLL

#include "..\\LFCore\\LFSearchResult.h"
#include "..\\LFCore\\LFTransactionList.h"
