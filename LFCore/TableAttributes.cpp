
#include "stdafx.h"
#include "TableAttributes.h"
#include "TableIndexes.h"


#pragma data_seg(".shared")

#define MINMEDIAVIEWS         ((1<<LFViewIcons) | (1<<LFViewList))
#define MINVIEWS              (MINMEDIAVIEWS | (1<<LFViewDetails))
#define ADVVIEWS              ((1<<LFViewCalendar) | (1<<LFViewTimeline) | (1<<LFViewGlobe) | (1<<LFViewTagcloud))
#define MEDIAVIEWS            (MINMEDIAVIEWS | ADVVIEWS)
#define ALLVIEWS              (MINVIEWS | ADVVIEWS)
#define REMOVEFROMCONTEXT     (1ull<<LFContextAllFiles)

extern const LFContextProperties CtxProperties[LFContextCount] = {
	// LFContextAllFiles
	{ LFAttrFileTime, TRUE, LFContextSubfolderDefault,
	MEDIAVIEWS, LFViewTimeline,
	IDXATTRS_ALL,
	ADVATTRS_DEFAULT | (1ull<<LFAttrApplication),
	0 },

	// LFContextFilters
	{ LFAttrFileName, FALSE, LFContextSubfolderDefault,
	(1<<LFViewIcons), LFViewIcons,
	(1ull<<LFAttrFileName) | (1ull<<LFAttrComments) | (1ull<<LFAttrCreationTime) | (1ull<<LFAttrFileTime) | (1ull<<LFAttrFileSize),
	(1ull<<LFAttrFileName) | (1ull<<LFAttrComments),
	0 },

	// LFContextAudio
	{ LFAttrFileName, TRUE, LFContextSubfolderDefault,
	MINVIEWS, LFViewIcons,
	IDXATTRS_CORE | IDXATTRS_AUDIO,
	ADVATTRS_AUDIO,
	(1ull<<LFContextMusic) | (1ull<<LFContextPodcasts) },

	// LFContextPictures
	{ LFAttrFileTime, TRUE, LFContextSubfolderPictures,
	MEDIAVIEWS, LFViewTimeline,
	IDXATTRS_CORE | IDXATTRS_PICTURES,
	ADVATTRS_PICTURES,
	0 },

	// LFContextVideos
	{ LFAttrFileTime, TRUE, LFContextSubfolderVideos,
	MEDIAVIEWS, LFViewTimeline,
	IDXATTRS_CORE | IDXATTRS_VIDEOS,
	ADVATTRS_VIDEOS,
	(1ull<<LFContextMovies) | (1ull<<LFContextMusic) | (1ull<<LFContextPodcasts) | (1ull<<LFContextTVShows) },

	// LFContextDocuments
	{ LFAttrFileTime, TRUE, LFContextSubfolderDefault,
	MEDIAVIEWS, LFViewTimeline,
	IDXATTRS_CORE | IDXATTRS_DOCUMENTS,
	ADVATTRS_DOCUMENTS,
	(1ull<<LFContextBooks) },

	// LFContextContacts
	{ LFAttrFileName, TRUE, LFContextSubfolderDefault,
	MINMEDIAVIEWS, LFViewIcons,
	IDXATTRS_CORE,
	ADVATTRS_MINIMAL,
	0 },

	// LFContextMessages
	{ LFAttrFileName, TRUE, LFContextSubfolderMessages,
	(1<<LFViewList) | (1<<LFViewTagcloud), LFViewList,
	IDXATTRS_CORE | IDXATTRS_MESSAGES,
	ADVATTRS_MINIMAL | (1ull<<LFAttrTitle) | (1ull<<LFAttrFrom) | (1ull<<LFAttrTo),
	0 },

	// LFContextFonts
	{ LFAttrFileName, TRUE, LFContextSubfolderFonts,
	MINVIEWS, LFViewIcons,
	IDXATTRS_CORE,
	ADVATTRS_MINIMAL | (1ull<<LFAttrRating),
	0 },

	// LFContextApps
	{ LFAttrFileName, TRUE, LFContextSubfolderDefault,
	MINVIEWS, LFViewIcons,
	IDXATTRS_CORE,
	ADVATTRS_MINIMAL | (1ull<<LFAttrFileSize),
	0 },

	// LFContextBooks
	{ LFAttrFileName, TRUE, LFContextSubfolderBooks,
	MINMEDIAVIEWS, LFViewIcons,
	IDXATTRS_CORE | IDXATTRS_BOOKS,
	ADVATTRS_BOOKS,
	(1ull<<LFContextDocuments) },

	// LFContextMovies
	{ LFAttrFileName, TRUE, LFContextSubfolderMovies,
	MINMEDIAVIEWS, LFViewIcons,
	IDXATTRS_CORE | IDXATTRS_MOVIES,
	ADVATTRS_MOVIES,
	(1ull<<LFContextTVShows) | REMOVEFROMCONTEXT },

	// LFContextMusic
	{ LFAttrGenre, TRUE, LFContextSubfolderMusic,
	MINMEDIAVIEWS, LFViewIcons,
	IDXATTRS_CORE | IDXATTRS_MUSIC,
	ADVATTRS_MUSIC,
	REMOVEFROMCONTEXT },

	// LFContextPodcasts
	{ LFAttrMediaCollection, TRUE, LFContextSubfolderPodcasts,
	MINMEDIAVIEWS, LFViewList,
	IDXATTRS_CORE | IDXATTRS_TVSHOWS,
	ADVATTRS_TVSHOWS,
	REMOVEFROMCONTEXT },

	// LFContextTVShows
	{ LFAttrMediaCollection, TRUE, LFContextSubfolderTVShows,
	MINMEDIAVIEWS, LFViewList,
	IDXATTRS_CORE | IDXATTRS_TVSHOWS,
	ADVATTRS_TVSHOWS,
	(1ull<<LFContextMovies) | REMOVEFROMCONTEXT },

	// LFContextColorTables
	{ LFAttrRating, TRUE, LFContextSubfolderDefault,
	MINVIEWS, LFViewList,
	IDXATTRS_CORE | IDXATTRS_COLORTABLES,
	ADVATTRS_MINIMAL | (1ull<<LFAttrRating) | (1ull<<LFAttrMediaCollection) | (1ull<<LFAttrApplication),
	0 },

	// LFContextFavorites
	{ LFAttrRating, TRUE, 0,
	ALLVIEWS, LFViewList,
	IDXATTRS_ALL,
	ADVATTRS_MINIMAL | (1ull<<LFAttrRating) | (1ull<<LFAttrHashtags),
	0 },

	// LFContextNew
	{ LFAttrAddTime, FALSE, 0,
	MINVIEWS, LFViewIcons,
	IDXATTRS_ALL,
	(1ull<<LFAttrFileName) | (1ull<<LFAttrAddTime),
	0 },

	// LFContextTasks
	{ LFAttrPriority, TRUE, 0,
	MINMEDIAVIEWS, LFViewList,
	(IDXATTRS_ALL | (1ull<<LFAttrDueTime) | (1ull<<LFAttrPriority)) & ~(1ull<<LFAttrDoneTime),
	ADVATTRS_MINIMAL | (1ull<<LFAttrDueTime) | (1ull<<LFAttrPriority) | (1ull<<LFAttrResponsible),
	0 },

	// LFContextArchive
	{ LFAttrArchiveTime, FALSE, 0,
	MINVIEWS, LFViewIcons,
	IDXATTRS_ALL | (1ull<<LFAttrArchiveTime),
	(1ull<<LFAttrFileName) | (1ull<<LFAttrArchiveTime) | (1ull<<LFAttrColor),
	0 },

	// LFContextTrash
	{ LFAttrDeleteTime, FALSE, 0,
	MINVIEWS, LFViewIcons,
	IDXATTRS_ALL | (1ull<<LFAttrArchiveTime) | (1ull<<LFAttrDeleteTime),
	(1ull<<LFAttrFileName) | (1ull<<LFAttrDeleteTime),
	0 },

	// LFContextSearch
	{ LFAttrFileName, TRUE, 0,
	ALLVIEWS, LFViewDetails,
	IDXATTRS_ALL,
	ADVATTRS_DEFAULT | (1ull<<LFAttrHashtags),
	0 },

	// LFContextStores
	{ LFAttrFileName, FALSE, 0,
	(1<<LFViewIcons), LFViewIcons,
	(1ull<<LFAttrFileName) | (1ull<<LFAttrComments) | (1ull<<LFAttrCreationTime) | (1ull<<LFAttrFileTime) | (1ull<<LFAttrFileCount) | (1ull<<LFAttrFileSize),
	(1ull<<LFAttrFileName) | (1ull<<LFAttrCreationTime),
	0 },

	// LFContextClipboard
	{ LFAttrFileName, FALSE, 0,
	MINVIEWS, LFViewDetails,
	IDXATTRS_ALL,
	ADVATTRS_DEFAULT,
	0 },

	// LFContextSubfolderDefault
	{ LFAttrFileName, FALSE, 0,
	MINVIEWS, LFViewDetails,
	IDXATTRS_ALL,
	ADVATTRS_DEFAULT,
	0 },

	// LFContextSubfolderBooks
	{ LFAttrFileName, FALSE, 0,
	MINVIEWS, LFViewIcons,
	IDXATTRS_CORE | IDXATTRS_BOOKS,
	ADVATTRS_BOOKS,
	0 },

	// LFContextSubfolderFonts
	{ LFAttrFileName, FALSE, 0,
	MINVIEWS, LFViewIcons,
	IDXATTRS_CORE,
	ADVATTRS_MINIMAL | (1ull<<LFAttrRating),
	0 },

	// LFContextSubfolderMessages
	{ LFAttrFileName, FALSE, 0,
	(1<<LFViewList), LFViewList,
	IDXATTRS_CORE | IDXATTRS_MESSAGES,
	ADVATTRS_MINIMAL | (1ull<<LFAttrTitle) | (1ull<<LFAttrFrom) | (1ull<<LFAttrTo),
	0 },

	// LFContextSubfolderMovies
	{ LFAttrFileName, FALSE, 0,
	MINVIEWS, LFViewIcons,
	IDXATTRS_CORE | IDXATTRS_MOVIES,
	ADVATTRS_MOVIES,
	0 },

	// LFContextSubfolderMusic
	{ LFAttrCreator, FALSE, 0,
	(1<<LFViewList), LFViewList,
	IDXATTRS_CORE | IDXATTRS_MUSIC,
	ADVATTRS_MUSIC,
	0 },

	// LFContextSubfolderPictures
	{ LFAttrFileName, FALSE, 0,
	MINVIEWS, LFViewIcons,
	IDXATTRS_CORE | IDXATTRS_PICTURES,
	ADVATTRS_PICTURES,
	0 },

	// LFContextSubfolderPodcasts
	{ LFAttrMediaCollection, FALSE, 0,
	(1<<LFViewList), LFViewList,
	IDXATTRS_CORE | IDXATTRS_TVSHOWS,
	ADVATTRS_TVSHOWS,
	0 },

	// LFContextSubfolderTVShows
	{ LFAttrMediaCollection, FALSE, 0,
	MINMEDIAVIEWS, LFViewList,
	IDXATTRS_CORE | IDXATTRS_TVSHOWS,
	ADVATTRS_TVSHOWS,
	0 },

	// LFContextSubfolderVideos
	{ LFAttrFileName, FALSE, 0,
	MINVIEWS, LFViewIcons,
	IDXATTRS_CORE | IDXATTRS_VIDEOS,
	ADVATTRS_VIDEOS,
	0 }
};

