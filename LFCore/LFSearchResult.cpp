#include "StdAfx.h"
#include "Categorizer.h"
#include "IdxTables.h"
#include "LFCore.h"
#include "LFItemDescriptor.h"
#include "LFSearchResult.h"
#include "LFVariantData.h"
#include "StoreCache.h"
#include <assert.h>
#include <malloc.h>
#include <shellapi.h>


extern HMODULE LFCoreModuleHandle;
extern unsigned char AttrTypes[];


LFSearchResult::LFSearchResult(int ctx)
{
	m_RawCopy = true;
	m_LastError = LFOk;
	m_Context = ctx;
	m_ContextView = ctx;
	m_RecommendedView = LFViewDetails;
	m_HidingItems = false;
	m_HasCategories = false;
	m_QueryTime = 0;
	m_Items = NULL;
	m_ItemCount = 0;
	m_FileCount = 0;
	m_FileSize = 0;
	m_StoreID[0] = '\0';
	m_Allocated = 0;
}

LFSearchResult::LFSearchResult(int ctx, LFSearchResult* res)
{
	m_RawCopy = false;
	m_LastError = res->m_LastError;
	m_Context = res->m_Context;
	m_ContextView = ctx;
	m_RecommendedView = res->m_RecommendedView;
	m_HidingItems = res->m_HidingItems;
	m_HasCategories = res->m_HasCategories;
	m_QueryTime = res->m_QueryTime;
	m_Items = static_cast<LFItemDescriptor**>(_aligned_malloc(res->m_ItemCount*sizeof(LFItemDescriptor*), LFSR_MemoryAlignment));
	m_FileCount = res->m_FileCount;
	m_FileSize = res->m_FileSize;
	strcpy_s(m_StoreID, LFKeySize, res->m_StoreID);
	m_Allocated = res->m_ItemCount;

	if (m_Items)
	{
		memcpy(m_Items, res->m_Items, res->m_ItemCount*sizeof(LFItemDescriptor*));
		m_ItemCount = res->m_ItemCount;
		for (unsigned int a=0; a<res->m_ItemCount; a++)
			m_Items[a]->RefCount++;
	}
	else
	{
		m_LastError = LFMemoryError;
		m_ItemCount = 0;
		m_Allocated =0;
	}
}

LFSearchResult::~LFSearchResult()
{
	if (m_Items)
	{
		for (unsigned int a=0; a<m_ItemCount; a++)
			LFFreeItemDescriptor(m_Items[a]);
		_aligned_free(m_Items);
	}
}

bool LFSearchResult::AddItemDescriptor(LFItemDescriptor* i)
{
	assert(i);

	if (!m_Items)
	{
		m_Items = static_cast<LFItemDescriptor**>(_aligned_malloc(LFSR_FirstAlloc*sizeof(LFItemDescriptor*), LFSR_MemoryAlignment));
		if (!m_Items)
		{
			m_LastError = LFMemoryError;
			return false;
		}
		m_Allocated = LFSR_FirstAlloc;
	}

	if (m_ItemCount==m_Allocated)
	{
		m_Items = static_cast<LFItemDescriptor**>(_aligned_realloc(m_Items, (m_Allocated+LFSR_SubsequentAlloc)*sizeof(LFItemDescriptor*), LFSR_MemoryAlignment));
		if (!m_Items)
		{
			m_LastError = LFMemoryError;
			return false;
		}
		m_Allocated += LFSR_SubsequentAlloc;
	}

	if (m_RawCopy)
		i->Position = m_ItemCount;
	m_Items[m_ItemCount++] = i;

	if ((i->Type & LFTypeFile)==LFTypeFile)
	{
		m_FileCount++;
		m_FileSize += i->CoreAttributes.FileSize;
	}

	return true;
}

bool LFSearchResult::AddStoreDescriptor(LFStoreDescriptor* s, LFFilter* f)
{
	assert(s);

	LFFilter* nf = LFAllocFilter();
	nf->Mode = LFFilterModeStoreHome;
	if (f)
		nf->Options = f->Options;
	strcpy_s(nf->StoreID, LFKeySize, s->StoreID);
	wcscpy_s(nf->Name, 256, s->StoreName);

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
		d->IconID = (s->StoreMode==LFStoreModeInternal ? IDI_STORE_Empty : IDI_STORE_Bag);
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
	{
		d->Type |= LFTypeGhosted | LFTypeNotMounted;
	}
	else
		// TODO
		if ((s->IndexVersion<CurIdxVersion) /*|| (s->MaintenanceTime<)*/)
			d->Type |= LFTypeRequiresMaintenance;

	d->CategoryID = s->StoreMode;
	d->Type |= LFTypeStore;
	SetAttribute(d, LFAttrFileName, s->StoreName);
	SetAttribute(d, LFAttrComment, s->Comment);
	SetAttribute(d, LFAttrStoreID, s->StoreID);
	SetAttribute(d, LFAttrFileID, s->StoreID);
	SetAttribute(d, LFAttrCreationTime, &s->CreationTime);
	SetAttribute(d, LFAttrFileTime, &s->FileTime);
	d->NextFilter = nf;

	bool res = AddItemDescriptor(d);
	if (!res)
		LFFreeItemDescriptor(d);

	return res;
}

