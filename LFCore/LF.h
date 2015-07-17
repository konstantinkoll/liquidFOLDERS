
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


// Progress Message

#define WM_UPDATEPROGRESS       WM_USER

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

struct LFLicenseVersion
{
	UINT Major;
	UINT Minor;
	UINT Release;
};

struct LFLicense
{
	WCHAR PurchaseID[256];
	WCHAR ProductID[256];
	WCHAR PurchaseDate[16];				// Either DD/MM/YYYY or DD.MM.YYYY
	WCHAR Quantity[8];
	WCHAR RegName[256];
	LFLicenseVersion Version;
};


// Messages

struct LFMessageIDs
{
	UINT ItemsDropped;

	UINT StoresChanged;
	UINT StoreAttributesChanged;
	UINT DefaultStoreChanged;
	UINT VolumesChanged;
	UINT StatisticsChanged;
};


// Globals

#define LFKeySize      16
#define LFKeyChars     'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '_', '-'

#define LFExtSize      16


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


// Item categories

#define LFItemCategoryLocal       0
#define LFItemCategoryRemote      1
#define LFItemCategoryVolumes     2
#define LFItemCategoryNight       3

#define LFItemCategoryCount       4


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
#define LFAttrEquipment                44
#define LFAttrSignature                45

#define LFAttrFrom                     46
#define LFAttrTo                       47
#define LFAttrResponsible              48
#define LFAttrDueTime                  49
#define LFAttrDoneTime                 50
#define LFAttrCustomer                 51
#define LFAttrLikeCount                52

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
		struct
		{
			UINT Flags;
			UINT Mask;
		} Flags;
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


// Shell property

struct LFShellProperty
{
	GUID Schema;
	INT ID;
};


// Attribute descriptor

struct LFAttributeDescriptor
{
	WCHAR Name[256];
	WCHAR XMLID[256];
	BOOL AlwaysVisible;
	BOOL Sortable;
	BOOL PreferDescendingSort;
	BOOL ReadOnly;
	BOOL FormatRight;
	BYTE Type;
	BYTE Category;
	UINT RecommendedWidth;
	UINT cCharacters;
	LFShellProperty ShPropertyMapping;
};


// Context descriptor

struct LFContextDescriptor
{
	WCHAR Name[256];
	WCHAR Comment[256];
	BOOL AllowGroups;
	unsigned long AllowedAttributes[(LFAttributeCount+31)>>5];
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
	// For LFFilterModeStores
	BOOL AddVolumes;						// If TRUE, volumes are added

	// For LFFilterModeDirectoryTree and above
	BOOL IgnoreSlaves;						// If TRUE, only core properties are retrieved
	BOOL IsSubfolder;						// If TRUE, you are already inside a grouped subdirectory
	BOOL IsPersistent;						// If TRUE, the filter is a custom search filter

	// For subfolders
	UINT GroupAttribute;					// Attribute on which parent folder was grouped
};

struct LFFilterCondition
{
	LFFilterCondition* Next;
	LFVariantData AttrData;					// Never use for LFAttrDesciption or LFAttrStoreID
	BYTE Compare;
};

struct LFFilter
{
	WCHAR OriginalName[256];
	WCHAR ResultName[256];
	UINT Mode;
	LFFilterOptions Options;

