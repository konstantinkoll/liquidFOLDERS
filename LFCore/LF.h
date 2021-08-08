
#pragma once
#include <assert.h>
#include <shellapi.h>


typedef double DOUBLE;


// Resource IDs from LFCORE.DLL

#include "..\\LFCore\\resource.h"


// Progress message

#define LFProgressWorking       1
#define LFProgressError         2
#define LFProgressCancelled     3

struct LFProgress
{
	HWND hWnd;
	BYTE ProgressState;
	WCHAR Object[256];
	UINT MajorCurrent;					// Starting from 0, must not exceed max(0, MajorCount-1)
	UINT MajorCount;					// May be 0
	UINT MinorCurrent;					// Starting from 0, must nox exceed max(0, MinorCount-1)
	UINT MinorCount;					// Must be 1 or higher
	BOOL UserAbort;						// Set TRUE if aborted by user
	BOOL NoMinorCounter;				// Set TRUE if thread does not wish minor progress counter to be displayed
};

struct LFVersion
{
	UINT Major;
	UINT Minor;
	UINT Build;
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


// IDs

#define LFKeySize     16
#define LFExtSize     16

typedef struct _ID
{
public:
	operator LPSTR() { return Key; }
	operator LPCSTR() const { return Key; }

	friend BOOL operator==(const _ID& a, const _ID& b) { return strcmp(a.Key, b.Key)==0; }
	friend BOOL operator!=(const _ID& a, const _ID& b) { return strcmp(a.Key, b.Key)!=0; }

protected:
	union
	{
		CHAR Key[LFKeySize];
		UINT64 Key64[LFKeySize/8];
	};
} ID, *LPID;

typedef const ID* LPCID;

typedef struct _STOREID : ID {} STOREID, *LPSTOREID;
typedef const STOREID* LPCSTOREID;

inline void DEFAULTSTOREID(STOREID& ID) { ID[0] = '\0'; }
inline STOREID DEFAULTSTOREID() { STOREID ID; DEFAULTSTOREID(ID); return ID; }

typedef struct _ABSOLUTESTOREID : STOREID
{
	friend BOOL operator==(const _ABSOLUTESTOREID& a, const _ABSOLUTESTOREID& b) { return (a.Key64[0]==b.Key64[0]) && (a.Key64[1]==b.Key64[1]); }
	friend BOOL operator!=(const _ABSOLUTESTOREID& a, const _ABSOLUTESTOREID& b) { return (a.Key64[0]!=b.Key64[0]) || (a.Key64[1]!=b.Key64[1]); }
} ABSOLUTESTOREID, *LPABSOLUTESTOREID;

typedef const ABSOLUTESTOREID* LPCABSOLUTESTOREID;

inline ABSOLUTESTOREID MAKEABSOLUTESTOREID(const STOREID& ID) { assert(ID[0]!='\0'); return *((LPABSOLUTESTOREID)&ID); }
inline ABSOLUTESTOREID MAKEABSOLUTESTOREID(LPCSTR pStr) { ABSOLUTESTOREID ID; strcpy_s(ID, LFKeySize, pStr); return ID; }
inline void free(LPCABSOLUTESTOREID lpcStoreIDs) { free((LPVOID)lpcStoreIDs); }

typedef struct _FILEID : ID
{
	friend BOOL operator==(const _FILEID& a, const _FILEID& b) { return (a.Key64[0]==b.Key64[0]) && (a.Key64[1]==b.Key64[1]); }
	friend BOOL operator!=(const _FILEID& a, const _FILEID& b) { return (a.Key64[0]!=b.Key64[0]) || (a.Key64[1]!=b.Key64[1]); }
} FILEID, *LPFILEID;

typedef const FILEID* LPCFILEID;


// Clipboard

#define CFSTR_LIQUIDFILES     L"liquidFOLDERS.liquidFILES"

struct LIQUIDFILEITEM
{
	ABSOLUTESTOREID StoreID;
	FILEID FileID;
};

struct LIQUIDFILES
{
	UINT32 cFiles;
	LIQUIDFILEITEM FileItems[1];
};

typedef LIQUIDFILES* LPLIQUIDFILES;
typedef const LIQUIDFILES* LPCLIQUIDFILES;
typedef HGLOBAL HLIQUIDFILES;


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
	CHAR Name[31];
};

