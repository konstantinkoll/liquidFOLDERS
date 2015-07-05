#include "stdafx.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "CIndex.h"
#include "StoreCache.h"
#include <assert.h>
#include <malloc.h>


extern const INT CoreOffsets[LFLastCoreAttribute+1] = {
	offsetof(LFCoreAttributes, FileName),
	-1,
	offsetof(LFCoreAttributes, FileID),
	offsetof(LFCoreAttributes, Comment),
	-1,
	offsetof(LFCoreAttributes, CreationTime),
	offsetof(LFCoreAttributes, FileTime),
	offsetof(LFCoreAttributes, AddTime),
	offsetof(LFCoreAttributes, DeleteTime),
	offsetof(LFCoreAttributes, ArchiveTime),
	offsetof(LFCoreAttributes, FileFormat),
	-1,
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

extern const SIZE_T AttrSizes[LFTypeCount] = {
	0,							// LFTypeUnicodeString
	0,							// LFTypeUnicodeArray
	0,							// LFTypeAnsiString
	sizeof(DWORD),				// LFTypeFourCC
	sizeof(BYTE),				// LFTypeRating
	sizeof(UINT),				// LFTypeUINT
	sizeof(INT64),				// LFTypeSize
	sizeof(LFFraction),			// LFTypeFraction
	sizeof(DOUBLE),				// LFTypeDouble
	sizeof(UINT),				// LFTypeFlags
	sizeof(LFGeoCoordinates),	// LFTypeGeoCoordinates
	sizeof(FILETIME),			// LFTypeTime
	sizeof(UINT32),				// LFTypeDuration
	sizeof(UINT),				// LFTypeBitrate,
	sizeof(DOUBLE)				// LFTypeMegapixel
};

extern const BYTE AttrTypes[LFAttributeCount] = {
	LFTypeUnicodeString,		// LFAttrFileName
	LFTypeAnsiString,			// LFAttrStoreID
	LFTypeAnsiString,			// LFAttrFileID
	LFTypeUnicodeString,		// LFAttrComment
	LFTypeUnicodeString,		// LFAttrDescription
	LFTypeTime,					// LFAttrCreationTime
	LFTypeTime,					// LFAttrAddTime
	LFTypeTime,					// LFAttrFileTime
	LFTypeTime,					// LFAttrDeleteTime
	LFTypeTime,					// LFAttrArchiveTime
	LFTypeAnsiString,			// LFAttrFileFormat
	LFTypeUINT,					// LFAttrFileCount
	LFTypeSize,					// LFAttrFileSize
	LFTypeFlags,				// LFAttrFlags
	LFTypeAnsiString,			// LFAttrURL
	LFTypeUnicodeArray,			// LFAttrTags
	LFTypeRating,				// LFAttrRating
	LFTypeRating,				// LFAttrPriority
	LFTypeUnicodeString,		// LFAttrLocationName
	LFTypeAnsiString,			// LFAttrLocationIATA
	LFTypeGeoCoordinates,		// LFAttrLocationGPS

	LFTypeUINT,					// LFAttrWidth
	LFTypeUINT,					// LFAttrHeight
	LFTypeMegapixel,			// LFAttrDimension
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
	LFTypeBitrate,				// LFAttrBitrate

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
	LFTypeUnicodeString,		// LFAttrCustomer
	LFTypeUINT					// LFAttrLikeCount
};

extern HMODULE LFCoreModuleHandle;


// Attribute handling
//

__forceinline BOOL IsSlaveAttribute(LFItemDescriptor* i, UINT Attr)
{
	assert(i);

	if (!i->Slave)
		return FALSE;

	return (i->AttributeValues[Attr]>=(CHAR*)i->Slave) && (i->AttributeValues[Attr]<(CHAR*)i->Slave+_msize(i->Slave));
}

void FreeAttribute(LFItemDescriptor* i, UINT Attr)
{
	assert(i);
	assert(Attr<LFAttributeCount);

	// Attributwert nur dann freigeben wenn er nicht statischer Teil des LFItemDescriptor ist und auch nicht zum Slave gehört
	if ((Attr>LFLastCoreAttribute) && (!IsSlaveAttribute(i, Attr)) && (i->AttributeValues[Attr]))
	{
		free(i->AttributeValues[Attr]);
		i->AttributeValues[Attr] = NULL;
	}
}

SIZE_T GetAttributeMaxCharacterCount(UINT Attr)
{
	assert(Attr<LFAttributeCount);
	assert((AttrTypes[Attr]==LFTypeUnicodeString) || (AttrTypes[Attr]==LFTypeUnicodeArray) || (AttrTypes[Attr]==LFTypeAnsiString));

	switch (Attr)
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

__forceinline SIZE_T GetAttributeSize(UINT Attr, const void* v)
{
	assert(Attr<LFAttributeCount);
	assert(AttrTypes[Attr]<LFTypeCount);

	switch (AttrTypes[Attr])
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
		return (min(GetAttributeMaxCharacterCount(Attr), wcslen((WCHAR*)v))+1)*sizeof(WCHAR);
	case LFTypeAnsiString:
		return min(GetAttributeMaxCharacterCount(Attr), strlen((CHAR*)v))+1;
	default:
		return AttrSizes[AttrTypes[Attr]];
	}
}

void SetAttribute(LFItemDescriptor* i, UINT Attr, const void* v)
{
	assert(i);
	assert(Attr<LFAttributeCount);
	assert(v);

	// Altes Attribut freigeben
	FreeAttribute(i, Attr);

	// Größe ermitteln
	SIZE_T sz = GetAttributeSize(Attr, v);

	// Ggf. Speicher reservieren
	if (!i->AttributeValues[Attr])
	{
		i->AttributeValues[Attr] = malloc(sz);
		assert(i->AttributeValues[Attr]);
	}

	// Kopieren
	switch (AttrTypes[Attr])
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
		wcsncpy_s((WCHAR*)i->AttributeValues[Attr], sz/sizeof(WCHAR), (WCHAR*)v, (sz/sizeof(WCHAR))-1);
		break;
	case LFTypeAnsiString:
		strncpy_s((CHAR*)i->AttributeValues[Attr], sz, (CHAR*)v, sz-1);
		break;
	default:
		memcpy(i->AttributeValues[Attr], v, sz);
	}
}


