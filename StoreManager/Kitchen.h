
// Sortieren eines Suchergebnisses

int compare(const LFSearchResult* res, const LFViewParameters* vp, UINT eins, UINT zwei)
{
	ASSERT(theApp.m_Attributes[LFAttrFileName]->Type==LFTypeUnicodeString);
	ASSERT(theApp.m_Attributes[LFAttrStoreID]->Type==LFTypeAnsiString);
	ASSERT(theApp.m_Attributes[LFAttrFileID]->Type==LFTypeAnsiString);

	LFItemDescriptor* d1 = res->m_Files[eins];
	LFItemDescriptor* d2 = res->m_Files[zwei];

	// Dateien mit Symbol IDI_FLD_Back immer nach vorne
	if ((d1->IconID==IDI_FLD_Back) && (d2->IconID!=IDI_FLD_Back))
		return -1;
	if ((d1->IconID!=IDI_FLD_Back) && (d2->IconID==IDI_FLD_Back))
		return 1;

	// Wenn zwei Laufwerke anhand des Namens verglichen werden sollen, Laufwerksbuchstaben nehmen
	UINT Sort = vp->SortBy;
	if (((d1->Type & LFTypeMask)==LFTypeDrive) && ((d2->Type & LFTypeMask)==LFTypeDrive) && (Sort==LFAttrFileName) && (vp->ShowCategories) && (vp->Mode<=LFViewTiles))
		Sort = LFAttrFileID;

	// Dateien mit NULL-Werten oder leeren Strings im gewünschten Attribut hinten einsortieren
	BOOL d1null = (d1->AttributeValues[Sort]==NULL);
	if (!d1null)
		switch (theApp.m_Attributes[Sort]->Type)
		{
		case LFTypeUnicodeString:
			d1null = (*(wchar_t*)d1->AttributeValues[Sort]==0);
			break;
		case LFTypeAnsiString:
			d1null = (*(char*)d1->AttributeValues[Sort]==0);
			break;
		case LFTypeFraction:
			d1null = ((LFFraction*)d1->AttributeValues[Sort])->Denum == 0;
			break;
		case LFTypeGeoCoordinates:
			d1null = (((LFGeoCoordinates*)d1->AttributeValues[Sort])->Latitude==0) &&
				(((LFGeoCoordinates*)d1->AttributeValues[Sort])->Longitude==0);
			break;
		case LFTypeTime:
			d1null = (((FILETIME*)d1->AttributeValues[Sort])->dwHighDateTime==0) &&
				(((FILETIME*)d1->AttributeValues[Sort])->dwLowDateTime==0);
			break;
		case LFTypeFourCC:
		case LFTypeDuration:
			d1null = ((UINT*)d1->AttributeValues[Sort]) == 0;
			break;
		}

	BOOL d2null = (d2->AttributeValues[Sort]==NULL);
	if (!d2null)
		switch (theApp.m_Attributes[Sort]->Type)
		{
		case LFTypeUnicodeString:
			d2null = (*(wchar_t*)d2->AttributeValues[Sort]==0);
			break;
		case LFTypeAnsiString:
			d2null = (*(char*)d2->AttributeValues[Sort]==0);
			break;
		case LFTypeFraction:
			d2null = ((LFFraction*)d2->AttributeValues[Sort])->Denum == 0;
			break;
		case LFTypeGeoCoordinates:
			d2null = (((LFGeoCoordinates*)d2->AttributeValues[Sort])->Latitude==0) &&
				(((LFGeoCoordinates*)d2->AttributeValues[Sort])->Longitude==0);
			break;
		case LFTypeTime:
			d2null = (((FILETIME*)d2->AttributeValues[Sort])->dwHighDateTime==0) &&
				(((FILETIME*)d2->AttributeValues[Sort])->dwLowDateTime==0);
			break;
		case LFTypeFourCC:
		case LFTypeDuration:
			d2null = ((UINT*)d2->AttributeValues[Sort]) == 0;
		}

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
	const double ROUNDOFF = 0.001;

	if ((!d1null) && (!d2null))
	{
		switch (theApp.m_Attributes[Sort]->Type)
		{
		case LFTypeUnicodeString:
			cmp = _wcsicmp((wchar_t*)d1->AttributeValues[Sort], (wchar_t*)d2->AttributeValues[Sort]);
			break;
		case LFTypeAnsiString:
			cmp = _stricmp((char*)d1->AttributeValues[Sort], (char*)d2->AttributeValues[Sort]);
			break;
		case LFTypeFourCC:
		case LFTypeRating:
		case LFTypeUINT:
		case LFTypeDuration:
			eins32 = *(UINT*)d1->AttributeValues[Sort];
			zwei32 = *(UINT*)d2->AttributeValues[Sort];
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
			if (einsCoord.Latitude<zweiCoord.Latitude-ROUNDOFF)
			{
				cmp = -1;
			}
			else
				if (einsCoord.Latitude>zweiCoord.Latitude+ROUNDOFF)
				{
					cmp = 1;
				}
				else
					if (einsCoord.Longitude<zweiCoord.Longitude-ROUNDOFF)
					{
						cmp = -1;
					}
					else
						if (einsCoord.Longitude>zweiCoord.Longitude+ROUNDOFF)
						{
							cmp = 1;
						}
			break;
		case LFTypeTime:
			if (((FILETIME*)d1->AttributeValues[Sort])->dwHighDateTime<((FILETIME*)d2->AttributeValues[Sort])->dwHighDateTime)
			{
				cmp = -1;
			}
			else
				if (((FILETIME*)d1->AttributeValues[Sort])->dwHighDateTime>((FILETIME*)d2->AttributeValues[Sort])->dwHighDateTime)
				{
					cmp = 1;
				}
				else
					if (((FILETIME*)d1->AttributeValues[Sort])->dwLowDateTime<((FILETIME*)d2->AttributeValues[Sort])->dwLowDateTime)
					{
						cmp = -1;
					}
					else
						if (((FILETIME*)d1->AttributeValues[Sort])->dwLowDateTime>((FILETIME*)d2->AttributeValues[Sort])->dwLowDateTime)
						{
							cmp = 1;
						}
			break;
		default:
			assert(false);
		}

		// Ggf. Reihenfolge umkehren
		if (vp->Descending)
			cmp = -cmp;
	}

	// Dateien gleich bzgl. Attribut? Dann nach Name und notfalls FileID vergleichen für stabiles Ergebnis
	if (cmp==0)
	{
		if (Sort!=LFAttrFileName)
			cmp = _wcsicmp((wchar_t*)d1->AttributeValues[LFAttrFileName], (wchar_t*)d2->AttributeValues[LFAttrFileName]);
		if ((cmp==0) && (Sort!=LFAttrStoreID))
			cmp = strcmp((char*)d1->AttributeValues[LFAttrStoreID], (char*)d2->AttributeValues[LFAttrStoreID]);
		if ((cmp==0) && (Sort!=LFAttrFileID))
			cmp = strcmp((char*)d1->AttributeValues[LFAttrFileID], (char*)d2->AttributeValues[LFAttrFileID]);
	}

	// Wenn die Dateien noch immer gleich sind, ist irgendwas sehr kaputt
	ASSERT(cmp!=0);

	return cmp;
}

