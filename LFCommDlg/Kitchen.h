
// Sortieren eines Suchergebnisses

int compare(const LFSearchResult* res, unsigned int eins, unsigned int zwei)
{
	LFItemDescriptor* d1 = res->m_Items[eins];
	LFItemDescriptor* d2 = res->m_Items[zwei];

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
			std::swap(res->m_Items[wurzel], res->m_Items[idx]);
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
	if (res->m_ItemCount>1)
	{
		for (int a=res->m_ItemCount/2-1; a>=0; a--)
			Heap(res, a, res->m_ItemCount);
		for (int a=res->m_ItemCount-1; a>0; )
		{
			std::swap(res->m_Items[0], res->m_Items[a]);
			Heap(res, 0, a--);
		}
	}
}
