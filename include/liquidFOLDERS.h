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


// Item categories

#define LFItemCategoryInternalStores    0
#define LFItemCategoryHybridStores      1
#define LFItemCategoryExternalStores    2
#define LFItemCategoryRemoteStores      3
#define LFItemCategoryDrives            4
#define LFItemCategoryStore             5
#define LFItemCategoryMediaTypes        6
#define LFItemCategoryOtherTypes        7
#define LFItemCategoryHousekeeping      8
#define LFItemCategoryNight             9

#define LFItemCategoryCount             10


// Context descriptor

struct LFItemCategoryDescriptor
{
	wchar_t Caption[256];
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
	wchar_t Name[256];
	bool AllowGroups;
	LFBitArray* AllowedAttributes;
};


// Attributes

#define LFAttrFileName                  0
#define LFAttrStoreID                   1
#define LFAttrFileID                    2
#define LFAttrComment                   3
#define LFAttrDescription               4
#define LFAttrCreationTime              5
#define LFAttrFileTime                  6
#define LFAttrAddTime                   7
#define LFAttrDeleteTime                8
#define LFAttrArchiveTime               9
#define LFAttrFileFormat               10
#define LFAttrFileCount                11
#define LFAttrFileSize                 12
#define LFAttrFlags                    13
#define LFAttrURL                      14
#define LFAttrTags                     15
#define LFAttrRating                   16
#define LFAttrPriority                 17
#define LFAttrLocationName             18
#define LFAttrLocationIATA             19
#define LFAttrLocationGPS              20

#define LFAttrWidth                    21
#define LFAttrHeight                   22
#define LFAttrDimension                23
#define LFAttrAspectRatio              24
#define LFAttrVideoCodec               25
#define LFAttrRoll                     26

#define LFAttrExposure                 27
#define LFAttrFocus                    28
#define LFAttrAperture                 29
#define LFAttrChip                     30

#define LFAttrAlbum                    31
#define LFAttrChannels                 32
#define LFAttrSamplerate               33
#define LFAttrAudioCodec               34

#define LFAttrDuration                 35
#define LFAttrBitrate                  36

#define LFAttrArtist                   37
#define LFAttrTitle                    38
#define LFAttrCopyright                39
#define LFAttrISBN                     40
#define LFAttrLanguage                 41
#define LFAttrPages                    42
#define LFAttrRecordingTime            43
#define LFAttrRecordingEquipment       44
#define LFAttrSignature                45

#define LFAttrFrom                     46
#define LFAttrTo                       47
#define LFAttrResponsible              48
#define LFAttrDueTime                  49
#define LFAttrDoneTime                 50

#define LFAttributeCount               51
#define LFLastCoreAttribute            20


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
#define LFTypeBitrate                  12
#define LFTypeDuration                 13
#define LFTypeMegapixel                14

#define LFTypeCount                    15
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
		unsigned int Bitrate;
		double Megapixel;
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
	bool FormatRight;
	unsigned char Type;
	unsigned char Category;
	unsigned int RecommendedWidth;
	unsigned int cCharacters;
	int IconID;
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
	bool IgnoreSlaves;						// If true, returns only core attributes
	bool IsSubfolder;						// If true, you are already inside a grouped subdirectory

	// For subfolders
	unsigned int GroupAttribute;			// Attribute on which parent folder was grouped
};

struct LFFilterResult
{
	SYSTEMTIME Time;
	unsigned int ItemCount;
	unsigned int StoreCount;
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

	bool ShowEmptyDrives;					// For LFFilterModeStores
	bool ShowEmptyDomains;					// For LFFilterModeStoreHome

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
	FILETIME AddTime;
	FILETIME FileTime;
	FILETIME DeleteTime;
	FILETIME ArchiveTime;
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

#define LFTypeDefaultStore              0x00000001
#define LFTypeNotMounted                0x00000002
#define LFTypeGhosted                   0x00000004
#define LFTypeRequiresMaintenance       0x00000008

#define LFTypeDrive                     0x10000000
#define LFTypeStore                     0x20000000
#define LFTypeFile                      0x40000000
#define LFTypeVirtual                   0x80000000
#define LFTypeMask                      0xF0000000

#define LFFlagTrash                     0x0001
#define LFFlagNew                       0x0002
#define LFFlagLink                      0x0004
#define LFFlagMissing                   0x0008
#define LFFlagArchive                   0x0010

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

	// Internal use only
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
#define LFStoreModeRemote               3

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
	wchar_t DatPath[MAX_PATH];
	wchar_t IdxPathMain[MAX_PATH];				// Volatile
	wchar_t IdxPathAux[MAX_PATH];				// Volatile
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
#define LFIndexTableLoadError           18
#define LFIndexRepairError              19
#define LFIndexAccessError              20
#define LFIndexCreateError              21
#define LFNotEnoughFreeDiscSpace        22
#define LFCannotDeleteFile              23
#define LFCannotRenameFile              24
#define LFCannotCopyIndex               25
#define LFNoFileBody                    26


// Structures and classes from LFCore.dll

#include "..\\LFCore\\LFFileIDList.h"
#include "..\\LFCore\\LFFileImportList.h"
#include "..\\LFCore\\LFMaintenanceList.h"
#include "..\\LFCore\\LFSearchResult.h"
#include "..\\LFCore\\LFTransactionList.h"