typedef const LFCountry* LPCCOUNTRY;

struct LFAirport
{
	UINT CountryID;
	CHAR Code[4];
	CHAR MetroCode[4];
	CHAR Name[44];
	LFGeoCoordinates Location;
};

typedef const LFAirport* LPCAIRPORT;

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

typedef const LFMusicGenre* LPCMUSICGENRE;

#pragma pack(pop)


// iCloud

struct LFICloudPaths
{
	WCHAR Drive[MAX_PATH];
	WCHAR PhotoLibrary[MAX_PATH];
};


// OneDrive

struct LFOneDrivePaths
{
	WCHAR OneDrive[MAX_PATH];
	WCHAR CameraRoll[MAX_PATH];
	WCHAR Documents[MAX_PATH];
	WCHAR Pictures[MAX_PATH];
};


// Item categories

#define LFItemCategoryLocal      0
#define LFItemCategoryRemote     1

#define LFItemCategoryCount      2

struct LFItemCategoryDescriptor
{
	WCHAR Caption[256];
	WCHAR Hint[256];
};


// Contexts

typedef BYTE ITEMCONTEXT;

#define LFContextAllFiles              0
#define LFContextFilters               1
#define LFContextAudio                 2
#define LFContextPictures              3
#define LFContextVideos                4
#define LFContextDocuments             5
#define LFContextContacts              6
#define LFContextMessages              7
#define LFContextFonts                 8
#define LFContextApps                  9
#define LFContextBooks                 10
#define LFContextMovies                11
#define LFContextMusic                 12
#define LFContextPodcasts              13
#define LFContextTVShows               14
#define LFContextColorTables           15

#define LFContextFavorites             16
#define LFContextNew                   17
#define LFContextTasks                 18
#define LFContextArchive               19
#define LFContextTrash                 20

#define LFContextSearch                21
#define LFContextStores                22
#define LFContextClipboard             23
#define LFContextSubfolderDefault      24
#define LFContextSubfolderBooks        25
#define LFContextSubfolderFonts        26
#define LFContextSubfolderMessages     27
#define LFContextSubfolderMovies       28
#define LFContextSubfolderMusic        29
#define LFContextSubfolderPictures     30
#define LFContextSubfolderPodcasts     31
#define LFContextSubfolderTVShows      32
#define LFContextSubfolderVideos       33

#define LFLastPersistentContext        15
#define LFLastQueryContext             20
#define LFContextCount                 34

#define LFContextAuto                  0xFF	// Internal use only
#define LFContextRemove                LFContextAllFiles


// Attributes

typedef UINT ATTRIBUTE;
typedef INT SUBFOLDERATTRIBUTE;

#define LFAttrFileName                 0	// Core
#define LFAttrPriority                 1
#define LFAttrCreationTime             2
#define LFAttrFileTime                 3
#define LFAttrAddTime                  4
#define LFAttrDueTime                  5
#define LFAttrDoneTime                 6
#define LFAttrArchiveTime              7
#define LFAttrDeleteTime               8
#define LFAttrColor                    9
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

#define LFAttrCreator                  20	// Media
#define LFAttrMediaCollection          21
#define LFAttrSequenceInCollection     22
#define LFAttrTitle                    23
#define LFAttrReleased                 24
#define LFAttrLength                   25
#define LFAttrBitrate                  26
#define LFAttrLanguage                 27
#define LFAttrRecordingTime            28
#define LFAttrRecordingEquipment       29
#define LFAttrCopyright                30

