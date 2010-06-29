#include "StdAfx.h"
#include "..\\include\\LFCore.h"
#include "LFItemDescriptor.h"
#include <assert.h>
#include <malloc.h>


// Der Inhalt dieses Segments wird über alle Instanzen von LFCore geteilt.
// Der Zugriff muss daher über Mutex-Objekte serialisiert/synchronisiert werden.
// Alle Variablen im Segment müssen initalisiert werden !

#pragma data_seg("common_attrs")

int CoreOffsets[LFLastCoreAttribute+1] = {
	offsetof(LFCoreAttributes, FileName),
	-1,
	offsetof(LFCoreAttributes, FileID),
	offsetof(LFCoreAttributes, Comment),
	-1,
	offsetof(LFCoreAttributes, CreationTime),
	offsetof(LFCoreAttributes, FileTime),
	offsetof(LFCoreAttributes, DeleteTime),
	offsetof(LFCoreAttributes, FileFormat),
	offsetof(LFCoreAttributes, FileSize),
	offsetof(LFCoreAttributes, Flags),
	offsetof(LFCoreAttributes, URL),
	offsetof(LFCoreAttributes, Tags),
	offsetof(LFCoreAttributes, Rating),
	offsetof(LFCoreAttributes, Priority),
	offsetof(LFCoreAttributes, LocationName),
	offsetof(LFCoreAttributes, LocationIATA),
	offsetof(LFCoreAttributes, LocationGPS)
};

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
	LFTypeTime,					// LFAttrDeleteTime
	LFTypeAnsiString,			// LFAttrFileFormat
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

	// Attributwert nur dann freigeben wenn er nicht statischer Teil des LFItemDescriptor ist und auch nicht zum Slave gehört
	if ((attr>LFLastCoreAttribute) && (!IsSlaveAttribute(i, attr)) && (i->AttributeValues[attr]))
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
	case LFAttrFileFormat:
		return LFExtSize-1;
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

void SetAttribute(LFItemDescriptor* i, unsigned int attr, const void* v)
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
	memcpy(i->AttributeValues[attr], v, sz);
}


// LFItemDescriptor
//

LFCore_API LFItemDescriptor* LFAllocItemDescriptor(LFItemDescriptor* i)
{
	LFItemDescriptor* d = static_cast<LFItemDescriptor*>(malloc(sizeof(LFItemDescriptor)));
	ZeroMemory(d, sizeof(LFItemDescriptor));
	d->Position = d->FirstAggregate = d->LastAggregate = -1;
	d->RefCount = 1;

	// Zeiger auf statische Attributwerte initalisieren
	d->AttributeValues[LFAttrStoreID] = &d->StoreID;
	d->AttributeValues[LFAttrHint] = &d->Hint[0];

	for (unsigned int a=0; a<=LFLastCoreAttribute; a++)
		if (CoreOffsets[a]!=-1)
			d->AttributeValues[a] = (char*)&d->CoreAttributes + CoreOffsets[a];

	if (i)
	{
		d->CategoryID = i->CategoryID;
		d->CoreAttributes = i->CoreAttributes;
		d->DeleteFlag = i->DeleteFlag;
		strcpy_s(d->StoreID, LFKeySize, i->StoreID);
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

		for (unsigned int a=LFLastCoreAttribute+1; a<LFAttributeCount; a++)
			if (i->AttributeValues[a])
				if (IsSlaveAttribute(i, a))
				{
					__int64 ofs = (char*)i->AttributeValues[a]-(char*)i->Slave;
					d->AttributeValues[a] = (char*)d->Slave+ofs;
				}
				else
				{
					size_t sz = _msize(i->AttributeValues[a]);
					d->AttributeValues[a] = malloc(sz);
					memcpy(d->AttributeValues[a], i->AttributeValues[a], sz);
				}
	}

	return d;
}

LFItemDescriptor* AllocFolderDescriptor(const wchar_t* Name, const wchar_t* Comment, const wchar_t* Hint, const char* StoreID, const char* FileID, __int64* Size, unsigned int IconID, unsigned int CategoryID, LFFilter* Filter)
{
	LFItemDescriptor* d = LFAllocItemDescriptor();

	d->IconID = IconID;
	d->CategoryID = CategoryID;
	d->Type = LFTypeVirtual;
	d->NextFilter = Filter;

	if (Name)
		SetAttribute(d, LFAttrFileName, Name);
	if (StoreID)
		SetAttribute(d, LFAttrStoreID, StoreID);
	if (FileID)
		SetAttribute(d, LFAttrFileID, FileID);
	if (Comment)
		SetAttribute(d, LFAttrComment, Comment);
	if (Hint)
		SetAttribute(d, LFAttrHint, Hint);
	if (Size)
		SetAttribute(d, LFAttrFileSize, Size);

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
