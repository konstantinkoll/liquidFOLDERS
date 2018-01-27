
#include "stdafx.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "LFVariantData.h"
#include "Stores.h"
#include "TableApplications.h"
#include "TableAttributes.h"
#include "TableIndexes.h"
#include "TableMusicGenres.h"
#include <assert.h>
#include <malloc.h>


extern HMODULE LFCoreModuleHandle;
extern CHAR DefaultStore[LFKeySize];


// Dynamic attribute handling
//

__forceinline BOOL IsStaticAttribute(const LFItemDescriptor* pItemDescriptor, UINT Attr)
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
	if (pItemDescriptor->AttributeValues[Attr] && !IsStaticAttribute(pItemDescriptor, Attr))
	{
		free(pItemDescriptor->AttributeValues[Attr]);
		pItemDescriptor->AttributeValues[Attr] = NULL;
	}
}

SIZE_T GetAttributeSize(UINT Attr, LPCVOID pValue)
{
	assert(Attr<LFAttributeCount);
	assert(AttrProperties[Attr].Type<LFTypeCount);

	switch (AttrProperties[Attr].Type)
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
		return (min(AttrProperties[Attr].cCharacters, wcslen((LPCWSTR)pValue))+1)*sizeof(WCHAR);

	case LFTypeAnsiString:
	case LFTypeIATACode:
		return min(AttrProperties[Attr].cCharacters, strlen((LPCSTR)pValue))+1;

	default:
		return TypeProperties[AttrProperties[Attr].Type].Size;
	}
}

void SetAttribute(LFItemDescriptor* pItemDescriptor, UINT Attr, LPCVOID pValue)
{
	assert(pItemDescriptor);
	assert(Attr<LFAttributeCount);
	assert(AttrProperties[Attr].Type<LFTypeCount);
	assert(pValue);

	// Altes Attribut freigeben
	FreeAttribute(pItemDescriptor, Attr);

	// Gr��e ermitteln
	const SIZE_T Size = GetAttributeSize(Attr, pValue);

	// Ggf. Speicher reservieren
	if (!pItemDescriptor->AttributeValues[Attr])
		pItemDescriptor->AttributeValues[Attr] = malloc(Size);

	// Kopieren
	switch (AttrProperties[Attr].Type)
	{
	case LFTypeUnicodeString:
	case LFTypeUnicodeArray:
		wcsncpy_s((LPWSTR)pItemDescriptor->AttributeValues[Attr], Size/sizeof(WCHAR), (LPCWSTR)pValue, _TRUNCATE);
		break;

	case LFTypeAnsiString:
	case LFTypeIATACode:
		strncpy_s((LPSTR)pItemDescriptor->AttributeValues[Attr], Size/sizeof(CHAR), (LPCSTR)pValue, _TRUNCATE);
		break;

	case LFTypeColor:
		*((BYTE*)pItemDescriptor->AttributeValues[Attr]) = *((BYTE*)pValue);

		if (Attr==LFAttrColor)
		{
			assert(pItemDescriptor->CoreAttributes.Color<LFItemColorCount);
			pItemDescriptor->AggregateColorSet = (1 << pItemDescriptor->CoreAttributes.Color);
		}

		break;

	default:
		memcpy(pItemDescriptor->AttributeValues[Attr], pValue, Size);
	}
}

UINT GetColoredFolderIcon(const LFFileSummary& FileSummary)
{
	UINT IconColorIndex = 0;
	UINT ItemColorCount = 0;

	for (UINT a=1; a<LFItemColorCount; a++)
		if (FileSummary.ItemColors[a]>ItemColorCount)
			ItemColorCount = FileSummary.ItemColors[IconColorIndex=a];

	return IDI_FLD_DEFAULT+IconColorIndex;
}

UINT GetAttributeIcon(UINT Attr, const LFFileSummary& FileSummary)
{
	assert(Attr<LFAttributeCount);

	UINT IconID = 0;

	for (UINT a=0; a<SPECIALATTRIBUTENAMESCOUNT; a++)
		if ((SpecialAttributeNames[a].Attr==Attr) && (SpecialAttributeNames[a].ContextSet & FileSummary.ContextSet))
			if (!IconID)
			{
				IconID = SpecialAttributeNames[a].IconID;
			}
			else
				if (IconID!=SpecialAttributeNames[a].IconID)
				{
					IconID = AttrProperties[Attr].DefaultIconID;
					break;
				}
			

	if (!IconID)
		IconID = AttrProperties[Attr].DefaultIconID;

	return IconID;
}

