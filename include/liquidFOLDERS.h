#pragma once
#include <windows.h>


// Resource IDs from LFCore.DLL

#include "..\\LFCore\\resource.h"


// BitArray

#include "..\\LFCore\\LFBitArray.h"


// License

struct LFLicenseVersion
{
	unsigned int Major;
	unsigned int Minor;
	unsigned int Release;
};

struct LFLicense
{
	wchar_t PurchaseID[256];
	wchar_t ProductID[256];
	wchar_t PurchaseDate[16];			// Either DD/MM/YYYY or DD.MM.YYYY
	wchar_t Quantity[8];
	wchar_t RegName[256];
	LFLicenseVersion Version;
};


// Messages

struct LFMessageIDs
{
	unsigned int LookChanged;

	unsigned int StoresChanged;
	unsigned int StoreAttributesChanged;
	unsigned int DefaultStoreChanged;
	unsigned int DrivesChanged;
};

#define LFMSGF_IntStores                1
#define LFMSGF_ExtHybStores             2


// Globals

#define LFKeySize                       16
#define LFKeyLength                     LFKeySize-1
#define LFKeyChars                      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '_', '-'

#define LFExtSize                       16

// Mutex objects

#define LFCM_Watchdog                   "Local\\LFWatchdogMutex"
#define LFCM_Store                      "Local\\LFCoreMutex_Store_"
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
	unsigned int ID;
	char Name[64];
};

struct LFGeoCoordinates
{
	double Latitude;
	double Longitude;
};

struct LFFraction
{
	unsigned int Num;
	unsigned int Denum;
};

struct LFAirport
{
	unsigned int CountryID;
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

#define LFViewCount                     13


// Item categories

#define LFCategoryInternalStores        0
#define LFCategoryHybridStores          1
#define LFCategoryExternalStores        2
#define LFCategoryDrives                3
#define LFCategoryStore                 4
#define LFCategoryMediaTypes            5
#define LFCategoryOtherTypes            6
#define LFCategoryHousekeeping          7

#define LFItemCategoryCount             8


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
#define LFContextHousekeeping           4
#define LFContextTrash                  5

#define LFContextCount                  6


// Context descriptor

struct LFContextDescriptor
{
	wchar_t Name[64];
	bool AllowExtendedViews;
	bool AllowGroups;
	LFBitArray* AllowedAttributes;
};


// Attributes

#define LFAttrFileName                  0
#define LFAttrStoreID                   1
#define LFAttrFileID                    2
#define LFAttrComment                   3
#define LFAttrHint                      4
#define LFAttrCreationTime              5
#define LFAttrFileTime                  6
#define LFAttrDeleteTime                7
#define LFAttrFileFormat                8
#define LFAttrFileSize                  9
#define LFAttrFlags                    10
#define LFAttrURL                      11
#define LFAttrTags                     12
#define LFAttrRating                   13
#define LFAttrPriority                 14
#define LFAttrLocationName             15
#define LFAttrLocationIATA             16
#define LFAttrLocationGPS              17

#define LFAttrHeight                   18
#define LFAttrWidth                    19
#define LFAttrResolution               20
#define LFAttrAspectRatio              21
#define LFAttrVideoCodec               22
#define LFAttrRoll                     23

#define LFAttrExposure                 24
#define LFAttrFocus                    25
#define LFAttrAperture                 26
#define LFAttrChip                     27

#define LFAttrAlbum                    28
#define LFAttrChannels                 29
#define LFAttrSamplerate               30
#define LFAttrAudioCodec               31

#define LFAttrDuration                 32
#define LFAttrBitrate                  33

#define LFAttrArtist                   34
#define LFAttrTitle                    35
#define LFAttrCopyright                36
#define LFAttrISBN                     37
#define LFAttrLanguage                 38
#define LFAttrPages                    39
#define LFAttrRecordingTime            40
#define LFAttrRecordingEquipment       41
#define LFAttrSignature                42

#define LFAttrFrom                     43
#define LFAttrTo                       44
#define LFAttrResponsible              45
#define LFAttrDueTime                  46
#define LFAttrDoneTime                 47

#define LFAttributeCount               48
#define LFLastCoreAttribute            17


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

#define LFTypeCount                    12
#define LFMaxRating                    10


// Variant attribute data

struct LFVariantData
{
	unsigned int Attr;
	unsigned char Type;
	bool IsNull;
	union
	{
		unsigned char Value;