extern const FMTID SHPropertyStorage =   { 0xB725F130, 0x47EF, 0x101A, { 0xA5, 0xF1, 0x02, 0x60, 0x8C, 0x9E, 0xEB, 0xAC } };
extern const FMTID SHPropertyQuery =     { 0x49691C90, 0x7E17, 0x101A, { 0xA9, 0x1C, 0x08, 0x00, 0x2B, 0x2E, 0xCD, 0xA9 } };
extern const FMTID SHPropertySummary =   { 0xF29F85E0, 0x4FF9, 0x1068, { 0xAB, 0x91, 0x08, 0x00, 0x2B, 0x27, 0xB3, 0xD9 } };
extern const FMTID SHPropertyDocuments = { 0xD5CDD502, 0x2E9C, 0x101B, { 0x93, 0x97, 0x08, 0x00, 0x2B, 0x2C, 0xF9, 0xAE } };
extern const FMTID SHPropertyImage =     { 0x6444048F, 0x4C8B, 0x11D1, { 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03 } };
extern const FMTID SHPropertyAudio =     { 0x64440490, 0x4C8B, 0x11D1, { 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03 } };
extern const FMTID SHPropertyVideo =     { 0x64440491, 0x4C8B, 0x11D1, { 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03 } };
extern const FMTID SHPropertyMedia =     { 0x64440492, 0x4C8B, 0x11D1, { 0x8B, 0x70, 0x08, 0x00, 0x36, 0xB1, 0x1A, 0x03 } };
extern const FMTID SHPropertyPhoto =     { 0x14B81DA1, 0x0135, 0x4D31, { 0x96, 0xD9, 0x6C, 0xBF, 0xC9, 0x67, 0x1A, 0x99 } };
extern const FMTID SHPropertyMusic =     { 0x56A3372E, 0xCE9C, 0x11D2, { 0x9F, 0x0E, 0x00, 0x60, 0x97, 0xC6, 0x86, 0xF6 } };
extern const FMTID SHPropertyVersion =   { 0x0CEF7D53, 0xFA64, 0x11D1, { 0xA2, 0x03, 0x00, 0x00, 0xF8, 0x1F, 0xED, 0xEE } };
extern const FMTID SHPropertyUnnamed1 =  { 0x3F8472B5, 0xE0AF, 0x4DB2, { 0x80, 0x71, 0xC5, 0x3F, 0xE7, 0x6A, 0xE7, 0xCE } };
extern const FMTID SHPropertyUnnamed2 =  { 0x72FAB781, 0xACDA, 0x43E5, { 0xB1, 0x55, 0xB2, 0x43, 0x4F, 0x85, 0xE6, 0x78 } };
extern const FMTID SHPropertyUnnamed3 =  { 0xE3E0584C, 0xB788, 0x4A5A, { 0xBB, 0x20, 0x7F, 0x5A, 0x44, 0xC9, 0xAC, 0xDD } };
extern const FMTID SHPropertyUnnamed4 =  { 0x2E4B640D, 0x5019, 0x46D8, { 0x88, 0x81, 0x55, 0x41, 0x4C, 0xC5, 0xCA, 0xA0 } };
extern const FMTID SHPropertyUnnamed5 =  { 0x2CBAA8F5, 0xD81F, 0x47CA, { 0xB1, 0x7A, 0xF8, 0xD8, 0x22, 0x30, 0x01, 0x31 } };
extern const FMTID SHPropertyUnnamed6 =  { 0x43F8D7B7, 0xA444, 0x4F87, { 0x93, 0x83, 0x52, 0x27, 0x1C, 0x9B, 0x91, 0x5C } };
extern const FMTID SHPropertyUnnamed7 =  { 0x276D7BB0, 0x5B34, 0x4FB0, { 0xAA, 0x4B, 0x15, 0x8E, 0xD1, 0x2A, 0x18, 0x09 } };

