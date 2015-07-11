
#include "stdafx.h"
#include "Categorizers.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "LFSearchResult.h"
#include "LFVariantData.h"
#include "StoreCache.h"
#include <assert.h>
#include <algorithm>
#include <hash_map>
#include <malloc.h>
#include <shellapi.h>


extern HMODULE LFCoreModuleHandle;
extern UINT VolumeTypes[];
extern void LoadTwoStrings(HINSTANCE hInstance, UINT uID, WCHAR* lpBuffer1, INT cchBufferMax1, WCHAR* lpBuffer, INT cchBufferMax);


LFCORE_API LFSearchResult* LFAllocSearchResult(INT Context)
{
	assert(pSearchResult);

	return new LFSearchResult(Context);
}

LFCORE_API void LFFreeSearchResult(LFSearchResult* pSearchResult)
{
	assert(pSearchResult);

	delete pSearchResult;
}

LFCORE_API BOOL LFAddItem(LFSearchResult* pSearchResult, LFItemDescriptor* pItemDescriptor)
{
	assert(pSearchResult);

	return pSearchResult->AddItem(pItemDescriptor);
}

LFCORE_API void LFRemoveFlaggedItems(LFSearchResult* pSearchResult)
{
	assert(pSearchResult);

	pSearchResult->RemoveFlaggedItems();
}

LFCORE_API void LFSortSearchResult(LFSearchResult* pSearchResult, UINT Attr, BOOL Descending)
{
	assert(pSearchResult);

	pSearchResult->Sort(Attr, Descending);
}

LFCORE_API LFSearchResult* LFGroupSearchResult(LFSearchResult* pSearchResult, UINT Attr, BOOL Descending, BOOL GroupOne, LFFilter* pFilter)
{
	assert(pFilter);

	if (pFilter->Options.IsSubfolder)
	{
		pSearchResult->Sort(Attr, Descending);

		return pSearchResult;
	}

	// Special treatment for string arrays
	if (AttrTypes[Attr]==LFTypeUnicodeArray)
	{
		pSearchResult = new LFSearchResult(pSearchResult);
		pSearchResult->GroupArray(Attr, pFilter);
		pSearchResult->Sort(Attr, Descending);

		return pSearchResult;
	}

	// Special treatment for missing GPS location
	if (Attr==LFAttrLocationGPS)
		for (UINT a=0; a<pSearchResult->m_ItemCount; a++)
			if (IsNullValue(AttrTypes[LFAttrLocationGPS], pSearchResult->m_Items[a]->AttributeValues[LFAttrLocationGPS]))
			{
				LFAirport* pAirport;
				if (LFIATAGetAirportByCode((CHAR*)pSearchResult->m_Items[a]->AttributeValues[LFAttrLocationIATA], &pAirport))
					pSearchResult->m_Items[a]->AttributeValues[LFAttrLocationGPS] = &pAirport->Location;
			}

	pSearchResult->Sort(Attr, Descending);

	LFSearchResult* pCookedFiles = new LFSearchResult(pSearchResult);
	pCookedFiles->Group(Attr, GroupOne, pFilter);

	// Revert to old GPS location
	if (Attr==LFAttrLocationGPS)
		for (UINT a=0; a<pSearchResult->m_ItemCount; a++)
			pSearchResult->m_Items[a]->AttributeValues[LFAttrLocationGPS] = &pSearchResult->m_Items[a]->CoreAttributes.LocationGPS;

	return pCookedFiles;
}


// LFSearchResult
//

LFSearchResult::LFSearchResult(UINT Context)
	: LFDynArray()
{
	assert(Context<LFContextCount);

	LoadTwoStrings(LFCoreModuleHandle, IDS_CONTEXT_FIRST+Context, m_Name, 256, m_Hint, 256);
	m_QueryTime = 0;
	m_Context = Context;
	m_GroupAttribute = LFAttrFileName;

	m_RawCopy = TRUE;
	m_HasCategories = FALSE;

	m_FileCount = m_StoreCount = 0;
	m_FileSize = 0;
}

