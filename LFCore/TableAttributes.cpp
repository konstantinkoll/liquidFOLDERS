
#include "stdafx.h"
#include "TableAttributes.h"
#include "TableIndexes.h"


#pragma data_seg(".shared")

#define ALLVIEWS     ((1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewCalendar) | (1<<LFViewTimeline) | (1<<LFViewGlobe) | (1<<LFViewTagcloud))

extern const LFContextProperties CtxProperties[LFContextCount] = {
	// LFContextAllFiles
	{ LFAttrFileTime, TRUE, FALSE,
	ALLVIEWS, LFViewTimeline,
	IDXATTRS_ALL,
	IDXATTRS_GENDETAILS },

	// LFContextFavorites
	{ LFAttrRating, TRUE, FALSE,
	ALLVIEWS, LFViewList,
	IDXATTRS_ALL,
	IDXATTRS_MINDETAILS | (1ull<<LFAttrRating) | (1ull<<LFAttrHashtags) },

	// LFContextAudio
	{ LFAttrGenre, TRUE, FALSE,
	ALLVIEWS, LFViewIcons,
	IDXATTRS_CORE | IDXATTRS_AUDIO,
	(1ull<<LFAttrRating) | (1ull<<LFAttrColor) | (1ull<<LFAttrTitle) | (1ull<<LFAttrAlbum) | (1ull<<LFAttrGenre) | (1ull<<LFAttrArtist) | (1ull<<LFAttrDuration) },

	// LFContextPictures
	{ LFAttrFileTime, TRUE, FALSE,
	ALLVIEWS, LFViewTimeline,
	IDXATTRS_CORE | IDXATTRS_PICTURES,
	IDXATTRS_VISDETAILS },

	// LFContextVideos
	{ LFAttrFileTime, TRUE, FALSE,
	ALLVIEWS, LFViewTimeline,
	IDXATTRS_CORE | IDXATTRS_VIDEOS,
	IDXATTRS_VISDETAILS | (1ull<<LFAttrDuration) },

	// LFContextDocuments
	{ LFAttrFileTime, TRUE, FALSE,
	ALLVIEWS, LFViewTimeline,
	IDXATTRS_CORE | IDXATTRS_DOCUMENTS,
	((1ull<<LFAttrFileName) | (1ull<<LFAttrComments) | (1ull<<LFAttrCreationTime) | (1ull<<LFAttrFileTime) | (1ull<<LFAttrRating) | (1ull<<LFAttrColor) | (1ull<<LFAttrCustomer) | (1ull<<LFAttrPages)) },

	// LFContextContacts
	{ LFAttrFileName, TRUE, FALSE,
	(1<<LFViewIcons) | (1<<LFViewList), LFViewIcons,
	IDXATTRS_CORE,
	IDXATTRS_MINDETAILS },

	// LFContextMessages
	{ LFAttrFileName, TRUE, FALSE,
	(1<<LFViewList) | (1<<LFViewTagcloud), LFViewList,
	IDXATTRS_CORE | IDXATTRS_MESSAGES,
	IDXATTRS_MINDETAILS | (1ull<<LFAttrTitle) | (1ull<<LFAttrFrom) | (1ull<<LFAttrTo) },

	// LFContextNew
	{ LFAttrAddTime, FALSE, FALSE,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails), LFViewIcons,
	IDXATTRS_ALL,
	(1ull<<LFAttrFileName) | (1ull<<LFAttrAddTime) },

	// LFContextTasks
	{ LFAttrPriority, TRUE, FALSE,
	ALLVIEWS, LFViewList,
	(IDXATTRS_ALL | (1ull<<LFAttrDueTime) | (1ull<<LFAttrPriority)) & ~(1ull<<LFAttrDoneTime),
	IDXATTRS_MINDETAILS | (1ull<<LFAttrDueTime) | (1ull<<LFAttrPriority) | (1ull<<LFAttrResponsible) },

	// LFContextArchive
	{ LFAttrArchiveTime, FALSE, FALSE,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails), LFViewIcons,
	IDXATTRS_ALL | (1ull<<LFAttrArchiveTime),
	(1ull<<LFAttrFileName) | (1ull<<LFAttrArchiveTime) | (1ull<<LFAttrColor) },

	// LFContextTrash
	{ LFAttrDeleteTime, FALSE, FALSE,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails), LFViewIcons,
	IDXATTRS_ALL | (1ull<<LFAttrArchiveTime) | (1ull<<LFAttrDeleteTime),
	(1ull<<LFAttrFileName) | (1ull<<LFAttrDeleteTime) },

	// LFContextFilters
	{ LFAttrFileName, FALSE, FALSE,
	(1<<LFViewIcons), LFViewIcons,
	(1ull<<LFAttrFileName) | (1ull<<LFAttrComments) | (1ull<<LFAttrCreationTime) | (1ull<<LFAttrFileTime) | (1ull<<LFAttrFileSize),
	(1ull<<LFAttrFileName) | (1ull<<LFAttrComments) },

	// LFContextSearch
	{ LFAttrFileName, TRUE, FALSE,
	ALLVIEWS, LFViewDetails,
	IDXATTRS_ALL,
	IDXATTRS_GENDETAILS | (1ull<<LFAttrHashtags) },

	// LFContextStores
	{ LFAttrFileName, FALSE, FALSE,
	(1<<LFViewIcons), LFViewIcons,
	(1ull<<LFAttrFileName) | (1ull<<LFAttrComments) | (1ull<<LFAttrCreationTime) | (1ull<<LFAttrFileTime) | (1ull<<LFAttrFileCount) | (1ull<<LFAttrFileSize),
	(1ull<<LFAttrFileName) | (1ull<<LFAttrCreationTime) },

	// LFContextClipboard
	{ LFAttrFileName, FALSE, FALSE,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails), LFViewDetails,
	IDXATTRS_ALL,
	IDXATTRS_GENDETAILS },

	// LFContextSubfolderDefault
	{ LFAttrFileName, FALSE, FALSE,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails), LFViewDetails,
	IDXATTRS_ALL,
	IDXATTRS_GENDETAILS },

	// LFContextSubfolderDay
	{ LFAttrFileName, FALSE, FALSE,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails), LFViewIcons,
	IDXATTRS_ALL,
	IDXATTRS_GENDETAILS },

	// LFContextSubfolderGenre
	{ LFAttrArtist, FALSE, FALSE,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails), LFViewList,
	IDXATTRS_CORE | IDXATTRS_AUDIO,
	(1ull<<LFAttrFileName) | (1ull<<LFAttrRating) | (1ull<<LFAttrComments) | (1ull<<LFAttrArtist) | (1ull<<LFAttrAlbum) | (1ull<<LFAttrTitle) | (1ull<<LFAttrDuration) },

	// LFContextSubfolderArtist
	{ LFAttrAlbum, FALSE, FALSE,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails), LFViewList,
	IDXATTRS_CORE | IDXATTRS_AUDIO,
	(1ull<<LFAttrFileName | (1ull<<LFAttrRating)) | (1ull<<LFAttrComments) | (1ull<<LFAttrAlbum) | (1ull<<LFAttrGenre) | (1ull<<LFAttrTitle) | (1ull<<LFAttrDuration) },

	// LFContextSubfolderAlbum
	{ LFAttrFileName, FALSE, TRUE,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails), LFViewList,
	IDXATTRS_CORE | IDXATTRS_AUDIO,
	(1ull<<LFAttrFileName) | (1ull<<LFAttrRating) | (1ull<<LFAttrComments) | (1ull<<LFAttrArtist) | (1ull<<LFAttrGenre) | (1ull<<LFAttrTitle) | (1ull<<LFAttrDuration) }
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
	{ LFTypeUnicodeString, 255, LFAttrCategoryBasic, 0, LFViewIcons, 0, FALSE, TRUE, FALSE, { SHPropertyStorage, 10 }, 0 },

	// LFAttrPriority
	{ LFTypeRating, 0, LFAttrCategoryTasks, 0, LFViewList, 0, FALSE, FALSE, FALSE, { SHPropertyUnnamed3, 11 }, 17 },

	// LFAttrCreationTime
	{ LFTypeTime, 0, LFAttrCategoryBasic, 0, LFViewTimeline, 4, TRUE, FALSE,FALSE, { SHPropertyStorage, 15 }, 5 },

	// LFAttrFileTime
	{ LFTypeTime, 0, LFAttrCategoryBasic, 0, LFViewTimeline, 4, TRUE, FALSE, FALSE, { SHPropertyStorage, 14 }, 6 },

	// LFAttrAddTime
	{ LFTypeTime, 0, LFAttrCategoryInternal, 0, LFViewTimeline, LFMaxAttributePriority, TRUE, FALSE, FALSE, { SHPropertyUnnamed5, 100 }, 7 },

	// LFAttrDueTime
	{ LFTypeTime, 0, LFAttrCategoryTasks, 0, LFViewCalendar, 0, FALSE, FALSE, FALSE, { SHPropertyUnnamed1, 100 }, 50 },

	// LFAttrDoneTime
	{ LFTypeTime, 0, LFAttrCategoryInternal, 0, LFViewTimeline, LFMaxAttributePriority, TRUE, FALSE, FALSE, { SHPropertyUnnamed2, 100 }, 51 },

	// LFAttrArchiveTime
	{ LFTypeTime, 0, LFAttrCategoryBasic, 0, LFViewCalendar, LFMaxAttributePriority, TRUE, FALSE, FALSE, { SHPropertyUnnamed6, 100 }, 9 },

	// LFAttrDeleteTime
	{ LFTypeTime, 0, LFAttrCategoryBasic, 0, LFViewCalendar, LFMaxAttributePriority, TRUE, FALSE, FALSE, { 0,0 }, 8 },

	// LFAttrColor
	{ LFTypeColor, 0, LFAttrCategoryBasic, 0, LFViewList, 3, FALSE, FALSE, FALSE, { 0,0 }, 54 },

	// LFAttrHashtags
	{ LFTypeUnicodeArray, 255, LFAttrCategoryBasic, 0, LFViewTagcloud, 3, FALSE, FALSE, FALSE, { SHPropertySummary, 5 }, 15 },

	// LFAttrRating
	{ LFTypeRating, 0, LFAttrCategoryBasic, 0, LFViewList, 4, FALSE, FALSE, FALSE, { SHPropertyMedia, 9 }, 16 },

	// LFAttrComments
	{ LFTypeUnicodeString, 255, LFAttrCategoryBasic, 0, LFViewList, LFMaxAttributePriority, FALSE, FALSE, FALSE, { SHPropertySummary, 6 }, 3 },

	// LFAttrFileFormat
	{ LFTypeAnsiString, LFExtSize-1, LFAttrCategoryInternal, 0, LFViewTagcloud, LFMaxAttributePriority, TRUE, FALSE, FALSE, { SHPropertyStorage, 4 }, 10 },

	// LFAttrFileCount
	{ LFTypeUINT, 0, LFAttrCategoryBasic, 0, LFViewIcons, LFMaxAttributePriority, TRUE, FALSE, FALSE, { 0,0 }, 11 },

	// LFAttrFileSize
	{ LFTypeSize, 0, LFAttrCategoryBasic, 0, LFViewList, LFMaxAttributePriority, TRUE, FALSE, FALSE, { SHPropertyStorage, 12 }, 12 },

	// LFAttrLocationName
	{ LFTypeUnicodeString, 255, LFAttrCategoryGeotags, 0, LFViewList, 4, FALSE, FALSE, FALSE, { 0,0 }, 18 },

	// LFAttrLocationIATA
	{ LFTypeIATACode, 3, LFAttrCategoryGeotags, 0, LFViewGlobe, 4, FALSE, FALSE, FALSE, { 0,0 }, 19 },

	// LFAttrLocationGPS
	{ LFTypeGeoCoordinates, 0, LFAttrCategoryGeotags, 0, LFViewGlobe, LFMaxAttributePriority, FALSE, FALSE, FALSE, { 0,0 }, 20 },

	// LFAttrURL
	{ LFTypeAnsiString, 255, LFAttrCategoryBasic, 0, LFViewList, LFMaxAttributePriority, FALSE, FALSE, FALSE, { SHPropertyQuery, 9 }, 14 },


	// LFAttrRoll
	{ LFTypeUnicodeString, 255, LFAttrCategoryVisual, IDI_FLD_DEFAULT, LFViewList, 1, FALSE, FALSE, FALSE, { SHPropertyPhoto, 18248 }, 26 },

	// LFAttrWidth
	{ LFTypeUINT, 0, LFAttrCategoryVisual, 0, LFViewTagcloud, LFMaxAttributePriority, TRUE, FALSE, FALSE, { SHPropertyImage, 3 }, 21 },

	// LFAttrHeight
	{ LFTypeUINT, 0, LFAttrCategoryVisual, 0, LFViewTagcloud, LFMaxAttributePriority, TRUE, FALSE, FALSE, { SHPropertyImage, 4 }, 22 },

	// LFAttrDimension
	{ LFTypeMegapixel, 0, LFAttrCategoryVisual, 0, LFViewList, 3, TRUE, FALSE, FALSE, { 0,0 }, 23 },

	// LFAttrAspectRatio
	{ LFTypeDouble, 0, LFAttrCategoryVisual, 0, LFViewList, LFMaxAttributePriority, TRUE, FALSE, FALSE, { 0,0 }, 24 },

	// LFAttrApplication
	{ LFTypeApplication, 0, LFAttrCategoryVisual, 0, LFViewList, 4, TRUE, FALSE, FALSE, { 0,0 }, 55 },

	// LFAttrVideoCodec
	{ LFTypeFourCC, 0, LFAttrCategoryVisual, 0, LFViewList, LFMaxAttributePriority, TRUE, FALSE, FALSE, { SHPropertyVideo, 44 }, 25 },


	// LFAttrExposure
	{ LFTypeUnicodeString, 31, LFAttrCategoryPhotographic, 0, LFViewList, LFMaxAttributePriority, TRUE, FALSE, FALSE, { 0,0 }, 27 },

	// LFAttrFocus
	{ LFTypeFraction, 0, LFAttrCategoryPhotographic, 0, LFViewIcons, LFMaxAttributePriority, TRUE, FALSE, FALSE, { 0,0 }, 28 },

	// LFAttrAperture
	{ LFTypeFraction, 0, LFAttrCategoryPhotographic, 0, LFViewIcons, LFMaxAttributePriority, TRUE, FALSE, FALSE, { 0,0 }, 29 },

	// LFAttrChip
	{ LFTypeUnicodeString, 31, LFAttrCategoryPhotographic, 0, LFViewList, LFMaxAttributePriority, TRUE, FALSE, FALSE, { 0,0 }, 30 },


	// LFAttrArtist
	{ LFTypeUnicodeString, 255, LFAttrCategoryBibliographic, IDI_FLD_PLACEHOLDER, LFViewList, 1, FALSE, FALSE, TRUE, { SHPropertyMusic, 13 }, 38 },

	// LFAttrAlbum
	{ LFTypeUnicodeString, 255, LFAttrCategoryAudio, IDI_FLD_PLACEHOLDER, LFViewList, 1, FALSE, FALSE, TRUE, { SHPropertyMusic, 4 }, 31 },

	// LFAttrGenre
	{ LFTypeGenre, 0, LFAttrCategoryAudio, IDI_FLD_DEFAULTGENRE, LFViewIcons, 2, FALSE, FALSE, FALSE, { SHPropertyMusic, 11 }, 32 },

	// LFAttrChannels
	{ LFTypeUINT, 0, LFAttrCategoryAudio, 0, LFViewList, LFMaxAttributePriority, TRUE, FALSE, FALSE, { SHPropertyMedia, 7 }, 33 },

	// LFAttrSamplerate
	{ LFTypeUINT, 0, LFAttrCategoryAudio, 0, LFViewList, LFMaxAttributePriority, TRUE, FALSE, FALSE, { SHPropertyMedia, 5 }, 34 },

	// LFAttrAudioCodec
	{ LFTypeFourCC, 0, LFAttrCategoryAudio, 0, LFViewList, LFMaxAttributePriority, TRUE, FALSE, FALSE, { SHPropertyAudio, 10 }, 35 },


	// LFAttrDuration
	{ LFTypeDuration, 0, LFAttrCategoryTimebased, 0, LFViewList, 2, TRUE, FALSE, FALSE, { SHPropertyAudio, 3 }, 36 },

	// LFAttrBitrate
	{ LFTypeBitrate, 0, LFAttrCategoryTimebased, 0, LFViewList, 3, TRUE, FALSE, FALSE, { SHPropertyMedia, 4 }, 37 },

	// LFAttrRecordingTime
	{ LFTypeTime, 0, LFAttrCategoryBibliographic, 0, LFViewCalendar, 3, TRUE, FALSE, FALSE, { SHPropertyUnnamed4, 100 }, 44 },

	// LFAttrRecordingEquipment
	{ LFTypeUnicodeString, 255, LFAttrCategoryBibliographic, 0, LFViewList, 3, TRUE, FALSE, FALSE, { SHPropertyPhoto, 272 }, 45 },


	// LFAttrAuthor
	{ LFTypeUnicodeString, 255, LFAttrCategoryBibliographic, 0, LFViewList, 3, FALSE, FALSE, FALSE, { SHPropertySummary, 4 }, 53 },

	// LFAttrCopyright
	{ LFTypeUnicodeString, 255, LFAttrCategoryBibliographic, 0, LFViewList, LFMaxAttributePriority, FALSE, FALSE, FALSE, { SHPropertyMedia, 11 }, 40 },

	// LFAttrTitle
	{ LFTypeUnicodeString, 255, LFAttrCategoryBibliographic, 0, LFViewIcons, 1, FALSE, FALSE, FALSE, { SHPropertySummary, 2 }, 39 },

	// LFAttrISBN
	{ LFTypeAnsiString, 31, LFAttrCategoryBibliographic, 0, LFViewList, LFMaxAttributePriority, FALSE, FALSE, FALSE, { 0,0 }, 41 },

	// LFAttrLanguage
	{ LFTypeAnsiString, 2, LFAttrCategoryBibliographic, 0, LFViewList, LFMaxAttributePriority, FALSE, FALSE, FALSE, { 0,0 }, 42 },

	// LFAttrPages
	{ LFTypeUINT, 0, LFAttrCategoryBibliographic, 0, LFViewList, 3, TRUE, FALSE, FALSE, { SHPropertyDocuments, 14 }, 43 },

	// LFAttrSignature
	{ LFTypeAnsiString, 31, LFAttrCategoryBibliographic, 0, LFViewList, LFMaxAttributePriority, FALSE, FALSE, FALSE, { 0,0 }, 46 },


	// LFAttrFrom
	{ LFTypeAnsiString, 255, LFAttrCategoryTasks, 0, LFViewList, 0, TRUE, FALSE, FALSE, { 0,0 }, 47 },

	// LFAttrTo
	{ LFTypeAnsiString, 255, LFAttrCategoryTasks, 0, LFViewList, 0, TRUE, FALSE, FALSE, { 0,0 }, 48 },


	// LFAttrResponsible
	{ LFTypeUnicodeString, 255, LFAttrCategoryTasks, 0, LFViewList, LFMaxAttributePriority, FALSE, FALSE, FALSE, { 0,0 }, 49 },

	// LFAttrCustomer
	{ LFTypeUnicodeString, 255, LFAttrCategoryTasks, 0, LFViewList, 3, FALSE, FALSE, FALSE, { SHPropertyUnnamed7, 100 }, 52 }
};