extern const LFAttributeProperties AttrProperties[LFAttributeCount] = {
	// LFAttrFileName
	{ LFTypeUnicodeString, LFAttrCategoryBasic, LFDataEditable | LFDataAlwaysVisible, 255,
	0, LFViewIcons, 0, 0, { &SHPropertyStorage, 10 }, 0 },

	// LFAttrPriority
	{ LFTypeRating, LFAttrCategoryTasks, LFDataEditable, 0,
	0, LFViewList, 0, 0, { &SHPropertyUnnamed3, 11 }, 17 },

	// LFAttrCreationTime
	{ LFTypeTime, LFAttrCategoryBasic, 0, 0,
	0, LFViewTimeline, 4, 0, { &SHPropertyStorage, 15 }, 5 },

	// LFAttrFileTime
	{ LFTypeTime, LFAttrCategoryBasic, 0, 0,
	0, LFViewTimeline, 4, 0, { &SHPropertyStorage, 14 }, 6 },

	// LFAttrAddTime
	{ LFTypeTime, LFAttrCategoryInternal, 0, 0,
	0, LFViewTimeline, LFMinAttributePriority, 0, { &SHPropertyUnnamed5, 100 }, 7 },

	// LFAttrDueTime
	{ LFTypeTime, LFAttrCategoryTasks, LFDataEditable, 0,
	0, LFViewCalendar, 0, 0, { &SHPropertyUnnamed1, 100 }, 50 },

	// LFAttrDoneTime
	{ LFTypeTime, LFAttrCategoryInternal, 0, 0,
	0, LFViewTimeline, LFMinAttributePriority, 0, { &SHPropertyUnnamed2, 100 }, 51 },

	// LFAttrArchiveTime
	{ LFTypeTime, LFAttrCategoryBasic, 0, 0,
	0, LFViewCalendar, LFMinAttributePriority, 0, { &SHPropertyUnnamed6, 100 }, 9 },

	// LFAttrDeleteTime
	{ LFTypeTime, LFAttrCategoryBasic, 0, 0,
	0, LFViewCalendar, LFMinAttributePriority, 0, { 0,0 }, 8 },

	// LFAttrColor
	{ LFTypeColor, LFAttrCategoryBasic, LFDataEditable, 0,
	0, LFViewList, 3, 0, { 0,0 }, 54 },

	// LFAttrHashtags
	{ LFTypeUnicodeArray, LFAttrCategoryBasic, LFDataEditable, 255,
	0, LFViewTagcloud, 3, 0, { &SHPropertySummary, 5 }, 15 },

	// LFAttrRating
	{ LFTypeRating, LFAttrCategoryBasic, LFDataEditable, 0,
	0, LFViewList, 4, 0, { &SHPropertyMedia, 9 }, 16 },

	// LFAttrComments
	{ LFTypeUnicodeString, LFAttrCategoryBasic, LFDataEditable, 255,
	0, LFViewList, LFMinAttributePriority, 0, { &SHPropertySummary, 6 }, 3 },

	// LFAttrFileFormat
	{ LFTypeAnsiString, LFAttrCategoryInternal, LFDataBucket, LFExtSize-1,
	0, LFViewTagcloud, LFMinAttributePriority, 0, { &SHPropertyStorage, 4 }, 10 },

	// LFAttrFileCount
	{ LFTypeUINT, LFAttrCategoryBasic, 0, 0,
	0, LFViewIcons, LFMinAttributePriority, 0, { 0,0 }, 11 },

	// LFAttrFileSize
	{ LFTypeSize, LFAttrCategoryBasic, LFDataBucket, 0,
	0, LFViewList, LFMinAttributePriority, 0, { &SHPropertyStorage, 12 }, 12 },

	// LFAttrLocationName
	{ LFTypeUnicodeString, LFAttrCategoryGeotagging, LFDataEditable | LFDataBucket, 255,
	0, LFViewList, 4, 0, { 0,0 }, 18 },

	// LFAttrLocationIATA
	{ LFTypeIATACode, LFAttrCategoryGeotagging, LFDataEditable, 3,
	IDI_FLD_PLACEHOLDER_LOCATION, LFViewList, 4, 0, { 0,0 }, 19 },

	// LFAttrLocationGPS
	{ LFTypeGeoCoordinates, LFAttrCategoryGeotagging, LFDataEditable, 0,
	0, LFViewGlobe, LFMinAttributePriority, 0, { 0,0 }, 20 },

	// LFAttrURL
	{ LFTypeAnsiString, LFAttrCategoryBasic, LFDataEditable, 255,
	0, LFViewList, LFMinAttributePriority, 0, { &SHPropertyQuery, 9 }, 14 },


	// LFAttrCreator
	{ LFTypeUnicodeString, LFAttrCategoryMedia, LFDataEditable | LFDataShowRepresentativeThumbnail | LFDataBucket, 255,
	IDI_FLD_PLACEHOLDER_CONTACT, LFViewIcons, 1, LFAttrMediaCollection, { &SHPropertyMusic, 13 }, 38 },

	// LFAttrMediaCollection
	{ LFTypeUnicodeString, LFAttrCategoryMedia, LFDataEditable | LFDataShowRepresentativeThumbnail | LFDataBucket, 255,
	IDI_FLD_PLACEHOLDER_DEFAULT, LFViewIcons, 1, LFAttrSequenceInCollection, { &SHPropertyMusic, 4 }, 31 },

	// LFAttrSequenceInCollection
	{ LFTypeUINT, LFAttrCategoryMedia, LFDataEditable | LFDataNeverSortable, 0,
	0, LFViewList, 1, 0, { &SHPropertyMusic, 7 }, 56 },

	// LFAttrTitle
	{ LFTypeUnicodeString, LFAttrCategoryMedia, LFDataEditable, 255,
	0, LFViewIcons, 1, 0, { &SHPropertySummary, 2 }, 39 },

	// LFAttrReleased
	{ LFTypeYear, LFAttrCategoryMedia, LFDataEditable, 0,
	0, LFViewList, 2, LFAttrMediaCollection, { &SHPropertyMusic, 5 }, 57 },

	// LFAttrLength
	{ LFTypeDuration, LFAttrCategoryMedia, 0, 0,
	0, LFViewList, 2, 0, { &SHPropertyAudio, 3 }, 36 },

	// LFAttrBitrate
	{ LFTypeBitrate, LFAttrCategoryMedia, 0, 0,
	0, LFViewList, LFMinAttributePriority, 0, { &SHPropertyMedia, 4 }, 37 },

	// LFAttrLanguage
	{ LFTypeAnsiString, LFAttrCategoryMedia, LFDataEditable | LFDataBucket, 2,
	0, LFViewList, LFMinAttributePriority, 0, { 0,0 }, 42 },

	// LFAttrRecordingTime
	{ LFTypeTime, LFAttrCategoryMedia, 0, 0, 0, LFViewCalendar, 3, 0, { &SHPropertyUnnamed4, 100 }, 44 },

	// LFAttrRecordingEquipment
	{ LFTypeUnicodeString, LFAttrCategoryMedia, LFDataBucket, 255,
	0, LFViewList, 3, 0, { &SHPropertyPhoto, 272 }, 45 },

	// LFAttrCopyright
	{ LFTypeUnicodeString, LFAttrCategoryMedia, LFDataEditable, 255
	, 0, LFViewList, LFMinAttributePriority, 0, { &SHPropertyMedia, 11 }, 40 },


	// LFAttrWidth
	{ LFTypeUINT, LFAttrCategoryVisual, 0, 0,
	0, LFViewTagcloud, LFMinAttributePriority, 0, { &SHPropertyImage, 3 }, 21 },

	// LFAttrHeight
	{ LFTypeUINT, LFAttrCategoryVisual, 0, 0,
	0, LFViewTagcloud, LFMinAttributePriority, 0, { &SHPropertyImage, 4 }, 22 },

	// LFAttrDimension
	{ LFTypeMegapixel, LFAttrCategoryVisual, 0, 0,
	0, LFViewList, 3, 0, { 0,0 }, 23 },

	// LFAttrAspectRatio
	{ LFTypeDouble, LFAttrCategoryVisual, 0, 0,
	0, LFViewList, LFMinAttributePriority, 0, { 0,0 }, 24 },

	// LFAttrFramerate
	{ LFTypeFramerate, LFAttrCategoryVisual, 0, 0,
	0, LFViewList, 3, 0, { &SHPropertyVideo, 6 }, 58 },

	// LFAttrApplication
	{ LFTypeApplication, LFAttrCategoryVisual, 0, 0,
	IDI_APP_DEFAULT, LFViewList, 4, 0, { 0,0 }, 55 },

	// LFAttrVideoCodec
	{ LFTypeFourCC, LFAttrCategoryVisual, 0, 0,
	0, LFViewList, LFMinAttributePriority, 0, { &SHPropertyVideo, 44 }, 25 },

	// LFAttrExposure
	{ LFTypeUnicodeString, LFAttrCategoryVisual, 0, 31,
	0, LFViewList, LFMinAttributePriority, 0, { 0,0 }, 27 },

	// LFAttrFocus
	{ LFTypeFraction, LFAttrCategoryVisual, 0, 0,
	0, LFViewIcons, LFMinAttributePriority, 0, { 0,0 }, 28 },

	// LFAttrAperture
	{ LFTypeFraction, LFAttrCategoryVisual, 0, 0,
	0, LFViewIcons, LFMinAttributePriority, 0, { 0,0 }, 29 },

	// LFAttrChip
	{ LFTypeUnicodeString, LFAttrCategoryVisual, LFDataBucket, 31,
	0, LFViewList, LFMinAttributePriority, 0, { 0,0 }, 30 },


	// LFAttrChannels
	{ LFTypeUINT, LFAttrCategoryAudio, LFDataBucket, 0,
	0, LFViewList, LFMinAttributePriority, 0, { &SHPropertyMedia, 7 }, 33 },

	// LFAttrSamplerate
	{ LFTypeUINT, LFAttrCategoryAudio, LFDataBucket, 0,
	0, LFViewList, LFMinAttributePriority, 0, { &SHPropertyMedia, 5 }, 34 },

	// LFAttrAudioCodec
	{ LFTypeFourCC, LFAttrCategoryAudio, 0, 0,
	0, LFViewList, LFMinAttributePriority, 0, { &SHPropertyAudio, 10 }, 35 },

	// LFAttrGenre
	{ LFTypeGenre, LFAttrCategoryAudio, LFDataEditable, 0,
	IDI_FLD_DEFAULTGENRE, LFViewIcons, 2, LFAttrCreator, { &SHPropertyMusic, 11 }, 32 },


	// LFAttrPages
	{ LFTypeUINT, LFAttrCategoryDocuments, 0, 0,
	0, LFViewList, 3, 0, { &SHPropertyDocuments, 14 }, 43 },

	// LFAttrISBN
	{ LFTypeAnsiString, LFAttrCategoryDocuments, LFDataEditable, 31,
	0, LFViewList, LFMinAttributePriority, 0, { 0,0 }, 41 },

	// LFAttrSignature
	{ LFTypeAnsiString, LFAttrCategoryDocuments, LFDataEditable, 31,
	0, LFViewList, LFMinAttributePriority, 0, { 0,0 }, 46 },


	// LFAttrFrom
	{ LFTypeAnsiString, LFAttrCategoryTasks, LFDataBucket, 255,
	0, LFViewList, 0, 0, { 0,0 }, 47 },

	// LFAttrTo
	{ LFTypeAnsiString, LFAttrCategoryTasks, LFDataBucket, 255,
	0, LFViewList, 0, 0, { 0,0 }, 48 },


	// LFAttrResponsible
	{ LFTypeUnicodeString, LFAttrCategoryTasks, LFDataEditable | LFDataBucket, 255,
	0, LFViewList, LFMinAttributePriority, 0, { 0,0 }, 49 },

	// LFAttrCustomer
	{ LFTypeUnicodeString, LFAttrCategoryTasks, LFDataEditable | LFDataBucket, 255,
	0, LFViewList, 3, 0, { &SHPropertyUnnamed7, 100 }, 52 }
};


