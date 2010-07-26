#pragma once
#include <windows.h>


// Resource IDs from LFCORE.DLL

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
	unsigned int ItemsDropped;

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
#define LFCategoryRemoteStores          3
#define LFCategoryDrives                4
#define LFCategoryStore                 5
#define LFCategoryMediaTypes            6
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
#define LFContextHousekeeping           4
#define LFContextTrash                  5
#define LFContextSubfolderDefault       6
#define LFContextSubfolderDay           7
#define LFContextSubfolderLocation      8

#define LFContextCount                  9


// Context descriptor

struct LFContextDescriptor
{
	wchar_t Name[64];
	bool AllowGroups;
	LFBitArray* AllowedAttributes;
	LFBitArray* AllowedViews;
};


// Attributes

#define LFAttrFileName                  0
#define LFAttrStoreID                   1
#define LFAttrFileID                    2
#define LFAttrComment                   3
#define LFAttrDescription               4
#define LFAttrCreationTime              5
#define LFAttrFileTime                  6
#define LFAttrDeleteTime                7
#define LFAttrFileFormat                8
#define LFAttrFileCount                 9
#define LFAttrFileSize                 10
#define LFAttrFlags                    11
#define LFAttrURL                      12
#define LFAttrTags                     13
#define LFAttrRating                   14
#define LFAttrPriority                 15
#define LFAttrLocationName             16
#define LFAttrLocationIATA             17
#define LFAttrLocationGPS              18

#define LFAttrHeight                   19
#define LFAttrWidth                    20
#define LFAttrResolution               21
#define LFAttrAspectRatio              22
#define LFAttrVideoCodec               23
#define LFAttrRoll                     24

#define LFAttrExposure                 25
#define LFAttrFocus                    26
#define LFAttrAperture                 27
#define LFAttrChip                     28

#define LFAttrAlbum                    29
#define LFAttrChannels                 30
#define LFAttrSamplerate               31
#define LFAttrAudioCodec               32

#define LFAttrDuration                 33
#define LFAttrBitrate                  34

#define LFAttrArtist                   35
#define LFAttrTitle                    36
#define LFAttrCopyright                37
#define LFAttrISBN                     38
#define LFAttrLanguage                 39
#define LFAttrPages                    40
#define LFAttrRecordingTime            41
#define LFAttrRecordingEquipment       42
#define LFAttrSignature                43

#define LFAttrFrom                     44
#define LFAttrTo                       45
#define LFAttrResponsible              46
#define LFAttrDueTime                  47
#define LFAttrDoneTime                 48

#define LFAttributeCount               49
#define LFLastCoreAttribute            18


// Attribute types

#define LFTypeUnicodeString             0
#define LFTypeUnicodeArray              1
#define LFTypeAnsiString                2
#define LFTypeFourCC                    3
#define LFTypeRating                    4
#define LFTypeUINT                      5
#define LFTypeINT64                     6
#define LFTypeFraction                  7
#define LFTypeDouble                    8
#define LFTypeFlags                     9
#define LFTypeGeoCoordinates           10
#define LFTypeTime                     11
#define LFTypeDuration                 12

#define LFTypeCount                    13
#define LFMaxRating                    10


// Variant attribute data

struct LFVariantData
{
	unsigned int Attr;
	unsigned char Type;
	bool IsNull;
	union
	{
		unsigned char Value[512];

		wchar_t UnicodeString[256];
		wchar_t UnicodeArray[256];
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
	unsigned int IconID;
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
#define LFFilterModeSearch              4

#define LFFilterTypeStores              0
#define LFFilterTypeStoreHome           1
#define LFFilterTypeSubfolder           2
#define LFFilterTypeQueryFilter         3
#define LFFilterTypeTrash               4
#define LFFilterTypeUnknownFileFormats  5
#define LFFilterTypeIllegalRequest      6
#define LFFilterTypeError               7
#define LFFilterTypeDefault             -1

#define LFFilterCompareIgnore           0
#define LFFilterCompareIsNull           1
#define LFFilterCompareSubfolder        2
#define LFFilterCompareIsEqual          3
#define LFFilterCompareIsNotEqual       4
#define LFFilterCompareIsAboveOrEqual   5
#define LFFilterCompareBeginsWith       5	// Strings
#define LFFilterCompareIsBelowOrEqual   6
#define LFFilterCompareEndsWith         6	// Strings
#define LFFilterCompareContains         7	// Strings


struct LFFilterOptions
{
	bool AddBacklink;						// If true, backlink to higher levels of virtual directory tree is added

	// For LFFilterModeStores
	bool OnlyInternalStores;				// If true, only internal stores are added
	bool AddDrives;							// If true, drives are added

	// For LFFilterModeDirectoryTree and above
	bool IsSubfolder;						// If true, you are already inside a grouped subdirectory
	bool IgnoreSlaves;						// If true, returns only core attributes
};

struct LFFilterResult
{
	SYSTEMTIME Time;
	unsigned int ItemCount;
	unsigned int FileCount;
	__int64 FileSize;
	int FilterType;
};

struct LFFilterCondition
{
	LFFilterCondition* Next;
	LFVariantData AttrData;					// Never use for LFAttrDesciption or LFAttrStoreID
	unsigned char Compare;
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
	unsigned char DomainID;					// For LFFilterModeDirectoryTree and above
	wchar_t Searchterm[256];				// For LFFilterModeDirectoryTree and above
	LFFilterCondition* ConditionList;		// For LFFilterModeDirectoryTree and above

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
	char URL[256];
	wchar_t Tags[256];
	unsigned char Rating;
	unsigned char Priority;
	wchar_t LocationName[256];
	char LocationIATA[4];
	LFGeoCoordinates LocationGPS;

	// Private
	unsigned char SlaveID;
	unsigned char DomainID;
};


// Item structure

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
#define LFFlagMissing                   0x0008

struct LFItemDescriptor
{
	LFFilter* NextFilter;
	unsigned int CategoryID;
	unsigned int IconID;
	unsigned int Type;

	LFCoreAttributes CoreAttributes;
	char StoreID[LFKeySize];
	wchar_t Description[256];
	void* AttributeValues[LFAttributeCount];

	int Position;
	int FirstAggregate;
	int LastAggregate;
	int AggregateCount;
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
#define LFIndexAccessError              19
#define LFIndexRepairError              20
#define LFNotEnoughFreeDiscSpace        21
#define LFCannotDeleteFile              22
#define LFCannotCopyIndex               23
#define LFNoFileBody                    24


// Structures and classes from LFCore.DLL

#include "..\\LFCore\\LFFileImportList.h"
#include "..\\LFCore\\LFSearchResult.h"
#include "..\\LFCore\\LFTransactionList.h"
