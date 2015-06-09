
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
extern const unsigned char AttrTypes[];
extern unsigned int VolumeTypes[];
extern void LoadTwoStrings(HINSTANCE hInstance, unsigned int uID, wchar_t* lpBuffer1, int cchBufferMax1, wchar_t* lpBuffer, int cchBufferMax);


// LFSearchResult

LFSearchResult::LFSearchResult(int ctx)
	: LFDynArray()
{
	assert(ctx<LFContextCount);

	LoadTwoStrings(LFCoreModuleHandle, IDS_FirstContext+ctx, m_Name, 256, m_Hint, 256);
	m_RawCopy = true;
	m_Context = ctx;
	m_GroupAttribute = LFAttrFileName;
	m_HasCategories = false;
	m_QueryTime = 0;
	m_FileCount = 0;
	m_FileSize = 0;
	m_StoreCount = 0;
}

LFSearchResult::LFSearchResult(LFFilter* f)
{
	m_RawCopy = true;
	m_GroupAttribute = LFAttrFileName;
	m_HasCategories = false;
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
		LoadTwoStrings(LFCoreModuleHandle, IDS_FirstContext+LFContextStores, m_Name, 256, m_Hint, 256);
		m_Context = LFContextStores;
	}
}

LFSearchResult::LFSearchResult(LFSearchResult* res)
	: LFDynArray()
{
	m_Items = (LFItemDescriptor**)_aligned_malloc(res->m_ItemCount*sizeof(LFItemDescriptor*), Dyn_MemoryAlignment);

	wcscpy_s(m_Name, 256, res->m_Name);
	wcscpy_s(m_Hint, 256, res->m_Hint);
	m_RawCopy = false;
	m_Context = res->m_Context;
	m_GroupAttribute = res->m_GroupAttribute;
	m_HasCategories = res->m_HasCategories;
	m_QueryTime = res->m_QueryTime;
	m_FileCount = res->m_FileCount;
	m_FileSize = res->m_FileSize;
	m_StoreCount = res->m_StoreCount;

	if (m_Items)
	{
		m_LastError = res->m_LastError;
		m_ItemCount = m_Allocated = res->m_ItemCount;

		memcpy(m_Items, res->m_Items, res->m_ItemCount*sizeof(LFItemDescriptor*));
		for (unsigned int a=0; a<res->m_ItemCount; a++)
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
		for (unsigned int a=0; a<m_ItemCount; a++)
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
		LoadTwoStrings(LFCoreModuleHandle, IDS_FirstContext+m_Context, m_Name, 256, m_Hint, 256);
	}
	else
	{
		wcscpy_s(m_Name, 256, f->OriginalName);

		if (m_Context<=LFLastQueryContext)
		{
			LoadString(LFCoreModuleHandle, IDS_FirstContext+m_Context, m_Hint, 256);
			wchar_t* brk = wcschr(m_Hint, L'\n');
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

bool LFSearchResult::AddItemDescriptor(LFItemDescriptor* i)
{
	assert(i);

	if (!LFDynArray::AddItem(i))
		return false;

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

	return true;
}

bool LFSearchResult::AddStoreDescriptor(LFStoreDescriptor* s)
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
	bool HideVolumesWithNoMedia = LFHideVolumesWithNoMedia();

	for (char cVolume='A'; cVolume<='Z'; cVolume++, VolumesOnSystem>>=1)
	{
		if ((VolumesOnSystem & 1)==0)
			continue;

		wchar_t szVolumeRoot[] = L" :\\";
		szVolumeRoot[0] = cVolume;
		SHFILEINFO sfi;
		if (SHGetFileInfo(szVolumeRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_ATTRIBUTES))
		{
			if ((!sfi.dwAttributes) && HideVolumesWithNoMedia)
				continue;

			unsigned int l = (unsigned int)wcslen(sfi.szDisplayName);
			if (l>=5)
				if ((sfi.szDisplayName[l-5]==L' ') && (sfi.szDisplayName[l-4]==L'(') && (sfi.szDisplayName[l-2]==L':') && (sfi.szDisplayName[l-1]==L')'))
					sfi.szDisplayName[l-5] = L'\0';

			LFItemDescriptor* d = LFAllocItemDescriptor();
			d->Type = LFTypeVolume;
			if (!sfi.dwAttributes)
				d->Type |= LFTypeGhosted | LFTypeNotMounted;
			
			d->CategoryID = LFItemCategoryVolumes;
			SetAttribute(d, LFAttrFileName, sfi.szDisplayName);

			char FileID[] = " :";
			FileID[0] = cVolume;
			SetAttribute(d, LFAttrFileID, FileID);
			SetAttribute(d, LFAttrDescription, sfi.szTypeName);

			if (!AddItemDescriptor(d))
				LFFreeItemDescriptor(d);
		}
	}
}

void LFSearchResult::RemoveItemDescriptor(unsigned int idx, bool updatecount)
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

void LFSearchResult::RemoveFlaggedItemDescriptors(bool updatecount)
{
	unsigned int idx = 0;

	while (idx<m_ItemCount)
	{
		if (m_Items[idx]->DeleteFlag)
		{
			m_Items[idx]->DeleteFlag = false;
			RemoveItemDescriptor(idx, updatecount);
		}
		else
		{
			idx++;
		}
	}
}

void LFSearchResult::KeepRange(int first, int last)
{
	for (int a=m_ItemCount-1; a>last; a--)
		RemoveItemDescriptor((unsigned int)a);
	for (int a=first-1; a>=0; a--)
		RemoveItemDescriptor((unsigned int)a);
}

int LFSearchResult::Compare(LFItemDescriptor* d1, LFItemDescriptor* d2, unsigned int attr, bool descending)
{
	// Kategorien
	if ((m_HasCategories) && (d1->CategoryID!=d2->CategoryID))
		return (int)d1->CategoryID-(int)d2->CategoryID;

	// Wenn zwei Laufwerke anhand des Namens verglichen werden sollen, Laufwerksbuchstaben nehmen
	unsigned int Sort = attr;
	unsigned int SortSecond = LFAttrFileName;
	if (((d1->Type & LFTypeMask)==LFTypeVolume) && ((d2->Type & LFTypeMask)==LFTypeVolume))
		if (Sort==LFAttrFileName)
		{
			Sort = LFAttrFileID;
		}
		else
		{
			SortSecond = LFAttrFileID;
		}

	// Dateien mit NULL-Werten oder leeren Strings im gewünschten Attribut hinten einsortieren
	bool d1null = IsNullValue(Sort, d1->AttributeValues[Sort]);
	bool d2null = IsNullValue(Sort, d2->AttributeValues[Sort]);

	if (d1null!=d2null)
		return (int)d1null-(int)d2null;

	// Gewünschtes Attribut vergleichen
	int cmp = 0;
	UINT eins32;
	UINT zwei32;
	__int64 eins64;
	__int64 zwei64;
	double einsDbl;
	double zweiDbl;
	LFGeoCoordinates einsCoord;
	LFGeoCoordinates zweiCoord;

	if ((!d1null) && (!d2null))
	{
		switch (AttrTypes[Sort])
		{
		case LFTypeUnicodeString:
		case LFTypeUnicodeArray:
			cmp = _wcsicmp((wchar_t*)d1->AttributeValues[Sort], (wchar_t*)d2->AttributeValues[Sort]);
			break;
		case LFTypeAnsiString:
			cmp = _stricmp((char*)d1->AttributeValues[Sort], (char*)d2->AttributeValues[Sort]);
			break;
		case LFTypeFourCC:
		case LFTypeUINT:
			eins32 = *(unsigned int*)d1->AttributeValues[Sort];
			zwei32 = *(unsigned int*)d2->AttributeValues[Sort];
			if (eins32<zwei32)
			{
				cmp = -1;
			}
			else
				if (eins32>zwei32)
				{
					cmp = 1;
				}
			break;
		case LFTypeRating:
			eins32 = *(unsigned char*)d1->AttributeValues[Sort];
			zwei32 = *(unsigned char*)d2->AttributeValues[Sort];
			if (eins32<zwei32)
			{
				cmp = -1;
			}
			else
				if (eins32>zwei32)
				{
					cmp = 1;
				}
			break;
		case LFTypeINT64:
			eins64 = *(__int64*)d1->AttributeValues[Sort];
			zwei64 = *(__int64*)d2->AttributeValues[Sort];
			if (eins64<zwei64)
			{
				cmp = -1;
			}
			else
				if (eins64>zwei64)
				{
					cmp = 1;
				}
			break;
		case LFTypeFraction:
			einsDbl = (double)((LFFraction*)d1->AttributeValues[Sort])->Num / (double)((LFFraction*)d1->AttributeValues[Sort])->Denum;
			zweiDbl = (double)((LFFraction*)d2->AttributeValues[Sort])->Num / (double)((LFFraction*)d2->AttributeValues[Sort])->Denum;
			if (einsDbl<zweiDbl)
			{
				cmp = -1;
			}
			else
				if (einsDbl>zweiDbl)
				{
					cmp = 1;
				}
			break;
		case LFTypeDouble:
			einsDbl = *(double*)d1->AttributeValues[Sort];
			zweiDbl = *(double*)d2->AttributeValues[Sort];
			if (einsDbl<zweiDbl)
			{
				cmp = -1;
			}
			else
				if (einsDbl>zweiDbl)
				{
					cmp = 1;
				}
			break;
		case LFTypeGeoCoordinates:
			einsCoord = *(LFGeoCoordinates*)d1->AttributeValues[Sort];
			zweiCoord = *(LFGeoCoordinates*)d2->AttributeValues[Sort];
			if (einsCoord.Latitude<zweiCoord.Latitude)
			{
				cmp = -1;
			}
			else
				if (einsCoord.Latitude>zweiCoord.Latitude)
				{
					cmp = 1;
				}
				else
					if (einsCoord.Longitude<zweiCoord.Longitude)
					{
						cmp = -1;
					}
					else
						if (einsCoord.Longitude>zweiCoord.Longitude)
						{
							cmp = 1;
						}
			break;
		case LFTypeTime:
			cmp = CompareFileTime((FILETIME*)d1->AttributeValues[Sort], (FILETIME*)d2->AttributeValues[Sort]);
			break;
		case LFTypeDuration:
		case LFTypeBitrate:
			eins32 = *(unsigned int*)d1->AttributeValues[Sort]/1000;
			zwei32 = *(unsigned int*)d2->AttributeValues[Sort]/1000;
			if (eins32<zwei32)
			{
				cmp = -1;
			}
			else
				if (eins32>zwei32)
				{
					cmp = 1;
				}
			break;
		case LFTypeMegapixel:
			einsDbl = (double)((int)(*(double*)d1->AttributeValues[Sort]*10))/(double)10;
			zweiDbl = (double)((int)(*(double*)d2->AttributeValues[Sort]*10))/(double)10;
			if (einsDbl<zweiDbl)
			{
				cmp = -1;
			}
			else
				if (einsDbl>zweiDbl)
				{
					cmp = 1;
				}
			break;
		default:
			assert(false);
		}

		// Ggf. Reihenfolge umkehren
		if (descending)
			cmp = -cmp;
	}

	// Dateien gleich bzgl. Attribut? Dann nach Name und notfalls FileID vergleichen für stabiles Ergebnis
	if (cmp==0)
	{
		if (Sort!=SortSecond)
			cmp = _wcsicmp((wchar_t*)d1->AttributeValues[SortSecond], (wchar_t*)d2->AttributeValues[SortSecond]);
		if ((cmp==0) && (Sort!=LFAttrStoreID) && (SortSecond!=LFAttrStoreID))
			cmp = strcmp((char*)d1->AttributeValues[LFAttrStoreID], (char*)d2->AttributeValues[LFAttrStoreID]);
		if ((cmp==0) && (Sort!=LFAttrFileID) && (SortSecond!=LFAttrFileID))
			cmp = strcmp((char*)d1->AttributeValues[LFAttrFileID], (char*)d2->AttributeValues[LFAttrFileID]);
	}

	// Wenn die Dateien noch immer gleich sind, ist irgendwas sehr kaputt
	assert(cmp!=0);

	return cmp;
}

void LFSearchResult::Heap(unsigned int wurzel, const unsigned int anz, const unsigned int attr, const bool descending)
{
	LFItemDescriptor* i = m_Items[wurzel];
	unsigned int parent = wurzel;
	unsigned int child;

	while ((child=(parent+1)*2)<anz)
	{
		if (Compare(m_Items[child-1], m_Items[child], attr, descending)>0)
			child--;

		m_Items[parent] = m_Items[child];
		parent = child;
	}

	if (child==anz)
	{
		if (Compare(m_Items[--child], i, attr, descending)>=0)
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

		if (Compare(m_Items[parent], i, attr, descending)>=0)
		{
			m_Items[parent] = i;
			return;
		}

		child = (parent-1)/2;
	}

	while (child!=wurzel)
	{
		parent = (child-1)/2;

		if (Compare(m_Items[parent], i, attr, descending)>=0)
			break;

		m_Items[child] = m_Items[parent];
		child = parent;
	}

	m_Items[child] = i;
}

void LFSearchResult::Sort(unsigned int attr, bool descending)
{
	if (m_ItemCount>1)
	{
		for (int a=m_ItemCount/2-1; a>=0; a--)
			Heap(a, m_ItemCount, attr, descending);
		for (int a=m_ItemCount-1; a>0; a--)
		{
			std::swap(m_Items[0], m_Items[a]);
			Heap(0, a, attr, descending);
		}
	}
}

unsigned int LFSearchResult::Aggregate(unsigned int write, unsigned int read1, unsigned int read2, void* c, unsigned int attr, bool groupone, LFFilter* f)
{
	if (((read2==read1+1) && ((!groupone) || ((m_Items[read1]->Type & LFTypeMask)==LFTypeFolder))) || (IsNullValue(attr, m_Items[read1]->AttributeValues[attr])))
	{
		for (unsigned int a=read1; a<read2; a++)
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

		__int64 size = 0;
		unsigned int Source = m_Items[read1]->Type & LFTypeSourceMask;
		for (unsigned int a=read1; a<read2; a++)
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

void LFSearchResult::Group(unsigned int attr, bool groupone, LFFilter* f)
{
	if (!m_ItemCount)
		return;

	// Choose categorizer
	CCategorizer* c = NULL;

	switch (attr)
	{
	case LFAttrFileSize:
		c = new SizeCategorizer(attr);
		break;
	case LFAttrLocationIATA:
		c = new IATACategorizer(attr);
		break;
	case LFAttrURL:
		c = new URLCategorizer(attr);
		break;
	case LFAttrFileName:
		if (!groupone)
		{
			c = new NameCategorizer(attr);
			break;
		}
	default:
		switch (AttrTypes[attr])
		{
		case LFTypeUnicodeString:
			c = new UnicodeCategorizer(attr);
			break;
		case LFTypeAnsiString:
			c = new AnsiCategorizer(attr);
			break;
		case LFTypeUINT:
			c = new UINTCategorizer(attr);
			break;
		case LFTypeRating:
			c = new RatingCategorizer(attr);
			break;
		case LFTypeGeoCoordinates:
			c = new CoordCategorizer(attr);
			break;
		case LFTypeTime:
			c = new DateCategorizer(attr);
			break;
		case LFTypeDuration:
			c = new DurationCategorizer(attr);
			break;
		case LFTypeBitrate:
			c = new DurationBitrateCategorizer(attr);
			break;
		case LFTypeMegapixel:
			c = new MegapixelCategorizer(attr);
			break;
		default:
			// Generic (slow) categorizer that compares the string representation of attributes
			c = new CCategorizer(attr);
		}
	}

	if (!c)
		return;

	// Process
	unsigned int WritePtr = 0;
	unsigned int ReadPtr1 = 0;
	unsigned int ReadPtr2 = 1;

	while (ReadPtr2<m_ItemCount)
	{
		if (!c->IsEqual(m_Items[ReadPtr1], m_Items[ReadPtr2]))
		{
			WritePtr += Aggregate(WritePtr, ReadPtr1, ReadPtr2, c, attr, groupone, f);
			ReadPtr1 = ReadPtr2;
		}

		ReadPtr2++;
	}

	WritePtr += Aggregate(WritePtr, ReadPtr1, m_ItemCount, c, attr, groupone, f);
	m_ItemCount = WritePtr;

	delete c;
}

void LFSearchResult::GroupArray(unsigned int attr, LFFilter* f)
{
	assert(AttrTypes[attr]==LFTypeUnicodeArray);

	typedef struct { std::wstring name; bool multiple; unsigned int count; __int64 size; unsigned int source; } tagitem;
	typedef stdext::hash_map<std::wstring, tagitem> hashtags;
	hashtags tags;

	for (unsigned int a=0; a<m_ItemCount; a++)
	{
		wchar_t* tagarray = (wchar_t*)m_Items[a]->AttributeValues[attr];
		bool found = false;

		if (tagarray)
		{
			wchar_t tag[256];
			while (GetNextTag(&tagarray, tag, 256))
			{
				std::wstring key(tag);
				transform(key.begin(), key.end(), key.begin(), towlower);

				hashtags::iterator location = tags.find(key);
				if (location==tags.end())
				{
					tagitem item;
					item.name.assign(tag);
					item.multiple = false;
					item.count = 1;
					item.size = m_Items[a]->CoreAttributes.FileSize;
					item.source = m_Items[a]->Type & LFTypeSourceMask;
					tags[key] = item;
				}
				else
				{
					if (!location->second.multiple)
						if (location->second.name.compare(tag)!=0)
							location->second.multiple = true;

					if ((m_Items[a]->Type & LFTypeSourceMask)!=location->second.source)
						location->second.source = LFTypeSourceUnknown;

					location->second.count++;
					location->second.size += m_Items[a]->CoreAttributes.FileSize;
				}

				found = true;
			}
		}

		m_Items[a]->DeleteFlag = found;
	}

	RemoveFlaggedItemDescriptors(false);

	for (hashtags::iterator it=tags.begin(); it!=tags.end(); it++)
	{
		wchar_t tag[256];
		wcscpy_s(tag, 256, it->second.name.c_str());

		if (it->second.multiple)
		{
			bool first = true;
			for (wchar_t* ptr=tag; *ptr; ptr++)
				switch (*ptr)
				{
				case L' ':
				case L',':
				case L':':
				case L';':
				case L'|':
				case L'-':
				case L'"':
					first = true;
					break;
				default:
					*ptr = first ? (wchar_t)towupper(*ptr) : (wchar_t)towlower(*ptr);
					first = false;
				}
		}

		LFItemDescriptor* folder = AllocFolderDescriptor();
		folder->AggregateCount = it->second.count;

		SetAttribute(folder, LFAttrFileName, tag);
		SetAttribute(folder, LFAttrFileSize, &it->second.size);
		SetAttribute(folder, attr, tag);
		LFCombineFileCountSize(folder->AggregateCount, it->second.size, folder->Description, 256);

		LFFilterCondition* c = LFAllocFilterCondition();
		c->Compare = LFFilterCompareSubfolder;
		c->AttrData.Attr = attr;
		c->AttrData.Type = AttrTypes[attr];
		wcscpy_s(c->AttrData.UnicodeArray, 256, tag);

		folder->NextFilter = LFAllocFilter(f);
		folder->NextFilter->Options.IsSubfolder = true;
		c->Next = folder->NextFilter->ConditionList;
		folder->NextFilter->ConditionList = c;
		wcscpy_s(folder->NextFilter->OriginalName, 256, tag);

		AddItemDescriptor(folder);
	}
}