#define LFAttrWidth                    31	// Visual media
#define LFAttrHeight                   32
#define LFAttrDimension                33
#define LFAttrAspectRatio              34
#define LFAttrFramerate                35
#define LFAttrApplication              36
#define LFAttrVideoCodec               37	// Videos
#define LFAttrExposure                 38	// Photos
#define LFAttrFocus                    39
#define LFAttrAperture                 40
#define LFAttrChip                     41

#define LFAttrChannels                 42	// Audio
#define LFAttrSamplerate               43
#define LFAttrAudioCodec               44
#define LFAttrGenre                    45	// Music

#define LFAttrPages                    46	// Documents
#define LFAttrISBN                     47	// Books
#define LFAttrSignature                48

#define LFAttrFrom                     49	// Messages
#define LFAttrTo                       50

#define LFAttrResponsible              51	// Tasks
#define LFAttrCustomer                 52

#define LFAttributeCount               53
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
#define LFTypeApplication        17
#define LFTypeYear               18
#define LFTypeFramerate          19

#define LFTypeCount              20
#define LFMaxRating              10


// Variant attribute data

inline BOOL operator==(const FILETIME& a, const FILETIME& b) { return *((UINT64*)&a)==*((UINT64*)&b); }
inline BOOL operator!=(const FILETIME& a, const FILETIME& b) { return *((UINT64*)&a)!=*((UINT64*)&b); }

struct LFFraction
{
	UINT Num;
	UINT Denum;
};

#pragma warning(push)
#pragma warning(disable: 4201)

struct LFVariantData
{
	ATTRIBUTE Attr;
	BYTE Type;
	BYTE IsNull;
	union
	{
		BYTE Value[512];

		WCHAR UnicodeString[256];
		WCHAR UnicodeArray[256];
		CHAR AnsiString[256];
		CHAR IATACode[4];
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
		UINT Genre;
		BYTE Application;

		struct
		{
			BYTE Color;
			BYTE ColorSet;
		};
	};
};

#pragma warning(pop)


// Attribute categories

#define LFAttrCategoryBasic                   0
#define LFAttrCategoryMedia                   1
#define LFAttrCategoryVisual                  2
#define LFAttrCategoryAudio                   3
#define LFAttrCategoryDocuments               4
#define LFAttrCategoryMessages                5
#define LFAttrCategoryTasks                   6
#define LFAttrCategoryGeotagging              7
#define LFAttrCategoryInternal                8

#define LFAttrCategoryCount                   9


// Attribute descriptor

#define LFMinAttributePriority                5

#define LFAttributeNameSize                   64

#define LFAttributeContextRecordCount         5

#define LFDataContainsLetters                 0x01	// For type
#define LFDataSortableInSubfolder             0x02
#define LFDataSortDescending                  0x04
#define LFDataFormatRight                     0x08

#define LFDataEditable                        0x01	// For attribute
#define LFDataShowRepresentativeThumbnail     0x02
#define LFDataAlwaysVisible                   0x04
#define LFDataTaxonomyPickGlobally            0x08

#define LFDataTaxonomy                        0x40	// For both
#define LFDataBucket                          0x80

#pragma pack(push, 1)

struct LFShellProperty
{
	LPCGUID Schema;
	INT ID;
};

struct LFAttributeProperties
{
	BYTE Type;
	BYTE Category;
	BYTE DataFlags;
	SIZE_T cCharacters;
	UINT DefaultIconID;
	UINT DefaultView;
	UINT DefaultPriority;
	UINT AlternateSort;
	LFShellProperty ShPropertyMapping;
	UINT PersistentID;
};

struct LFTypeProperties
{
	SIZE_T Size;
	INT DefaultColumnWidth;
	BYTE DataFlags;
	UINT AllowedViews;
	UINT DefaultView;
};

struct LFAttributeContextRecord
{
	UINT64 ContextSet;
	WCHAR Name[LFAttributeNameSize];
	INT IconID;
	BOOL SortDescending;
};