UINT GetFolderIcon(const LFFileSummary& FileSummary, const LFVariantData& VData, BOOL IgnoreDefaultIcon)
{
	assert(VData.Attr<LFAttributeCount);
	assert(VData.Type==AttrProperties[VData.Attr].Type);
	assert(VData.Type<LFTypeCount);

	UINT IconID;

	// Attribute icons
	switch (VData.Attr)
	{
	case LFAttrApplication:
		IconID = GetApplicationIcon(VData.Application);
		break;

	case LFAttrGenre:
		IconID = GetGenreIcon(VData.Genre);
		break;

	default:
		IconID = GetAttributeIcon(VData.Attr, FileSummary);
	}

	// Colored default icon
	if ((!IconID || (IconID==IDI_FLD_DEFAULT)) && !IgnoreDefaultIcon)
		IconID = GetColoredFolderIcon(FileSummary);

	return IconID;
}


// LFItemDescriptor
//

LFCORE_API LFItemDescriptor* LFAllocItemDescriptor(const LFCoreAttributes* pCoreAttributes, LPVOID pStoreData, SIZE_T StoreDataSize)
{
	LFItemDescriptor* pItemDescriptor = new LFItemDescriptor;

	if (pCoreAttributes)
	{
		ZeroMemory(pItemDescriptor, offsetof(LFItemDescriptor, CoreAttributes));

		pItemDescriptor->CoreAttributes = *pCoreAttributes;
		pItemDescriptor->Description[0] = L'\0';

		assert(pItemDescriptor->CoreAttributes.Color<LFItemColorCount);
		pItemDescriptor->AggregateColorSet = (1 << pItemDescriptor->CoreAttributes.Color);
	}
	else
	{
		ZeroMemory(pItemDescriptor, offsetof(LFItemDescriptor, Description)+sizeof(pItemDescriptor->Description[0]));
	}

	if (pStoreData)
		memcpy_s(pItemDescriptor->StoreData, LFMaxStoreDataSize, pStoreData, StoreDataSize);

	pItemDescriptor->AggregateFirst = pItemDescriptor->AggregateLast = -1;
	pItemDescriptor->RefCount = 1;

	// Zeiger auf statische Attributwerte initalisieren
	for (UINT a=0; a<IndexTables[IDXTABLE_MASTER].cTableEntries; a++)
		pItemDescriptor->AttributeValues[CoreAttributeEntries[a].Attr] = (BYTE*)&pItemDescriptor->CoreAttributes+CoreAttributeEntries[a].Offset;

	pItemDescriptor->AttributeValues[LFAttrFileCount] = &pItemDescriptor->AggregateCount;

	return pItemDescriptor;
}

