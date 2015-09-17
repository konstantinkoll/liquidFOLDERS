
#include "stdafx.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "LFVariantData.h"
#include "IndexTables.h"
#include <assert.h>
#include <malloc.h>


extern HMODULE LFCoreModuleHandle;
extern CHAR DefaultStore[LFKeySize];


// Dynamic attribute handling
//

__forceinline BOOL IsStaticAttribute(LFItemDescriptor* pItemDescriptor, UINT Attr)
{
	assert(pItemDescriptor);
	assert(Attr<LFAttributeCount);

	return (pItemDescriptor->AttributeValues[Attr]>=(BYTE*)pItemDescriptor) && (pItemDescriptor->AttributeValues[Attr]<(BYTE*)pItemDescriptor+sizeof(LFItemDescriptor));
}

void FreeAttribute(LFItemDescriptor* pItemDescriptor, UINT Attr)
{
	assert(pItemDescriptor);
	assert(Attr<LFAttributeCount);

	// Attribut nur dann freigeben, wenn es nicht im statischer Teil des LFItemDescriptor liegt
	if ((pItemDescriptor->AttributeValues[Attr]) && !IsStaticAttribute(pItemDescriptor, Attr))
		{
			free(pItemDescriptor->AttributeValues[Attr]);
			pItemDescriptor->AttributeValues[Attr] = NULL;
		}
}

SIZE_T GetAttributeMaxCharacterCount(UINT Attr)
{
	assert(Attr<LFAttributeCount);
	assert((AttrTypes[Attr]==LFTypeUnicodeString) || (AttrTypes[Attr]==LFTypeUnicodeArray) || (AttrTypes[Attr]==LFTypeAnsiString));

	switch(Attr)
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

__forceinline SIZE_T GetAttributeSize(UINT Attr, const void* Value)
{
	assert(Attr<LFAttributeCount);
	assert(AttrTypes[Attr]<LFTypeCount);

	switch(AttrTypes[Attr])
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
		return (min(GetAttributeMaxCharacterCount(Attr), wcslen((WCHAR*)Value))+1)*sizeof(WCHAR);

	case LFTypeAnsiString:
		return min(GetAttributeMaxCharacterCount(Attr), strlen((CHAR*)Value))+1;

	default:
		return AttrSizes[AttrTypes[Attr]];
	}
}

void SetAttribute(LFItemDescriptor* pItemDescriptor, UINT Attr, const void* Value)
{
	assert(pItemDescriptor);
	assert(Attr<LFAttributeCount);
	assert(AttrTypes[Attr]<LFTypeCount);
	assert(Value);

	// Altes Attribut freigeben
	FreeAttribute(pItemDescriptor, Attr);

	// Größe ermitteln
	SIZE_T Size = GetAttributeSize(Attr, Value);

	// Ggf. Speicher reservieren
	if (!pItemDescriptor->AttributeValues[Attr])
		pItemDescriptor->AttributeValues[Attr] = malloc(Size);

	// Kopieren
	switch(AttrTypes[Attr])
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
		wcscpy_s((WCHAR*)pItemDescriptor->AttributeValues[Attr], Size/sizeof(WCHAR), (WCHAR*)Value);
		break;

	case LFTypeAnsiString:
		strcpy_s((CHAR*)pItemDescriptor->AttributeValues[Attr], Size/sizeof(CHAR), (CHAR*)Value);
		break;

	default:
		memcpy(pItemDescriptor->AttributeValues[Attr], Value, Size);
	}
}


// LFItemDescriptor
//

LFCORE_API LFItemDescriptor* LFAllocItemDescriptor(LFCoreAttributes* pCoreAttributes)
{
	LFItemDescriptor* pItemDescriptor = new LFItemDescriptor;
	ZeroMemory(pItemDescriptor, sizeof(LFItemDescriptor)-LFMaxSlaveSize-(pCoreAttributes ? sizeof(LFCoreAttributes) : 0));

	if (pCoreAttributes)
		pItemDescriptor->CoreAttributes = *pCoreAttributes;

	pItemDescriptor->FirstAggregate = pItemDescriptor->LastAggregate = -1;
	pItemDescriptor->RefCount = 1;
	pItemDescriptor->Dimension = pItemDescriptor->AspectRatio = 0.0;

	// Zeiger auf statische Attributwerte initalisieren
	for (UINT a=0; a<LFIndexTables[IDXTABLE_MASTER].cTableEntries; a++)
		pItemDescriptor->AttributeValues[LFCoreAttributeEntries[a].Attr] = (BYTE*)&pItemDescriptor->CoreAttributes+LFCoreAttributeEntries[a].Offset;

	pItemDescriptor->AttributeValues[LFAttrStoreID] = &pItemDescriptor->StoreID;
	pItemDescriptor->AttributeValues[LFAttrDescription] = &pItemDescriptor->Description[0];
	pItemDescriptor->AttributeValues[LFAttrFileCount] = &pItemDescriptor->AggregateCount;

	return pItemDescriptor;
}