#define WIDTH_HIDE        0
#define WIDTH_TINY       80
#define WIDTH_SMALL     100
#define WIDTH_MEDIUM    120
#define WIDTH_LARGE     150
#define WIDTH_HUGE      200

extern const LFTypeProperties TypeProperties[LFTypeCount] = {
	// LFTypeUnicodeString
	{ 0, TRUE, TRUE, FALSE, FALSE, WIDTH_LARGE,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewIcons },

	// LFTypeUnicodeArray
	{ 0, TRUE, FALSE, FALSE, FALSE, WIDTH_HUGE,
	(1<<LFViewIcons) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewTagcloud },

	// LFTypeAnsiString
	{ 0, TRUE, TRUE, FALSE, FALSE, WIDTH_LARGE,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewIcons },

	// LFTypeIATACode
	{ sizeof(CHAR)*4, TRUE, TRUE, FALSE, FALSE, WIDTH_TINY,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewGlobe) | (1<<LFViewTagcloud), LFViewList },

	// LFTypeFourCC
	{ sizeof(DWORD), TRUE, TRUE, FALSE, FALSE, WIDTH_SMALL,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewList },

	// LFTypeRating
	{ sizeof(BYTE), FALSE, TRUE, TRUE, FALSE, WIDTH_SMALL,
	(1<<LFViewList), LFViewList },

	// LFTypeUINT
	{ sizeof(UINT), FALSE, TRUE, TRUE, FALSE, WIDTH_SMALL,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewList },

	// LFTypeSize
	{ sizeof(INT64), TRUE, TRUE, TRUE, FALSE, WIDTH_TINY,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewIcons },

	// LFTypeFraction
	{ sizeof(LFFraction), FALSE, TRUE, FALSE, FALSE, WIDTH_TINY,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails), LFViewIcons },

	// LFTypeDouble
	{ sizeof(DOUBLE), FALSE, TRUE, TRUE, TRUE, WIDTH_SMALL,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails), LFViewIcons },

	// LFTypeColor
	{ sizeof(BYTE), FALSE, TRUE, FALSE, FALSE, WIDTH_HIDE,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails), LFViewList },

	// LFTypeGeoCoordinates
	{ sizeof(LFGeoCoordinates), TRUE, TRUE, FALSE, FALSE, WIDTH_LARGE,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewGlobe), LFViewIcons },

	// LFTypeTime
	{ sizeof(FILETIME), TRUE, TRUE, TRUE, FALSE, WIDTH_MEDIUM,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewCalendar) | (1<<LFViewTimeline) | (1<<LFViewTagcloud), LFViewTimeline },

	// LFTypeBitrate,
	{ sizeof(UINT), TRUE, TRUE, TRUE, TRUE, WIDTH_SMALL,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewIcons },

	// LFTypeDuration
	{ sizeof(UINT), TRUE, TRUE, TRUE, TRUE, WIDTH_SMALL,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewIcons },

	// LFTypeMegapixel
	{ sizeof(DOUBLE), TRUE, TRUE, TRUE, TRUE, WIDTH_MEDIUM,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewIcons },

	// LFTypeGenre
	{ sizeof(UINT), TRUE, TRUE, FALSE, FALSE, WIDTH_LARGE,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewIcons },

	// LFTypeApplication
	{ sizeof(BYTE), TRUE, TRUE, FALSE, FALSE, WIDTH_MEDIUM,
	(1<<LFViewIcons) | (1<<LFViewList) | (1<<LFViewDetails) | (1<<LFViewTagcloud), LFViewIcons }
};

#pragma data_seg()