#pragma pack(pop)

struct LFAttributeDescriptor
{
	LFAttributeContextRecord ContextRecords[LFAttributeContextRecordCount];
	WCHAR XMLID[LFAttributeNameSize];
	LFAttributeProperties AttrProperties;
	LFTypeProperties TypeProperties;
};

typedef UINT LFAttributeList[LFAttributeCount];


// Context descriptor

#pragma pack(push, 1)

struct LFContextProperties
{
	ATTRIBUTE DefaultAttribute;
	BOOL AllowGroups;
	ITEMCONTEXT SubfolderContext;
	UINT AvailableViews;
	UINT DefaultView;
	UINT64 AvailableAttributes;
	UINT64 AdvertisedAttributes;
	UINT64 AllowMoveToContext;
};

struct LFContextDescriptor
{
	WCHAR Name[256];
	WCHAR Comment[256];
	LFContextProperties CtxProperties;
};

#pragma pack(pop)


// File colors

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


// Store colors

#define LFItemColorTrash           0x383030		// For stores only
#define LFItemColorNew             0xFF6020		// For stores only

#define LFColorCount               LFItemColorCount+2


// Applications

#define LFApplicationNone            0
#define LFApplicationBoomerang       1
#define LFApplicationCinemagraph     2
#define LFApplicationFacebook        3
#define LFApplicationHipstamatic     4
#define LFApplicationHyperlapse      5
#define LFApplicationIncrediBooth    6
#define LFApplicationInstagram       7
#define LFApplicationLabelbox        8
#define LFApplicationLayout          9
#define LFApplicationPinterest       10
#define LFApplicationRetrica         11
#define LFApplicationSnapchat        12
#define LFApplicationVintageCam      13
#define LFApplicationVSCO            14
#define LFApplicationYouTube         15
#define LFApplicationJodel           16
#define LFApplicationVimeo           17
#define LFApplicationPicFX           18
#define LFApplicationAfterlight      19
#define LFApplicationSnapseed        20
#define LFApplicationRNIFilms        21
#define LFApplicationTikTok          22

#define LFApplicationCount           23


// Search filter

#define LFFilterModeStores                0
#define LFFilterModeDirectoryTree         1
#define LFFilterModeQuery                 2

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


#pragma pack(push,1)

struct LFFilterCondition
{
	LFFilterCondition* pNext;
	LFVariantData VData;
	BYTE Compare;
};

struct LFFilterQuery
{
	BYTE Mode;

	ITEMCONTEXT Context;					// For LFFilterModeDirectoryTree and above
	BOOL IgnoreSlaves;						// If TRUE, only core properties are retrieved
	STOREID StoreID;						// For LFFilterModeDirectoryTree and above
	WCHAR SearchTerm[256];					// For LFFilterModeDirectoryTree and above
	LFFilterCondition* pConditionList;		// For LFFilterModeDirectoryTree and above
};

struct LFFilterResult
{
	WCHAR Name[256];
	ITEMCONTEXT Context;
};

struct LFFilter
{
	WCHAR Name[256];
	BOOL IsPersistent;						// If TRUE, the filter is a custom search filter
	BOOL IsSubfolder;						// If TRUE, the filter is a subfolder

	LFFilterQuery Query;
	LFFilterResult Result;
};

#pragma pack(pop)


// Sources

#define LFSourceUnknown           0
#define LFSourceInternal          1
#define LFSourceWindows           2
#define LFSource1394              3
#define LFSourceUSB               4
#define LFSourceNethood           5
#define LFSourceBox               6
#define LFSourceDropbox           7
#define LFSourceICloudDrive       8
#define LFSourceOneDrive          9
#define LFSourceICloudPhotos     10
#define LFSourceGoogleDrive      11

#define LFSourceCount            12

typedef BYTE SOURCE;


// Statistics

