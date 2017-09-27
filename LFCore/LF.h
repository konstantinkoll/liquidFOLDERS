
#pragma once
#include <shellapi.h>


typedef double DOUBLE;


// Resource IDs from LFCORE.DLL

#include "..\\LFCore\\resource.h"


// Clipboard

#define CFSTR_LIQUIDFILES     L"liquidFOLDERS.liquidFILES"

typedef HANDLE HLIQUID;

struct LIQUIDFILES
{
	DWORD pFiles;
	UINT32 cFiles;
};


// Progress message

#define LFProgressWorking       1
#define LFProgressError         2
#define LFProgressCancelled     3

struct LFProgress
{
	HWND hWnd;
	WCHAR Object[256];
	BYTE ProgressState;
	UINT MajorCurrent;					// Starting from 0, must not exceed max(0, MajorCount-1)
	UINT MajorCount;					// May be 0
	UINT MinorCurrent;					// Starting from 0, must nox exceed max(0, MinorCount-1)
	UINT MinorCount;					// Must be 1 or higher
	BOOL UserAbort;						// Set TRUE if aborted by user
	BOOL NoMinorCounter;				// Set TRUE if thread does not wish minor progress counter to be displayed
};


// License

struct LFVersion
{
	UINT Major;
	UINT Minor;
	UINT Build;
};

struct LFLicense
{
	CHAR PurchaseID[256];
	CHAR ProductID[256];
	CHAR PurchaseDate[16];				// Either DD/MM/YYYY or DD.MM.YYYY
	CHAR Quantity[8];
	CHAR RegName[256];
	LFVersion Version;
};


// Messages

struct LFMessageIDs
{
	UINT UpdateProgress;
	UINT ItemsDropped;
	UINT StoresChanged;
	UINT StoreAttributesChanged;
	UINT DefaultStoreChanged;
	UINT StatisticsChanged;
};


// Globals

#define LFKeySize     16
#define LFExtSize     16


// Views

#define LFViewIcons        0
#define LFViewList         1
#define LFViewDetails      2
#define LFViewCalendar     3
#define LFViewTimeline     4
#define LFViewGlobe        5
#define LFViewTagcloud     6

#define LFViewCount        7


// Globe textures

#define LFTextureNone     -1
#define LFTextureAuto     0
#define LFTexture1024     1
#define LFTexture2048     2
#define LFTexture4096     3
#define LFTexture8192     4


// IATA database

struct LFGeoCoordinates
{
	DOUBLE Latitude;
	DOUBLE Longitude;
};

#pragma pack(push, 1)

struct LFCountry
{
	UINT ID;
	CHAR Name[31];
};

struct LFAirport
{
	UINT CountryID;
	CHAR Code[4];
	CHAR MetroCode[4];
	CHAR Name[44];
	LFGeoCoordinates Location;
};

#pragma pack(pop)


// Music database

#pragma pack(push, 1)

struct LFMusicGenre
{
	WCHAR Name[23];
	UINT IconID;
	BOOL Primary;
	BOOL Show;
};

#pragma pack(pop)

// OneDrive

struct LFOneDrivePaths
{
	WCHAR OneDrive[MAX_PATH];
	WCHAR CameraRoll[MAX_PATH];
	WCHAR Documents[MAX_PATH];
	WCHAR Pictures[MAX_PATH];
};


// Volumes

struct LFVolumeDescriptor
{
	BOOL Mounted;
	UINT LogicalVolumeType;
	UINT Source;
};


// Item categories

#define LFItemCategoryLocal      0
#define LFItemCategoryRemote     1
#define LFItemCategoryNight      2
#define LFItemCategory0600       3
#define LFItemCategory0700       4
#define LFItemCategory0800       5
#define LFItemCategory0900       6
#define LFItemCategory1000       7
#define LFItemCategory1100       8
#define LFItemCategory1200       9
#define LFItemCategory1300      10
#define LFItemCategory1400      11
#define LFItemCategory1500      12
#define LFItemCategory1600      13
#define LFItemCategory1700      14
#define LFItemCategory1800      15
#define LFItemCategory1900      16
#define LFItemCategory2000      17
#define LFItemCategory2100      18
#define LFItemCategory2200      19
#define LFItemCategory2300      20

#define LFItemCategoryCount     21

struct LFItemCategoryDescriptor
{
	WCHAR Caption[256];
	WCHAR Hint[256];
};


// Contexts

