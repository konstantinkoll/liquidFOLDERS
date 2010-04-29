#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFItemDescriptor.h"
#include <assert.h>
#include <cmath>
#include <malloc.h>
#include <shlwapi.h>
#include <wchar.h>


// Der Inhalt dieses Segments wird über alle Instanzen von LFCore geteilt.
// Der Zugriff muss daher über Mutex-Objekte serialisiert/synchronisiert werden.
// Alle Variablen im Segment müssen initalisiert werden !

#pragma data_seg("common_attrs")

wchar_t RatingString[6] = L"\x2605\x2605\x2605\x2605\x2605";
wchar_t FlagStrings[8][4] = { L"---", L"--T", L"-N-", L"-NT", L"L--", L"L-T", L"LN-", L"LNT" };

size_t AttrSizes[LFTypeCount] = {
	0,							// LFTypeUnicodeString
	0,							// LFTypeAnsiString
	sizeof(UINT),				// LFTypeFourCC
	sizeof(unsigned char),		// LFTypeRating
	sizeof(UINT),				// LFTypeUINT
	sizeof(__int64),			// LFTypeINT64
	sizeof(LFFraction),			// LFTypeFraction
	sizeof(double),				// LFTypeDouble
	sizeof(UINT),				// LFTypeFlags
	sizeof(LFGeoCoordinates),	// LFTypeGeoCoordinates
	sizeof(FILETIME),			// LFTypeTime
	sizeof(UINT),				// LFTypeDuration
};

unsigned char AttrTypes[LFAttributeCount] = {
	LFTypeUnicodeString,		// LFAttrFileName
	LFTypeAnsiString,			// LFAttrStoreID
	LFTypeAnsiString,			// LFAttrFileID
	LFTypeUnicodeString,		// LFAttrComment
	LFTypeUnicodeString,		// LFAttrHint
	LFTypeTime,					// LFAttrCreationTime
	LFTypeTime,					// LFAttrFileTime
	LFTypeUINT,					// LFAttrFileFormat
	LFTypeINT64,				// LFAttrFileSize
	LFTypeFlags,				// LFAttrFlags
	LFTypeAnsiString,			// LFAttrURL
	LFTypeUnicodeString,		// LFAttrTags
	LFTypeRating,				// LFAttrRating
	LFTypeRating,				// LFAttrPriority
	LFTypeUnicodeString,		// LFAttrLocationName
	LFTypeAnsiString,			// LFAttrLocationIATA
	LFTypeGeoCoordinates,		// LFAttrLocationGPS

	LFTypeUINT,					// LFAttrHeight
	LFTypeUINT,					// LFAttrWidth
	LFTypeUINT,					// LFAttrResolution
	LFTypeDouble,				// LFAttrAspectRatio
	LFTypeFourCC,				// LFAttrVideoCodec
	LFTypeUnicodeString,		// LFAttrRoll

	LFTypeUnicodeString,		// LFAttrExposure
	LFTypeFraction,				// LFAttrFocus
	LFTypeFraction,				// LFAttrAperture
	LFTypeUnicodeString,		// LFAttrChip

	LFTypeUnicodeString,		// LFAttrAlbum
	LFTypeUINT,					// LFAttrChannels
	LFTypeUINT,					// LFAttrSamplerate
	LFTypeFourCC,				// LFAttrAudioCodec

	LFTypeDuration,				// LFAttrDuration
	LFTypeUINT,					// LFAttrBitrate

	LFTypeUnicodeString,		// LFAttrArtist
	LFTypeUnicodeString,		// LFAttrTitle
	LFTypeUnicodeString,		// LFAttrCopyright
	LFTypeAnsiString,			// LFAttrISBN
	LFTypeAnsiString,			// LFAttrLanguage
	LFTypeUINT,					// LFAttrPages
	LFTypeTime,					// LFAttrRecordingTime
	LFTypeUnicodeString,		// LFAttrRecordingEquipment
	LFTypeAnsiString,			// LFAttrSignature

	LFTypeAnsiString,			// LFAttrFrom
	LFTypeAnsiString,			// LFAttrTo
	LFTypeUnicodeString,		// LFAttrResponsible
	LFTypeTime,					// LFAttrDueTime
	LFTypeTime,					// LFAttrDoneTime
};