#define WIDTH_HIDE        0
#define WIDTH_TINY       60
#define WIDTH_SMALL      90
#define WIDTH_MEDIUM    120
#define WIDTH_LARGE     150
#define WIDTH_HUGE      200

extern const LFTypeProperties TypeProperties[LFTypeCount] = {
	// LFTypeUnicodeString
	{ 0, WIDTH_LARGE, LFDataContainsLetters | LFDataSortableInSubfolder,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewIcons },

	// LFTypeUnicodeArray
	{ 0, WIDTH_HUGE, LFDataContainsLetters,
	(1<<LFViewIcons) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewTagcloud },

	// LFTypeAnsiString
	{ 0, WIDTH_LARGE, LFDataContainsLetters | LFDataSortableInSubfolder,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewIcons },

	// LFTypeIATACode
	{ sizeof(CHAR)*4, WIDTH_TINY, LFDataContainsLetters | LFDataSortableInSubfolder | LFDataBucket,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewGlobe) | (1<<LFViewTagcloud), LFViewList },

	// LFTypeFourCC
	{ sizeof(DWORD), WIDTH_SMALL, LFDataContainsLetters | LFDataSortableInSubfolder | LFDataBucket,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewList },

	// LFTypeRating
	{ sizeof(BYTE), WIDTH_SMALL, LFDataSortableInSubfolder | LFDataSortDescending | LFDataBucket,
	(1<<LFViewList), LFViewList },

	// LFTypeUINT
	{ sizeof(UINT), WIDTH_TINY, LFDataSortableInSubfolder | LFDataSortDescending | LFDataFormatRight,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewList },

	// LFTypeSize
	{ sizeof(INT64), WIDTH_TINY, LFDataContainsLetters | LFDataSortableInSubfolder | LFDataSortDescending | LFDataFormatRight,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewTagcloud), LFViewList },

	// LFTypeFraction
	{ sizeof(LFFraction), WIDTH_TINY, LFDataSortableInSubfolder,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails), LFViewIcons },

	// LFTypeDouble
	{ sizeof(DOUBLE), WIDTH_SMALL, LFDataSortableInSubfolder | LFDataSortDescending | LFDataFormatRight,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails), LFViewIcons },

	// LFTypeColor
	{ sizeof(BYTE), WIDTH_HIDE, LFDataSortableInSubfolder | LFDataBucket,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails), LFViewList },

	// LFTypeGeoCoordinates
	{ sizeof(LFGeoCoordinates), WIDTH_LARGE, LFDataContainsLetters | LFDataSortableInSubfolder | LFDataBucket,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewGlobe), LFViewIcons },

	// LFTypeTime
	{ sizeof(FILETIME), WIDTH_MEDIUM, LFDataContainsLetters | LFDataSortableInSubfolder | LFDataSortDescending | LFDataBucket,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewCalendar) | (1<<LFViewTimeline) | (1<<LFViewTagcloud), LFViewTimeline },

	// LFTypeBitrate,
	{ sizeof(UINT), WIDTH_SMALL, LFDataContainsLetters | LFDataSortableInSubfolder | LFDataSortDescending | LFDataFormatRight,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewIcons },

	// LFTypeDuration
	{ sizeof(UINT), WIDTH_TINY, LFDataContainsLetters | LFDataSortableInSubfolder | LFDataSortDescending | LFDataFormatRight | LFDataBucket,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewIcons },

	// LFTypeMegapixel
	{ sizeof(DOUBLE), WIDTH_MEDIUM, LFDataContainsLetters | LFDataSortableInSubfolder | LFDataSortDescending | LFDataFormatRight | LFDataBucket,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewIcons },

	// LFTypeGenre
	{ sizeof(UINT), WIDTH_LARGE, LFDataContainsLetters | LFDataSortableInSubfolder | LFDataBucket,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewIcons },

	// LFTypeApplication
	{ sizeof(BYTE), WIDTH_MEDIUM, LFDataContainsLetters | LFDataSortableInSubfolder | LFDataBucket,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewList },

	// LFTypeYear
	{ sizeof(UINT), WIDTH_TINY, LFDataContainsLetters | LFDataSortableInSubfolder | LFDataSortDescending | LFDataFormatRight | LFDataBucket,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewList },

	// LFTypeFramerate
	{ sizeof(UINT), WIDTH_SMALL, LFDataContainsLetters | LFDataSortableInSubfolder | LFDataSortDescending | LFDataFormatRight | LFDataBucket,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewList }
};


