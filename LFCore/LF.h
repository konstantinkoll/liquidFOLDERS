
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

#pragma pack(push,1)

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

#pragma pack(push,1)

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

#define LFItemCategoryLocal       0
#define LFItemCategoryRemote      1
#define LFItemCategoryNight       2

#define LFItemCategoryCount       3


// Context descriptor

struct LFItemCategoryDescriptor
{
	WCHAR Caption[256];
	WCHAR Hint[256];
};


// Contexts

#define LFContextAllFiles               0	// All views
#define LFContextFavorites              1	// All views
#define LFContextAudio                  2	// All views
#define LFContextPictures               3	// All views
#define LFContextVideos                 4	// All views
#define LFContextDocuments              5	// All views
#define LFContextContacts               6	// All views
#define LFContextMessages               7	// All views
#define LFContextEvents                 8	// All views
#define LFContextNew                    9	// Only limited views
#define LFContextArchive               10	// Only limited views
#define LFContextTrash                 11	// Only limited views
#define LFContextFilters               12	// Only limites views
#define LFContextSearch                13	// All views
#define LFContextStores                14	// Only limited views, no preview
#define LFContextClipboard             15	// Only limited views
#define LFContextSubfolderDefault      16	// Only limited views
#define LFContextSubfolderDay          17	// Only limited views
#define LFContextSubfolderLocation     18	// Only limited views

#define LFLastGroupContext              8
#define LFLastQueryContext             12
#define LFContextCount                 19

#define LFContextAuto                  0xFF	// Internal use only


// Attributes

#define LFAttrFileName                  0
#define LFAttrStoreID                   1
#define LFAttrFileID                    2
#define LFAttrComments                  3
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
#define LFAttrHashtags                 15
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
#define LFAttrGenre                    32
#define LFAttrChannels                 33
#define LFAttrSamplerate               34
#define LFAttrAudioCodec               35

#define LFAttrDuration                 36
#define LFAttrBitrate                  37

#define LFAttrArtist                   38
#define LFAttrTitle                    39
#define LFAttrCopyright                40
#define LFAttrISBN                     41
#define LFAttrLanguage                 42
#define LFAttrPages                    43
#define LFAttrRecordingTime            44
#define LFAttrEquipment                45
#define LFAttrSignature                46

#define LFAttrFrom                     47
#define LFAttrTo                       48
#define LFAttrResponsible              49
#define LFAttrDueTime                  50
#define LFAttrDoneTime                 51
#define LFAttrCustomer                 52

#define LFAttributeCount               53
#define LFLastCoreAttribute            20


// Attribute types

#define LFTypeUnicodeString      0
#define LFTypeUnicodeArray       1
#define LFTypeAnsiString         2
#define LFTypeFourCC             3
#define LFTypeRating             4
#define LFTypeUINT               5
#define LFTypeSize               6
#define LFTypeFraction           7
#define LFTypeDouble             8
#define LFTypeFlags              9
#define LFTypeGeoCoordinates     10
#define LFTypeTime               11
#define LFTypeBitrate            12
#define LFTypeDuration           13
#define LFTypeMegapixel          14

#define LFTypeCount              15
#define LFMaxRating              10


// Variant attribute data

struct LFFraction
{
	UINT Num;
	UINT Denum;
};

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
		DWORD FourCC;
		BYTE Rating;
		UINT UINT32;
		INT64 INT64;
		LFFraction Fraction;
		DOUBLE Double;
		UINT Flags;
		LFGeoCoordinates GeoCoordinates;
		FILETIME Time;
		UINT Duration;
		UINT Bitrate;
		DOUBLE Megapixel;
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


#define LFAlwaysVisible        1
#define LFNotSortable          2
#define LFPreferDescending     4
#define LFFormatRight          16

#pragma pack(push,1)

struct LFShellProperty
{
	GUID Schema;
	INT ID;
};

struct LFAttributeProperties
{
	BYTE Type;
	UINT cCharacters;
	BYTE Category;
	BOOL ReadOnly;
	LFShellProperty ShPropertyMapping;
};

struct LFTypeProperties
{
	SIZE_T Size;
	BOOL ContainsLetters;
	BOOL Sortable;
	BOOL PreferDescendingSort;
	BOOL FormatRight;
	INT DefaultColumnWidth;
};

#pragma pack(pop)

struct LFAttributeDescriptor
{
	WCHAR Name[256];
	WCHAR XMLID[256];
	LFAttributeProperties AttrProperties;
	LFTypeProperties TypeProperties;
};


// Context descriptor