// LFItemDescriptor
//

LFCORE_API LFItemDescriptor* LFAllocItemDescriptor(LFItemDescriptor* i)
{
	LFItemDescriptor* d = new LFItemDescriptor;
	ZeroMemory(d, sizeof(LFItemDescriptor));
	d->FirstAggregate = d->LastAggregate = -1;
	d->RefCount = 1;

	// Zeiger auf statische Attributwerte initalisieren
	for (UINT a=0; a<=LFLastCoreAttribute; a++)
		d->AttributeValues[a] = (CHAR*)&d->CoreAttributes + CoreOffsets[a];

	d->AttributeValues[LFAttrStoreID] = &d->StoreID;
	d->AttributeValues[LFAttrDescription] = &d->Description[0];
	d->AttributeValues[LFAttrFileCount] = &d->AggregateCount;

	// Ggf. Werte kopieren
	if (i)
	{
		d->CategoryID = i->CategoryID;
		d->CoreAttributes = i->CoreAttributes;
		d->DeleteFlag = i->DeleteFlag;
		d->AggregateCount = i->AggregateCount;
		d->FirstAggregate = i->FirstAggregate;
		d->LastAggregate = i->LastAggregate;
		strcpy_s(d->StoreID, LFKeySize, i->StoreID);
		wcscpy_s(d->Description, 256, i->Description);
		d->IconID = i->IconID;
		d->Type = i->Type;

		if (i->NextFilter)
			d->NextFilter = LFAllocFilter(i->NextFilter);

		if (i->Slave)
		{
			SIZE_T sz = _msize(i->Slave);
			d->Slave = malloc(sz);
			memcpy(d->Slave, i->Slave, sz);
		}

		for (UINT a=LFLastCoreAttribute+1; a<LFAttributeCount; a++)
			if (i->AttributeValues[a])
				if (IsSlaveAttribute(i, a))
				{
					INT64 ofs = (CHAR*)i->AttributeValues[a]-(CHAR*)i->Slave;
					d->AttributeValues[a] = (CHAR*)d->Slave+ofs;
				}
				else
				{
					SIZE_T sz = _msize(i->AttributeValues[a]);
					d->AttributeValues[a] = malloc(sz);
					memcpy(d->AttributeValues[a], i->AttributeValues[a], sz);
				}
	}

	return d;
}

LFCORE_API LFItemDescriptor* LFAllocItemDescriptor(LFCoreAttributes* Attr)
{
	assert(Attr);

	LFItemDescriptor* d = LFAllocItemDescriptor();
	d->CoreAttributes = *Attr;

	return d;
}

LFCORE_API LFItemDescriptor* LFAllocItemDescriptor(LFStoreDescriptor* s)
{
	assert(s);

	LFItemDescriptor* d = LFAllocItemDescriptor();
	BOOL IsMounted = LFIsStoreMounted(s);

	d->Type |= LFTypeStore | s->Source;
	if (strcmp(s->StoreID, DefaultStore)==0)
		d->Type |= LFTypeDefault;
	if (!IsMounted)
		d->Type |= LFTypeNotMounted | LFTypeGhosted;
	if ((s->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal)
		d->Type |= LFTypeShortcutAllowed;

	if ((s->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
		if (wcscmp(s->LastSeen, L"")!=0)
		{
			WCHAR ls[256];
			LoadString(LFCoreModuleHandle, IsMounted ? IDS_SEENON : IDS_LASTSEEN, ls, 256);

			WCHAR descr[256];
			wsprintf(descr, ls, s->LastSeen);
			SetAttribute(d, LFAttrDescription, descr);
		}

	d->CategoryID = (s->Source>LFTypeSourceUSB) ? LFItemCategoryRemote : LFItemCategoryLocal;
	d->IconID = LFGetStoreIcon(s);

	SetAttribute(d, LFAttrFileName, s->StoreName);
	SetAttribute(d, LFAttrComments, s->StoreComment);
	SetAttribute(d, LFAttrStoreID, s->StoreID);
	SetAttribute(d, LFAttrCreationTime, &s->CreationTime);
	SetAttribute(d, LFAttrFileTime, &s->FileTime);
	SetAttribute(d, LFAttrFileCount, &s->FileCount[LFContextAllFiles]);
	SetAttribute(d, LFAttrFileSize, &s->FileSize[LFContextAllFiles]);

	return d;
}

LFItemDescriptor* AllocFolderDescriptor()
{
	LFItemDescriptor* d = LFAllocItemDescriptor();
	d->IconID = IDI_FLD_DEFAULT;
	d->Type = LFTypeFolder;

	return d;
}

LFCORE_API void LFFreeItemDescriptor(LFItemDescriptor* i)
{
	if (i)
	{
		i->RefCount--;
		if (!i->RefCount)
		{
			LFFreeFilter(i->NextFilter);
			for (UINT a=LFLastCoreAttribute+1; a<LFAttributeCount; a++)
				FreeAttribute(i, a);
			if (i->Slave)
				free(i->Slave);
			delete i;
		}
	}
}