typedef BYTE ITEMSTATE;

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
	UINT64 ContextSet;
	ITEMCONTEXT Context;
	UINT ItemColors[LFItemColorCount];
	BYTE ItemColorSet;
	ITEMSTATE State;
	SOURCE Source;
	UINT64 Duration;
	BOOL OnlyTimebasedMediaFiles;
};


// Flags

// Item type
#define LFTypeStore                    0
#define LFTypeFile                     1
#define LFTypeFolder                   2
#define LFTypeAggregatedFolder         3

#define LFTypeMask                     0x00000003

// Badge
#define LFFlagsBadgeError              1
#define LFFlagsBadgeDefault            2
#define LFFlagsBadgeNew                3
#define LFFlagsBadgeEmpty              4

#define LFFlagsBadgeMask               0x0000001C

// Visual
#define LFFlagsItemSelected            0x00000020
#define LFFlagsGhosted                 0x00000040

// Stores
#define LFFlagsStoreMaintained         0x00000100
#define LFFlagsStoreIsDefault          0x00000200

// Capabilities
#define LFFlagsMounted                 0x00010000
#define LFFlagsCompressionAllowed      0x00020000
#define LFFlagsWriteable               0x00040000
#define LFFlagsRenameDeleteAllowed     0x00100000
#define LFFlagsShortcutAllowed         0x00200000
#define LFFlagsSynchronizeAllowed      0x00400000
#define LFFlagsExplorerAllowed         0x00800000

// Mask for stores when volume is unmounted
#define LFFlagsMaskUnmount             0xFF100000

// Mask Volume->Store
#define LFFlagsMaskVolume              0xFF070000

// Mask Store
#define LFFlagsMaskStore               0xFFF70100

// Mask Store->Item
#define LFFlagsMaskItem                0xFFF7035C

// Mask Store->Files
#define LFFlagsMaskFiles               0xFFF70000

#pragma warning(push)
#pragma warning(disable: 4201)

#define FLAGS \
	union \
	{ \
		UINT Flags; \
		struct \
		{ \
			BYTE Type :2; \
			BYTE Badge :3; \
			BYTE Visual: 2; \
			BYTE Reserved1 :1; \
			BYTE StoreFlags :2; \
			BYTE Reserved2 :6; \
			BYTE Capabilities; \
			SOURCE Source; \
		}; \
	};


// Volumes

struct VOLUME
{
	FLAGS;
	BYTE LogicalVolumeType;
};

typedef VOLUME* LPVOLUME;
typedef const VOLUME* LPCVOLUME;


// Stores

#define LFStoreIndexModeInternal     0
#define LFStoreIndexModeHybrid       1
#define LFStoreIndexModeExternal     2

#define LFStoreBackendInternal       0
#define LFStoreBackendWindows        2

#define LFStoreStateAutoLocation     0x00000001
#define LFStoreStateError            0x00000002

typedef UINT STORESTATE;

struct LFStoreDescriptor
{
	ABSOLUTESTOREID StoreID;
	WCHAR StoreName[256];
	WCHAR LastSeen[256];
	WCHAR Comments[256];
	union
	{
		UINT Mode;
		struct
		{
			BYTE IndexMode;
			BYTE Reserved1;
			BYTE Reserved2;
			BYTE Backend;
		};
	};
	GUID UniqueID;
	STORESTATE State;
	FILETIME CreationTime;
	FILETIME FileTime;
	FILETIME MaintenanceTime;
	UINT IndexVersion;
	union
	{
		WCHAR DatPath[MAX_PATH];
		WCHAR Volume;
	};
	FILETIME SynchronizeTime;

	WCHAR IdxPathMain[MAX_PATH];				// Volatile, must be first
	WCHAR IdxPathAux[MAX_PATH];					// Volatile
	FILETIME MountTime;							// Volatile
	FLAGS;										// Volatile
	LFStatistics Statistics;					// Volatile
	ULARGE_INTEGER FreeBytesAvailable;			// Volatile
	ULARGE_INTEGER TotalNumberOfBytes;			// Volatile
	ULARGE_INTEGER TotalNumberOfBytesFree;		// Volatile