LFCORE_API LFItemDescriptor* LFAllocItemDescriptorEx(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);

	const BOOL IsMounted = LFIsStoreMounted(pStoreDescriptor);

	LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptor();

	pItemDescriptor->Type = LFTypeStore | pStoreDescriptor->Source;

	if (strcmp(pStoreDescriptor->StoreID, DefaultStore)==0)
		pItemDescriptor->Type |= LFTypeDefault;

	if (!IsMounted)
		pItemDescriptor->Type |= LFTypeNotMounted | LFTypeGhosted;

	if ((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexExternal)
		pItemDescriptor->Type |= LFTypeShortcutAllowed;

	pItemDescriptor->CategoryID = (pStoreDescriptor->Source>LFTypeSourceUSB) ? LFItemCategoryRemote : LFItemCategoryLocal;
	pItemDescriptor->IconID = LFGetStoreIcon(pStoreDescriptor);

	// Description
	if ((pStoreDescriptor->Mode & LFStoreModeIndexMask)!=LFStoreModeIndexInternal)
		if (wcscmp(pStoreDescriptor->LastSeen, L"")!=0)
		{
			WCHAR LastSeen[256];
			LoadString(LFCoreModuleHandle, IsMounted ? IDS_SEENON : IDS_LASTSEEN, LastSeen, 256);

			wsprintf(pItemDescriptor->Description, LastSeen, pStoreDescriptor->LastSeen);
		}

	// Standard-Attribute kopieren
	wcscpy_s(pItemDescriptor->CoreAttributes.FileName, 256, pStoreDescriptor->StoreName);
	wcscpy_s(pItemDescriptor->CoreAttributes.Comments, 256, pStoreDescriptor->Comments);
	strcpy_s(pItemDescriptor->StoreID, LFKeySize, pStoreDescriptor->StoreID);

	pItemDescriptor->CoreAttributes.CreationTime = pStoreDescriptor->CreationTime;
	pItemDescriptor->CoreAttributes.FileTime = pStoreDescriptor->FileTime;
	pItemDescriptor->AggregateCount = pStoreDescriptor->FileCount[LFContextAllFiles];
	pItemDescriptor->CoreAttributes.FileSize = pStoreDescriptor->FileSize[LFContextAllFiles];

	return pItemDescriptor;
}

LFCORE_API LFItemDescriptor* LFCloneItemDescriptor(LFItemDescriptor* pItemDescriptor)
{
	if (!pItemDescriptor)
		return LFAllocItemDescriptor();

	LFItemDescriptor* i = new LFItemDescriptor;
	memcpy(i, pItemDescriptor, sizeof(LFItemDescriptor));

	i->RefCount = 1;

	if (pItemDescriptor->NextFilter)
		i->NextFilter = LFAllocFilter(pItemDescriptor->NextFilter);

	// Zeiger anpassen
	for (UINT a=0; a<LFAttributeCount; a++)
		if (i->AttributeValues[a])
			if (IsStaticAttribute(pItemDescriptor, a))
			{
				INT_PTR Offset = (BYTE*)pItemDescriptor->AttributeValues[a]-(BYTE*)pItemDescriptor;
				i->AttributeValues[a] = (BYTE*)i+Offset;
			}
			else
			{
				SIZE_T Size = _msize(pItemDescriptor->AttributeValues[a]);
				memcpy(i->AttributeValues[a] = malloc(Size), pItemDescriptor->AttributeValues[a], Size);
			}

	return i;
}

LFItemDescriptor* AllocFolderDescriptor()
{
	LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptor();

	pItemDescriptor->Type = LFTypeFolder;
	pItemDescriptor->IconID = IDI_FLD_DEFAULT;

	return pItemDescriptor;
}

LFCORE_API void LFFreeItemDescriptor(LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	if (--pItemDescriptor->RefCount==0)
	{
		LFFreeFilter(pItemDescriptor->NextFilter);

		for (UINT a=LFLastCoreAttribute+1; a<LFAttributeCount; a++)
			FreeAttribute(pItemDescriptor, a);

		delete pItemDescriptor;
	}
}

void AttachSlave(LFItemDescriptor* pItemDescriptor, BYTE SlaveID, void* pSlaveData)
{
	assert(SlaveID>0);
	assert(SlaveID<IDXTABLECOUNT);
	assert(pSlaveData);
	assert(pItemDescriptor);
	assert(pItemDescriptor->CoreAttributes.SlaveID==SlaveID);
	assert(LFIndexTables[SlaveID].Size<=LFMaxSlaveSize);

	const LFIdxTable* pTable = &LFIndexTables[SlaveID];

	memcpy(pItemDescriptor->SlaveData, pSlaveData, pTable->Size);

	for (UINT a=0; a<pTable->cTableEntries; a++)
		pItemDescriptor->AttributeValues[pTable->pTableEntries[a].Attr] = pItemDescriptor->SlaveData + pTable->pTableEntries[a].Offset;

	// LFAttrDimension und LFAttrAspectRatio werden dynamisch berechnet
	if ((pItemDescriptor->AttributeValues[LFAttrWidth]) && (pItemDescriptor->AttributeValues[LFAttrHeight]))
	{
		const UINT Width = *((UINT*)pItemDescriptor->AttributeValues[LFAttrWidth]);
		const UINT Height = *((UINT*)pItemDescriptor->AttributeValues[LFAttrHeight]);

		if ((Width) && (Height))
		{
			pItemDescriptor->Dimension = ((DOUBLE)Width*Height)/((DOUBLE)1000000);
			pItemDescriptor->AttributeValues[LFAttrDimension] = &pItemDescriptor->Dimension;

			pItemDescriptor->AspectRatio = ((DOUBLE)Width)/((DOUBLE)Height);
			pItemDescriptor->AttributeValues[LFAttrAspectRatio] = &pItemDescriptor->AspectRatio;
		}
	}
}
