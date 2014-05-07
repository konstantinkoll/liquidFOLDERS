#pragma once
#include <shellapi.h>
#include <windows.h>


// Resource IDs from LFCORE.DLL

#include "..\\LFCore\\resource.h"


// Clipboard

#define CFSTR_LIQUIDFILES     L"liquidFOLDERS.liquidFILES"

typedef HANDLE HLIQUID;

struct LIQUIDFILES
{
	DWORD pFiles;
	UINT cFiles;
};


// Progress message

#define WM_UPDATEPROGRESS     WM_USER

#define LFProgressWorking     1
#define LFProgressError       2
#define LFProgressCancelled   3

struct LFProgress
{
	HWND hWnd;
	wchar_t Object[256];
	unsigned char ProgressState;
	unsigned int MajorCurrent;			// Starting from 0, must not exceed max(0, MajorCount-1)
	unsigned int MajorCount;			// May be 0
	unsigned int MinorCurrent;			// Starting from 0, must nox exceed max(0, MinorCount-1)
	unsigned int MinorCount;			// Must be 1 or higher
	bool UserAbort;						// Set true if aborted by user
	bool NoMinorCounter;				// Set true if thread does not wish minor progress counter to be displayed
};


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
	unsigned int VolumesChanged;
	unsigned int StatisticsChanged;
};


// Globals

#define LFKeySize                       16
#define LFKeyChars                      'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '_', '-'

#define LFExtSize                       16


// Mutex objects

#define LFCM_Store                      "Global\\LFCoreMutex_Store_"
#define LFCM_Stores                     "Global\\LFCoreMutex_Stores"


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
	char Name[31];
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
	char Name[44];
	LFGeoCoordinates Location;
};


// Item categories

#define LFItemCategoryLocalStores       0
#define LFItemCategoryRemoteStores      1
#define LFItemCategoryVolumes           2
#define LFItemCategoryNight             3

#define LFItemCategoryCount             4


// Context descriptor

struct LFItemCategoryDescriptor
{
	wchar_t Caption[256];
	wchar_t Hint[256];
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
#define LFAttrCustomer                 51
#define LFAttrLikeCount                52

#define LFAttributeCount               53
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
	wchar_t Name[256];
	wchar_t XMLID[256];
	bool AlwaysVisible;
	bool Sortable;
	bool PreferDescendingSort;
	bool ReadOnly;
	bool FormatRight;
	unsigned char Type;
	unsigned char Category;
	unsigned int RecommendedWidth;
	unsigned int cCharacters;
	LFShellProperty ShPropertyMapping;
};


// Context descriptor

#include "..\\include\\LFBitArray.h"

class LFCore_API LFAllowedAttributes : public LFBitArray
{
public:
	LFAllowedAttributes() : LFBitArray(LFAttributeCount) {};
};

struct LFContextDescriptor
{
	wchar_t Name[256];
	wchar_t Comment[256];
	bool AllowGroups;
	LFAllowedAttributes AllowedAttributes;
};


// Statistics

struct LFStatistics
{
	unsigned int FileCount[LFLastQueryContext+1];
	__int64 FileSize[LFLastQueryContext+1];
	unsigned int LastError;
};


// Search filter

#define LFFilterModeStores              1
#define LFFilterModeDirectoryTree       2
#define LFFilterModeSearch              3

#define LFFilterCompareIgnore           0
#define LFFilterCompareIsNull           1
#define LFFilterCompareSubfolder        2
#define LFFilterCompareIsEqual          3
#define LFFilterCompareIsNotEqual       4
#define LFFilterCompareIsAboveOrEqual   5
#define LFFilterCompareBeginsWith       6	// Strings
#define LFFilterCompareIsBelowOrEqual   7
#define LFFilterCompareEndsWith         8	// Strings
#define LFFilterCompareContains         9	// Strings

#define LFFilterCompareCount            10


struct LFFilterOptions
{
	// For LFFilterModeStores
	bool AddVolumes;						// If true, volumes are added

	// For LFFilterModeDirectoryTree and above
	bool IgnoreSlaves;						// If true, only core properties are retrieved
	bool IsSubfolder;						// If true, you are already inside a grouped subdirectory
	bool IsPersistent;						// If true, the filter is a custom search filter