LFSearchResult::LFSearchResult(LFFilter* pFilter)
{
	m_QueryTime = 0;

	m_RawCopy = TRUE;
	m_GroupAttribute = LFAttrFileName;
	m_HasCategories = FALSE;

	m_FileCount = m_StoreCount = 0;
	m_FileSize = 0;

	if (pFilter)
	{
		SetMetadataFromFilter(pFilter);
	}
	else
	{
		LoadTwoStrings(LFCoreModuleHandle, IDS_CONTEXT_FIRST+LFContextStores, m_Name, 256, m_Hint, 256);
		m_Context = LFContextStores;
	}
}

LFSearchResult::LFSearchResult(LFSearchResult* pSearchResult)
	: LFDynArray()
{
	assert(pSearchResult);

	m_LastError = pSearchResult->m_LastError;

	wcscpy_s(m_Name, 256, pSearchResult->m_Name);
	wcscpy_s(m_Hint, 256, pSearchResult->m_Hint);
	m_QueryTime = pSearchResult->m_QueryTime;
	m_Context = pSearchResult->m_Context;
	m_GroupAttribute = pSearchResult->m_GroupAttribute;

	m_RawCopy = FALSE;
	m_HasCategories = pSearchResult->m_HasCategories;

	m_StoreCount = pSearchResult->m_StoreCount;
	m_FileCount = pSearchResult->m_FileCount;
	m_FileSize = pSearchResult->m_FileSize;

	if (pSearchResult->m_ItemCount)
	{
		assert(pSearchResult->m_Items);

		SIZE_T Size = pSearchResult->m_ItemCount*sizeof(LFItemDescriptor*);

		m_Items = (LFItemDescriptor**)malloc(Size);
		memcpy(m_Items, pSearchResult->m_Items, pSearchResult->m_ItemCount*sizeof(LFItemDescriptor*));

		for (UINT a=0; a<pSearchResult->m_ItemCount; a++)
			m_Items[a]->RefCount++;

		m_ItemCount = m_Allocated = pSearchResult->m_ItemCount;
	}
}

LFSearchResult::~LFSearchResult()
{
	if (m_Items)
		for (UINT a=0; a<m_ItemCount; a++)
			LFFreeItemDescriptor(m_Items[a]);
}

void LFSearchResult::SetMetadataFromFilter(LFFilter* pFilter)
{
	assert(pFilter);

	if (pFilter->Options.IsSubfolder)
	{
		m_Context = LFContextSubfolderDefault;

		if (pFilter->ConditionList)
		{
			m_GroupAttribute = pFilter->Options.GroupAttribute;

			switch (pFilter->ConditionList->AttrData.Attr)
			{
			case LFAttrLocationName:
			case LFAttrLocationIATA:
			case LFAttrLocationGPS:
				m_Context = LFContextSubfolderLocation;
				break;
			default:
				if (AttrTypes[pFilter->ConditionList->AttrData.Attr]==LFTypeTime)
					m_Context = LFContextSubfolderDay;
			}
		}
	}
	else
		switch (pFilter->Mode)
		{
		case LFFilterModeStores:
			m_Context = LFContextStores;
			break;
		case LFFilterModeDirectoryTree:
			m_Context = pFilter->ContextID;
			break;
		case LFFilterModeSearch:
			m_Context = (pFilter->Options.IsPersistent || (pFilter->Searchterm[0]!=L'\0') || (pFilter->ConditionList!=NULL)) ? LFContextSearch : pFilter->ContextID;
			break;
		}

	if ((pFilter->OriginalName[0]==L'\0') || (m_Context==LFContextStores))
	{
		LoadTwoStrings(LFCoreModuleHandle, IDS_CONTEXT_FIRST+m_Context, m_Name, 256, m_Hint, 256);
	}
	else
	{
		wcscpy_s(m_Name, 256, pFilter->OriginalName);

		if (m_Context<=LFLastQueryContext)
		{
			LoadString(LFCoreModuleHandle, IDS_CONTEXT_FIRST+m_Context, m_Hint, 256);

			WCHAR* Ptr = wcschr(m_Hint, L'\n');
			if (Ptr)
				*Ptr = L'\0';

			if (wcscmp(m_Name, m_Hint)==0)
				m_Hint[0] = L'\0';
		}
		else
		{
			m_Hint[0] = L'\0';
		}
	}

	wcscpy_s(pFilter->ResultName, 256, m_Name);
}

