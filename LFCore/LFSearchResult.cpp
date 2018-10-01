
#include "stdafx.h"
#include "Categorizers.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "LFSearchResult.h"
#include "LFVariantData.h"
#include "TableApplications.h"
#include "TableAttributes.h"
#include "TableMusicGenres.h"
#include <algorithm>
#include <hash_map>
#include <malloc.h>
#include <shellapi.h>


extern HMODULE LFCoreModuleHandle;
extern UINT VolumeTypes[];
extern void LoadTwoStrings(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer1, SIZE_T cchBufferMax1, LPWSTR lpBuffer, SIZE_T cchBufferMax);


LFCORE_API LFSearchResult* LFAllocSearchResult(BYTE Context)
{
	assert(Context>=0);
	assert(Context<LFContextCount);

	LFSearchResult* pSearchResult = new LFSearchResult(Context);

	LoadTwoStrings(LFCoreModuleHandle, IDS_CONTEXT_FIRST+Context, pSearchResult->m_Name, 256, pSearchResult->m_Hint, 256);

	return pSearchResult;
}

LFCORE_API void LFFreeSearchResult(LFSearchResult* pSearchResult)
{
	delete pSearchResult;
}

LFCORE_API BOOL LFAddItem(LFSearchResult* pSearchResult, LFItemDescriptor* pItemDescriptor)
{
	assert(pSearchResult);
	assert(pItemDescriptor);

	// Skip if item already exists
	if (LFIsFile(pItemDescriptor))
		for (UINT a=0; a<pSearchResult->m_ItemCount; a++)
			if ((pItemDescriptor->StoreID==(*pSearchResult)[a]->StoreID) &&
				(pItemDescriptor->CoreAttributes.FileID==(*pSearchResult)[a]->CoreAttributes.FileID))
				{
					LFFreeItemDescriptor(pItemDescriptor);

					return TRUE;
				}

	// Select item
	pItemDescriptor->Type |= LFTypeSelected;

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

LFCORE_API LFSearchResult* LFGroupSearchResult(LFSearchResult* pSearchResult, UINT Attr, BOOL Descending, BOOL GroupSingle, LFFilter* pFilter)
{
	assert(pSearchResult);

	// Special treatment for string arrays
	if (AttrProperties[Attr].Type==LFTypeUnicodeArray)
	{
		pSearchResult = new LFSearchResult(pSearchResult);
		pSearchResult->GroupArray(Attr, pFilter);
		pSearchResult->Sort(Attr, Descending);

		return pSearchResult;
	}

	// Special treatment for missing GPS location
	if (Attr==LFAttrLocationGPS)
		for (UINT a=0; a<pSearchResult->m_ItemCount; a++)
			if (IsNullValue(AttrProperties[LFAttrLocationGPS].Type, (*pSearchResult)[a]->AttributeValues[LFAttrLocationGPS]))
			{
				LFAirport* pAirport;
				if (LFIATAGetAirportByCode((LPCSTR)(*pSearchResult)[a]->AttributeValues[LFAttrLocationIATA], pAirport))
					(*pSearchResult)[a]->AttributeValues[LFAttrLocationGPS] = &pAirport->Location;
			}

	pSearchResult->Sort(Attr, Descending);

	LFSearchResult* pCookedFiles = new LFSearchResult(pSearchResult);
	pCookedFiles->Group(Attr, GroupSingle, pFilter);

	// Revert to old GPS location
	if (Attr==LFAttrLocationGPS)
		for (UINT a=0; a<pSearchResult->m_ItemCount; a++)
			(*pSearchResult)[a]->AttributeValues[LFAttrLocationGPS] = &(*pSearchResult)[a]->CoreAttributes.LocationGPS;

	return pCookedFiles;
}

LFCORE_API void LFUpdateFolderColors(LFSearchResult* pCookedFiles, const LFSearchResult* pRawFiles)
{
	assert(pRawFiles);
	assert(pCookedFiles);

	pCookedFiles->UpdateFolderColors(pRawFiles);
}


// LFSearchResult
//

LFSearchResult::LFSearchResult(BYTE Context)
	: LFDynArray()
{
	assert(Context<LFContextCount);

	m_LastError = LFOk;

	m_Name[0] = m_Hint[0] = L'\0';
	m_QueryTime = 0;
	m_Context = Context;
	m_IconID = 0;

	m_RawCopy = TRUE;
	m_HasCategories = FALSE;

	InitializeFileSummary(m_FileSummary);
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
	m_IconID = pSearchResult->m_IconID;

	m_RawCopy = FALSE;
	m_HasCategories = pSearchResult->m_HasCategories;

	m_FileSummary = pSearchResult->m_FileSummary;

	if (pSearchResult->m_ItemCount)
	{
		assert(pSearchResult->m_Items);

		const SIZE_T Size = pSearchResult->m_ItemCount*sizeof(LFItemDescriptor*);

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

void LFSearchResult::FinishQuery(LFFilter* pFilter)
{
	if (!pFilter)
	{
		LoadTwoStrings(LFCoreModuleHandle, IDS_CONTEXT_FIRST+LFContextStores, m_Name, 256, m_Hint, 256);
		m_Context = LFContextStores;

		return;
	}

	// Determine context
	CloseFileSummary(m_FileSummary);

	if (pFilter->IsSubfolder)
	{
		if (pFilter->Query.pConditionList)
		{
			m_Context = CtxProperties[m_FileSummary.Context].SubfolderContext;
			m_IconID = GetFolderIcon(m_FileSummary, pFilter->Query.pConditionList->VData, TRUE);
		}
		else
		{
			m_Context = LFContextSubfolderDefault;
		}
	}
	else
		switch (pFilter->Query.Mode)
		{
		case LFFilterModeStores:
			m_Context = LFContextStores;
			break;

		case LFFilterModeDirectoryTree:
			m_Context = (pFilter->Query.Context==LFContextAuto) ? (m_FileSummary.Context==LFContextAuto) ? LFContextAllFiles : m_FileSummary.Context : pFilter->Query.Context;
			break;

		case LFFilterModeQuery:
			m_Context = (pFilter->IsPersistent || (pFilter->Query.SearchTerm[0]!=L'\0') || (pFilter->Query.pConditionList!=NULL)) ? LFContextSearch : pFilter->Query.Context;
			break;
		}

	// Name and hint
	if ((pFilter->Name[0]==L'\0') || (m_Context==LFContextStores))
	{
		LoadTwoStrings(LFCoreModuleHandle, IDS_CONTEXT_FIRST+m_Context, m_Name, 256, m_Hint, 256);
	}
	else
	{
		wcscpy_s(m_Name, 256, pFilter->Name);

		if (m_Context<=LFLastQueryContext)
		{
			LoadString(LFCoreModuleHandle, IDS_CONTEXT_FIRST+m_Context, m_Hint, 256);

			WCHAR* pChar = wcschr(m_Hint, L'\n');
			if (pChar)
				*pChar = L'\0';

			if (wcscmp(m_Name, m_Hint)==0)
				m_Hint[0] = L'\0';
		}
		else
		{
			m_Hint[0] = L'\0';
		}
	}

	wcscpy_s(pFilter->Result.Name, 256, m_Name);
	pFilter->Result.Context = m_Context;
}

void LFSearchResult::AddFileToSummary(LFFileSummary& FileSummary, LFItemDescriptor* pItemDescriptor)
{
	assert(LFIsFile(pItemDescriptor));

	if (FileSummary.FileCount++)
	{
		if ((pItemDescriptor->Type & LFTypeSourceMask)!=FileSummary.Source)
			FileSummary.Source = LFTypeSourceUnknown;
	}
	else
	{
		FileSummary.Source = pItemDescriptor->Type & LFTypeSourceMask;
	}

	FileSummary.FileSize += pItemDescriptor->CoreAttributes.FileSize;
	FileSummary.Flags |= (pItemDescriptor->CoreAttributes.Flags & (LFFlagNew | LFFlagTask | LFFlagMissing));
	FileSummary.ItemColors[pItemDescriptor->CoreAttributes.Color]++;
	FileSummary.ItemColorSet |= (BYTE)pItemDescriptor->AggregateColorSet;
	FileSummary.ContextSet |= (1ull<<LFGetUserContextID(pItemDescriptor));
	FileSummary.OnlyTimebasedMediaFiles &= LFIsTimebasedMediaFile(pItemDescriptor);

	if (pItemDescriptor->AttributeValues[LFAttrLength])
		FileSummary.Duration += *((UINT*)pItemDescriptor->AttributeValues[LFAttrLength]);
}

void LFSearchResult::CloseFileSummary(LFFileSummary& FileSummary)
{
	// Aggregate context
	for (BYTE a=0; a<=LFLastPersistentContext; a++)
		if ((FileSummary.ContextSet>>a) & 1)
			FileSummary.Context = (FileSummary.Context==LFContextAuto) ? a : LFContextAllFiles;

	if (FileSummary.Context==LFContextAuto)
		FileSummary.Context = LFContextAllFiles;
}

BOOL LFSearchResult::AddItem(LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	if (!LFDynArray::AddItem(pItemDescriptor))
		return FALSE;

	if (LFIsFile(pItemDescriptor))
	{
		// Special icon for filter
		if (_stricmp(pItemDescriptor->CoreAttributes.FileFormat, "filter")==0)
			pItemDescriptor->IconID = IDI_FLD_CONTENT;

		AddFileToSummary(m_FileSummary, pItemDescriptor);
	}

	return TRUE;
}

BOOL LFSearchResult::AddStoreDescriptor(const LFStoreDescriptor& StoreDescriptor)
{
	LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptorEx(StoreDescriptor);

	if (AddItem(pItemDescriptor))
	{
		m_HasCategories = TRUE;

		pItemDescriptor->pNextFilter = LFAllocFilter(LFFilterModeDirectoryTree);
		pItemDescriptor->pNextFilter->Query.StoreID = StoreDescriptor.StoreID;
		wcscpy_s(pItemDescriptor->pNextFilter->Name, 256, StoreDescriptor.StoreName);

		AddStoreToSummary(m_FileSummary, StoreDescriptor);

		return TRUE;
	}

	LFFreeItemDescriptor(pItemDescriptor);

	return FALSE;
}

void LFSearchResult::UpdateFileSummary(BOOL Close)
{
	InitializeFileSummary(m_FileSummary);

	for (UINT a=0; a<m_ItemCount; a++)
		AddFileToSummary(m_FileSummary, m_Items[a]);

	if (Close)
		CloseFileSummary(m_FileSummary);
}

void LFSearchResult::RemoveFlaggedItems(BOOL UpdateSummary)
{
	REMOVEITEMS(!m_Items[ReadIdx]->RemoveFlag,);

	if (UpdateSummary)
		UpdateFileSummary();
}

void LFSearchResult::KeepRange(UINT First, UINT Last)
{
	// Deselect all remaining items
	REMOVEITEMS((ReadIdx>=First) && (ReadIdx<=Last), ->Type &= ~LFTypeSelected);

	UpdateFileSummary(FALSE);
}

INT LFSearchResult::Compare(LFItemDescriptor* pItem1, LFItemDescriptor* pItem2, UINT Attr, BOOL Descending) const
{
	// Categories
	if (m_HasCategories && (pItem1->CategoryID!=pItem2->CategoryID))
		return (INT)pItem1->CategoryID-(INT)pItem2->CategoryID;

	// Items with NULL values or empty strings shall be last
	const UINT Type = AttrProperties[Attr].Type;
	BOOL i1Null = IsNullValue(Type, pItem1->AttributeValues[Attr]);
	BOOL i2Null = IsNullValue(Type, pItem2->AttributeValues[Attr]);

	if (i1Null!=i2Null)
		return (INT)i1Null-(INT)i2Null;

	// Compare attribute
	INT Result = 0;

	if (!i1Null && !i2Null)
	{
		Result = CompareValues(Type, pItem1->AttributeValues[Attr], pItem2->AttributeValues[Attr], FALSE);

		// Invert order
		if (Descending)
			Result = -Result;
	}

	// Media collections employ the sequence number as secondary
	if (!Result)
		switch (Attr)
		{
		case LFAttrMediaCollection:
			return Compare(pItem1, pItem2, LFAttrSequenceInCollection, IsAttributeSortDescending(m_Context, LFAttrSequenceInCollection));

		case LFAttrSequenceInCollection:
			return Compare(pItem1, pItem2, LFAttrTitle, FALSE);

		case LFAttrFileName:
			break;

		default:
			Result = _wcsicmp(pItem1->CoreAttributes.FileName, pItem2->CoreAttributes.FileName);
	}

	// Are the files identical? Then use store ID and file ID as secondaries
	if (!Result)
		if ((Result=strcmp(pItem1->StoreID, pItem2->StoreID))==0)
			Result = strcmp(pItem1->CoreAttributes.FileID, pItem2->CoreAttributes.FileID);

	return Result;
}

void LFSearchResult::Heap(UINT Element, const UINT Count, const UINT Attr, const BOOL Descending)
{
	LFItemDescriptor* pItemDescriptor = m_Items[Element];
	UINT Parent = Element;
	UINT Child;

	while ((Child=(Parent+1)*2)<Count)
	{
		if (Compare(m_Items[Child-1], m_Items[Child], Attr, Descending)>0)
			Child--;

		m_Items[Parent] = m_Items[Child];
		Parent = Child;
	}

	if (Child==Count)
	{
		if (Compare(m_Items[--Child], pItemDescriptor, Attr, Descending)>=0)
		{
			m_Items[Parent] = m_Items[Child];
			m_Items[Child] = pItemDescriptor;

			return;
		}

		Child = Parent;
	}
	else
	{
		if (Parent==Element)
			return;

		if (Compare(m_Items[Parent], pItemDescriptor, Attr, Descending)>=0)
		{
			m_Items[Parent] = pItemDescriptor;

			return;
		}

		Child = (Parent-1)/2;
	}

	while (Child!=Element)
	{
		Parent = (Child-1)/2;

		if (Compare(m_Items[Parent], pItemDescriptor, Attr, Descending)>=0)
			break;

		m_Items[Child] = m_Items[Parent];
		Child = Parent;
	}

	m_Items[Child] = pItemDescriptor;
}

void LFSearchResult::Sort(UINT Attr, BOOL Descending)
{
	if (m_ItemCount>1)
	{
		for (INT a=m_ItemCount/2-1; a>=0; a--)
			Heap(a, m_ItemCount, Attr, Descending);

		for (INT a=m_ItemCount-1; a>0; a--)
		{
			LFItemDescriptor* pItemDescriptor = m_Items[0];
			m_Items[0] = m_Items[a];
			m_Items[a] = pItemDescriptor;

			Heap(0, a, Attr, Descending);
		}
	}
}

UINT LFSearchResult::Aggregate(UINT WriteIndex, UINT ReadIndex1, UINT ReadIndex2, LPVOID pCategorizer, UINT Attr, BOOL GroupSingle, LFFilter* pFilter)
{
	// Retain item
	if (((ReadIndex2==ReadIndex1+1) && (!GroupSingle || LFIsFolder(m_Items[ReadIndex1]))) || (IsNullValue(AttrProperties[Attr].Type, m_Items[ReadIndex1]->AttributeValues[Attr])))
	{
		for (UINT a=ReadIndex1; a<ReadIndex2; a++)
			m_Items[WriteIndex++] = m_Items[a];

		return ReadIndex2-ReadIndex1;
	}

	// Create file summary
	LFFileSummary FileSummary;
	InitializeFileSummary(FileSummary);

	for (UINT a=ReadIndex1; a<ReadIndex2; a++)
	{
		AddFileToSummary(FileSummary, m_Items[a]);

		LFFreeItemDescriptor(m_Items[a]);
	}

	CloseFileSummary(FileSummary);

	// Replace with folder
	m_Items[WriteIndex] = ((CCategorizer*)pCategorizer)->GetFolder(m_Items[ReadIndex1], pFilter, FileSummary, m_RawCopy ? -1 : ReadIndex1, m_RawCopy ? -1 : ReadIndex2-1);

	return 1;
}

void LFSearchResult::Group(UINT Attr, BOOL GroupSingle, LFFilter* pFilter)
{
	if (!m_ItemCount)
		return;

	// Choose categorizer
	CCategorizer* pCategorizer = NULL;

	// Special attributes
	switch (Attr)
	{
	case LFAttrURL:
		pCategorizer = new CURLCategorizer();
		break;

	case LFAttrChannels:
		pCategorizer = new CChannelsCategorizer();
		break;

	case LFAttrFileName:
		if (!GroupSingle)
		{
			pCategorizer = new CNameCategorizer();
			break;
		}

		// Fall-through to type categorizers

	default:
		switch (AttrProperties[Attr].Type)
		{
		case LFTypeIATACode:
			pCategorizer = new CIATACategorizer();
			break;

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
			// Default categorizer
			pCategorizer = new CCategorizer(Attr);
		}
	}

	// Process
	UINT WritePtr = 0;
	UINT ReadIdx1 = 0;
	UINT ReadIdx2 = 1;

	while (ReadIdx2<m_ItemCount)
	{
		if (!pCategorizer->IsEqual(m_Items[ReadIdx1], m_Items[ReadIdx2]))
		{
			WritePtr += Aggregate(WritePtr, ReadIdx1, ReadIdx2, pCategorizer, Attr, GroupSingle, pFilter);
			ReadIdx1 = ReadIdx2;
		}

		ReadIdx2++;
	}

	WritePtr += Aggregate(WritePtr, ReadIdx1, m_ItemCount, pCategorizer, Attr, GroupSingle, pFilter);
	m_ItemCount = WritePtr;

	delete pCategorizer;
}

void LFSearchResult::GroupArray(UINT Attr, LFFilter* pFilter)
{
	assert(AttrProperties[Attr].Type==LFTypeUnicodeArray);
	assert(!AttrProperties[Attr].DefaultIconID);

	typedef struct { std::wstring Name; BOOL Multiple; UINT Source; LFFileSummary FileSummary; } TagItem;
	typedef stdext::hash_map<std::wstring, TagItem> Hashtags;
	Hashtags Tags;

	for (UINT a=0; a<m_ItemCount; a++)
	{
		BOOL Remove = FALSE;

		LPCWSTR pHashtagArray = (LPCWSTR)m_Items[a]->AttributeValues[Attr];
		if (pHashtagArray)
		{
			WCHAR Hashtag[256];
			while (GetNextHashtag(&pHashtagArray, Hashtag, 256))
			{
				std::wstring Key(Hashtag);
				transform(Key.begin(), Key.end(), Key.begin(), towlower);

				Hashtags::iterator Iterator = Tags.find(Key);
				if (Iterator==Tags.end())
				{
					TagItem Item = { Hashtag, FALSE, 1, m_Items[a]->Type & LFTypeSourceMask };

					InitializeFileSummary(Item.FileSummary);
					AddFileToSummary(Item.FileSummary, m_Items[a]);

					Tags[Key] = Item;
				}
				else
				{
					if (!Iterator->second.Multiple)
						if (Iterator->second.Name.compare(Hashtag)!=0)
							Iterator->second.Multiple = TRUE;

					if ((m_Items[a]->Type & LFTypeSourceMask)!=Iterator->second.Source)
						Iterator->second.Source = LFTypeSourceUnknown;

					AddFileToSummary(Iterator->second.FileSummary, m_Items[a]);
				}

				Remove = TRUE;
			}
		}

		m_Items[a]->RemoveFlag = Remove;
	}

	RemoveFlaggedItems(FALSE);

	for (Hashtags::iterator it=Tags.begin(); it!=Tags.end(); it++)
	{
		WCHAR Hashtag[256];
		wcscpy_s(Hashtag, 256, it->second.Name.c_str());

		if (it->second.Multiple)
		{
			BOOL First = TRUE;
			for (WCHAR* pChar=Hashtag; *pChar; pChar++)
				switch (*pChar)
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
					*pChar = First ? (WCHAR)towupper(*pChar) : (WCHAR)towlower(*pChar);
					First = FALSE;
				}
		}

		CloseFileSummary(it->second.FileSummary);

		// Filter condition
		LFVariantData VData;
		LFInitVariantData(VData, Attr);

		VData.IsNull = FALSE;
		wcscpy_s(VData.UnicodeArray, 256, Hashtag);

		// Folder
		LFItemDescriptor* pFolder = AllocFolderDescriptor(it->second.FileSummary, VData, pFilter);
		wcscpy_s(pFolder->pNextFilter->Name, 256, Hashtag);

		SetAttribute(pFolder, LFAttrFileName, Hashtag);
		SetAttribute(pFolder, Attr, Hashtag);

		AddItem(pFolder);
	}
}

void LFSearchResult::UpdateFolderColors(const LFSearchResult* pRawFiles)
{
	for (UINT a=0; a<m_ItemCount; a++)
	{
		LFItemDescriptor* pItemDescriptor = m_Items[a];

		if (LFIsAggregatedFolder(pItemDescriptor))
		{
			// Create file summary
			LFFileSummary FileSummary;
			InitializeFileSummary(FileSummary);

			for (UINT Index=(UINT)pItemDescriptor->AggregateFirst; Index<=(UINT)pItemDescriptor->AggregateLast; Index++)
				AddFileToSummary(FileSummary, (*pRawFiles)[Index]);

			CloseFileSummary(FileSummary);

			// Color set
			pItemDescriptor->AggregateColorSet = FileSummary.ItemColorSet;

			// Colored icon
			if ((pItemDescriptor->IconID>=IDI_FLD_DEFAULT) && (pItemDescriptor->IconID<IDI_FLD_DEFAULT+LFItemColorCount))
				pItemDescriptor->IconID = GetColoredFolderIcon(FileSummary);
		}
	}
}
