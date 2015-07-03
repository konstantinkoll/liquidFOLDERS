
#include "stdafx.h"
#include "Categorizer.h"
#include "IdxTables.h"
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
extern OSVERSIONINFO osInfo;
extern const BYTE AttrTypes[];
extern UINT VolumeTypes[];
extern void LoadTwoStrings(HINSTANCE hInstance, UINT uID, WCHAR* lpBuffer1, INT cchBufferMax1, WCHAR* lpBuffer, INT cchBufferMax);


// LFSearchResult

LFSearchResult::LFSearchResult(INT ctx)
	: LFDynArray()
{
	assert(ctx<LFContextCount);

	LoadTwoStrings(LFCoreModuleHandle, IDS_CONTEXT_FIRST+ctx, m_Name, 256, m_Hint, 256);
	m_RawCopy = TRUE;
	m_Context = ctx;
	m_GroupAttribute = LFAttrFileName;
	m_HasCategories = FALSE;
	m_QueryTime = 0;
	m_FileCount = 0;
	m_FileSize = 0;
	m_StoreCount = 0;
}

LFSearchResult::LFSearchResult(LFFilter* f)
{
	m_RawCopy = TRUE;
	m_GroupAttribute = LFAttrFileName;
	m_HasCategories = FALSE;
	m_QueryTime = 0;
	m_FileCount = 0;
	m_FileSize = 0;
	m_StoreCount = 0;

	if (f)
	{
		SetMetadataFromFilter(f);
	}
	else
	{
		LoadTwoStrings(LFCoreModuleHandle, IDS_CONTEXT_FIRST+LFContextStores, m_Name, 256, m_Hint, 256);
		m_Context = LFContextStores;
	}
}

LFSearchResult::LFSearchResult(LFSearchResult* Result)
	: LFDynArray()
{
	m_Items = (LFItemDescriptor**)_aligned_malloc(Result->m_ItemCount*sizeof(LFItemDescriptor*), DYN_MEMORYALIGNMENT);

	wcscpy_s(m_Name, 256, Result->m_Name);
	wcscpy_s(m_Hint, 256, Result->m_Hint);
	m_RawCopy = FALSE;
	m_Context = Result->m_Context;
	m_GroupAttribute = Result->m_GroupAttribute;
	m_HasCategories = Result->m_HasCategories;
	m_QueryTime = Result->m_QueryTime;
	m_FileCount = Result->m_FileCount;
	m_FileSize = Result->m_FileSize;
	m_StoreCount = Result->m_StoreCount;

	if (m_Items)
	{
		m_LastError = Result->m_LastError;
		m_ItemCount = m_Allocated = Result->m_ItemCount;

		memcpy(m_Items, Result->m_Items, Result->m_ItemCount*sizeof(LFItemDescriptor*));
		for (UINT a=0; a<Result->m_ItemCount; a++)
			m_Items[a]->RefCount++;
	}
	else
	{
		m_LastError = LFMemoryError;
		m_ItemCount = m_Allocated =0;
	}
}

LFSearchResult::~LFSearchResult()
{
	if (m_Items)
		for (UINT a=0; a<m_ItemCount; a++)
			LFFreeItemDescriptor(m_Items[a]);
}

void LFSearchResult::SetMetadataFromFilter(LFFilter* f)
{
	assert(f);

	if (f->Options.IsSubfolder)
	{
		m_Context = LFContextSubfolderDefault;

		if (f->ConditionList)
		{
			m_GroupAttribute = f->Options.GroupAttribute;

			switch (f->ConditionList->AttrData.Attr)
			{
			case LFAttrLocationName:
			case LFAttrLocationIATA:
			case LFAttrLocationGPS:
				m_Context = LFContextSubfolderLocation;
				break;
			default:
				if (AttrTypes[f->ConditionList->AttrData.Attr]==LFTypeTime)
					m_Context = LFContextSubfolderDay;
			}
		}
	}
	else
		switch (f->Mode)
		{
		case LFFilterModeStores:
			m_Context = LFContextStores;
			break;
		case LFFilterModeDirectoryTree:
			m_Context = f->ContextID;
			break;
		case LFFilterModeSearch:
			m_Context = ((f->Options.IsPersistent) || (f->ContextID) || ((f->Searchterm[0]==L'\0') && (f->ConditionList==NULL))) ? f->ContextID : LFContextSearch;
			break;
		}

	if ((f->OriginalName[0]==L'\0') || (m_Context==LFContextStores))
	{
		LoadTwoStrings(LFCoreModuleHandle, IDS_CONTEXT_FIRST+m_Context, m_Name, 256, m_Hint, 256);
	}
	else
	{
		wcscpy_s(m_Name, 256, f->OriginalName);

		if (m_Context<=LFLastQueryContext)
		{
			LoadString(LFCoreModuleHandle, IDS_CONTEXT_FIRST+m_Context, m_Hint, 256);
			WCHAR* brk = wcschr(m_Hint, L'\n');
			if (brk)
				*brk = L'\0';

			if (wcscmp(m_Name, m_Hint)==0)
				m_Hint[0] = L'\0';
		}
		else
		{
			m_Hint[0] = L'\0';
		}
	}

	wcscpy_s(f->ResultName, 256, m_Name);
}