	BOOL HideMainIndex;							// Private flag
	BOOL Victim;								// Private flag
};


// Attribute structures

struct LFCoreAttributes
{
	// Public
	WCHAR FileName[256];
	FILEID FileID;
	WCHAR Comments[256];
	FILETIME CreationTime;
	FILETIME AddTime;
	FILETIME FileTime;
	FILETIME DeleteTime;
	FILETIME ArchiveTime;
	CHAR FileFormat[LFExtSize];
	INT64 FileSize;
	ITEMSTATE State;
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
	ITEMCONTEXT SystemContextID;
	ITEMCONTEXT UserContextID;
	BYTE Reserved4;

	// Public extended
	FILETIME DueTime;
	FILETIME DoneTime;
};

typedef LFCoreAttributes* LPCOREATTRIBUTES;
typedef const LPCOREATTRIBUTES LPCCOREATTRIBUTES;

#pragma warning(pop)


// Item structure

#define LFItemStateTrash              0x01
#define LFItemStateNew                0x02
#define LFItemStateTask               0x04
#define LFItemStateMissing            0x08
#define LFItemStateArchive            0x10
#define LFItemStateCompressed         0x20


// Item data structure
#define LFMaxSlaveSize         3236			// Check when new attributes are added
#define LFMaxStoreDataSize     sizeof(WCHAR)*MAX_PATH

#pragma warning(push)
#pragma warning(disable: 4201)

struct LFItemDescriptor
{
	// Basic
	FLAGS;
	UINT CategoryID;
	UINT IconID;
	BOOL RemoveFlag;

	// Item aggregation
	LFFilter* pNextFilter;
	UINT AggregateCount;
	WORD AggregateColorSet;

	// Pointer to attribute values
	LPVOID AttributeValues[LFAttributeCount];

	// Volatile attributes
	ABSOLUTESTOREID StoreID;

	// Must be last in struct in this order, as zero-filling depends on it
	LFCoreAttributes CoreAttributes;

	//
	// Variables below this line are NOT zeroed out except the first WCHAR of Description!
	//

	// Item aggregation
	WCHAR Description[256];
	INT AggregateFirst;
	INT AggregateLast;

	// Internal use only
	UINT RefCount;

	// Space for additional data
	union
	{
		// For stores
		LFStoreDescriptor StoreDescriptor;

		// For files and folders
		struct
		{
			// Division of main caption and subcaption
			UINT FolderMainCaptionCount;
			UINT FolderSubcaptionStart;

			// File data
			DOUBLE Dimension;
			DOUBLE AspectRatio;
			BYTE SlaveData[LFMaxSlaveSize];
			BYTE StoreData[LFMaxStoreDataSize];
		};
	};
};

#pragma warning(pop)


// Transaction types

#define LFTransactionAddToSearchResult     0x000
#define LFTransactionResolveLocations      0x001
#define LFTransactionSendTo                0x002

#define LFTransactionArchive               0x100
#define LFTransactionPutInTrash            0x101
#define LFTransactionRecover               0x102
#define LFTransactionUpdate                0x103
#define LFTransactionUpdateTask            0x104
#define LFTransactionSetUserContext        0x105
#define LFTransactionCompress              0x106
#define LFTransactionDelete                0x107

#define LFTransactionLastReadonly          0x0FF


// Error codes

#define LFOk                         0
#define LFCancel                     1
#define LFVolumeWriteProtected       2
#define LFSharingViolation1          3
#define LFSharingViolation2          4
#define LFMemoryError                5
#define LFIllegalQuery               6
#define LFIllegalStoreDescriptor     7
#define LFStoreNotFound              8
#define LFVolumeNotReady             9
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
#define LFCannotCompressFile         32

#define LFErrorCount                 33
#define LFFirstFatalError            5
