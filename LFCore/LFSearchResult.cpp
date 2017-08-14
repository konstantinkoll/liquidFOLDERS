
#include "stdafx.h"
#include "AttributeTables.h"
#include "Categorizers.h"
#include "ID3.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "LFSearchResult.h"
#include "LFVariantData.h"
#include <assert.h>
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
				if (LFIATAGetAirportByCode((LPCSTR)(*pSearchResult)[a]->AttributeValues[LFAttrLocationIATA], &pAirport))
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

	m_StoreCount = 0;
	InitFileSummary(m_FileSummary);

	m_AutoContext = LFContextAuto;
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

	m_StoreCount = pSearchResult->m_StoreCount;
	m_FileSummary = pSearchResult->m_FileSummary;

	m_AutoContext = pSearchResult->m_AutoContext;

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

void LFSearchResult::FinishQuery(LFFilter* pFilter)
{
	if (!pFilter)
	{
		LoadTwoStrings(LFCoreModuleHandle, IDS_CONTEXT_FIRST+LFContextStores, m_Name, 256, m_Hint, 256);
		m_Context = LFContextStores;

		return;
	}

	// Determine context
	if (pFilter->Options.IsSubfolder)
	{
		m_Context = LFContextSubfolderDefault;

		if (pFilter->pConditionList)
		{
			const UINT Attr = pFilter->pConditionList->AttrData.Attr;

			switch (Attr)
			{
			case LFAttrGenre:
				m_Context = LFContextSubfolderGenre;
				m_IconID = GetGenreIcon(pFilter->pConditionList->AttrData.UINT32);

				break;

			case LFAttrArtist:
				m_Context = LFContextSubfolderArtist;

				break;

			case LFAttrAlbum:
				m_Context = LFContextSubfolderAlbum;
				m_IconID = IDI_FLD_PLACEHOLDER;

				break;

			default:
				if (AttrProperties[Attr].Type==LFTypeTime)
				{
					m_Context = LFContextSubfolderDay;
					m_HasCategories = TRUE;

					// Set item categories
					for (UINT a=0; a<m_ItemCount; a++)
					{
						LFItemDescriptor* pItemDescriptor = m_Items[a];

						assert((pItemDescriptor->Type & LFTypeMask)==LFTypeFile);

						SYSTEMTIME stUTC;
						SYSTEMTIME stLocal;
						FileTimeToSystemTime((FILETIME*)pItemDescriptor->AttributeValues[Attr], &stUTC);
						SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

						pItemDescriptor->CategoryID = stLocal.wHour<6 ? LFItemCategoryNight : LFItemCategory0600+stLocal.wHour-6;
					}
				}
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
			m_Context = (pFilter->QueryContext==LFContextAuto) ? (m_AutoContext==LFContextAuto) ? LFContextAllFiles : m_AutoContext : pFilter->QueryContext;
			break;

		case LFFilterModeSearch:
			m_Context = (pFilter->Options.IsPersistent || (pFilter->Searchterm[0]!=L'\0') || (pFilter->pConditionList!=NULL)) ? LFContextSearch : pFilter->QueryContext;
			break;
		}

	// Name and hint
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

	wcscpy_s(pFilter->ResultName, 256, m_Name);
	pFilter->ResultContext = m_Context;
}

void LFSearchResult::AddFileToSummary(LFFileSummary& FileSummary, LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);
	assert((pItemDescriptor->Type & LFTypeMask)==LFTypeFile);

	if (FileSummary.FileCount++)
	{
		FileSummary.Source = pItemDescriptor->Type & LFTypeSourceMask;
	}
	else
	{
		if ((pItemDescriptor->Type & LFTypeSourceMask)!=FileSummary.Source)
			FileSummary.Source = LFTypeSourceUnknown;
	}

	FileSummary.FileSize += pItemDescriptor->CoreAttributes.FileSize;
	FileSummary.Flags |= (pItemDescriptor->CoreAttributes.Flags & (LFFlagNew | LFFlagMissing));
	FileSummary.OnlyMediaFiles &= (pItemDescriptor->CoreAttributes.ContextID==LFContextAudio) || (pItemDescriptor->CoreAttributes.ContextID==LFContextVideos);

	if (pItemDescriptor->AttributeValues[LFAttrDuration])
		FileSummary.Duration += *((UINT*)pItemDescriptor->AttributeValues[LFAttrDuration]);
}

void LFSearchResult::RemoveFileFromSummary(LFFileSummary& FileSummary, LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);
	assert((pItemDescriptor->Type & LFTypeMask)==LFTypeFile);

	FileSummary.FileCount--;
	FileSummary.FileSize -= pItemDescriptor->CoreAttributes.FileSize;

	if (pItemDescriptor->AttributeValues[LFAttrDuration])
		FileSummary.Duration -= *((UINT*)pItemDescriptor->AttributeValues[LFAttrDuration]);
}