BOOL LFSearchResult::AddItem(LFItemDescriptor* i)
{
	assert(i);

	if (!LFDynArray::AddItem(i))
		return FALSE;

	switch (i->Type & LFTypeMask)
	{
	case LFTypeStore:
		m_StoreCount++;
		break;
	case LFTypeFile:
		m_FileCount++;
		m_FileSize += i->CoreAttributes.FileSize;
		break;
	}

	return TRUE;
}

BOOL LFSearchResult::AddStoreDescriptor(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);

	LFItemDescriptor* i = LFAllocItemDescriptorEx(pStoreDescriptor);

	if (AddItem(i))
	{
		i->NextFilter = LFAllocFilter();

		i->NextFilter->Mode = LFFilterModeDirectoryTree;
		strcpy_s(i->NextFilter->StoreID, LFKeySize, pStoreDescriptor->StoreID);
		wcscpy_s(i->NextFilter->OriginalName, 256, pStoreDescriptor->StoreName);

		return TRUE;
	}

	LFFreeItemDescriptor(i);

	return FALSE;
}

void LFSearchResult::AddVolumes()
{
	DWORD VolumesOnSystem = LFGetLogicalVolumes(LFGLV_EXTERNAL);
	BOOL HideVolumesWithNoMedia = LFHideVolumesWithNoMedia();

	for (CHAR cVolume='A'; cVolume<='Z'; cVolume++, VolumesOnSystem>>=1)
	{
		if ((VolumesOnSystem & 1)==0)
			continue;

		WCHAR szVolumeRoot[] = L" :\\";
		szVolumeRoot[0] = cVolume;

		SHFILEINFO sfi;
		if (SHGetFileInfo(szVolumeRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_ATTRIBUTES))
		{
			if ((!sfi.dwAttributes) && HideVolumesWithNoMedia)
				continue;

			SIZE_T Len = wcslen(sfi.szDisplayName);
			if (Len>=5)
				if ((sfi.szDisplayName[Len-5]==L' ') && (sfi.szDisplayName[Len-4]==L'(') && (sfi.szDisplayName[Len-2]==L':') && (sfi.szDisplayName[Len-1]==L')'))
					sfi.szDisplayName[Len-5] = L'\0';

			LFItemDescriptor* i = LFAllocItemDescriptor();
			i->Type = LFTypeVolume;
			if (!sfi.dwAttributes)
				i->Type |= LFTypeGhosted | LFTypeNotMounted;
			
			i->CategoryID = LFItemCategoryVolumes;
			SetAttribute(i, LFAttrFileName, sfi.szDisplayName);

			CHAR FileID[] = " :";
			FileID[0] = cVolume;

			SetAttribute(i, LFAttrFileID, FileID);
			SetAttribute(i, LFAttrDescription, sfi.szTypeName);

			if (!AddItem(i))
				LFFreeItemDescriptor(i);
		}
	}
}

void LFSearchResult::RemoveItem(UINT Index, BOOL UpdateCount)
{
	assert(Index<m_ItemCount);

	if (UpdateCount)
		if ((m_Items[Index]->Type & LFTypeMask)==LFTypeFile)
		{
			m_FileCount--;
			m_FileSize -= m_Items[Index]->CoreAttributes.FileSize;
		}

	LFFreeItemDescriptor(m_Items[Index]);

	if (Index<--m_ItemCount)
		m_Items[Index] = m_Items[m_ItemCount];
}