BOOL LFSearchResult::AddItemDescriptor(LFItemDescriptor* i)
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

BOOL LFSearchResult::AddStoreDescriptor(LFStoreDescriptor* s)
{
	assert(s);

	LFFilter* nf = LFAllocFilter();
	nf->Mode = LFFilterModeDirectoryTree;
	strcpy_s(nf->StoreID, LFKeySize, s->StoreID);
	wcscpy_s(nf->OriginalName, 256, s->StoreName);

	LFItemDescriptor* d = LFAllocItemDescriptor(s);
	d->NextFilter = nf;

	return AddItemDescriptor(d);
}

void LFSearchResult::AddVolumes()
{
	DWORD VolumesOnSystem = LFGetLogicalVolumes(LFGLV_External);
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

			UINT l = (UINT)wcslen(sfi.szDisplayName);
			if (l>=5)
				if ((sfi.szDisplayName[l-5]==L' ') && (sfi.szDisplayName[l-4]==L'(') && (sfi.szDisplayName[l-2]==L':') && (sfi.szDisplayName[l-1]==L')'))
					sfi.szDisplayName[l-5] = L'\0';

			LFItemDescriptor* d = LFAllocItemDescriptor();
			d->Type = LFTypeVolume;
			if (!sfi.dwAttributes)
				d->Type |= LFTypeGhosted | LFTypeNotMounted;
			
			d->CategoryID = LFItemCategoryVolumes;
			SetAttribute(d, LFAttrFileName, sfi.szDisplayName);

			CHAR FileID[] = " :";
			FileID[0] = cVolume;
			SetAttribute(d, LFAttrFileID, FileID);
			SetAttribute(d, LFAttrDescription, sfi.szTypeName);

			if (!AddItemDescriptor(d))
				LFFreeItemDescriptor(d);
		}
	}
}

void LFSearchResult::RemoveItemDescriptor(UINT idx, BOOL updatecount)
{
	assert(idx<m_ItemCount);

	if (updatecount)
		if ((m_Items[idx]->Type & LFTypeMask)==LFTypeFile)
		{
			m_FileCount--;
			m_FileSize -= m_Items[idx]->CoreAttributes.FileSize;
		}

	LFFreeItemDescriptor(m_Items[idx]);

	if (idx<--m_ItemCount)
		m_Items[idx] = m_Items[m_ItemCount];
}

void LFSearchResult::RemoveFlaggedItemDescriptors(BOOL updatecount)
{
	UINT idx = 0;

	while (idx<m_ItemCount)
	{
		if (m_Items[idx]->DeleteFlag)
		{
			m_Items[idx]->DeleteFlag = FALSE;
			RemoveItemDescriptor(idx, updatecount);
		}
		else
		{
			idx++;
		}
	}
}

void LFSearchResult::KeepRange(INT first, INT last)
{
	for (INT a=m_ItemCount-1; a>last; a--)
		RemoveItemDescriptor((UINT)a);
	for (INT a=first-1; a>=0; a--)
		RemoveItemDescriptor((UINT)a);
}