#pragma data_seg()
#pragma comment(linker, "/SECTION:common_attrs,RWS")


// Conversion ToString
//

inline double GetMinutes(double c)
{
	c = fabs(c)+ROUNDOFF;
	return (c-(double)(int)c)*60.0;
}

inline double GetSeconds(double c)
{
	c = fabs(c)*60.0+ROUNDOFF;
	return (c-(double)(int)c)*60.0;
}


LFCore_API void LFFourCCToString(const unsigned int c, wchar_t* str, size_t cCount)
{
	if (cCount>=5)
	{
		str[0] = c & 0xFF;
		str[1] = (c>>8) & 0xFF;
		str[2] = (c>>16) & 0xFF;
		str[3] = c>>24;
		str[4] = '\0';
	}
}

LFCore_API void LFUINTToString(const unsigned int v, wchar_t* str, size_t cCount)
{
	swprintf(str, cCount, L"%d", v);
}

LFCore_API void LFINT64ToString(const __int64 v, wchar_t* str, size_t cCount)
{
	StrFormatByteSizeW(v, str, (unsigned int)cCount);
}

LFCore_API void LFFractionToString(const LFFraction frac, wchar_t* str, size_t cCount)
{
	swprintf(str, cCount, L"%u/%u", frac.Num, frac.Denum);
}

LFCore_API void LFDoubleToString(const double d, wchar_t* str, size_t cCount)
{
	swprintf(str, cCount, L"%f", d);
}

LFCore_API void LFGeoCoordinateToString(const double c, wchar_t* str, size_t cCount, bool IsLatitude)
{
	wchar_t Hemisphere[2];
	if (IsLatitude)
	{
		Hemisphere[0] = 'N';
		Hemisphere[1] = 'S';
	}
	else
	{
		Hemisphere[0] = 'W';
		Hemisphere[1] = 'E';
	}

	swprintf(str, cCount, L"%u°%u\'%u\"%c",
		(unsigned int)(fabs(c)+ROUNDOFF),
		(unsigned int)GetMinutes(c),
		(unsigned int)(GetSeconds(c)+0.5),
		Hemisphere[c>0]);
}

LFCore_API void LFGeoCoordinatesToString(const LFGeoCoordinates c, wchar_t* str, size_t cCount)
{
	if ((c.Latitude==0) && (c.Longitude==0))
	{
		wcscpy_s(str, cCount, L"");
	}
	else
	{
		wchar_t tmpStr[256];
		LFGeoCoordinateToString(c.Longitude, tmpStr, 256, false);

		LFGeoCoordinateToString(c.Latitude, str, cCount, true);
		wcscat_s(str, cCount, L", ");
		wcscat_s(str, cCount, tmpStr);
	}
}

LFCore_API void LFTimeToString(const FILETIME t, wchar_t* str, size_t cCount, unsigned int mask)
{
	*str = '\0';

	if ((t.dwHighDateTime) || (t.dwLowDateTime))
	{
		SYSTEMTIME st;
		FileTimeToSystemTime(&t, &st);

		if (mask & 1)
		{
			int cDate = GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, NULL, 0);
			if (cDate>256)
				cDate = 256;
			GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, str, cDate);
		}

		if (mask==3)
			wcscat_s(str, cCount, L", ");

		if (mask & 2)
		{
			wchar_t tmpStr[256];
			int cTime = GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &st, NULL, NULL, 0);
			if (cTime>256)
				cTime = 256;
			GetTimeFormat(LOCALE_USER_DEFAULT, TIME_FORCE24HOURFORMAT, &st, NULL, tmpStr, cTime);
			wcscat_s(str, cCount, tmpStr);
		}
	}
}