void LFSearchResult::RemoveFlaggedItems(BOOL UpdateCount)
{
	UINT Index = 0;

	while (Index<m_ItemCount)
	{
		if (m_Items[Index]->RemoveFlag)
		{
			RemoveItem(Index, UpdateCount);
		}
		else
		{
			Index++;
		}
	}
}

void LFSearchResult::KeepRange(INT First, INT Last)
{
	for (INT a=m_ItemCount-1; a>Last; a--)
		RemoveItem((UINT)a);

	for (INT a=First-1; a>=0; a--)
		RemoveItem((UINT)a);
}

INT LFSearchResult::Compare(LFItemDescriptor* i1, LFItemDescriptor* i2, UINT Attr, BOOL Descending)
{
	// Kategorien
	if ((m_HasCategories) && (i1->CategoryID!=i2->CategoryID))
		return (INT)i1->CategoryID-(INT)i2->CategoryID;

	// Wenn zwei Laufwerke anhand des Namens verglichen werden sollen, Laufwerksbuchstaben nehmen
	if (((i1->Type & LFTypeMask)==LFTypeVolume) && ((i2->Type & LFTypeMask)==LFTypeVolume))
		if (Attr==LFAttrFileName)
			Attr = LFAttrFileID;

	// Dateien mit NULL-Werten oder leeren Strings im gewünschten Attribut hinten einsortieren
	const UINT Type = AttrTypes[Attr];
	BOOL i1Null = IsNullValue(Type, i1->AttributeValues[Attr]);
	BOOL i2Null = IsNullValue(Type, i2->AttributeValues[Attr]);

	if (i1Null!=i2Null)
		return (INT)i1Null-(INT)i2Null;

	// Gewünschtes Attribut vergleichen
	INT Compare = 0;

	if ((!i1Null) && (!i2Null))
	{
		Compare = CompareValues(Type, i1->AttributeValues[Attr], i2->AttributeValues[Attr], FALSE);

		// Ggf. Reihenfolge umkehren
		if (Descending)
			Compare = -Compare;
	}

	// Dateien gleich bzgl. Attribut? Dann nach Name und notfalls FileID vergleichen für stabiles Ergebnis
	if ((Compare==0) && (Attr!=LFAttrFileName))
		Compare = _wcsicmp(i1->CoreAttributes.FileName, i2->CoreAttributes.FileName);

	if ((Compare==0) && (Attr!=LFAttrStoreID))
		Compare = strcmp(i1->StoreID, i2->StoreID);

	if ((Compare==0) && (Attr!=LFAttrFileID))
		Compare = strcmp(i1->CoreAttributes.FileID, i2->CoreAttributes.FileID);

	return Compare;
}

void LFSearchResult::Heap(UINT Wurzel, const UINT Anz, const UINT Attr, const BOOL Descending)
{
	LFItemDescriptor* i = m_Items[Wurzel];
	UINT Parent = Wurzel;
	UINT Child;

	while ((Child=(Parent+1)*2)<Anz)
	{
		if (Compare(m_Items[Child-1], m_Items[Child], Attr, Descending)>0)
			Child--;

		m_Items[Parent] = m_Items[Child];
		Parent = Child;
	}

	if (Child==Anz)
	{
		if (Compare(m_Items[--Child], i, Attr, Descending)>=0)
		{
			m_Items[Parent] = m_Items[Child];
			m_Items[Child] = i;

			return;
		}

		Child = Parent;
	}
	else
	{
		if (Parent==Wurzel)
			return;

		if (Compare(m_Items[Parent], i, Attr, Descending)>=0)
		{
			m_Items[Parent] = i;

			return;
		}

		Child = (Parent-1)/2;
	}

	while (Child!=Wurzel)
	{
		Parent = (Child-1)/2;

		if (Compare(m_Items[Parent], i, Attr, Descending)>=0)
			break;

		m_Items[Child] = m_Items[Parent];
		Child = Parent;
	}

	m_Items[Child] = i;
}