	// For subfolders
	unsigned int GroupAttribute;			// Attribute on which parent folder was grouped
};

struct LFFilterCondition
{
	LFFilterCondition* Next;
	LFVariantData AttrData;					// Never use for LFAttrDesciption or LFAttrStoreID
	unsigned char Compare;
};

struct LFFilter
{
	wchar_t OriginalName[256];
	wchar_t ResultName[256];
	unsigned int Mode;
	LFFilterOptions Options;

	char StoreID[LFKeySize];				// For LFFilterModeDirectoryTree and above
	unsigned char ContextID;				// For LFFilterModeDirectoryTree and above
	wchar_t Searchterm[256];				// For LFFilterModeDirectoryTree and above
	LFFilterCondition* ConditionList;		// For LFFilterModeDirectoryTree and above
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
	unsigned char ContextID;
};


// Souces

#define LFSourceCount                   13


// Item structure

#define LFTypeSourceUnknown             0x00000000	// Must be lowest bits
#define LFTypeSourceInternal            0x00000001
#define LFTypeSourceNTFS                0x00000002
#define LFTypeSource1394                0x00000003
#define LFTypeSourceUSB                 0x00000004
#define LFTypeSourceDropbox             0x00000005
#define LFTypeSourceFacebook            0x00000006
#define LFTypeSourceFlickr              0x00000007
#define LFTypeSourceInstagram           0x00000008
#define LFTypeSourcePinterest           0x00000009
#define LFTypeSourceSoundCloud          0x0000000A
#define LFTypeSourceTwitter             0x0000000B
#define LFTypeSourceYouTube             0x0000000C
#define LFTypeSourceMask                0x0000000F

#define LFTypeDefault                   0x01000000	// Volatile
#define LFTypeNotMounted                0x02000000
#define LFTypeGhosted                   0x04000000
#define LFTypeShortcutAllowed           0x08000000

#define LFTypeVolume                    0x00000000	// Volatile
#define LFTypeStore                     0x10000000
#define LFTypeFile                      0x20000000
#define LFTypeFolder                    0x30000000
#define LFTypeOther                     0xF0000000
#define LFTypeMask                      0xF0000000

#define LFFlagTrash                     0x0001		// Persistent, DO NOT CHANGE
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
	unsigned int AggregateCount;
	bool DeleteFlag;
	unsigned int RefCount;
	void* Slave;
};


// Store structure

#define LFStoreModeIndexInternal       0x00
#define LFStoreModeIndexHybrid         0x01
#define LFStoreModeIndexExternal       0x02
#define LFStoreModeIndexMask           0x0F

#define LFStoreModeBackendInternal     0x00000000
#define LFStoreModeBackendMask         0xF0000000

#define LFStoreFlagAutoLocation        1
#define LFStoreFlagUnchecked           2

struct LFStoreDescriptor
{
	char StoreID[LFKeySize];
	wchar_t StoreName[256];
	wchar_t LastSeen[256];
	wchar_t StoreComment[256];
	int Mode;
	GUID guid;
	unsigned int Flags;
	FILETIME CreationTime;
	FILETIME FileTime;
	FILETIME MaintenanceTime;
	unsigned int IndexVersion;
	wchar_t DatPath[MAX_PATH];
	FILETIME SynchronizeTime;
	wchar_t IdxPathMain[MAX_PATH];				// Volatile
	wchar_t IdxPathAux[MAX_PATH];				// Volatile
	unsigned int Source;						// Volatile
	unsigned int FileCount[32];					// Volatile
	__int64 FileSize[32];						// Volatile
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
#define LFCannotImportFile              23
#define LFCannotDeleteFile              24
#define LFCannotRenameFile              25
#define LFCannotCopyIndex               26
#define LFNoFileBody                    27


// Structures and classes from LFCore.dll

#include "..\\LFCore\\LFFileIDList.h"
#include "..\\LFCore\\LFFileImportList.h"
#include "..\\LFCore\\LFMaintenanceList.h"
#include "..\\LFCore\\LFSearchResult.h"
#include "..\\LFCore\\LFTransactionList.h"
