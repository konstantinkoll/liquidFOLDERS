
// Sortieren eines Suchergebnisses

int compare(const LFSearchResult* res, unsigned int eins, unsigned int zwei)
{
	LFItemDescriptor* d1 = res->m_Files[eins];
	LFItemDescriptor* d2 = res->m_Files[zwei];

	int cmp = _wcsicmp((wchar_t*)d1->AttributeValues[LFAttrFileName], (wchar_t*)d2->AttributeValues[LFAttrFileName]);
	if (cmp==0)
		cmp = strcmp((char*)d1->AttributeValues[LFAttrStoreID], (char*)d2->AttributeValues[LFAttrStoreID]);
	if (cmp==0)
		cmp = strcmp((char*)d1->AttributeValues[LFAttrFileID], (char*)d2->AttributeValues[LFAttrFileID]);

	// Wenn die Dateien noch immer gleich sind, ist irgendwas sehr kaputt
	ASSERT(cmp!=0);

	return cmp;
}

void Heap(const LFSearchResult* res, int wurzel, int anz)
{
	while (wurzel<=anz/2-1)
	{
		int idx = (wurzel+1)*2-1;
		if (idx+1<anz)
			if (compare(res, idx, idx+1)<0)
				idx++;
		if (compare(res, wurzel, idx)<0)
		{
			std::swap(res->m_Files[wurzel], res->m_Files[idx]);
			wurzel = idx;
		}
		else
		{
			break;
		}
	}
}

void SortSearchResult(LFSearchResult* res)
{
	if (res->m_Count>1)
	{
		for (int a=res->m_Count/2-1; a>=0; a--)
			Heap(res, a, res->m_Count);
		for (int a=res->m_Count-1; a>0; )
		{
			std::swap(res->m_Files[0], res->m_Files[a]);
			Heap(res, 0, a--);
		}
	}
}

void RemoveNoninternalStores(LFSearchResult* res)
{
	UINT idx = 0;
	while (idx<res->m_Count)
	{
		if (((res->m_Files[idx]->Type & LFTypeMask)==LFTypeStore) && (res->m_Files[idx]->CategoryID!=LFCategoryInternalStores))
		{
			LFRemoveItemDescriptor(res, idx);
		}
		else
		{
			idx++;
		}
	}
}