void LFSearchResult::Sort(UINT Attr, BOOL Descending)
{
	if (m_ItemCount>1)
	{
		for (INT a=m_ItemCount/2-1; a>=0; a--)
			Heap(a, m_ItemCount, Attr, Descending);

		for (INT a=m_ItemCount-1; a>0; a--)
		{
			LFItemDescriptor* Temp = m_Items[0];
			m_Items[0] = m_Items[a];
			m_Items[a] = Temp;

			Heap(0, a, Attr, Descending);
		}
	}
}

UINT LFSearchResult::Aggregate(UINT WriteIndex, UINT ReadIndex1, UINT ReadIndex2, void* pCategorizer, UINT Attr, BOOL GroupOne, LFFilter* pFilter)
{
	if (((ReadIndex2==ReadIndex1+1) && ((!GroupOne) || ((m_Items[ReadIndex1]->Type & LFTypeMask)==LFTypeFolder))) || (IsNullValue(AttrTypes[Attr], m_Items[ReadIndex1]->AttributeValues[Attr])))
	{
		for (UINT a=ReadIndex1; a<ReadIndex2; a++)
			m_Items[WriteIndex++] = m_Items[a];

		return ReadIndex2-ReadIndex1;
	}

	LFItemDescriptor* pFolder = ((CCategorizer*)pCategorizer)->GetFolder(m_Items[ReadIndex1], pFilter);

	pFolder->AggregateCount = ReadIndex2-ReadIndex1;
	if (!m_RawCopy)
	{
		pFolder->FirstAggregate = ReadIndex1;
		pFolder->LastAggregate = ReadIndex2-1;
	}

	INT64 Size = 0;
	UINT Source = m_Items[ReadIndex1]->Type & LFTypeSourceMask;

	for (UINT a=ReadIndex1; a<ReadIndex2; a++)
	{
		if ((m_Items[a]->Type & LFTypeSourceMask)!=Source)
			Source = LFTypeSourceUnknown;

		Size += m_Items[a]->CoreAttributes.FileSize;
		LFFreeItemDescriptor(m_Items[a]);
	}

	pFolder->Type |= Source;
	SetAttribute(pFolder, LFAttrFileSize, &Size);
	LFCombineFileCountSize(pFolder->AggregateCount, Size, pFolder->Description, 256);

	m_Items[WriteIndex] = pFolder;

	return 1;
}

void LFSearchResult::Group(UINT Attr, BOOL GroupOne, LFFilter* pFilter)
{
	if (!m_ItemCount)
		return;

	// Choose categorizer
	CCategorizer* pCategorizer = NULL;

	switch (Attr)
	{
	case LFAttrLocationIATA:
		pCategorizer = new CIATACategorizer();
		break;

	case LFAttrURL:
		pCategorizer = new CURLCategorizer();
		break;

	case LFAttrFileName:
		if (!GroupOne)
		{
			pCategorizer = new CNameCategorizer();
			break;
		}

	default:
		switch (AttrTypes[Attr])
		{
		case LFTypeRating:
			pCategorizer = new CRatingCategorizer(Attr);
			break;

		case LFTypeTime:
			pCategorizer = new CDateCategorizer(Attr);
			break;

		case LFTypeDuration:
			pCategorizer = new CDurationCategorizer(Attr);
			break;

		case LFTypeMegapixel:
			pCategorizer = new CMegapixelCategorizer(Attr);
			break;

		case LFTypeSize:
			pCategorizer = new CSizeCategorizer(Attr);
			break;

		default:
			// Generic categorizer
			pCategorizer = new CCategorizer(Attr);
		}
	}

	// Process
	UINT WritePtr = 0;
	UINT ReadPtr1 = 0;
	UINT ReadPtr2 = 1;

	while (ReadPtr2<m_ItemCount)
	{
		if (!pCategorizer->IsEqual(m_Items[ReadPtr1], m_Items[ReadPtr2]))
		{
			WritePtr += Aggregate(WritePtr, ReadPtr1, ReadPtr2, pCategorizer, Attr, GroupOne, pFilter);
			ReadPtr1 = ReadPtr2;
		}

		ReadPtr2++;
	}

	WritePtr += Aggregate(WritePtr, ReadPtr1, m_ItemCount, pCategorizer, Attr, GroupOne, pFilter);
	m_ItemCount = WritePtr;

	delete pCategorizer;
}