BOOL LFSearchResult::AddItem(LFItemDescriptor* pItemDescriptor)
{
	assert(pItemDescriptor);

	if (!LFDynArray::AddItem(pItemDescriptor))
		return FALSE;

	if ((pItemDescriptor->Type & LFTypeMask)==LFTypeFile)
	{
		if (strcmp(pItemDescriptor->CoreAttributes.FileFormat, "filter")==0)
			pItemDescriptor->IconID = IDI_FLD_ALL;

		switch (m_AutoContext)
		{
		case LFContextAuto:
			m_AutoContext = pItemDescriptor->CoreAttributes.ContextID;

		case LFContextAllFiles:
			break;

		default:
			if (m_AutoContext!=pItemDescriptor->CoreAttributes.ContextID)
				m_AutoContext = LFContextAllFiles;
		}

		AddFileToSummary(m_FileSummary, pItemDescriptor);
	}

	return TRUE;
}

BOOL LFSearchResult::AddStoreDescriptor(LFStoreDescriptor* pStoreDescriptor)
{
	assert(pStoreDescriptor);

	LFItemDescriptor* pItemDescriptor = LFAllocItemDescriptorEx(pStoreDescriptor);

	if (AddItem(pItemDescriptor))
	{
		m_HasCategories = TRUE;

		pItemDescriptor->pNextFilter = LFAllocFilter();

		pItemDescriptor->pNextFilter->Mode = LFFilterModeDirectoryTree;
		pItemDescriptor->pNextFilter->QueryContext = LFContextAuto;

		strcpy_s(pItemDescriptor->pNextFilter->StoreID, LFKeySize, pStoreDescriptor->StoreID);
		wcscpy_s(pItemDescriptor->pNextFilter->OriginalName, 256, pStoreDescriptor->StoreName);

		AddStoreToSummary(m_FileSummary, pStoreDescriptor);

		return TRUE;
	}

	LFFreeItemDescriptor(pItemDescriptor);

	return FALSE;
}

void LFSearchResult::RemoveItem(UINT Index, BOOL UpdateCount)
{
	assert(Index<m_ItemCount);

	if (UpdateCount && ((m_Items[Index]->Type & LFTypeMask)==LFTypeFile))
		RemoveFileFromSummary(m_FileSummary, m_Items[Index]);

	LFFreeItemDescriptor(m_Items[Index]);

	if (Index<--m_ItemCount)
		m_Items[Index] = m_Items[m_ItemCount];
}

void LFSearchResult::RemoveFlaggedItems(BOOL UpdateCount)
{
	UINT Index = 0;

	while (Index<m_ItemCount)
		if (m_Items[Index]->RemoveFlag)
		{
			RemoveItem(Index, UpdateCount);
		}
		else
		{
			Index++;
		}
}

void LFSearchResult::KeepRange(INT First, INT Last)
{
	for (INT a=m_ItemCount-1; a>Last; a--)
		RemoveItem((UINT)a);

	for (INT a=First-1; a>=0; a--)
		RemoveItem((UINT)a);

	// Deselect all remaining items
	for (UINT a=0; a<m_ItemCount; a++)
		m_Items[a]->Type &= ~LFTypeSelected;
}