LFCore_API void LFDurationToString(unsigned int d, wchar_t* str, size_t cCount)
{
	d = (d+999)/1000;
	swprintf(str, cCount, L"%02d:%02d:%02d", d/3600, (d/60)%60, d%60);
}

inline void AttributeToString(LFItemDescriptor* i, unsigned int attr)
{
	assert(i);
	assert(attr<LFAttributeCount);
	assert(AttrTypes[attr]<LFTypeCount);
	assert(i->AttributeValues[attr]);

	if (!i->AttributeStrings[attr])
	{
		wchar_t tmpStr[256];
		size_t sz;

		switch (AttrTypes[attr])
		{
		case LFTypeUnicodeString:
			i->AttributeStrings[attr] = (wchar_t*)i->AttributeValues[attr];
			return;
		case LFTypeAnsiString:
			sz = strlen((char*)i->AttributeValues[attr])+1;
			MultiByteToWideChar(CP_ACP, 0, (char*)i->AttributeValues[attr], (int)sz, tmpStr, (int)sz);
			break;
		case LFTypeFourCC:
			LFFourCCToString(*((unsigned int*)i->AttributeValues[attr]), tmpStr, 5);
			break;
		case LFTypeRating:
			assert(*((unsigned char*)i->AttributeValues[attr])<=LFMaxRating);
			i->AttributeStrings[attr] = &RatingString[5-*((unsigned char*)i->AttributeValues[attr])/2];
			return;
		case LFTypeUINT:
			LFUINTToString(*((unsigned int*)i->AttributeValues[attr]), tmpStr, 256);
			break;
		case LFTypeINT64:
			LFINT64ToString(*((__int64*)i->AttributeValues[attr]), tmpStr, 256);
			break;
		case LFTypeFraction:
			LFFractionToString(*((LFFraction*)i->AttributeValues[attr]), tmpStr, 256);
			break;
		case LFTypeDouble:
			LFDoubleToString(*((double*)i->AttributeValues[attr]), tmpStr, 256);
			break;
		case LFTypeFlags:
			i->AttributeStrings[attr] = &FlagStrings[(unsigned int)i->AttributeValues[attr] & ((1<<(LFLastFlagBit+1))-1)][0];
			return;
		case LFTypeGeoCoordinates:
			LFGeoCoordinatesToString(*((LFGeoCoordinates*)i->AttributeValues[attr]), tmpStr, 256);
			break;
		case LFTypeTime:
			LFTimeToString(*((FILETIME*)i->AttributeValues[attr]), tmpStr, 256);
			break;
		case LFTypeDuration:
			LFDurationToString(*((unsigned int*)i->AttributeValues[attr]), tmpStr, 256);
			break;
		default:
			assert(false);
		}

		sz = wcslen(tmpStr)+1;
		if (sz>1)
		{
			i->AttributeStrings[attr] = (wchar_t*)malloc(sz*sizeof(wchar_t));
			assert(i->AttributeStrings[attr]);
			wcscpy_s(i->AttributeStrings[attr], sz, tmpStr);
		}
	}
}

inline void AttributesToString(LFItemDescriptor* i)
{
	assert(i);

	for (unsigned int a=0; a<LFAttributeCount; a++)
		if (i->AttributeValues[a])
			AttributeToString(i, a);
}


// Attribute handling
//

inline bool IsSlaveAttribute(LFItemDescriptor* i, unsigned int attr)
{
	assert(i);

	if (!i->Slave)
		return false;

	return (i->AttributeValues[attr]>=(char*)i->Slave) && (i->AttributeValues[attr]<(char*)i->Slave+_msize(i->Slave));
}

