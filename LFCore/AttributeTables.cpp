
#include "stdafx.h"
#include "AttributeTables.h"


#pragma data_seg(".shared")

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
	{ LFTypeUnicodeString, 255, LFAttrCategoryBasic, FALSE, { SHPropertyStorage, 10 },
	},

	// LFAttrStoreID
	{ LFTypeAnsiString, LFKeySize-1, LFAttrCategoryInternal, TRUE, { 0, 0 },
	},

	// LFAttrFileID
	{ LFTypeAnsiString, LFKeySize-1, LFAttrCategoryInternal, TRUE, { SHPropertyStorage, 8 },
	},

	// LFAttrComments
	{ LFTypeUnicodeString, 255, LFAttrCategoryBasic, FALSE, { SHPropertySummary, 6 },
	},

	// LFAttrDescription
	{ LFTypeUnicodeString, 255, LFAttrCategoryBasic, TRUE, { SHPropertyVersion, 3 },
	},

	// LFAttrCreationTime
	{ LFTypeTime, 0, LFAttrCategoryBasic, TRUE, { SHPropertyStorage, 15 },
	},

	// LFAttrAddTime
	{ LFTypeTime, 0, LFAttrCategoryInternal, TRUE, { SHPropertyUnnamed5, 100 },
	},

	// LFAttrFileTime
	{ LFTypeTime, 0, LFAttrCategoryInternal, TRUE, { SHPropertyStorage, 14 },
	},

	// LFAttrDeleteTime
	{ LFTypeTime, 0, LFAttrCategoryBasic, TRUE, { 0, 0 },
	},

	// LFAttrArchiveTime
	{ LFTypeTime, 0, LFAttrCategoryBasic, TRUE, { SHPropertyUnnamed6, 100 },
	},

	// LFAttrFileFormat
	{ LFTypeAnsiString, LFExtSize-1, LFAttrCategoryInternal, TRUE, { SHPropertyStorage, 4 },
	},

	// LFAttrFileCount
	{ LFTypeUINT, 0, LFAttrCategoryBasic, TRUE, { 0, 0 },
	},

	// LFAttrFileSize
	{ LFTypeSize, 0, LFAttrCategoryBasic, TRUE, { SHPropertyStorage, 12 },
	},

	// LFAttrFlags
	{ LFTypeFlags, 0, LFAttrCategoryInternal, TRUE, { 0, 0 },
	},

	// LFAttrURL
	{ LFTypeAnsiString, 255, LFAttrCategoryBasic, FALSE, { SHPropertyQuery, 9 },
	},

	// LFAttrHashtags
	{ LFTypeUnicodeString, 255, LFAttrCategoryBasic, FALSE, { SHPropertySummary, 5 },
	},

	// LFAttrRating
	{ LFTypeRating, 0, LFAttrCategoryBasic, FALSE, { SHPropertyMedia, 9 },
	},

	// LFAttrPriority
	{ LFTypeRating, 0, LFAttrCategoryWorkflow, FALSE, { SHPropertyUnnamed3, 11 },
	},

	// LFAttrLocationName
	{ LFTypeUnicodeString, 255, LFAttrCategoryGeotags, FALSE, { 0, 0 },
	},

	// LFAttrLocationIATA
	{ LFTypeAnsiString, 3, LFAttrCategoryGeotags, FALSE, { 0, 0 },
	},

	// LFAttrLocationGPS
	{ LFTypeGeoCoordinates, 0, LFAttrCategoryGeotags, FALSE, { 0, 0 },
	},


	// LFAttrWidth
	{ LFTypeUINT, 0, LFAttrCategoryVisual, TRUE, { SHPropertyImage, 3 },
	},

	// LFAttrHeight
	{ LFTypeUINT, 0, LFAttrCategoryVisual, TRUE, { SHPropertyImage, 4 },
	},

	// LFAttrDimension
	{ LFTypeMegapixel, 0, LFAttrCategoryVisual, TRUE, { 0, 0 },
	},

	// LFAttrAspectRatio
	{ LFTypeDouble, 0, LFAttrCategoryVisual, TRUE, { 0, 0 },
	},

	// LFAttrVideoCodec
	{ LFTypeFourCC, 0, LFAttrCategoryVisual, TRUE, { SHPropertyVideo, 44 },
	},

	// LFAttrRoll
	{ LFTypeUnicodeString, 255, LFAttrCategoryVisual, FALSE, { SHPropertyPhoto, 18248 },
	},


	// LFAttrExposure
	{ LFTypeUnicodeString, 31, LFAttrCategoryPhotographic, TRUE, { 0, 0 },
	},

	// LFAttrFocus
	{ LFTypeFraction, 0, LFAttrCategoryPhotographic, TRUE, { 0, 0 },
	},

	// LFAttrAperture
	{ LFTypeFraction, 0, LFAttrCategoryPhotographic, TRUE, { 0, 0 },
	},

	// LFAttrChip
	{ LFTypeUnicodeString, 31, LFAttrCategoryPhotographic, TRUE, { 0, 0 },
	},


	// LFAttrAlbum
	{ LFTypeUnicodeString, 255, LFAttrCategoryAudio, FALSE, { SHPropertyMusic, 4 },
	},

	// LFAttrGenre
	{ LFTypeUINT, 0, LFAttrCategoryAudio, FALSE, { SHPropertyMusic, 11 },
	},

	// LFAttrChannels
	{ LFTypeUINT, 0, LFAttrCategoryAudio, TRUE, { SHPropertyMedia, 7 },
	},

	// LFAttrSamplerate
	{ LFTypeUINT, 0, LFAttrCategoryAudio, TRUE, { SHPropertyMedia, 5 },
	},

	// LFAttrAudioCodec
	{ LFTypeFourCC, 0, LFAttrCategoryAudio, TRUE, { 0, 0 },
	},


	// LFAttrDuration
	{ LFTypeDuration, 0, LFAttrCategoryTimebased, TRUE, { SHPropertyAudio, 3 },
	},

	// LFAttrBitrate
	{ LFTypeBitrate, 0, LFAttrCategoryTimebased, TRUE, { SHPropertyMedia, 4 },
	},


	// LFAttrArtist
	{ LFTypeUnicodeString, 255, LFAttrCategoryBibliographic, FALSE, { SHPropertySummary, 4 },
	},

	// LFAttrTitle
	{ LFTypeUnicodeString, 255, LFAttrCategoryBibliographic, FALSE, { SHPropertySummary, 2 },
	},

	// LFAttrCopyright
	{ LFTypeUnicodeString, 255, LFAttrCategoryBibliographic, FALSE, { SHPropertyMedia, 11 },
	},

	// LFAttrISBN
	{ LFTypeAnsiString, 31, LFAttrCategoryBibliographic, FALSE, { 0, 0 },
	},

	// LFAttrLanguage
	{ LFTypeAnsiString, 2, LFAttrCategoryBibliographic, FALSE, { 0, 0 },
	},

	// LFAttrPages
	{ LFTypeUINT, 0, LFAttrCategoryBibliographic, TRUE, { SHPropertyDocuments, 14 },
	},

	// LFAttrRecordingTime
	{ LFTypeTime, 0, LFAttrCategoryBibliographic, FALSE, { SHPropertyUnnamed4, 100 },
	},

	// LFAttrRecordingEquipment
	{ LFTypeUnicodeString, 255, LFAttrCategoryBibliographic, TRUE, { SHPropertyPhoto, 272 },
	},

	// LFAttrSignature
	{ LFTypeAnsiString, 31, LFAttrCategoryBibliographic, FALSE, { 0, 0 },
	},


	// LFAttrFrom
	{ LFTypeAnsiString, 255, LFAttrCategoryWorkflow, TRUE, { 0, 0 },
	},

	// LFAttrTo
	{ LFTypeAnsiString, 255, LFAttrCategoryWorkflow, TRUE, { 0, 0 },
	},

	// LFAttrResponsible
	{ LFTypeUnicodeString, 255, LFAttrCategoryWorkflow, FALSE, { 0, 0 },
	},

	// LFAttrDueTime
	{ LFTypeTime, 0, LFAttrCategoryWorkflow, FALSE, { SHPropertyUnnamed1, 100 },
	},

	// LFAttrDoneTime
	{ LFTypeTime, 0, LFAttrCategoryWorkflow, FALSE, { SHPropertyUnnamed2, 100 },
	},

	// LFAttrCustomer
	{ LFTypeUnicodeString, 255, LFAttrCategoryWorkflow, FALSE, { SHPropertyUnnamed7, 100 },
	}
};

