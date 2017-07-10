
#include "stdafx.h"
#include "AttributeTables.h"
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

	return (pItemDescriptor->AttributeValues[Attr]>=(LPBYTE)pItemDescriptor) && (pItemDescriptor->AttributeValues[Attr]<(LPBYTE)pItemDescriptor+sizeof(LFItemDescriptor));
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

SIZE_T GetAttributeSize(UINT Attr, LPCVOID Value)
{
	assert(Attr<LFAttributeCount);
	assert(AttrProperties[Attr].Type<LFTypeCount);

	switch (AttrProperties[Attr].Type)
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
		return (min(AttrProperties[Attr].cCharacters, wcslen((LPCWSTR)Value))+1)*sizeof(WCHAR);

	case LFTypeAnsiString:
	case LFTypeIATACode:
		return min(AttrProperties[Attr].cCharacters, strlen((LPCSTR)Value))+1;

	default:
		return TypeProperties[AttrProperties[Attr].Type].Size;
	}
}

void SetAttribute(LFItemDescriptor* pItemDescriptor, UINT Attr, LPCVOID Value)
{
	assert(pItemDescriptor);
	assert(Attr<LFAttributeCount);
	assert(AttrProperties[Attr].Type<LFTypeCount);
	assert(Value);

	// Altes Attribut freigeben
	FreeAttribute(pItemDescriptor, Attr);

	// Größe ermitteln
	const SIZE_T Size = GetAttributeSize(Attr, Value);

	// Ggf. Speicher reservieren
	if (!pItemDescriptor->AttributeValues[Attr])
		pItemDescriptor->AttributeValues[Attr] = malloc(Size);

	// Kopieren
	switch (AttrProperties[Attr].Type)
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
		wcsncpy_s((WCHAR*)pItemDescriptor->AttributeValues[Attr], Size/sizeof(WCHAR), (LPCWSTR)Value, _TRUNCATE);
		break;

	case LFTypeAnsiString:
	case LFTypeIATACode:
		strncpy_s((CHAR*)pItemDescriptor->AttributeValues[Attr], Size/sizeof(CHAR), (LPCSTR)Value, _TRUNCATE);
		break;

	default:
		memcpy(pItemDescriptor->AttributeValues[Attr], Value, Size);
	}
}


// LFItemDescriptor
//

LFCORE_API LFItemDescriptor* LFAllocItemDescriptor(LFCoreAttributes* pCoreAttributes, LPVOID pStoreData, SIZE_T StoreDataSize)
{
	LFItemDescriptor* pItemDescriptor = new LFItemDescriptor;
	ZeroMemory(pItemDescriptor, offsetof(LFItemDescriptor, CoreAttributes)+(pCoreAttributes ? 0 : sizeof(LFCoreAttributes)));

	if (pCoreAttributes)
		pItemDescriptor->CoreAttributes = *pCoreAttributes;

	if (pStoreData)
		memcpy_s(pItemDescriptor->StoreData, LFMaxStoreDataSize, pStoreData, StoreDataSize);

	pItemDescriptor->FirstAggregate = pItemDescriptor->LastAggregate = -1;
	pItemDescriptor->RefCount = 1;

	// Zeiger auf statische Attributwerte initalisieren
	for (UINT a=0; a<IndexTables[IDXTABLE_MASTER].cTableEntries; a++)
		pItemDescriptor->AttributeValues[CoreAttributeEntries[a].Attr] = (BYTE*)&pItemDescriptor->CoreAttributes+CoreAttributeEntries[a].Offset;

	pItemDescriptor->AttributeValues[LFAttrFileCount] = &pItemDescriptor->AggregateCount;

	return pItemDescriptor;
}

LFCORE_API LFItemDescriptor* LFAllocItemDescriptorEx(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);

	LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptor();

	// Category, icon and type
	pItemDescriptor->CategoryID = (pStoreDescriptor->Source>LFTypeSourceUSB) ? LFItemCategoryRemote : LFItemCategoryLocal;
	pItemDescriptor->IconID = LFGetStoreIcon(pStoreDescriptor, &pItemDescriptor->Type);

	// Description
	LFGetFileSummary(pItemDescriptor->Description, 256, pStoreDescriptor->Statistics.FileCount[LFContextAllFiles], pStoreDescriptor->Statistics.FileSize[LFContextAllFiles]);

	WCHAR Hint[256] = L"";
	if (LFIsStoreMounted(pStoreDescriptor))
	{
		if (pStoreDescriptor->Source>LFTypeSourceInternal)
		{
			Hint[0] = L' ';
			LoadString(LFCoreModuleHandle, IDS_QSRC_UNKNOWN+pStoreDescriptor->Source, &Hint[1], 255);
			Hint[1] = (WCHAR)tolower(Hint[1]);
		}
	}
	else
		if (wcscmp(pStoreDescriptor->LastSeen, L"")!=0)
		{
			WCHAR LastSeen[256];
			LoadString(LFCoreModuleHandle, IDS_LASTSEEN, LastSeen, 256);

			wcscpy_s(Hint, 256, L", ");
			swprintf_s(&Hint[2], 254, LastSeen, pStoreDescriptor->LastSeen);
			Hint[2] = (WCHAR)tolower(Hint[2]);
		}

	if (Hint[0])
		wcscat_s(pItemDescriptor->Description, 256, Hint);

	// Copy properties
	wcscpy_s(pItemDescriptor->CoreAttributes.FileName, 256, pStoreDescriptor->StoreName);
	wcscpy_s(pItemDescriptor->CoreAttributes.Comments, 256, pStoreDescriptor->Comments);
	strcpy_s(pItemDescriptor->StoreID, LFKeySize, pStoreDescriptor->StoreID);

	pItemDescriptor->CoreAttributes.CreationTime = pStoreDescriptor->CreationTime;
	pItemDescriptor->CoreAttributes.FileTime = pStoreDescriptor->FileTime;
	pItemDescriptor->AggregateCount = pStoreDescriptor->Statistics.FileCount[LFContextAllFiles];
	pItemDescriptor->CoreAttributes.FileSize = pStoreDescriptor->Statistics.FileSize[LFContextAllFiles];

	return pItemDescriptor;
}