#define LFContextAllFiles               0
#define LFContextFavorites              1
#define LFContextAudio                  2
#define LFContextPictures               3
#define LFContextVideos                 4
#define LFContextDocuments              5
#define LFContextContacts               6
#define LFContextMessages               7
#define LFContextNew                    8
#define LFContextTasks                  9
#define LFContextArchive               10
#define LFContextTrash                 11
#define LFContextFilters               12
#define LFContextSearch                13
#define LFContextStores                14
#define LFContextClipboard             15
#define LFContextSubfolderDefault      16
#define LFContextSubfolderDay          17
#define LFContextSubfolderGenre        18
#define LFContextSubfolderArtist       19
#define LFContextSubfolderAlbum        20

#define LFLastGroupContext              7
#define LFLastQueryContext             12
#define LFContextCount                 21

#define LFContextAuto                  0xFF	// Internal use only


// Attributes

#define LFAttrFileName                  0	// Core
#define LFAttrPriority                  1
#define LFAttrCreationTime              2
#define LFAttrFileTime                  3
#define LFAttrAddTime                   4
#define LFAttrDueTime                   5
#define LFAttrDoneTime                  6
#define LFAttrArchiveTime               7
#define LFAttrDeleteTime                8
#define LFAttrColor                     9
#define LFAttrHashtags                 10
#define LFAttrRating                   11
#define LFAttrComments                 12
#define LFAttrFileFormat               13
#define LFAttrFileCount                14
#define LFAttrFileSize                 15
#define LFAttrLocationName             16
#define LFAttrLocationIATA             17
#define LFAttrLocationGPS              18
#define LFAttrURL                      19

#define LFAttrRoll                     20	// Visual
#define LFAttrWidth                    21
#define LFAttrHeight                   22
#define LFAttrDimension                23
#define LFAttrAspectRatio              24
#define LFAttrVideoCodec               25

#define LFAttrExposure                 26	// Photos
#define LFAttrFocus                    27
#define LFAttrAperture                 28
#define LFAttrChip                     29

#define LFAttrArtist                   30	// Audio
#define LFAttrAlbum                    31
#define LFAttrGenre                    32
#define LFAttrChannels                 33
#define LFAttrSamplerate               34
#define LFAttrAudioCodec               35

#define LFAttrDuration                 36	// Media
#define LFAttrBitrate                  37
#define LFAttrRecordingTime            38
#define LFAttrRecordingEquipment       39

#define LFAttrAuthor                   40	// Documents
#define LFAttrCopyright                41
#define LFAttrTitle                    42
#define LFAttrISBN                     43
#define LFAttrLanguage                 44
#define LFAttrPages                    45
#define LFAttrSignature                46

#define LFAttrFrom                     47	// Messages
#define LFAttrTo                       48

#define LFAttrResponsible              49	// Workflow
#define LFAttrCustomer                 50

#define LFAttributeCount               51
#define LFLastCoreAttribute            19


// Attribute types

#define LFTypeUnicodeString      0
#define LFTypeUnicodeArray       1
#define LFTypeAnsiString         2
#define LFTypeIATACode           3
#define LFTypeFourCC             4
#define LFTypeRating             5
#define LFTypeUINT               6
#define LFTypeSize               7
#define LFTypeFraction           8
#define LFTypeDouble             9
#define LFTypeColor              10
#define LFTypeGeoCoordinates     11
#define LFTypeTime               12
#define LFTypeBitrate            13
#define LFTypeDuration           14
#define LFTypeMegapixel          15
#define LFTypeGenre              16

#define LFTypeCount              17
#define LFMaxRating              10


// Variant attribute data

struct LFFraction
{
	UINT Num;
	UINT Denum;
};

#pragma warning(push)
#pragma warning(disable: 4201)

struct LFVariantData
{
	UINT Attr;
	BYTE Type;
	BYTE IsNull;
	union
	{
		BYTE Value[512];

		WCHAR UnicodeString[256];
		WCHAR UnicodeArray[256];
		CHAR AnsiString[256];
		CHAR IATAString[4];
		DWORD FourCC;
		BYTE Rating;
		UINT UINT32;
		INT64 INT64;
		LFFraction Fraction;
		DOUBLE Double;
		LFGeoCoordinates GeoCoordinates;
		FILETIME Time;
		UINT Duration;
		UINT Bitrate;
		DOUBLE Megapixel;

		struct
		{
			BYTE Color;
			BYTE ColorSet;
		};
	};
};

#pragma warning(pop)


// Attribute categories