		wchar_t UnicodeString[256];
		char AnsiString[256];
		unsigned int FourCC;
		unsigned char Rating;
		unsigned int UINT;
		__int64 INT64;
		LFFraction Fraction;
		double Double;
		struct
		{
			unsigned int Flags;
			unsigned int Mask;
		} Flags;
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


// Shell property

struct LFShellProperty
{
	GUID Schema;
	int ID;
};


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
	LFShellProperty ShPropertyMapping;
};


// Domains

#define LFDomainAllFiles                0
#define LFDomainAllMediaFiles           1
#define LFDomainFavorites               2

#define LFDomainTrash                   3
#define LFDomainUnknown                 4

#define LFDomainFilters                 5
#define LFDomainAudio                   6
#define LFDomainPhotos                  7
#define LFDomainPictures                8
#define LFDomainVideos                  9
#define LFDomainArchives               10
#define LFDomainContacts               11
#define LFDomainDocuments              12
#define LFDomainEvents                 13
#define LFDomainFonts                  14
#define LFDomainGeodata                15
#define LFDomainMessages               16
#define LFDomainPresentations          17
#define LFDomainSpreadsheets           18
#define LFDomainWeb                    19

#define LFFirstSoloDomain               3
#define LFFirstPhysicalDomain           5
#define LFDomainCount                  20


// Domain descriptor

struct LFDomainDescriptor
{
	wchar_t DomainName[64];
	wchar_t Comment[256];
	unsigned int IconID;
	unsigned int CategoryID;
	LFBitArray* ImportantAttributes;
};


// Search filter

#define LFFilterModeStores              1
#define LFFilterModeStoreHome           2
#define LFFilterModeDirectoryTree       3
#define LFFilterModeSearchInStore       4
#define LFFilterModeSearchAllStores     5

#define LFFilterTypeStores              0
#define LFFilterTypeStoreHome           1
#define LFFilterTypeSubfolder           2
#define LFFilterTypeQueryFilter         3
#define LFFilterTypeTrash               4
#define LFFilterTypeUnknownFileFormats  5
#define LFFilterTypeIllegalRequest      6
#define LFFilterTypeError               7
#define LFFilterTypeDefault             -1

struct LFFilterOptions
{
	bool AddBacklink;						// If true, backlink to higher levels of virtual directory tree is added

	// For LFFilterModeStores
	bool OnlyInternalStores;				// If true, only internal stores are added
	bool AddDrives;							// If true, drives are added
};

struct LFFilterResult
{
	SYSTEMTIME Time;
	unsigned int ItemCount;
	unsigned int FileCount;
	__int64 FileSize;
	int FilterType;
};

struct LFFilter
{
	wchar_t Name[256];
	unsigned int Mode;
	LFFilterOptions Options;

	bool UnhideAll;							// Disables the next two flags
	bool HideEmptyDrives;					// For LFFilterModeStores
	bool HideEmptyDomains;					// For LFFilterModeStoreHome

	char StoreID[LFKeySize];				// For LFFilterModeStoreHome and above
	unsigned char DomainID;					// For LFFilterModeStoreHome and above
	wchar_t Searchterm[256];				// For LFFilterModeDirectoryTree and above

	LFFilterResult Result;					// Set by the query engine
};


// Core attribute structure

struct LFCoreAttributes
{
	// Public
	wchar_t FileName[256];
	char FileID[LFKeySize];
	wchar_t Comment[256];
	FILETIME CreationTime;
	FILETIME FileTime;
	FILETIME DeleteTime;
	char FileFormat[LFExtSize];
	__int64 FileSize;
	unsigned int Flags;
	wchar_t Tags[256];
	unsigned char Rating;
	unsigned char Priority;
	char URL[256];
	wchar_t LocationName[256];
	char LocationIATA[4];
	LFGeoCoordinates LocationGPS;

	// Private
	unsigned char SlaveID;
	unsigned char DomainID;
};


// File structure

#define LFTypeDefaultStore              0x0001
#define LFTypeNotMounted                0x0002
#define LFTypeGhosted                   0x0004
#define LFTypeRequiresMaintenance       0x0008
#define LFTypeDrive                     0x0100
#define LFTypeStore                     0x0200
#define LFTypeFile                      0x0400
#define LFTypeVirtual                   0x0800
#define LFTypeMask                      0xFF00

#define LFFlagTrash                     0x0001
#define LFFlagNew                       0x0002
#define LFFlagLink                      0x0004

struct LFItemDescriptor
{
	LFFilter* NextFilter;
	unsigned int CategoryID;
	unsigned int IconID;
	unsigned int Type;

	LFCoreAttributes CoreAttributes;
	char StoreID[LFKeySize];
	wchar_t Hint[256];
	void* AttributeValues[LFAttributeCount];

	int Position;
	int FirstAggregate;
	int LastAggregate;
	bool DeleteFlag;
	unsigned int RefCount;
	void* Slave;
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
	GUID guid;
	unsigned int AutoLocation;
	FILETIME CreationTime;
	FILETIME FileTime;
	FILETIME MaintenanceTime;
	unsigned int IndexVersion;
	char DatPath[MAX_PATH];
	char IdxPathMain[MAX_PATH];					// Volatile
	char IdxPathAux[MAX_PATH];					// Volatile
	bool NeedsCheck;							// Volatile
};


// Error codes

#define LFOk                            0
#define LFCancel                        1
#define LFMemoryError                   2
#define LFIllegalQuery                  3
#define LFIllegalStoreDescriptor        4
#define LFStoreNotFound                 5
#define LFDriveNotReady                 6
#define LFDriveWriteProtected           7
#define LFIllegalPhysicalPath           8
#define LFRegistryError                 9
#define LFIllegalKey                    10
#define LFNoDefaultStore                11
#define LFTooManyStores                 12
#define LFStoreNotMounted               13
#define LFMutexError                    14
#define LFIllegalAttribute              15
#define LFIllegalItemType               16
#define LFIllegalValue                  17
#define LFIndexNotCreated               18
#define LFIndexError                    19
#define LFIndexRepairError              20
#define LFNotEnoughFreeDiscSpace        21
#define LFCannotDeleteFile              22
#define LFCannotCopyIndex               23


// Structures and classes from LFCore.DLL

#include "..\\LFCore\\LFFileImportList.h"
#include "..\\LFCore\\LFSearchResult.h"
#include "..\\LFCore\\LFTransactionList.h"