LFCORE_API LFItemDescriptor* LFCloneItemDescriptor(LFItemDescriptor* pItemDescriptor)
{
	if (!pItemDescriptor)
		return LFAllocItemDescriptor();

	LFItemDescriptor* pClone = new LFItemDescriptor;
	memcpy(pClone, pItemDescriptor, sizeof(LFItemDescriptor));

	pClone->RefCount = 1;

	if (pItemDescriptor->pNextFilter)
		pClone->pNextFilter = LFAllocFilter(pItemDescriptor->pNextFilter);

	// Zeiger anpassen
	for (UINT a=0; a<LFAttributeCount; a++)
		if (pClone->AttributeValues[a])
			if (IsStaticAttribute(pItemDescriptor, a))
			{
				INT_PTR Offset = (BYTE*)pItemDescriptor->AttributeValues[a]-(BYTE*)pItemDescriptor;
				pClone->AttributeValues[a] = (BYTE*)pClone+Offset;
			}
			else
			{
				SIZE_T Size = _msize(pItemDescriptor->AttributeValues[a]);
				memcpy(pClone->AttributeValues[a] = malloc(Size), pItemDescriptor->AttributeValues[a], Size);
			}

	return pClone;
}

LFItemDescriptor* AllocFolderDescriptor(UINT Attr, const LFFileSummary& FileSummary, INT FirstAggregate, INT LastAggregate)
{
	assert(Attr<LFAttributeCount);

	LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptor();

	pItemDescriptor->Type = LFTypeFolder | LFTypeHasDescription | FileSummary.Source;
	pItemDescriptor->FirstAggregate = FirstAggregate;
	pItemDescriptor->LastAggregate = LastAggregate;
	pItemDescriptor->AggregateCount = FileSummary.FileCount;
	pItemDescriptor->CoreAttributes.FileSize = FileSummary.FileSize;
	pItemDescriptor->CoreAttributes.Flags = FileSummary.Flags;
	pItemDescriptor->IconID = AttrProperties[Attr].IconID ? AttrProperties[Attr].IconID : IDI_FLD_DEFAULT;

	// Description
	LFGetFileSummaryEx(pItemDescriptor->Description, 256, FileSummary);

	// Additional properties
	SetAttribute(pItemDescriptor, LFAttrDuration, &FileSummary.Duration);

	return pItemDescriptor;
}

LFCORE_API void LFFreeItemDescriptor(LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	if (--pItemDescriptor->RefCount==0)
	{
		LFFreeFilter(pItemDescriptor->pNextFilter);

		for (UINT a=LFLastCoreAttribute+1; a<LFAttributeCount; a++)
			FreeAttribute(pItemDescriptor, a);

		delete pItemDescriptor;
	}
}

void AttachSlave(LFItemDescriptor* pItemDescriptor, BYTE SlaveID, LPVOID pSlaveData)
{
	assert(SlaveID>0);
	assert(SlaveID<IDXTABLECOUNT);
	assert(pSlaveData);
	assert(pItemDescriptor);
	assert(pItemDescriptor->CoreAttributes.SlaveID==SlaveID);
	assert(IndexTables[SlaveID].Size<=LFMaxSlaveSize);

	const IdxTable* pTable = &IndexTables[SlaveID];

	memcpy(pItemDescriptor->SlaveData, pSlaveData, pTable->Size);

	for (UINT a=0; a<pTable->cTableEntries; a++)
		pItemDescriptor->AttributeValues[pTable->pTableEntries[a].Attr] = pItemDescriptor->SlaveData + pTable->pTableEntries[a].Offset;

	// LFAttrDimension and LFAttrAspectRatio are computed on the fly
	if (pItemDescriptor->AttributeValues[LFAttrWidth] && pItemDescriptor->AttributeValues[LFAttrHeight])
	{
		const UINT Width = *((UINT*)pItemDescriptor->AttributeValues[LFAttrWidth]);
		const UINT Height = *((UINT*)pItemDescriptor->AttributeValues[LFAttrHeight]);

		if (Width && Height)
		{
			pItemDescriptor->Dimension = ((DOUBLE)Width*Height)/((DOUBLE)1000000);
			pItemDescriptor->AttributeValues[LFAttrDimension] = &pItemDescriptor->Dimension;

			pItemDescriptor->AspectRatio = ((DOUBLE)Width)/((DOUBLE)Height);
			pItemDescriptor->AttributeValues[LFAttrAspectRatio] = &pItemDescriptor->AspectRatio;
		}
	}
}
