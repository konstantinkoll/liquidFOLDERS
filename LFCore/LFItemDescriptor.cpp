#include "stdafx.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "CIndex.h"
#include "StoreCache.h"
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

size_t AttrSizes[LFTypeCount] = {
	0,							// LFTypeUnicodeString
	0,							// LFTypeUnicodeArray
	0,							// LFTypeAnsiString
	sizeof(unsigned int),		// LFTypeFourCC
	sizeof(unsigned char),		// LFTypeRating
	sizeof(unsigned int),		// LFTypeUINT
	sizeof(__int64),			// LFTypeINT64
	sizeof(LFFraction),			// LFTypeFraction
	sizeof(double),				// LFTypeDouble
	sizeof(unsigned int),		// LFTypeFlags
	sizeof(LFGeoCoordinates),	// LFTypeGeoCoordinates
	sizeof(FILETIME),			// LFTypeTime
	sizeof(UINT),				// LFTypeDuration
	sizeof(unsigned int),		// LFTypeBitrate,
	sizeof(double)				// LFTypeMegapixel
};

unsigned char AttrTypes[LFAttributeCount] = {
	LFTypeUnicodeString,		// LFAttrFileName
	LFTypeAnsiString,			// LFAttrStoreID
	LFTypeAnsiString,			// LFAttrFileID
	LFTypeUnicodeString,		// LFAttrComments
	LFTypeUnicodeString,		// LFAttrDescription
	LFTypeTime,					// LFAttrCreationTime
	LFTypeTime,					// LFAttrAddTime
	LFTypeTime,					// LFAttrFileTime
	LFTypeTime,					// LFAttrDeleteTime
	LFTypeTime,					// LFAttrArchiveTime
	LFTypeAnsiString,			// LFAttrFileFormat
	LFTypeUINT,					// LFAttrFileCount
	LFTypeINT64,				// LFAttrFileSize
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
	LFTypeUnicodeString			// LFAttrCustomer
};

#pragma data_seg()
#pragma comment(linker, "/SECTION:common_attrs,RWS")

extern HMODULE LFCoreModuleHandle;


// Attribute handling
//

__forceinline bool IsSlaveAttribute(LFItemDescriptor* i, unsigned int attr)
{
	assert(i);

	if (!i->Slave)
		return false;

	return (i->AttributeValues[attr]>=(char*)i->Slave) && (i->AttributeValues[attr]<(char*)i->Slave+_msize(i->Slave));
}

__forceinline void FreeAttribute(LFItemDescriptor* i, unsigned int attr)
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
	assert((AttrTypes[attr]==LFTypeUnicodeString) || (AttrTypes[attr]==LFTypeUnicodeArray) || (AttrTypes[attr]==LFTypeAnsiString));

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

__forceinline size_t GetAttributeSize(unsigned int attr, const void* v)
{
	assert(attr<LFAttributeCount);
	assert(AttrTypes[attr]<LFTypeCount);

	switch (AttrTypes[attr])
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
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
	d->FirstAggregate = d->LastAggregate = -1;
	d->RefCount = 1;

	// Zeiger auf statische Attributwerte initalisieren
	d->AttributeValues[LFAttrStoreID] = &d->StoreID;
	d->AttributeValues[LFAttrDescription] = &d->Description[0];
	d->AttributeValues[LFAttrFileCount] = &d->AggregateCount;

	for (unsigned int a=0; a<=LFLastCoreAttribute; a++)
		if (CoreOffsets[a]!=-1)
			d->AttributeValues[a] = (char*)&d->CoreAttributes + CoreOffsets[a];

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

LFCore_API LFItemDescriptor* LFAllocItemDescriptor(LFCoreAttributes* attr)
{
	assert(attr);

	LFItemDescriptor* d = LFAllocItemDescriptor();
	d->CoreAttributes = *attr;

	return d;
}

LFCore_API LFItemDescriptor* LFAllocItemDescriptor(LFStoreDescriptor* s)
{
	assert(s);

	LFItemDescriptor* d = LFAllocItemDescriptor();
	bool IsMounted = IsStoreMounted(s);

	if (strcmp(s->StoreID, DefaultStore)==0)
	{
		d->IconID = IDI_STORE_Default;
		d->Type |= LFTypeDefaultStore;
		wchar_t ds[256];
		LoadString(LFCoreModuleHandle, IDS_DefaultStore, ds, 256);
		SetAttribute(d, LFAttrDescription, ds);
	}
	else
	{
		d->IconID = (s->StoreMode==LFStoreModeInternal ? IDI_STORE_Internal : s->StoreMode==LFStoreModeRemote ? IDI_STORE_Server : IDI_STORE_Bag);
		if ((s->StoreMode==LFStoreModeHybrid) || (s->StoreMode==LFStoreModeExternal))
			if (wcscmp(s->LastSeen, L"")!=0)
			{
				wchar_t ls[256];
				LoadString(LFCoreModuleHandle, IsMounted ? IDS_SeenOn :IDS_LastSeen, ls, 256);
				wchar_t descr[256];
				wsprintf(descr, ls, s->LastSeen);
				SetAttribute(d, LFAttrDescription, descr);
			}
	}

	if (!IsMounted)
		d->Type |= LFTypeGhosted | LFTypeNotMounted;
	if (s->IndexVersion<CurIdxVersion)
		d->Type |= LFTypeRequiresMaintenance;

	d->CategoryID = s->StoreMode;
	d->Type |= LFTypeStore;
	SetAttribute(d, LFAttrFileName, s->StoreName);
	SetAttribute(d, LFAttrComments, s->Comment);
	SetAttribute(d, LFAttrStoreID, s->StoreID);
	SetAttribute(d, LFAttrFileID, s->StoreID);
	SetAttribute(d, LFAttrCreationTime, &s->CreationTime);
	SetAttribute(d, LFAttrFileTime, &s->FileTime);

	return d;
}

LFItemDescriptor* AllocFolderDescriptor(const wchar_t* Name, const wchar_t* Comment, const wchar_t* Description, const char* StoreID, const char* FileID, __int64* Size, unsigned int IconID, unsigned int CategoryID, unsigned int Count, LFFilter* Filter)
{
	LFItemDescriptor* d = LFAllocItemDescriptor();

	d->IconID = IconID;
	d->CategoryID = CategoryID;
	d->Type = LFTypeVirtual;
	d->AggregateCount = Count;
	d->NextFilter = Filter;

	if (Name)
		SetAttribute(d, LFAttrFileName, Name);
	if (StoreID)
		SetAttribute(d, LFAttrStoreID, StoreID);
	if (FileID)
		SetAttribute(d, LFAttrFileID, FileID);
	if (Comment)
		SetAttribute(d, LFAttrComments, Comment);
	if (Description)
		SetAttribute(d, LFAttrDescription, Description);
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
