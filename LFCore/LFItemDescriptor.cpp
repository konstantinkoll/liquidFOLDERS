
#include "stdafx.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "LFVariantData.h"
#include "IndexTables.h"
#include "StoreCache.h"
#include <assert.h>
#include <malloc.h>


extern HMODULE LFCoreModuleHandle;


// Dynamic attribute handling
//

__forceinline BOOL IsStaticAttribute(LFItemDescriptor* i, UINT Attr)
{
	assert(i);
	assert(Attr<LFAttributeCount);

	return (i->AttributeValues[Attr]>=(BYTE*)i) && (i->AttributeValues[Attr]<(BYTE*)i+sizeof(LFItemDescriptor));
}

void FreeAttribute(LFItemDescriptor* i, UINT Attr)
{
	assert(i);
	assert(Attr<LFAttributeCount);

	// Attribut nur dann freigeben, wenn es nicht im statischer Teil des LFItemDescriptor liegt
	if ((i->AttributeValues[Attr]) && !IsStaticAttribute(i, Attr))
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

	default:
		return 255;
	}
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
	assert(AttrTypes[Attr]<LFTypeCount);
	assert(v);

	// Altes Attribut freigeben
	FreeAttribute(i, Attr);

	// Größe ermitteln
	SIZE_T Size = GetAttributeSize(Attr, v);

	// Ggf. Speicher reservieren
	if (!i->AttributeValues[Attr])
		i->AttributeValues[Attr] = malloc(Size);

	// Kopieren
	switch (AttrTypes[Attr])
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
		wcscpy_s((WCHAR*)i->AttributeValues[Attr], Size/sizeof(WCHAR), (WCHAR*)v);
		break;

	case LFTypeAnsiString:
		strcpy_s((CHAR*)i->AttributeValues[Attr], Size/sizeof(CHAR), (CHAR*)v);
		break;

	default:
		memcpy(i->AttributeValues[Attr], v, Size);
	}
}


// LFItemDescriptor
//

LFCORE_API LFItemDescriptor* LFAllocItemDescriptor(LFCoreAttributes* pCoreAttributes)
{
	LFItemDescriptor* i = new LFItemDescriptor;
	ZeroMemory(i, sizeof(LFItemDescriptor)-LFMaxSlaveSize-(pCoreAttributes ? sizeof(LFCoreAttributes) : 0));

	if (pCoreAttributes)
		i->CoreAttributes = *pCoreAttributes;

	i->FirstAggregate = i->LastAggregate = -1;
	i->RefCount = 1;
	i->Dimension = i->AspectRatio = 0.0;

	// Zeiger auf statische Attributwerte initalisieren
	for (UINT a=0; a<LFIndexTables[IDXTABLE_MASTER].cTableEntries; a++)
		i->AttributeValues[LFCoreAttributeEntries[a].Attr] = (BYTE*)&i->CoreAttributes+LFCoreAttributeEntries[a].Offset;

	i->AttributeValues[LFAttrStoreID] = &i->StoreID;
	i->AttributeValues[LFAttrDescription] = &i->Description[0];
	i->AttributeValues[LFAttrFileCount] = &i->AggregateCount;

	return i;
}