inline void FreeAttribute(LFItemDescriptor* i, unsigned int attr)
{
	assert(i);
	assert(attr<LFAttributeCount);

	if ((AttrTypes[attr]!=LFTypeRating) && (AttrTypes[attr]!=LFTypeFlags) && (i->AttributeStrings[attr]))
		// Repräsentation als Unicode-String nur dann freigeben, wenn ungleich dem eigentlich Attributwert
		if (i->AttributeStrings[attr]!=i->AttributeValues[attr])
		{
			free(i->AttributeStrings[attr]);
			i->AttributeStrings[attr] = NULL;
		}

	// Attributwert nur dann freigeben wenn er nicht statischer Teil des LFItemDescriptor ist und auch nicht zum Slave gehört
	if ((attr>LFLastLocalAttribute) && (!IsSlaveAttribute(i, attr)) && (i->AttributeValues[attr]))
	{
		free(i->AttributeValues[attr]);
		i->AttributeValues[attr] = NULL;
	}
}

size_t GetAttributeMaxCharacterCount(unsigned int attr)
{
	assert(attr<LFAttributeCount);
	assert((AttrTypes[attr]==LFTypeUnicodeString) || (AttrTypes[attr]==LFTypeAnsiString));

	switch (attr)
	{
	case LFAttrStoreID:
	case LFAttrFileID:
		return LFKeySize-1;
	case LFAttrLanguage:
		return 2;
	case LFAttrLocationIATA:
		return 3;
	case LFAttrExposure:
	case LFAttrChip:
	case LFAttrISBN:
	case LFAttrSignature:
		return 31;
	}

	return 255;
}

inline size_t GetAttributeSize(unsigned int attr, const void* v)
{
	assert(attr<LFAttributeCount);
	assert(AttrTypes[attr]<LFTypeCount);

	switch (AttrTypes[attr])
	{
	case LFTypeUnicodeString:
		return (min(GetAttributeMaxCharacterCount(attr), wcslen((wchar_t*)v))+1)*sizeof(wchar_t);
	case LFTypeAnsiString:
		return min(GetAttributeMaxCharacterCount(attr), strlen((char*)v))+1;
	default:
		return AttrSizes[AttrTypes[attr]];
	}
}

void SetAttribute(LFItemDescriptor* i, unsigned int attr, const void* v, bool toString, wchar_t* ustr)
{
	assert(i);
	assert(attr<LFAttributeCount);
	assert(v);

	// Altes Attribut freigeben
	FreeAttribute(i, attr);

	// Größe ermitteln
	size_t sz = GetAttributeSize(attr, v);

	// Ggf. Speicher reservieren
	if (!i->AttributeValues[attr])
	{
		i->AttributeValues[attr] = malloc(sz);
		assert(i->AttributeValues[attr]);
	}

	// Kopieren
	memcpy_s(i->AttributeValues[attr], sz, v, sz);

	// Repräsentation als Unicode-String
	if (toString)
		if ((ustr) && (AttrTypes[attr]!=LFTypeRating) && (AttrTypes[attr]!=LFTypeFlags))
		{
			sz = wcslen(ustr)+1;
			if (sz>1)
			{
				i->AttributeStrings[attr] = (wchar_t*)malloc(sz*sizeof(wchar_t));
				assert(i->AttributeStrings[attr]);
				wcscpy_s(i->AttributeStrings[attr], sz, ustr);
			}
		}
		else
		{
			AttributeToString(i, attr);
		}
}


// LFItemDescriptor
//