#define LFAttrCategoryBasic             0
#define LFAttrCategoryTasks             1
#define LFAttrCategoryGeotags           2
#define LFAttrCategoryVisual            3
#define LFAttrCategoryPhotographic      4
#define LFAttrCategoryAudio             5
#define LFAttrCategoryTimebased         6
#define LFAttrCategoryBibliographic     7
#define LFAttrCategoryInternal          8

#define LFAttrCategoryCount             9


// Attribute descriptor

#define LFMaxAttributePriority          5

#pragma pack(push, 1)

struct LFShellProperty
{
	GUID Schema;
	INT ID;
};

struct LFAttributeProperties
{
	BYTE Type;
	SIZE_T cCharacters;
	BYTE Category;
	INT IconID;
	UINT DefaultView;
	UINT DefaultPriority;
	BOOL ReadOnly;
	BOOL AlwaysShow;
	BOOL ShowRepresentativeThumbnail;
	LFShellProperty ShPropertyMapping;
	UINT PersistentID;
};

struct LFTypeProperties
{
	SIZE_T Size;
	BOOL ContainsLetters;
	BOOL SortableSubfolder;
	BOOL DefaultDescending;
	BOOL FormatRight;
	INT DefaultColumnWidth;
	UINT AllowedViews;
	UINT DefaultView;
};

#pragma pack(pop)

struct LFAttributeDescriptor
{
	WCHAR Name[256];
	WCHAR XMLID[256];
	LFAttributeProperties AttrProperties;
	LFTypeProperties TypeProperties;
};

typedef UINT LFAttributeList[LFAttributeCount];


// Context descriptor

#pragma pack(push, 1)

struct LFContextProperties
{
	UINT DefaultAttribute;
	BOOL AllowGroups;
	BOOL ShowRepresentativeThumbnail;
	UINT AvailableViews;
	UINT DefaultView;
	UINT64 AvailableAttributes;
	UINT64 AdvertisedAttributes;
};

struct LFContextDescriptor
{
	WCHAR Name[256];
	WCHAR Comment[256];
	LFContextProperties CtxProperties;
};

#pragma pack(pop)


// Item colors

#define LFItemColorDefault         0xFFFFFF
#define LFItemColorRed             0x303BFF
#define LFItemColorOrange          0x0095FF
#define LFItemColorYellow          0x00CCFF
#define LFItemColorGreen           0x64D94C
#define LFItemColorBlue            0xFF7A00
#define LFItemColorPurple          0xE173CC
#define LFItemColorGray            0x938E8E

#define LFItemColorFadePure        0
#define LFItemColorFadeBold        1
#define LFItemColorFadeMedium      2
#define LFItemColorFadeLight       3

#define LFItemColorCount           8
#define LFItemColorFadeCount       4


// Statistics

struct LFStatistics
{
	UINT FileCount[LFLastQueryContext+1];
	INT64 FileSize[LFLastQueryContext+1];
	UINT TaskCount[LFMaxRating+1];
};

struct LFFileSummary
{
	UINT FileCount;
	INT64 FileSize;
	UINT ItemColors[LFItemColorCount];
	BYTE ItemColorSet;
	BYTE Flags;
	UINT Source;
	UINT64 Duration;
	BOOL OnlyMediaFiles;
};


// Search filter

#define LFFilterModeStores                0
#define LFFilterModeDirectoryTree         1
#define LFFilterModeSearch                2

#define LFFilterCompareIgnore             0
#define LFFilterCompareIsNull             1
#define LFFilterCompareSubfolder          2
#define LFFilterCompareIsEqual            3
#define LFFilterCompareIsNotEqual         4
#define LFFilterCompareIsAboveOrEqual     5
#define LFFilterCompareBeginsWith         6		// Nur für Strings
#define LFFilterCompareIsBelowOrEqual     7
#define LFFilterCompareEndsWith           8		// Nur für Strings
#define LFFilterCompareContains           9		// Nur für Strings

#define LFFilterCompareCount              10


struct LFFilterOptions
{
	// For LFFilterModeDirectoryTree and above
	BOOL IgnoreSlaves;						// If TRUE, only core properties are retrieved
	BOOL IsSubfolder;						// If TRUE, you are already inside a grouped subdirectory
	BOOL IsPersistent;						// If TRUE, the filter is a custom search filter

	// For subfolders
	UINT GroupAttribute;					// Attribute on which parent folder was grouped
};

struct LFFilterCondition
{
	LFFilterCondition* pNext;
	LFVariantData AttrData;					// Never use for LFAttrDesciption or LFAttrStoreID
	BYTE Compare;
};