INT LFSearchResult::Compare(LFItemDescriptor* d1, LFItemDescriptor* d2, UINT Attr, BOOL descending)
{
	// Kategorien
	if ((m_HasCategories) && (d1->CategoryID!=d2->CategoryID))
		return (INT)d1->CategoryID-(INT)d2->CategoryID;

	// Wenn zwei Laufwerke anhand des Namens verglichen werden sollen, Laufwerksbuchstaben nehmen
	if (((d1->Type & LFTypeMask)==LFTypeVolume) && ((d2->Type & LFTypeMask)==LFTypeVolume))
		if (Attr==LFAttrFileName)
			Attr = LFAttrFileID;

	// Dateien mit NULL-Werten oder leeren Strings im gewünschten Attribut hinten einsortieren
	const UINT Type = AttrTypes[Attr];
	BOOL d1null = IsNullValue(Type, d1->AttributeValues[Attr]);
	BOOL d2null = IsNullValue(Type, d2->AttributeValues[Attr]);

	if (d1null!=d2null)
		return (INT)d1null-(INT)d2null;

	// Gewünschtes Attribut vergleichen
	INT cmp = 0;

	if ((!d1null) && (!d2null))
	{
		cmp = CompareValues(Type, d1->AttributeValues[Attr], d2->AttributeValues[Attr], FALSE);

		// Ggf. Reihenfolge umkehren
		if (descending)
			cmp = -cmp;
	}

	// Dateien gleich bzgl. Attribut? Dann nach Name und notfalls FileID vergleichen für stabiles Ergebnis
	if ((cmp==0) && (Attr!=LFAttrFileName))
		cmp = _wcsicmp(d1->CoreAttributes.FileName, d2->CoreAttributes.FileName);

	if ((cmp==0) && (Attr!=LFAttrStoreID))
		cmp = strcmp(d1->StoreID, d2->StoreID);

	if ((cmp==0) && (Attr!=LFAttrFileID))
		cmp = strcmp(d1->CoreAttributes.FileID, d2->CoreAttributes.FileID);

	// Wenn die Dateien noch immer gleich sind, ist irgendwas sehr kaputt
	assert(cmp!=0);

	return cmp;
}

void LFSearchResult::Heap(UINT wurzel, const UINT anz, const UINT Attr, const BOOL descending)
{
	LFItemDescriptor* i = m_Items[wurzel];
	UINT parent = wurzel;
	UINT child;

	while ((child=(parent+1)*2)<anz)
	{
		if (Compare(m_Items[child-1], m_Items[child], Attr, descending)>0)
			child--;

		m_Items[parent] = m_Items[child];
		parent = child;
	}

	if (child==anz)
	{
		if (Compare(m_Items[--child], i, Attr, descending)>=0)
		{
			m_Items[parent] = m_Items[child];
			m_Items[child] = i;
			return;
		}

		child = parent;
	}
	else
	{
		if (parent==wurzel)
			return;

		if (Compare(m_Items[parent], i, Attr, descending)>=0)
		{
			m_Items[parent] = i;
			return;
		}

		child = (parent-1)/2;
	}

	while (child!=wurzel)
	{
		parent = (child-1)/2;

		if (Compare(m_Items[parent], i, Attr, descending)>=0)
			break;

		m_Items[child] = m_Items[parent];
		child = parent;
	}

	m_Items[child] = i;
}

void LFSearchResult::Sort(UINT Attr, BOOL descending)
{
	if (m_ItemCount>1)
	{
		for (INT a=m_ItemCount/2-1; a>=0; a--)
			Heap(a, m_ItemCount, Attr, descending);
		for (INT a=m_ItemCount-1; a>0; a--)
		{
			std::swap(m_Items[0], m_Items[a]);
			Heap(0, a, Attr, descending);
		}
	}
}

UINT LFSearchResult::Aggregate(UINT write, UINT read1, UINT read2, void* c, UINT Attr, BOOL groupone, LFFilter* f)
{
	if (((read2==read1+1) && ((!groupone) || ((m_Items[read1]->Type & LFTypeMask)==LFTypeFolder))) || (IsNullValue(AttrTypes[Attr], m_Items[read1]->AttributeValues[Attr])))
	{
		for (UINT a=read1; a<read2; a++)
			m_Items[write++] = m_Items[a];

		return read2-read1;
	}
	else
	{
		LFItemDescriptor* folder = ((CCategorizer*)c)->GetFolder(m_Items[read1], f);
		folder->AggregateCount = read2-read1;
		if (!m_RawCopy)
		{
			folder->FirstAggregate = read1;
			folder->LastAggregate = read2-1;
		}

		INT64 size = 0;
		UINT Source = m_Items[read1]->Type & LFTypeSourceMask;
		for (UINT a=read1; a<read2; a++)
		{
			if ((m_Items[a]->Type & LFTypeSourceMask)!=Source)
				Source = LFTypeSourceUnknown;

			size += m_Items[a]->CoreAttributes.FileSize;
			LFFreeItemDescriptor(m_Items[a]);
		}

		folder->Type |= Source;
		SetAttribute(folder, LFAttrFileSize, &size);
		LFCombineFileCountSize(folder->AggregateCount, size, folder->Description, 256);
		m_Items[write] = folder;

		return 1;
	}
}