void Heap(const LFSearchResult* res, const LFViewParameters* vp, int wurzel, int anz)
{
	while (wurzel<=anz/2-1)
	{
		int idx = (wurzel+1)*2-1;
		if (idx+1<anz)
			if (compare(res, vp, idx, idx+1)<0)
				idx++;
		if (compare(res, vp, wurzel, idx)<0)
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

void SortSearchResult(LFSearchResult* res, LFViewParameters* vp)
{
	if ((res->m_Count>1) && (theApp.m_Attributes[vp->SortBy]->Sortable))
	{
		for (int a=res->m_Count/2-1; a>=0; a--)
			Heap(res, vp, a, res->m_Count);
		for (int a=res->m_Count-1; a>0; )
		{
			std::swap(res->m_Files[0], res->m_Files[a]);
			Heap(res, vp, 0, a--);
		}
	}
}


// Gruppieren gleicher Attributwerte

void GroupSearchResult(LFSearchResult* res, LFViewParameters* vp)
{
	// Nur gruppieren wenn gewünscht oder die Ansicht Gruppieren voraussetzt
	if ((!vp->AutoDirs) && (vp->Mode<=LFViewTiles))
		return;

	// Nur gruppieren wenn der Kontext das Gruppieren erlaubt
	if (!theApp.m_Contexts[res->m_Context]->AllowGroups)
		return;
}