struct LFFilter
{
	WCHAR OriginalName[256];
	WCHAR ResultName[256];
	BYTE ResultContext;
	UINT Mode;
	LFFilterOptions Options;

	CHAR StoreID[LFKeySize];				// For LFFilterModeDirectoryTree and above
	BYTE QueryContext;						// For LFFilterModeDirectoryTree and above
	WCHAR Searchterm[256];					// For LFFilterModeDirectoryTree and above
	LFFilterCondition* pConditionList;		// For LFFilterModeDirectoryTree and above
};


// Attribute structures

struct LFCoreAttributes
{
	// Public
	WCHAR FileName[256];
	CHAR FileID[LFKeySize];
	WCHAR Comments[256];
	FILETIME CreationTime;
	FILETIME AddTime;
	FILETIME FileTime;
	FILETIME DeleteTime;
	FILETIME ArchiveTime;
	CHAR FileFormat[LFExtSize];
	INT64 FileSize;
	BYTE Flags;
	BYTE Color;
	BYTE Reserved1;
	BYTE Reserved2;
	CHAR URL[256];
	WCHAR Hashtags[256];
	BYTE Rating;
	BYTE Priority;
	WCHAR LocationName[256];
	CHAR LocationIATA[4];
	UINT Reserved3;
	LFGeoCoordinates LocationGPS;

	// Private
	BYTE SlaveID;
	BYTE ContextID;

	// Public extended
	FILETIME DueTime;
	FILETIME DoneTime;
};


// Sources

#define LFSourceCount              16


// Item structure

#define LFTypeSourceUnknown        0x00000000	// Must be lowest bits
#define LFTypeSourceInternal       0x00000001
#define LFTypeSourceWindows        0x00000002
#define LFTypeSource1394           0x00000003
#define LFTypeSourceUSB            0x00000004
#define LFTypeSourceNethood        0x00000005
#define LFTypeSourceBox            0x00000006
#define LFTypeSourceDropbox        0x00000007
#define LFTypeSourceICloud         0x00000008
#define LFTypeSourceOneDrive       0x00000009
#define LFTypeSourceFacebook       0x0000000A
#define LFTypeSourceFlickr         0x0000000B
#define LFTypeSourceInstagram      0x0000000C
#define LFTypeSourcePinterest      0x0000000D
#define LFTypeSourceSoundCloud     0x0000000E
#define LFTypeSourceTwitter        0x0000000F
#define LFTypeSourceYouTube        0x00000010
#define LFTypeSourceMask           0x000000FF

#define LFTypeBadgeError           0x00000100	// Volatile, must match image list
#define LFTypeBadgeDefault         0x00000200
#define LFTypeBadgeNew             0x00000300
#define LFTypeBadgeEmpty           0x00000400
#define LFTypeBadgeMask            0x00000F00

// Store type capabilities
#define LFTypeHasDescription       0x00001000	// Volatile
#define LFTypeShortcutAllowed      0x00002000
#define LFTypeSynchronizeAllowed   0x00004000

// Store type flags
#define LFTypeMounted              0x00100000	// Volatile
#define LFTypeMaintained           0x00200000
#define LFTypeWriteable            0x00400000

// Visual type
#define LFTypeSelected             0x01000000	// Volatile
#define LFTypeDefault              0x02000000
#define LFTypeGhosted              0x04000000

// Item type
#define LFTypeStore                0x00000000	// Volatile
#define LFTypeFile                 0x40000000
#define LFTypeFolder               0x80000000
#define LFTypeMask                 0xC0000000


// Persistent item flags
#define LFFlagTrash                0x01			// Persistent, DO NOT CHANGE
#define LFFlagNew                  0x02
#define LFFlagTask                 0x04
#define LFFlagMissing              0x08
#define LFFlagArchive              0x10
#define LFFlagLink                 0x20


// Item data structure
#define LFMaxSlaveSize             3236			// Check if new attributes are attached
#define LFMaxStoreDataSize         sizeof(WCHAR)*MAX_PATH

#pragma warning(push)
#pragma warning(disable: 4201)

struct LFItemDescriptor
{
	// Basic
	UINT Type;
	UINT CategoryID;
	UINT IconID;
	BOOL RemoveFlag;

	// Item aggregation
	LFFilter* pNextFilter;
	UINT AggregateCount;
	BYTE AggregateColorSet;

	// Pointer to attribute values
	LPVOID AttributeValues[LFAttributeCount];