void LFSearchResult::Group(UINT Attr, BOOL groupone, LFFilter* f)
{
	if (!m_ItemCount)
		return;

	// Choose categorizer
	CCategorizer* c = NULL;

	switch (Attr)
	{
	case LFAttrLocationIATA:
		c = new CIATACategorizer();
		break;
	case LFAttrURL:
		c = new CURLCategorizer();
		break;
	case LFAttrFileName:
		if (!groupone)
		{
			c = new CNameCategorizer();
			break;
		}
	default:
		switch (AttrTypes[Attr])
		{
		case LFTypeRating:
			c = new CRatingCategorizer(Attr);
			break;
		case LFTypeTime:
			c = new CDateCategorizer(Attr);
			break;
		case LFTypeDuration:
			c = new CDurationCategorizer(Attr);
			break;
		case LFTypeMegapixel:
			c = new CMegapixelCategorizer(Attr);
			break;
		case LFTypeSize:
			c = new CSizeCategorizer(Attr);
			break;
		default:
			// Generic categorizer
			c = new CCategorizer(Attr);
		}
	}

	// Process
	UINT WritePtr = 0;
	UINT ReadPtr1 = 0;
	UINT ReadPtr2 = 1;

	while (ReadPtr2<m_ItemCount)
	{
		if (!c->IsEqual(m_Items[ReadPtr1], m_Items[ReadPtr2]))
		{
			WritePtr += Aggregate(WritePtr, ReadPtr1, ReadPtr2, c, Attr, groupone, f);
			ReadPtr1 = ReadPtr2;
		}

		ReadPtr2++;
	}

	WritePtr += Aggregate(WritePtr, ReadPtr1, m_ItemCount, c, Attr, groupone, f);
	m_ItemCount = WritePtr;

	delete c;
}

void LFSearchResult::GroupArray(UINT Attr, LFFilter* f)
{
	assert(AttrTypes[Attr]==LFTypeUnicodeArray);

	typedef struct { std::wstring name; BOOL multiple; UINT count; INT64 size; UINT source; } tagitem;
	typedef stdext::hash_map<std::wstring, tagitem> hashtags;
	hashtags tags;

	for (UINT a=0; a<m_ItemCount; a++)
	{
		WCHAR* tagarray = (WCHAR*)m_Items[a]->AttributeValues[Attr];
		BOOL found = FALSE;

		if (tagarray)
		{
			WCHAR tag[256];
			while (GetNextTag(&tagarray, tag, 256))
			{
				std::wstring key(tag);
				transform(key.begin(), key.end(), key.begin(), towlower);

				hashtags::iterator location = tags.find(key);
				if (location==tags.end())
				{
					tagitem item;
					item.name.assign(tag);
					item.multiple = FALSE;
					item.count = 1;
					item.size = m_Items[a]->CoreAttributes.FileSize;
					item.source = m_Items[a]->Type & LFTypeSourceMask;
					tags[key] = item;
				}
				else
				{
					if (!location->second.multiple)
						if (location->second.name.compare(tag)!=0)
							location->second.multiple = TRUE;

					if ((m_Items[a]->Type & LFTypeSourceMask)!=location->second.source)
						location->second.source = LFTypeSourceUnknown;

					location->second.count++;
					location->second.size += m_Items[a]->CoreAttributes.FileSize;
				}

				found = TRUE;
			}
		}

		m_Items[a]->DeleteFlag = found;
	}

	RemoveFlaggedItemDescriptors(FALSE);

	for (hashtags::iterator it=tags.begin(); it!=tags.end(); it++)
	{
		WCHAR tag[256];
		wcscpy_s(tag, 256, it->second.name.c_str());

		if (it->second.multiple)
		{
			BOOL first = TRUE;
			for (WCHAR* ptr=tag; *ptr; ptr++)
				switch (*ptr)
				{
				case L' ':
				case L',':
				case L':':
				case L';':
				case L'|':
				case L'-':
				case L'"':
					first = TRUE;
					break;
				default:
					*ptr = first ? (WCHAR)towupper(*ptr) : (WCHAR)towlower(*ptr);
					first = FALSE;
				}
		}

		LFItemDescriptor* folder = AllocFolderDescriptor();
		folder->AggregateCount = it->second.count;

		SetAttribute(folder, LFAttrFileName, tag);
		SetAttribute(folder, LFAttrFileSize, &it->second.size);
		SetAttribute(folder, Attr, tag);
		LFCombineFileCountSize(folder->AggregateCount, it->second.size, folder->Description, 256);

		LFFilterCondition* c = LFAllocFilterConditionEx(LFFilterCompareSubfolder, Attr);
		wcscpy_s(c->AttrData.UnicodeArray, 256, tag);

		folder->NextFilter = LFAllocFilter(f);
		folder->NextFilter->Options.IsSubfolder = TRUE;
		c->Next = folder->NextFilter->ConditionList;
		folder->NextFilter->ConditionList = c;
		wcscpy_s(folder->NextFilter->OriginalName, 256, tag);

		AddItemDescriptor(folder);
	}
}