LFCore_API LFItemDescriptor* LFAllocItemDescriptor()
{
	LFItemDescriptor* d = static_cast<LFItemDescriptor*>(malloc(sizeof(LFItemDescriptor)));
	ZeroMemory(d, sizeof(LFItemDescriptor));
	d->Position = -1;
	d->RefCount = 1;

	// Zeiger auf statische Attributwerte initalisieren
	// Für Attribute, die Unicode-Strings sind, auch die Zeiger für Unicode-String initalisieren
	d->AttributeValues[LFAttrFileName] = d->AttributeStrings[LFAttrFileName] = &d->CoreAttributes.FileName[0];
	d->AttributeValues[LFAttrStoreID] = &d->CoreAttributes.StoreID;
	d->AttributeValues[LFAttrFileID] = &d->CoreAttributes.FileID;
	d->AttributeValues[LFAttrComment] = d->AttributeStrings[LFAttrComment] = &d->CoreAttributes.Comment[0];
	d->AttributeValues[LFAttrHint] = d->AttributeStrings[LFAttrHint] = &d->Hint[0];
	d->AttributeValues[LFAttrCreationTime] = &d->CoreAttributes.CreationTime;
	d->AttributeValues[LFAttrFileTime] = &d->CoreAttributes.FileTime;
	d->AttributeValues[LFAttrFileFormat] = &d->CoreAttributes.FileFormat;
	d->AttributeValues[LFAttrFileSize] = &d->CoreAttributes.FileSize;
	d->AttributeValues[LFAttrFlags] = &d->CoreAttributes.Flags;
	d->AttributeValues[LFAttrURL] = &d->CoreAttributes.URL;
	d->AttributeValues[LFAttrTags] = d->AttributeStrings[LFAttrTags] = &d->CoreAttributes.Tags[0];
	d->AttributeValues[LFAttrRating] = &d->CoreAttributes.Rating;
	d->AttributeValues[LFAttrPriority] = &d->CoreAttributes.Priority;
	d->AttributeValues[LFAttrLocationName] = d->AttributeStrings[LFAttrLocationName] = &d->CoreAttributes.LocationName[0];
	d->AttributeValues[LFAttrLocationIATA] = &d->CoreAttributes.LocationIATA;
	d->AttributeValues[LFAttrLocationGPS] = &d->CoreAttributes.LocationGPS;

	return d;
}

LFCore_API LFItemDescriptor* LFAllocItemDescriptor(LFItemDescriptor* i)
{
	LFItemDescriptor* d = LFAllocItemDescriptor();
	d->CategoryID = i->CategoryID;
	d->CoreAttributes = i->CoreAttributes;
	d->DeleteFlag = i->DeleteFlag;
	wcscpy_s(d->Hint, 256, i->Hint);
	d->IconID = i->IconID;
	d->Type = i->Type;

	if (i->NextFilter)
		d->NextFilter = LFAllocFilter(i->NextFilter);

	if (i->Slave)
	{
		size_t sz = _msize(i->Slave);
		d->Slave = malloc(sz);
		memcpy(d->Slave, i->Slave, sz);
	}

	for (unsigned int a=0; a<LFAttributeCount; a++)
	{
		// Value
		if ((a>LFLastLocalAttribute) && (i->AttributeValues[a]))
			if (IsSlaveAttribute(i, a))
			{
				__int64 ofs = (char*)i->AttributeValues[a]-(char*)i->Slave;
				d->AttributeValues[a] = (char*)d->Slave+ofs;
			}
			else
			{
				size_t sz = _msize(i->AttributeValues[a]);
				d->AttributeValues[a] = malloc(sz);
				memcpy_s(d->AttributeValues[a], sz, i->AttributeValues, sz);
			}

		// String
		if ((i->AttributeStrings[a]) && (!d->AttributeStrings[a]))
			if (i->AttributeStrings[a]==i->AttributeValues[a])
			{
				d->AttributeStrings[a] = (wchar_t*)d->AttributeValues[a];
			}
			else
			{
				if ((AttrTypes[a]==LFTypeFlags) || (AttrTypes[a]==LFTypeRating))
				{
					d->AttributeStrings[a] = i->AttributeStrings[a];
				}
				else
				{
					size_t sz = _msize(i->AttributeStrings[a]);
					d->AttributeStrings[a] = (wchar_t*)malloc(sz);
					memcpy_s(d->AttributeStrings[a], sz, i->AttributeStrings[a], sz);
				}
			}
	}

	return d;
}

LFCore_API void LFFreeItemDescriptor(LFItemDescriptor* i)
{
	if (i)
	{
		i->RefCount--;
		if (!i->RefCount)
		{
			LFFreeFilter(i->NextFilter);
			for (unsigned int a=0; a<LFAttributeCount; a++)
				FreeAttribute(i, a);
			if (i->Slave)
				free(i->Slave);
			free(i);
		}
	}
}