	// Volatile attributes
	CHAR StoreID[LFKeySize];

	// Internal data from store
	BYTE StoreData[LFMaxStoreDataSize];

	// Must be last in struct in this order, as zero-filling depends on it
	LFCoreAttributes CoreAttributes;

	//
	// Variables below this line are NOT zeroed out!
	//

	// Item aggregation
	INT AggregateFirst;
	INT AggregateLast;

	// Internal use only
	UINT RefCount;

	// Space for additional attributes
	union
	{
		struct
		{
			DOUBLE Dimension;
			DOUBLE AspectRatio;
			BYTE SlaveData[LFMaxSlaveSize];
		};

		// Item aggregation
		WCHAR Description[256];
	};
};

#pragma warning(pop)


// Store structure

#define LFStoreModeIndexInternal         0x00
#define LFStoreModeIndexHybrid           0x01
#define LFStoreModeIndexExternal         0x02
#define LFStoreModeIndexMask             0x0F

#define LFStoreModeBackendInternal       0x00000000
#define LFStoreModeBackendWindows        0x02000000
#define LFStoreModeBackendFacebook       0x09000000
#define LFStoreModeBackendFlickr         0x0A000000
#define LFStoreModeBackendInstagram      0x0B000000
#define LFStoreModeBackendPinterest      0x0C000000
#define LFStoreModeBackendSoundCloud     0x0D000000
#define LFStoreModeBackendTwitter        0x0E000000
#define LFStoreModeBackendYouTube        0x0F000000
#define LFStoreModeBackendMask           0xFF000000
#define LFStoreModeBackendShift          24

#define LFStoreFlagsAutoLocation         0x00000001
#define LFStoreFlagsError                0x00000002
#define LFStoreFlagsVictim               0x00000004
#define LFStoreFlagsMaintained           LFTypeMaintained
#define LFStoreFlagsWriteable            LFTypeWriteable

struct LFStoreDescriptor
{
	CHAR StoreID[LFKeySize];
	WCHAR StoreName[256];
	WCHAR LastSeen[256];
	WCHAR Comments[256];
	UINT Mode;
	GUID UniqueID;
	UINT Flags;
	FILETIME CreationTime;
	FILETIME FileTime;
	FILETIME MaintenanceTime;
	UINT IndexVersion;
	WCHAR DatPath[MAX_PATH];
	FILETIME SynchronizeTime;

	WCHAR IdxPathMain[MAX_PATH];				// Volatile, must be first
	WCHAR IdxPathAux[MAX_PATH];					// Volatile
	FILETIME MountTime;							// Volatile
	UINT Source;								// Volatile
	LFStatistics Statistics;					// Volatile
};


// Transaction types

#define LFTransactionTypeAddToSearchResult     0x000
#define LFTransactionTypeResolveLocations      0x001
#define LFTransactionTypeSendTo                0x002

#define LFTransactionTypeArchive               0x100
#define LFTransactionTypePutInTrash            0x101
#define LFTransactionTypeRecover               0x102
#define LFTransactionTypeUpdate                0x103
#define LFTransactionTypeUpdateTask            0x104
#define LFTransactionTypeDelete                0x105

#define LFTransactionTypeLastReadonly          0x0FF


// Error codes

#define LFOk                         0
#define LFCancel                     1
#define LFDriveWriteProtected        2
#define LFSharingViolation1          3
#define LFSharingViolation2          4
#define LFMemoryError                5
#define LFIllegalQuery               6
#define LFIllegalStoreDescriptor     7
#define LFStoreNotFound              8
#define LFDriveNotReady              9
#define LFIllegalPhysicalPath        10
#define LFRegistryError              11
#define LFNoAccessError              12
#define LFIllegalID                  13
#define LFNoDefaultStore             14
#define LFTooManyStores              15
#define LFStoreNotMounted            16
#define LFMutexError                 17
#define LFIllegalAttribute           18
#define LFIllegalItemType            19
#define LFIllegalValue               20
#define LFIndexTableLoadError        21
#define LFIndexRepairError           22
#define LFIndexAccessError           23
#define LFIndexCreateError           24
#define LFNotEnoughFreeDiscSpace     25
#define LFFileWriteProtected         26
#define LFCannotImportFile           27
#define LFCannotDeleteFile           28
#define LFCannotRenameFile           29
#define LFCannotCopyIndex            30
#define LFNoFileBody                 31

#define LFErrorCount                 32
#define LFFirstFatalError            5