LFCORE_API LFItemDescriptor* LFAllocItemDescriptorEx(const LFStoreDescriptor& StoreDescriptor)
{
	LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptor();

	// Category, icon and type
	pItemDescriptor->CategoryID = (StoreDescriptor.Source>LFTypeSourceUSB) ? LFItemCategoryRemote : LFItemCategoryLocal;
	pItemDescriptor->IconID = LFGetStoreIcon(&StoreDescriptor, &pItemDescriptor->Type);

	// Description
	LFGetFileSummary(pItemDescriptor->Description, 256, StoreDescriptor.Statistics.FileCount[LFContextAllFiles], StoreDescriptor.Statistics.FileSize[LFContextAllFiles]);

	WCHAR Hint[256] = L"";
	if (LFIsStoreMounted(&StoreDescriptor))
	{
		if (StoreDescriptor.Source>LFTypeSourceInternal)
		{
			Hint[0] = L' ';
			LoadString(LFCoreModuleHandle, IDS_QSRC_UNKNOWN+StoreDescriptor.Source, &Hint[1], 255);
			Hint[1] = (WCHAR)tolower(Hint[1]);
		}
	}
	else
		if (wcscmp(StoreDescriptor.LastSeen, L"")!=0)
		{
			WCHAR LastSeen[256];
			LoadString(LFCoreModuleHandle, IDS_LASTSEEN, LastSeen, 256);

			wcscpy_s(Hint, 256, L", ");
			swprintf_s(&Hint[2], 254, LastSeen, StoreDescriptor.LastSeen);
			Hint[2] = (WCHAR)tolower(Hint[2]);
		}

	if (Hint[0])
		wcscat_s(pItemDescriptor->Description, 256, Hint);

	// Copy properties
	wcscpy_s(pItemDescriptor->CoreAttributes.FileName, 256, StoreDescriptor.StoreName);
	wcscpy_s(pItemDescriptor->CoreAttributes.Comments, 256, StoreDescriptor.Comments);
	strcpy_s(pItemDescriptor->StoreID, LFKeySize, StoreDescriptor.StoreID);

	pItemDescriptor->CoreAttributes.CreationTime = StoreDescriptor.CreationTime;
	pItemDescriptor->CoreAttributes.FileTime = StoreDescriptor.FileTime;
	pItemDescriptor->AggregateCount = StoreDescriptor.Statistics.FileCount[LFContextAllFiles];
	pItemDescriptor->CoreAttributes.FileSize = StoreDescriptor.Statistics.FileSize[LFContextAllFiles];

	// Copy store descriptor
	pItemDescriptor->StoreDescriptor = StoreDescriptor;
	GetDiskFreeSpaceForStore(pItemDescriptor->StoreDescriptor);

	return pItemDescriptor;
}

LFCORE_API LFItemDescriptor* LFCloneItemDescriptor(const LFItemDescriptor* pItemDescriptor)
{
	if (!pItemDescriptor)
		return LFAllocItemDescriptor();

	LFItemDescriptor* pClone = new LFItemDescriptor;
	memcpy(pClone, pItemDescriptor, sizeof(LFItemDescriptor));

	// Reset reference counter
	pClone->RefCount = 1;

	// Clone attached filter
	if (pItemDescriptor->pNextFilter)
		pClone->pNextFilter = LFCloneFilter(pItemDescriptor->pNextFilter);

	// Adjust attribute value pointers
	for (UINT a=0; a<LFAttributeCount; a++)
		if (pClone->AttributeValues[a])
			if (IsStaticAttribute(pItemDescriptor, a))
			{
				const INT_PTR Offset = (BYTE*)pItemDescriptor->AttributeValues[a]-(BYTE*)pItemDescriptor;
				pClone->AttributeValues[a] = (BYTE*)pClone+Offset;
			}
			else
			{
				const SIZE_T Size = _msize(pItemDescriptor->AttributeValues[a]);
				memcpy(pClone->AttributeValues[a] = malloc(Size), pItemDescriptor->AttributeValues[a], Size);
			}

	return pClone;
}

LFItemDescriptor* AllocFolderDescriptor(const LFFileSummary& FileSummary, const LFVariantData& VData, LFFilter* pFilter, INT AggregateFirst, INT AggregateLast)
{
	LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptor();

	pItemDescriptor->Type = LFTypeFolder | FileSummary.Source;
	pItemDescriptor->CoreAttributes.FileSize = FileSummary.FileSize;
	pItemDescriptor->CoreAttributes.Flags = FileSummary.Flags;
	pItemDescriptor->CoreAttributes.UserContextID = pItemDescriptor->CoreAttributes.SystemContextID = FileSummary.Context;
	pItemDescriptor->AggregateCount = FileSummary.FileCount;
	pItemDescriptor->AggregateFirst = AggregateFirst;
	pItemDescriptor->AggregateLast = AggregateLast;
	pItemDescriptor->AggregateColorSet = FileSummary.ItemColorSet;

	// Description
	LFGetFileSummaryEx(pItemDescriptor->Description, 256, FileSummary);

	// Icon
	pItemDescriptor->IconID = GetFolderIcon(FileSummary, VData);

	// Additional properties
	SetAttribute(pItemDescriptor, LFAttrLength, &FileSummary.Duration);

	// Filter
	pItemDescriptor->pNextFilter = LFCloneFilter(pFilter);
	pItemDescriptor->pNextFilter->IsSubfolder = TRUE;
	pItemDescriptor->pNextFilter->Query.pConditionList = LFAllocFilterCondition(LFFilterCompareSubfolder, VData, pItemDescriptor->pNextFilter->Query.pConditionList);

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