struct LFContextDescriptor
{
	WCHAR Name[256];
	WCHAR Comment[256];
	BOOL AllowGroups;
	UINT AllowedAttributes[(LFAttributeCount+31)>>5];
};

#define LFIsAttributeAllowed(CD, Attr) (CD.AllowedAttributes[Attr>>5] & 1<<(Attr & 0x1F))


// Statistics

struct LFStatistics
{
	UINT FileCount[LFLastQueryContext+1];
	INT64 FileSize[LFLastQueryContext+1];
	UINT LastError;
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
	UINT Flags;
	CHAR URL[256];
	WCHAR Hashtags[256];
	BYTE Rating;
	BYTE Priority;
	WCHAR LocationName[256];
	CHAR LocationIATA[4];
	LFGeoCoordinates LocationGPS;

	// Private
	BYTE SlaveID;
	BYTE ContextID;
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
#define LFTypeSourceDropbox        0x00000006
#define LFTypeSourceICloud         0x00000007
#define LFTypeSourceOneDrive       0x00000008
#define LFTypeSourceFacebook       0x0000000A
#define LFTypeSourceFlickr         0x0000000B
#define LFTypeSourceInstagram      0x0000000C
#define LFTypeSourcePinterest      0x0000000D
#define LFTypeSourceSoundCloud     0x0000000E
#define LFTypeSourceTwitter        0x0000000F
#define LFTypeSourceYouTube        0x00000010
#define LFTypeSourceMask           0x000000FF

#define LFTypeBadgeError           0x00000100	// Must match Windows image list
#define LFTypeBadgeDefault         0x00000200
#define LFTypeBadgeNew             0x00000300
#define LFTypeBadgeEmpty           0x00000400
#define LFTypeBadgeMask            0x00000F00

// Store capabilities
#define LFTypeShortcutAllowed      0x00001000	// Volatile
#define LFTypeSynchronizeAllowed   0x00002000
#define LFTypeCapabilitiesMask     0x00003000

// Store flags
#define LFTypeMounted              0x00100000	// Volatile
#define LFTypeMaintained           0x00400000
#define LFTypeWriteable            0x00800000

// Visual flags
#define LFTypeDefault              0x01000000	// Volatile
#define LFTypeGhosted              0x02000000

// Type
#define LFTypeStore                0x00000000	// Volatile
#define LFTypeFile                 0x40000000
#define LFTypeFolder               0x80000000
#define LFTypeMask                 0xC0000000

#define LFFlagTrash                0x0001		// Persistent, DO NOT CHANGE
#define LFFlagNew                  0x0002
#define LFFlagLink                 0x0004
#define LFFlagMissing              0x0008
#define LFFlagArchive              0x0010

#define LFMaxSlaveSize             3236			// Check if new attributes are attached
#define LFMaxStoreDataSize         sizeof(WCHAR)*MAX_PATH

struct LFItemDescriptor
{
	// Basic
	UINT Type;
	UINT CategoryID;
	UINT IconID;
	LFFilter* pNextFilter;

	// Internal use only
	INT FirstAggregate;
	INT LastAggregate;
	UINT AggregateCount;
	UINT RefCount;
	BOOL RemoveFlag;

	// Volatile attributes
	CHAR StoreID[LFKeySize];
	WCHAR Description[256];
	DOUBLE Dimension;
	DOUBLE AspectRatio;

	// Pointer to attribute values
	void* AttributeValues[LFAttributeCount];

	// Internal data from store
	BYTE StoreData[LFMaxStoreDataSize];

	// Must be last in struct in this order, as zero-filling depends on it
	LFCoreAttributes CoreAttributes;
	BYTE SlaveData[LFMaxSlaveSize];
};


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
#define LFStoreModeBackendShift          24
#define LFStoreModeBackendMask           0xFF000000

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
	UINT FileCount[LFLastQueryContext+1];		// Volatile
	INT64 FileSize[LFLastQueryContext+1];		// Volatile
};


// Transaction types

#define LFTransactionTypeAddToSearchResult     0x000
#define LFTransactionTypeResolveLocations      0x001
#define LFTransactionTypeSendTo                0x002

#define LFTransactionTypeArchive               0x100
#define LFTransactionTypePutInTrash            0x101
#define LFTransactionTypeRestore               0x102
#define LFTransactionTypeUpdate                0x103
#define LFTransactionTypeDelete                0x104

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
#define LFCannotImportFile           26
#define LFCannotDeleteFile           27
#define LFCannotRenameFile           28
#define LFCannotCopyIndex            29
#define LFNoFileBody                 30

#define LFErrorCount                 31
#define LFFirstFatalError            5