LFCORE_API LFItemDescriptor* LFAllocItemDescriptorEx(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);

	const BOOL IsMounted = LFIsStoreMounted(pStoreDescriptor);

	LFItemDescriptor* i = LFAllocItemDescriptor();

	i->Type = LFTypeStore | pStoreDescriptor->Source;

	if (strcmp(pStoreDescriptor->StoreID, DefaultStore)==0)
		i->Type |= LFTypeDefault;
	if (!IsMounted)
		i->Type |= LFTypeNotMounted | LFTypeGhosted;
	if ((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal)
		i->Type |= LFTypeShortcutAllowed;

	i->CategoryID = (pStoreDescriptor->Source>LFTypeSourceUSB) ? LFItemCategoryRemote : LFItemCategoryLocal;
	i->IconID = LFGetStoreIcon(pStoreDescriptor);

	// Description
	if ((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
		if (wcscmp(pStoreDescriptor->LastSeen, L"")!=0)
		{
			WCHAR LastSeen[256];
			LoadString(LFCoreModuleHandle, IsMounted ? IDS_SEENON : IDS_LASTSEEN, LastSeen, 256);

			wsprintf(i->Description, LastSeen, pStoreDescriptor->LastSeen);
		}

	// Standard-Attribute kopieren
	wcscpy_s(i->CoreAttributes.FileName, 256, pStoreDescriptor->StoreName);
	wcscpy_s(i->CoreAttributes.Comments, 256, pStoreDescriptor->StoreComment);
	strcpy_s(i->StoreID, LFKeySize, pStoreDescriptor->StoreID);

	i->CoreAttributes.CreationTime = pStoreDescriptor->CreationTime;
	i->CoreAttributes.FileTime = pStoreDescriptor->FileTime;
	i->AggregateCount = pStoreDescriptor->FileCount[LFContextAllFiles];
	i->CoreAttributes.FileSize = pStoreDescriptor->FileSize[LFContextAllFiles];

	return i;
}

LFCORE_API LFItemDescriptor* LFCloneItemDescriptor(LFItemDescriptor* pItemDescriptor)
{
	if (!pItemDescriptor)
		return LFAllocItemDescriptor();

	LFItemDescriptor* i = new LFItemDescriptor();
	memcpy(i, pItemDescriptor, sizeof(LFItemDescriptor));

	if (pItemDescriptor->NextFilter)
		i->NextFilter = LFAllocFilter(pItemDescriptor->NextFilter);

	// Zeiger anpassen
	for (UINT a=0; a<LFAttributeCount; a++)
		if (i->AttributeValues[a])
			if (IsStaticAttribute(pItemDescriptor, a))
			{
				UINT_PTR Offset = (BYTE*)pItemDescriptor->AttributeValues[a]-(BYTE*)pItemDescriptor;
				i->AttributeValues[a] = (BYTE*)i+Offset;
			}
			else
			{
				SIZE_T Size = _msize(pItemDescriptor->AttributeValues[a]);
				memcpy(i->AttributeValues[a]=malloc(Size), pItemDescriptor->AttributeValues[a], Size);
			}
	

	return i;
}

LFItemDescriptor* AllocFolderDescriptor()
{
	LFItemDescriptor* i = LFAllocItemDescriptor();

	i->Type = LFTypeFolder;
	i->IconID = IDI_FLD_DEFAULT;

	return i;
}

LFCORE_API void LFFreeItemDescriptor(LFItemDescriptor* i)
{
	assert(i);

	if (--i->RefCount==0)
	{
		LFFreeFilter(i->NextFilter);

		for (UINT a=LFLastCoreAttribute+1; a<LFAttributeCount; a++)
			FreeAttribute(i, a);

		delete i;
	}
}

void AttachSlave(LFItemDescriptor* i, BYTE SlaveID, void* pSlaveData)
{
	assert(SlaveID>0);
	assert(SlaveID<IDXTABLECOUNT);
	assert(pSlaveData);
	assert(i);
	assert(i->CoreAttributes.SlaveID==SlaveID);
	assert(LFIndexTables[SlaveID].Size<=LFMaxSlaveSize);

	const LFIdxTable* pTable = &LFIndexTables[SlaveID];

	memcpy(i->SlaveData, pSlaveData, pTable->Size);

	for (UINT a=0; a<pTable->cTableEntries; a++)
		i->AttributeValues[pTable->pTableEntries[a].Attr] = i->SlaveData + pTable->pTableEntries[a].Offset;

	// LFAttrDimension und LFAttrAspectRatio werden dynamisch berechnet
	if ((i->AttributeValues[LFAttrWidth]) && (i->AttributeValues[LFAttrHeight]))
	{
		const UINT Width = *((UINT*)i->AttributeValues[LFAttrWidth]);
		const UINT Height = *((UINT*)i->AttributeValues[LFAttrHeight]);

		if ((Width) && (Height))
		{
			i->Dimension = ((DOUBLE)Width*Height)/((DOUBLE)1000000);
			i->AttributeValues[LFAttrDimension] = &i->Dimension;

			i->AspectRatio = ((DOUBLE)Width)/((DOUBLE)Height);
			i->AttributeValues[LFAttrAspectRatio] = &i->AspectRatio;
		}
	}
}