void LFSearchResult::GroupArray(UINT Attr, LFFilter* pFilter)
{
	assert(AttrTypes[Attr]==LFTypeUnicodeArray);

	typedef struct { std::wstring Name; BOOL Multiple; UINT Count; INT64 Size; UINT Source; } TagItem;
	typedef stdext::hash_map<std::wstring, TagItem> Hashtags;
	Hashtags Tags;

	for (UINT a=0; a<m_ItemCount; a++)
	{
		BOOL Remove = FALSE;

		WCHAR* TagArray = (WCHAR*)m_Items[a]->AttributeValues[Attr];
		if (TagArray)
		{
			WCHAR Tag[256];
			while (GetNextTag(&TagArray, Tag, 256))
			{
				std::wstring Key(Tag);
				transform(Key.begin(), Key.end(), Key.begin(), towlower);

				Hashtags::iterator Location = Tags.find(Key);
				if (Location==Tags.end())
				{
					TagItem Item = { Tag, FALSE, 1, 0, 0 };
					Tags[Key] = Item;
				}
				else
				{
					if (!Location->second.Multiple)
						if (Location->second.Name.compare(Tag)!=0)
							Location->second.Multiple = TRUE;

					if ((m_Items[a]->Type & LFTypeSourceMask)!=Location->second.Source)
						Location->second.Source = LFTypeSourceUnknown;

					Location->second.Count++;
					Location->second.Size += m_Items[a]->CoreAttributes.FileSize;
				}

				Remove = TRUE;
			}
		}

		m_Items[a]->RemoveFlag = Remove;
	}

	RemoveFlaggedItems(FALSE);

	for (Hashtags::iterator it=Tags.begin(); it!=Tags.end(); it++)
	{
		WCHAR Tag[256];
		wcscpy_s(Tag, 256, it->second.Name.c_str());

		if (it->second.Multiple)
		{
			BOOL First = TRUE;
			for (WCHAR* Ptr=Tag; *Ptr; Ptr++)
				switch (*Ptr)
				{
				case L' ':
				case L',':
				case L':':
				case L';':
				case L'|':
				case L'-':
				case L'"':
					First = TRUE;
					break;

				default:
					*Ptr = First ? (WCHAR)towupper(*Ptr) : (WCHAR)towlower(*Ptr);
					First = FALSE;
				}
		}

		LFItemDescriptor* pFolder = AllocFolderDescriptor();
		pFolder->AggregateCount = it->second.Count;

		SetAttribute(pFolder, LFAttrFileName, Tag);
		SetAttribute(pFolder, LFAttrFileSize, &it->second.Size);
		SetAttribute(pFolder, Attr, Tag);
		LFCombineFileCountSize(pFolder->AggregateCount, it->second.Size, pFolder->Description, 256);

		pFolder->NextFilter = LFAllocFilter(pFilter);
		pFolder->NextFilter->Options.IsSubfolder = TRUE;

		wcscpy_s(pFolder->NextFilter->OriginalName, 256, Tag);

		LFFilterCondition* pFilterCondition = LFAllocFilterConditionEx(LFFilterCompareSubfolder, Attr, pFolder->NextFilter->ConditionList);
		wcscpy_s(pFilterCondition->AttrData.UnicodeArray, 256, Tag);

		pFolder->NextFilter->ConditionList = pFilterCondition;

		AddItem(pFolder);
	}
}