#define CTX_AUDIO    ((1ull<<LFContextAudio) | (1ull<<LFContextMusic) | (1ull<<LFContextSubfolderMusic))
#define CTX_PICTURES ((1ull<<LFContextPictures) | (1ull<<LFContextSubfolderPictures))
#define CTX_PODCASTS ((1ull<<LFContextPodcasts) | (1ull<<LFContextSubfolderPodcasts))
#define CTX_TVSHOWS  ((1ull<<LFContextTVShows) | (1ull<<LFContextSubfolderTVShows))
#define CTX_VIDEOS   ((1ull<<LFContextVideos) | (1ull<<LFContextSubfolderVideos))

extern const SpecialAttributeName SpecialAttributeNames[SPECIALATTRIBUTENAMESCOUNT] = {
	{ LFAttrMediaCollection, IDS_ALBUM, IDI_FLD_PLACEHOLDER_NOTE, FALSE, CTX_AUDIO },
	{ LFAttrMediaCollection, IDS_ROLL, IDI_ROL_DEFAULT, FALSE, CTX_PICTURES | CTX_VIDEOS | (1ull<<LFContextColorTables) },
	{ LFAttrMediaCollection, IDS_PODCAST, IDI_FLD_PLACEHOLDER_PODCAST, FALSE,CTX_PODCASTS },
	{ LFAttrMediaCollection, IDS_TVSHOW, IDI_FLD_PLACEHOLDER_TV, FALSE, CTX_TVSHOWS },
	{ LFAttrSequenceInCollection, IDS_EPISODE, 0, TRUE, CTX_PODCASTS },
	{ LFAttrSequenceInCollection, IDS_EPISODE, 0, FALSE, CTX_TVSHOWS | CTX_VIDEOS },
	{ LFAttrSequenceInCollection, IDS_TRACK, 0, FALSE, CTX_AUDIO },
	{ LFAttrCreator, IDS_ARTIST, IDI_FLD_PLACEHOLDER_CONTACT, FALSE, CTX_AUDIO | CTX_PICTURES },
	{ LFAttrCreator, IDS_AUTHOR, IDI_FLD_PLACEHOLDER_CONTACT, FALSE, CTX_PODCASTS | CTX_TVSHOWS | CTX_VIDEOS | (1ull<<LFContextBooks) | (1ull<<LFContextDocuments) | (1ull<<LFContextColorTables) | (1ull<<LFContextMovies) }
};

#pragma data_seg()