	CHAR StoreID[LFKeySize];				// For LFFilterModeDirectoryTree and above
	BYTE ContextID;							// For LFFilterModeDirectoryTree and above
	WCHAR Searchterm[256];					// For LFFilterModeDirectoryTree and above
	LFFilterCondition* ConditionList;		// For LFFilterModeDirectoryTree and above
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

#define LFSourceCount              13


// Item structure

#define LFTypeSourceUnknown        0x00000000	// Must be lowest bits
#define LFTypeSourceInternal       0x00000001
#define LFTypeSourceWindows        0x00000002
#define LFTypeSource1394           0x00000003
#define LFTypeSourceUSB            0x00000004
#define LFTypeSourceDropbox        0x00000005
#define LFTypeSourceFacebook       0x00000006
#define LFTypeSourceFlickr         0x00000007
#define LFTypeSourceInstagram      0x00000008
#define LFTypeSourcePinterest      0x00000009
#define LFTypeSourceSoundCloud     0x0000000A
#define LFTypeSourceTwitter        0x0000000B
#define LFTypeSourceYouTube        0x0000000C
#define LFTypeSourceMask           0x000000FF

#define LFTypeDefault              0x01000000	// Volatile
#define LFTypeNotMounted           0x02000000
#define LFTypeGhosted              0x04000000
#define LFTypeShortcutAllowed      0x08000000

#define LFTypeVolume               0x00000000	// Volatile
#define LFTypeStore                0x10000000
#define LFTypeFile                 0x20000000
#define LFTypeFolder               0x30000000
#define LFTypeMask                 0xF0000000

#define LFFlagTrash                0x0001		// Persistent, DO NOT CHANGE
#define LFFlagNew                  0x0002
#define LFFlagLink                 0x0004
#define LFFlagMissing              0x0008
#define LFFlagArchive              0x0010

#define LFMaxSlaveSize             3236			// Check if new attributes are attached

struct LFItemDescriptor
{
	// Basic
	UINT Type;
	UINT CategoryID;
	UINT IconID;
	LFFilter* NextFilter;

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
#define LFStoreModeBackendNTFS           0x02000000
#define LFStoreModeBackendDropbox        0x05000000
#define LFStoreModeBackendFacebook       0x06000000
#define LFStoreModeBackendFlickr         0x07000000
#define LFStoreModeBackendInstagram      0x08000000
#define LFStoreModeBackendPinterest      0x09000000
#define LFStoreModeBackendSoundCloud     0x0A000000
#define LFStoreModeBackendTwitter        0x0B000000
#define LFStoreModeBackendYouTube        0x0C000000
#define LFStoreModeBackendShift          24
#define LFStoreModeBackendMask           0xFF000000

#define LFStoreFlagAutoLocation          1
#define LFStoreFlagUnchecked             2

struct LFStoreDescriptor
{
	CHAR StoreID[LFKeySize];
	WCHAR StoreName[256];
	WCHAR LastSeen[256];
	WCHAR StoreComment[256];
	UINT Mode;
	GUID guid;
	UINT Flags;
	FILETIME CreationTime;
	FILETIME FileTime;
	FILETIME MaintenanceTime;
	UINT IndexVersion;
	WCHAR DatPath[MAX_PATH];
	FILETIME SynchronizeTime;
	WCHAR IdxPathMain[MAX_PATH];				// Volatile
	WCHAR IdxPathAux[MAX_PATH];					// Volatile
	UINT Source;								// Volatile
	UINT FileCount[32];							// Volatile
	INT64 FileSize[32];							// Volatile
};


// Error codes

#define LFOk                         0
#define LFCancel                     1
#define LFMemoryError                2
#define LFIllegalQuery               3
#define LFIllegalStoreDescriptor     4
#define LFStoreNotFound              5
#define LFDriveNotReady              6
#define LFDriveWriteProtected        7
#define LFIllegalPhysicalPath        8
#define LFRegistryError              9
#define LFAccessError                10
#define LFIllegalKey                 11
#define LFNoDefaultStore             12
#define LFTooManyStores              13
#define LFStoreNotMounted            14
#define LFMutexError                 15
#define LFIllegalAttribute           16
#define LFIllegalItemType            17
#define LFIllegalValue               18
#define LFIndexTableLoadError        19
#define LFIndexRepairError           20
#define LFIndexAccessError           21
#define LFIndexCreateError           22
#define LFNotEnoughFreeDiscSpace     23
#define LFCannotImportFile           24
#define LFCannotDeleteFile           25
#define LFCannotRenameFile           26
#define LFCannotCopyIndex            27
#define LFNoFileBody                 28

#define LFErrorCount                 29