void LFSearchResult::AddDrives(LFFilter* filter)
{
	DWORD DrivesOnSystem = LFGetLogicalDrives(LFGLD_External);

	for (char cDrive='A'; cDrive<='Z'; cDrive++, DrivesOnSystem>>=1)
	{
		if (!(DrivesOnSystem & 1))
			continue;

		wchar_t szDriveRoot[] = L" :\\";
		szDriveRoot[0] = cDrive;
		SHFILEINFO sfi;
		if (SHGetFileInfo(szDriveRoot, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_ATTRIBUTES))
		{
			if ((!sfi.dwAttributes) && (filter->HideEmptyDrives) && (!filter->UnhideAll))
			{
				m_HidingItems = true;
				continue;
			}

			LFItemDescriptor* d = LFAllocItemDescriptor();
			d->Type = LFTypeDrive;
			if (sfi.dwAttributes)
			{
				d->IconID = IDI_DRV_Default;
			}
			else
			{
				d->IconID = IDI_DRV_Empty;
				d->Type |= LFTypeGhosted | LFTypeNotMounted;
			}
			d->CategoryID = LFCategoryDrives;
			SetAttribute(d, LFAttrFileName, sfi.szDisplayName);
			char key[] = " :";
			key[0] = cDrive;
			SetAttribute(d, LFAttrFileID, key);
			SetAttribute(d, LFAttrDescription, sfi.szTypeName);

			if (!AddItemDescriptor(d))
				LFFreeItemDescriptor(d);
		}
	}
}

void LFSearchResult::AddBacklink(char* StoreID, LFFilter* f)
{
	wchar_t BacklinkName[256];
	LoadString(LFCoreModuleHandle, IDS_BacklinkName, BacklinkName, 256);
	wchar_t BacklinkComment[256];
	LoadString(LFCoreModuleHandle, IDS_BacklinkComment, BacklinkComment, 256);

	LFItemDescriptor* d = AllocFolderDescriptor(BacklinkName, BacklinkComment, NULL, StoreID, "BACK", NULL, IDI_FLD_Back, LFCategoryStore, f);
	if (!AddItemDescriptor(d))
		delete d;
}

void LFSearchResult::RemoveItemDescriptor(unsigned int idx)
{
	assert(idx<m_ItemCount);

	if ((m_Items[idx]->Type & LFTypeMask)==LFTypeFile)
	{
		m_FileCount--;
		m_FileSize -= m_Items[idx]->CoreAttributes.FileSize;
	}

	LFFreeItemDescriptor(m_Items[idx]);

	if (idx<--m_ItemCount)
	{
		m_Items[idx] = m_Items[m_ItemCount];
		if (m_RawCopy)
			m_Items[idx]->Position = idx;
	}
}