INT LFSearchResult::Compare(LFItemDescriptor* pItem1, LFItemDescriptor* pItem2, UINT Attr, BOOL Descending) const
{
	// Kategorien
	if ((m_HasCategories) && (pItem1->CategoryID!=pItem2->CategoryID))
		return (INT)pItem1->CategoryID-(INT)pItem2->CategoryID;

	// Dateien mit NULL-Werten oder leeren Strings im gewünschten Attribut hinten einsortieren
	const UINT Type = AttrProperties[Attr].Type;
	BOOL i1Null = IsNullValue(Type, pItem1->AttributeValues[Attr]);
	BOOL i2Null = IsNullValue(Type, pItem2->AttributeValues[Attr]);

	if (i1Null!=i2Null)
		return (INT)i1Null-(INT)i2Null;

	// Gewünschtes Attribut vergleichen
	INT Compare = 0;

	if ((!i1Null) && (!i2Null))
	{
		Compare = CompareValues(Type, pItem1->AttributeValues[Attr], pItem2->AttributeValues[Attr], FALSE);

		// Ggf. Reihenfolge umkehren
		if (Descending)
			Compare = -Compare;
	}

	// Dateien gleich bzgl. Attribut? Dann nach Name und notfalls FileID vergleichen für stabiles Ergebnis
	if ((Compare==0) && (Attr!=LFAttrFileName))
		Compare = _wcsicmp(pItem1->CoreAttributes.FileName, pItem2->CoreAttributes.FileName);

	if (Compare==0)
		Compare = strcmp(pItem1->StoreID, pItem2->StoreID);

	if (Compare==0)
		Compare = strcmp(pItem1->CoreAttributes.FileID, pItem2->CoreAttributes.FileID);

	return Compare;
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
	if (((ReadIndex2==ReadIndex1+1) && (!GroupSingle || ((m_Items[ReadIndex1]->Type & LFTypeMask)==LFTypeFolder))) || (IsNullValue(AttrProperties[Attr].Type, m_Items[ReadIndex1]->AttributeValues[Attr])))
	{
		for (UINT a=ReadIndex1; a<ReadIndex2; a++)
			m_Items[WriteIndex++] = m_Items[a];

		return ReadIndex2-ReadIndex1;
	}

	// Create file summary
	LFFileSummary FileSummary;
	InitFileSummary(FileSummary);

	for (UINT a=ReadIndex1; a<ReadIndex2; a++)
	{
		AddFileToSummary(FileSummary, m_Items[a]);

		LFFreeItemDescriptor(m_Items[a]);
	}

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

	switch (Attr)
	{
	case LFAttrURL:
		pCategorizer = new CURLCategorizer();
		break;

	case LFAttrFileName:
		if (!GroupSingle)
		{
			pCategorizer = new CNameCategorizer();
			break;
		}

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
			WritePtr += Aggregate(WritePtr, ReadPtr1, ReadPtr2, pCategorizer, Attr, GroupSingle, pFilter);
			ReadPtr1 = ReadPtr2;
		}

		ReadPtr2++;
	}

	WritePtr += Aggregate(WritePtr, ReadPtr1, m_ItemCount, pCategorizer, Attr, GroupSingle, pFilter);
	m_ItemCount = WritePtr;

	delete pCategorizer;
}

void LFSearchResult::GroupArray(UINT Attr, LFFilter* pFilter)
{
	assert(AttrProperties[Attr].Type==LFTypeUnicodeArray);

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

				Hashtags::iterator Location = Tags.find(Key);
				if (Location==Tags.end())
				{
					TagItem Item = { Hashtag, FALSE, 1, m_Items[a]->Type & LFTypeSourceMask };

					InitFileSummary(Item.FileSummary);
					AddFileToSummary(Item.FileSummary, m_Items[a]);

					Tags[Key] = Item;
				}
				else
				{
					if (!Location->second.Multiple)
						if (Location->second.Name.compare(Hashtag)!=0)
							Location->second.Multiple = TRUE;

					if ((m_Items[a]->Type & LFTypeSourceMask)!=Location->second.Source)
						Location->second.Source = LFTypeSourceUnknown;

					AddFileToSummary(Location->second.FileSummary, m_Items[a]);
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

		LFItemDescriptor* pFolder = AllocFolderDescriptor(Attr, it->second.FileSummary);

		SetAttribute(pFolder, LFAttrFileName, Hashtag);
		SetAttribute(pFolder, Attr, Hashtag);

		pFolder->pNextFilter = LFAllocFilter(pFilter);
		pFolder->pNextFilter->Options.IsSubfolder = TRUE;

		wcscpy_s(pFolder->pNextFilter->OriginalName, 256, Hashtag);

		LFFilterCondition* pFilterCondition = LFAllocFilterConditionEx(LFFilterCompareSubfolder, Attr, pFolder->pNextFilter->pConditionList);
		wcscpy_s(pFilterCondition->AttrData.UnicodeArray, 256, Hashtag);

		pFolder->pNextFilter->pConditionList = pFilterCondition;

		AddItem(pFolder);
	}
}