extern const LFTypeProperties TypeProperties[LFTypeCount] = {
	// LFTypeUnicodeString
	{ 0, TRUE, TRUE, FALSE, FALSE, 200 },

	// LFTypeUnicodeArray
	{ 0, TRUE, TRUE, FALSE, FALSE, 200 },

	// LFTypeAnsiString
	{ 0, TRUE, TRUE, FALSE, FALSE, 200 },

	// LFTypeFourCC
	{ sizeof(DWORD), TRUE, TRUE, FALSE, FALSE, 100 },

	// LFTypeRating
	{ sizeof(BYTE), FALSE, TRUE, TRUE, FALSE, 100 },

	// LFTypeUINT
	{ sizeof(UINT), FALSE, TRUE, FALSE, TRUE, 100 },

	// LFTypeSize
	{ sizeof(INT64), TRUE, TRUE, FALSE, TRUE, 120 },

	// LFTypeFraction
	{ sizeof(LFFraction), FALSE, TRUE, FALSE, TRUE, 100 },

	// LFTypeDouble
	{ sizeof(DOUBLE), FALSE, TRUE, FALSE, TRUE, 100 },

	// LFTypeFlags
	{ sizeof(UINT), TRUE, FALSE, FALSE, TRUE, 100 },

	// LFTypeGeoCoordinates
	{ sizeof(LFGeoCoordinates), TRUE, TRUE, FALSE, TRUE, 150 },

	// LFTypeTime
	{ sizeof(FILETIME), TRUE, TRUE, TRUE, TRUE, 140 },

	// LFTypeBitrate,
	{ sizeof(UINT), TRUE, TRUE, TRUE, TRUE, 100 },

	// LFTypeDuration
	{ sizeof(UINT), TRUE, TRUE, FALSE, TRUE, 100 },

	// LFTypeMegapixel
	{ sizeof(DOUBLE), TRUE, TRUE, TRUE, TRUE, 120 },
};

#pragma data_seg()