void LFSearchResult::RemoveFlaggedItemDescriptors()
{
	unsigned int idx = 0;
	
	while (idx<m_ItemCount)
	{
		if (m_Items[idx]->DeleteFlag)
		{
			RemoveItemDescriptor(idx);
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

int LFSearchResult::Compare(int eins, int zwei, unsigned int attr, bool descending, bool categories)
{
	LFItemDescriptor* d1 = m_Items[eins];
	LFItemDescriptor* d2 = m_Items[zwei];

	// Dateien mit Symbol IDI_FLD_Back immer nach vorne
	if ((d1->IconID==IDI_FLD_Back) && (d2->IconID!=IDI_FLD_Back))
		return -1;
	if ((d1->IconID!=IDI_FLD_Back) && (d2->IconID==IDI_FLD_Back))
		return 1;

	// Wenn zwei Laufwerke anhand des Namens verglichen werden sollen, Laufwerksbuchstaben nehmen
	unsigned int Sort = attr;
	unsigned int SortSecond = LFAttrFileName;
	if (((d1->Type & LFTypeMask)==LFTypeDrive) && ((d2->Type & LFTypeMask)==LFTypeDrive) && (categories))
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

	if ((d1null) && (!d2null))
		return 1;
	if ((!d1null) && (d2null))
		return -1;

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
			cmp = _wcsicmp((wchar_t*)d1->AttributeValues[Sort], (wchar_t*)d2->AttributeValues[Sort]);
			break;
		case LFTypeAnsiString:
			cmp = _stricmp((char*)d1->AttributeValues[Sort], (char*)d2->AttributeValues[Sort]);
			break;
		case LFTypeFourCC:
		case LFTypeUINT:
		case LFTypeDuration:
			eins32 = *(unsigned int*)d1->AttributeValues[Sort];
			zwei32 = *(unsigned int*)d2->AttributeValues[Sort];
			if (eins32<zwei32)
			{
				cmp = -1;
			}
			else
				if (eins32>zwei32)
					cmp = 1;
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
					cmp = 1;
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
					cmp = 1;
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
					cmp = 1;
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
					cmp = 1;
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
		default:
			assert(FALSE);
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

void LFSearchResult::Heap(int wurzel, int anz, unsigned int attr, bool descending, bool categories)
{
	while (wurzel<=anz/2-1)
	{
		int idx = (wurzel+1)*2-1;
		if (idx+1<anz)
			if (Compare(idx, idx+1, attr, descending, categories)<0)
				idx++;

		if (Compare(wurzel, idx, attr, descending, categories)<0)
		{
			std::swap(m_Items[wurzel], m_Items[idx]);
			wurzel = idx;
		}
		else
		{
			break;
		}
	}
}

void LFSearchResult::Sort(unsigned int attr, bool descending, bool categories)
{
	if (m_ItemCount>1)
	{
		for (int a=m_ItemCount/2-1; a>=0; a--)
			Heap(a, m_ItemCount, attr, descending, categories);
		for (int a=m_ItemCount-1; a>0; )
		{
			std::swap(m_Items[0], m_Items[a]);
			Heap(0, a--, attr, descending, categories);
		}
	}
}

unsigned int LFSearchResult::Aggregate(unsigned int write, unsigned int read1, unsigned int read2, void* c, unsigned int attr, unsigned int icon, bool groupone, LFFilter* f)
{
	if (((read2==read1+1) && ((!groupone) || ((m_Items[read1]->Type & LFTypeMask)==LFTypeVirtual))) || (IsNullValue(attr, m_Items[read1]->AttributeValues[attr])))
	{
		for (unsigned int a=read1; a<read2; a++)
			m_Items[write++] = m_Items[a];

		return read2-read1;
	}
	else
	{
		LFItemDescriptor* folder = ((CCategorizer*)c)->GetFolder(m_Items[read1], f);
		folder->IconID = icon;
		if (!m_RawCopy)
		{
			folder->FirstAggregate = read1;
			folder->LastAggregate = read2-1;
		}

		wchar_t Mask[256];
		LoadString(LFCoreModuleHandle, (read2==read1+1) ? IDS_HintSingular : IDS_HintPlural, Mask, 256);
		wchar_t Hint[256];
		swprintf_s(Hint, 256, Mask, read2-read1);
		SetAttribute(folder, LFAttrDescription, &Hint);

		__int64 size = 0;
		for (unsigned int a=read1; a<read2; a++)
		{
			size += m_Items[a]->CoreAttributes.FileSize;
			LFFreeItemDescriptor(m_Items[a]);
		}

		SetAttribute(folder, LFAttrFileSize, &size);
		m_Items[write] = folder;

		return 1;
	}
}

void LFSearchResult::Group(unsigned int attr, unsigned int icon, bool groupone, LFFilter* f)
{
	assert(f);

	if (f->Options.IsSubfolder)
		return;

	// Choose categorizer
	CCategorizer* c = NULL;

	switch (attr)
	{
	case LFAttrFileName:
		c = new NameCategorizer(attr);
		break;
	case LFAttrFileSize:
		c = new SizeCategorizer(attr);
		break;
	case LFAttrLocationIATA:
		c = new IATACategorizer(attr);
		break;
	default:
		switch (AttrTypes[attr])
		{
		case LFTypeUnicodeString:
			c = new UnicodeCategorizer(attr);
			break;
		case LFTypeAnsiString:
			c = new AnsiCategorizer(attr);
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
			WritePtr += Aggregate(WritePtr, ReadPtr1, ReadPtr2, c, attr, icon, groupone, f);
			ReadPtr1 = ReadPtr2;
		}

		ReadPtr2++;
	}

	WritePtr += Aggregate(WritePtr, ReadPtr1, m_ItemCount, c, attr, icon, groupone, f);
	m_ItemCount = f->Result.ItemCount = WritePtr;

	delete c;
}

void LFSearchResult::SetContext(LFFilter* f)
{
	switch (f->Mode)
	{
	case LFFilterModeStores:
		m_Context = LFContextStores;
		break;
	case LFFilterModeStoreHome:
		m_Context = LFContextStoreHome;
		break;
	default:
		if (f->Options.IsSubfolder)
		{
			m_Context = LFContextSubfolderDefault;
			if (f->ConditionList)
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
		else
			switch (f->DomainID)
			{
			case LFDomainTrash:
				m_Context = LFContextTrash;
				break;
			case LFDomainUnknown:
				m_Context = LFContextHousekeeping;
				break;
			default:
				m_Context = LFContextDefault;
				break;
			}
	}
}
