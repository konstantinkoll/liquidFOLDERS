
// CListView.cpp: Implementierung der Klasse CListView
//

#include "stdafx.h"
#include "CListView.h"
#include "ChooseDetailsDlg.h"
#include "FooterGraph.h"
#include "StoreManager.h"


// CListView
//

#define GetItemData(idx)                    ((GridItemData*)(m_ItemData+(idx)*m_DataSize))
#define PADDING                             2
#define DrawLabel(dc, rect, i, format)      dc.DrawText(GetLabel(i), rect, DT_END_ELLIPSIS | format);
#define SwitchColor(dc, d)                  if ((Themed) && (!(i->CoreAttributes.Flags & LFFlagMissing)) && (!(i->Type & LFTypeRequiresMaintenance)) && ((hThemeList) || (!d->Hdr.Selected))) dc.SetTextColor(0x808080);
#define PrepareBlend()                      INT w = min(rect.Width(), RatingBitmapWidth); \
                                            INT h = min(rect.Height(), RatingBitmapHeight); \
                                            BLENDFUNCTION BF = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };
#define Blend(dc, rect, level, bitmaps)     { HDC hdcMem = CreateCompatibleDC(dc); \
                                            HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, bitmaps[level>LFMaxRating ? 0 : level]); \
                                            AlphaBlend(dc, rect.left, rect.top+1, w, h, hdcMem, 0, 0, w, h, BF); \
                                            SelectObject(hdcMem, hbmOld); \
                                            DeleteDC(hdcMem); }
#define RIGHTCOLUMN                         215
#define MAXAUTOWIDTH                        400
#define MINWIDTH                            32

CListView::CListView(UINT DataSize)
	: CGridView(DataSize)
{
	m_Icons[0] = m_Icons[1] = NULL;
	m_HeaderItemClicked = m_HeaderItemSort = -1;
	m_IgnoreHeaderItemChange = m_ShowLegend = FALSE;

	WCHAR tmpStr[256];
	SYSTEMTIME st;
	ZeroMemory(&st, sizeof(st));

	for (WORD a=6; a<24; a++)
	{
		st.wHour = a;
		GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, tmpStr, 256);
		AddItemCategory(tmpStr, L"");
	}
}

void CListView::SetViewOptions(BOOL Force)
{
	if ((p_ViewParameters->Mode!=m_ViewParameters.Mode) || (Force))
	{
		INT cx;
		INT cy;

		switch (p_ViewParameters->Mode)
		{
		case LFViewLargeIcons:
		case LFViewContent:
		case LFViewPreview:
			m_Icons[0] = &theApp.m_CoreImageListJumbo;
			m_Icons[1] = (theApp.OSVersion<OS_Vista) ? &theApp.m_SystemImageListExtraLarge : &theApp.m_SystemImageListJumbo;
			cx = cy = 128;
			break;
		case LFViewSmallIcons:
			m_Icons[0] = &theApp.m_CoreImageListLarge;
			m_Icons[1] = &theApp.m_SystemImageListLarge;
			cx = cy = 32;
			break;
		case LFViewTiles:
		case LFViewStrips:
			m_Icons[0] = &theApp.m_CoreImageListExtraLarge;
			m_Icons[1] = &theApp.m_SystemImageListExtraLarge;
			cx = cy = 48;
			break;
		default:
			m_Icons[0] = &theApp.m_CoreImageListSmall;
			m_Icons[1] = &theApp.m_SystemImageListSmall;
			cx = cy = 16;
		}

		ImageList_GetIconSize(*m_Icons[0], &cx, &cy);
		m_IconSize[0].cx = cx;
		m_IconSize[0].cy = cy;

		ImageList_GetIconSize(*m_Icons[1], &cx, &cy);
		m_IconSize[1].cx = min(cx, 128);
		m_IconSize[1].cy = min(cy, 128);
	}

	if ((p_ViewParameters->Mode==LFViewDetails) && (p_Result))
		for (UINT a=0; a<LFAttributeCount; a++)
			if (p_ViewParameters->ColumnWidth[a]!=m_ViewParameters.ColumnWidth[a])
			{
				m_ViewParameters = *p_ViewParameters;
				UpdateFooter();
				break;
			}

	AdjustHeader((p_ViewParameters->Mode==LFViewDetails) && (p_Result));
}

void CListView::SetSearchResult(LFSearchResult* Result, FVPersistentData* /*Data*/)
{
	m_ShowLegend = FALSE;

	if (Result)
	{
		m_HasCategories = Result->m_HasCategories;

		for (UINT a=0; a<Result->m_ItemCount; a++)
		{
			GridItemData* d = GetItemData(a);
			d->Hdr.Valid = TRUE;

			LFItemDescriptor* i = Result->m_Items[a];
			m_ShowLegend |= (i->Type & LFTypeRequiresMaintenance) | (i->CoreAttributes.Flags & LFFlagMissing);

			if ((i->Type & LFTypeMask)==LFTypeFile)
				if (!Result->m_HasCategories)
					if (Result->m_Context==LFContextSubfolderDay)
					{
						SYSTEMTIME stUTC;
						SYSTEMTIME stLocal;
						FileTimeToSystemTime((FILETIME*)i->AttributeValues[Result->m_GroupAttribute], &stUTC);
						SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

						i->CategoryID = stLocal.wHour<6 ? LFItemCategoryNight : LFItemCategoryCount+stLocal.wHour-6;
						m_HasCategories = TRUE;
					}
		}

		if (m_HasCategories!=(Result->m_HasCategories==true))
			SortCategories(Result);
	}

	AdjustHeader(m_ViewParameters.Mode==LFViewDetails);
}

void CListView::AdjustHeader(BOOL bShow)
{
	if (bShow)
	{
		m_wndHeader.SetRedraw(FALSE);
		m_IgnoreHeaderItemChange = TRUE;

		VERIFY(m_wndHeader.SetOrderArray(LFAttributeCount, p_ViewParameters->ColumnOrder));

		// Width
		for (UINT a=0; a<LFAttributeCount; a++)
		{
			HDITEM HdItem;
			HdItem.mask = HDI_WIDTH;
			HdItem.cxy = p_ViewParameters->ColumnWidth[a];

			if (HdItem.cxy)
				if (theApp.m_Attributes[a]->Type==LFTypeRating)
				{
					HdItem.cxy = p_ViewParameters->ColumnWidth[a] = RatingBitmapWidth+4*PADDING;
				}
				else
					if (HdItem.cxy<MINWIDTH)
						p_ViewParameters->ColumnWidth[a] = HdItem.cxy = MINWIDTH;

			m_wndHeader.SetItem(a, &HdItem);
		}

		// Sort indicator
		HDITEM hdi;
		hdi.mask = HDI_FORMAT;

		if ((m_HeaderItemSort!=(INT)p_ViewParameters->SortBy) && (m_HeaderItemSort!=-1))
		{
			hdi.fmt = 0;
			m_wndHeader.SetItem(m_HeaderItemSort, &hdi);
		}

		hdi.fmt = p_ViewParameters->Descending ? HDF_SORTDOWN : HDF_SORTUP;
		m_wndHeader.SetItem(p_ViewParameters->SortBy, &hdi);

		m_HeaderItemSort = p_ViewParameters->SortBy;

		m_wndHeader.ModifyStyle(HDS_HIDDEN, 0);
		m_wndHeader.SetRedraw(TRUE);
		m_wndHeader.Invalidate();

		m_IgnoreHeaderItemChange = FALSE;
	}
	else
	{
		m_wndHeader.ModifyStyle(0, HDS_HIDDEN);
	}
}

CBitmap* CListView::RenderLegend()
{
#ifndef DEBUG
	if (!m_ShowLegend)
		return NULL;
#endif

	CString colBlue;
	CString colRed;
	ENSURE(colBlue.LoadString(IDS_LEGEND_BLUE));
	ENSURE(colRed.LoadString(IDS_LEGEND_RED));

	ENSURE(m_FooterCaption.LoadString(IDS_LEGEND));
	BOOL Themed = IsCtrlThemed();

	CDC* pDC = GetWindowDC();
	CDC dcDraw;

	HGDIOBJ oldFont = pDC->SelectStockObject(DEFAULT_GUI_FONT);
	INT cx = max(pDC->GetTextExtent(colBlue).cx, pDC->GetTextExtent(colRed).cx)+m_FontHeight[2]+2*GraphSpacer-1;
	INT cy = 2*(m_FontHeight[2]+GraphSpacer)+GraphSpacer;

	CBitmap* pBmp = CreateFooterBitmap(pDC, cx, cy, dcDraw, Themed);
	CRect rect(0, GraphSpacer, cx, cy);

	DrawLegend(dcDraw, rect, 0xFF0000, colBlue, Themed);
	DrawLegend(dcDraw, rect, 0x0000FF, colRed, Themed);

	pDC->SelectObject(oldFont);
	ReleaseDC(pDC);

	return pBmp;
}

CBitmap* CListView::RenderStatistics()
{
	if (!p_Result || m_Nothing || !theApp.m_ShowStatistics)
		return NULL;

#define DomainCount 10
	const UINT Domains[DomainCount] = { LFDomainAudio, LFDomainPictures, LFDomainVideos, LFDomainDocuments,
		LFDomainMessages, LFDomainPresentations, LFDomainSpreadsheets, 0, LFDomainTrash, LFDomainUnknown };
	INT64 Counts[DomainCount] = { 0 };
	INT64 Sizes[DomainCount] = { 0 };
	COLORREF Colors[DomainCount] = { 0xB03000, 0xFFB000, 0x202020, 0x00C0FF, 0x00FFB0, 0xFF4080, 0xC028FF, 0xD0D0D0, 0x0000FF, 0xD00000 };

	for (UINT a=0; a<p_Result->m_ItemCount; a++)
	{
		GridItemData* d = GetItemData(a);
		if (d->Hdr.Valid)
		{
			LFItemDescriptor* i = p_Result->m_Items[a];
			if ((i->Type & LFTypeMask)==LFTypeVirtual)
			{
				UINT DomainID;
				if (sscanf_s(i->CoreAttributes.FileID, "%d", &DomainID)==1)
					if ((DomainID>=LFFirstSoloDomain) && (DomainID!=LFDomainPhotos))
					{
						INT idx = DomainCount-3;
						for (UINT a=0; a<DomainCount; a++)
							if (Domains[a]==DomainID)
								idx = a;

						Counts[idx] += i->AggregateCount;
						Sizes[idx] += i->CoreAttributes.FileSize;
					}
			}
		}
	}

	ENSURE(m_FooterCaption.LoadString(IDS_STATISTICS));
	BOOL Themed = IsCtrlThemed();

	CDC* pDC = GetWindowDC();
	CDC dcDraw;

	INT cy = 2*(m_FontHeight[0]+m_FontHeight[1]+3*GraphSpacer)+2*GraphSpacer+DomainCount*(m_FontHeight[2]+GraphSpacer);
	CBitmap* pBmp = CreateFooterBitmap(pDC, 400, cy, dcDraw, Themed);
	INT cx = m_FooterSize.cx-6;

	CRect rect(0, 0, cx, cy);

	DrawGraphCaption(dcDraw, rect, IDS_STATISTICS_BYCOUNT);
	DrawBarChart(dcDraw, rect, Counts, Colors, DomainCount, m_FontHeight[1], Themed);

	DrawGraphCaption(dcDraw, rect, IDS_STATISTICS_BYSIZE);
	DrawBarChart(dcDraw, rect, Sizes, Colors, DomainCount, m_FontHeight[1], Themed);

	rect.top += 2*GraphSpacer;

	for (INT a=0; a<DomainCount; a++)
		if (Counts[a] || Sizes[a])
		{
			CString tmpStr;
			if (Domains[a])
			{
				tmpStr = theApp.m_Domains[Domains[a]]->Name;
			}
			else
			{
				ENSURE(tmpStr.LoadString(IDS_STATISTICS_OTHER));
			}

			DrawChartLegend(dcDraw, rect, Counts[a], Sizes[a], Colors[a], tmpStr, Themed);
		}

	ReleaseDC(pDC);

	return pBmp;
}

CBitmap* CListView::RenderFooter()
{
	return (m_Context==LFContextStoreHome) ? RenderStatistics() : RenderLegend();
}

void CListView::AdjustLayout()
{
	// Header
	CRect rect;
	GetWindowRect(&rect);

	WINDOWPOS wp;
	HDLAYOUT HdLayout;
	HdLayout.prc = &rect;
	HdLayout.pwpos = &wp;
	m_wndHeader.Layout(&HdLayout);

	wp.x = 13-PADDING;
	wp.y = 0;
	m_HeaderHeight = wp.cy + (wp.cy ? 4 : 0);

	// Items
	GVArrange gva = { 0, 0, 15-PADDING, 2, PADDING, 1, -1 };

	switch (m_ViewParameters.Mode)
	{
	case LFViewLargeIcons:
	case LFViewSmallIcons:
	case LFViewPreview:
		gva.cx = max(m_IconSize[0].cx, m_FontHeight[0]*10);
		gva.cy = m_IconSize[0].cy+m_FontHeight[0]*2+PADDING;
		gva.guttery = 1;
		ArrangeHorizontal(gva);
		break;
	case LFViewList:
		gva.cx = m_IconSize[0].cx+PADDING+GetMaxLabelWidth(240-m_IconSize[0].cx-PADDING);
		if (gva.cx<140)
			gva.cx = 140;
		gva.cy = max(m_IconSize[0].cy, m_FontHeight[0]);
		gva.gutterx = 6;
		ArrangeVertical(gva);
		break;
	case LFViewDetails:
		gva.cx = -2*gva.padding;
		for (UINT a=0; a<LFAttributeCount; a++)
			gva.cx += m_ViewParameters.ColumnWidth[a];
		gva.cy = max(m_IconSize[0].cy, m_FontHeight[0]);
		ArrangeHorizontal(gva, FALSE, TRUE);
		break;
	case LFViewTiles:
		gva.cx = 240;
		gva.cy = max(m_IconSize[0].cy, m_FontHeight[0]*3+max(m_FontHeight[0], 18));
		gva.gutterx = gva.guttery = 3;
		ArrangeHorizontal(gva, FALSE);
		break;
	case LFViewStrips:
		gva.cy = 2+max(m_IconSize[0].cy, max(m_FontHeight[0]*3+m_FontHeight[1], m_FontHeight[0]*2+max(m_FontHeight[0], 18)*2+1));
		ArrangeHorizontal(gva, FALSE, TRUE, TRUE);
		break;
	case LFViewContent:
		gva.cy = m_IconSize[0].cy+18+3;
		ArrangeHorizontal(gva, FALSE, TRUE, TRUE);
		break;
	}

	// Header
	m_wndHeader.SetWindowPos(NULL, wp.x-m_HScrollPos, wp.y, wp.cx+m_HScrollMax+GetSystemMetrics(SM_CXVSCROLL), m_HeaderHeight, wp.flags | SWP_NOZORDER | SWP_NOACTIVATE);
	m_wndHeader.Invalidate();
}

RECT CListView::GetLabelRect(INT idx)
{
	RECT rect = GetItemRect(idx);

	switch (m_ViewParameters.Mode)
	{
	case LFViewLargeIcons:
	case LFViewSmallIcons:
	case LFViewPreview:
		rect.top += m_IconSize[0].cy+2*PADDING;
		break;
	case LFViewDetails:
		rect.right = rect.left+m_ViewParameters.ColumnWidth[0]-3*PADDING;
	case LFViewList:
	case LFViewTiles:
	case LFViewStrips:
	case LFViewContent:
		rect.left += m_IconSize[0].cx+2*PADDING;
		break;
	}

	return rect;
}

void CListView::DrawItem(CDC& dc, LPRECT rectItem, INT idx, BOOL Themed)
{
	LFItemDescriptor* i = p_Result->m_Items[idx];
	GridItemData* d = GetItemData(idx);
	INT Rows[4];
	BOOL Right = FALSE;

	CRect rectClient;

	CRect rect(rectItem);
	rect.DeflateRect(PADDING, PADDING);

	CRect rectIcon(rect);
	CRect rectLabel(rect);
	CRect rectLeft(rect);
	CRect rectRight(rect);

	switch (m_ViewParameters.Mode)
	{
	case LFViewLargeIcons:
	case LFViewSmallIcons:
	case LFViewPreview:
		rectIcon.bottom = rectIcon.top+m_IconSize[0].cy;
		DrawIcon(dc, rectIcon, i);

		if (IsEditing() && (idx==m_EditLabel))
			break;

		rectLabel.top += m_IconSize[0].cy+PADDING;
		DrawLabel(dc, rectLabel, i, DT_CENTER | DT_WORDBREAK);
		break;
	case LFViewDetails:
		rectIcon.right = rectIcon.left+m_IconSize[0].cx;
		DrawIcon(dc, rectIcon, i);

		rectLabel.right = rectLabel.left+m_ViewParameters.ColumnWidth[0]-3*PADDING;
		rectLabel.left = rectIcon.right+PADDING;

		GetClientRect(&rectClient);

		for (UINT a=0; a<LFAttributeCount; a++)
		{
			UINT attr = m_ViewParameters.ColumnOrder[a];
			if (m_ViewParameters.ColumnWidth[attr])
			{
				if (attr)
				{
					rectLabel.left = rectLabel.right+3*PADDING;
					rectLabel.right = rectLabel.left+m_ViewParameters.ColumnWidth[attr]-3*PADDING;
				}
				if ((attr!=LFAttrFileName) || (!IsEditing()) || (idx!=m_EditLabel))
					if ((rectLabel.left<=rectClient.right) && (rectLabel.right>=rectClient.left))
						DrawColumn(dc, rectLabel, i, attr);
			}
		}
		break;
	case LFViewList:
		rectIcon.right = rectIcon.left+m_IconSize[0].cx;
		DrawIcon(dc, rectIcon, i);

		if (IsEditing() && (idx==m_EditLabel))
			break;

		rectLabel.left += m_IconSize[0].cx+PADDING;
		DrawLabel(dc, rectLabel, i, DT_LEFT | DT_SINGLELINE);
		break;
	case LFViewTiles:
		rectIcon.right = rectIcon.left+m_IconSize[0].cx;
		DrawIcon(dc, rectIcon, i);

		if (IsEditing() && (idx==m_EditLabel))
			break;

		rectLabel.left += m_IconSize[0].cx+m_FontHeight[0]/2;

		Rows[0] = LFAttrFileName;
		switch (i->Type & LFTypeMask)
		{
		case LFTypeStore:
		case LFTypeVolume:
			Rows[1] = LFAttrComments;
			Rows[2] = LFAttrDescription;
			Rows[3] = LFAttrCreationTime;
			break;
		case LFTypeFile:
			Rows[1] = LFAttrFileTime;
			Rows[2] = LFAttrFileSize;
			Rows[3] = LFAttrRating;
			break;
		case LFTypeVirtual:
			if (m_Context==LFContextStoreHome)
			{
				Rows[1] = LFAttrComments;
				Rows[2] = LFAttrDescription;
				Rows[3] = LFAttrFileSize;
			}
			else
			{
				Rows[1] = LFAttrDescription;
				Rows[2] = LFAttrFileSize;
				Rows[3] = -1;
			}
			break;
		}

		DrawTileRows(dc, rectLabel, i, d, Rows, Themed);
		break;
	case LFViewStrips:
		rectIcon.right = rectIcon.left+m_IconSize[0].cx;
		DrawIcon(dc, rectIcon, i);

		if (IsEditing() && (idx==m_EditLabel))
			break;

		rectLeft.left += m_IconSize[0].cx+m_FontHeight[0]/2;
		rectLeft.top++;
		Right = (rect.Width()>600) && (((i->Type & LFTypeMask)==LFTypeStore) || ((i->Type & LFTypeMask)==LFTypeFile));
		if (Right)
			rectLeft.right -= RIGHTCOLUMN+2*PADDING;

		DrawProperty(dc, rectLeft, i, d, LFAttrFileName, Themed);
		switch (i->Type & LFTypeMask)
		{
		case LFTypeStore:
		case LFTypeVolume:
			DrawProperty(dc, rectLeft, i, d, LFAttrComments, Themed);
			DrawProperty(dc, rectLeft, i, d, LFAttrDescription, Themed);
			break;
		case LFTypeFile:
			DrawProperty(dc, rectLeft, i, d, LFAttrComments, Themed);
			DrawProperty(dc, rectLeft, i, d, LFAttrTags, Themed);
			DrawProperty(dc, rectLeft, i, d, LFAttrFileFormat, Themed);
			break;
		case LFTypeVirtual:
			if (m_Context==LFContextStoreHome)
			{
				DrawProperty(dc, rectLeft, i, d, LFAttrComments, Themed);
				DrawProperty(dc, rectLeft, i, d, LFAttrDescription, Themed);
				DrawProperty(dc, rectLeft, i, d, LFAttrFileSize, Themed);
			}
			else
			{
				DrawProperty(dc, rectLeft, i, d, LFAttrDescription, Themed);
				DrawProperty(dc, rectLeft, i, d, LFAttrFileSize, Themed);
			}
			break;
		}

		if (Right)
		{
			rectRight.left = rectLeft.right+2*PADDING;
			rectRight.top += 1+m_FontHeight[1]-m_FontHeight[0];

			switch (i->Type & LFTypeMask)
			{
			case LFTypeStore:
				DrawProperty(dc, rectRight, i, d, LFAttrCreationTime, Themed);
				DrawProperty(dc, rectRight, i, d, LFAttrFileTime, Themed);
				break;
			case LFTypeFile:
				DrawProperty(dc, rectRight, i, d, LFAttrFileTime, Themed);
				DrawProperty(dc, rectRight, i, d, LFAttrFileSize, Themed);
				DrawProperty(dc, rectRight, i, d, LFAttrRating, Themed);
				DrawProperty(dc, rectRight, i, d, LFAttrPriority, Themed);
				break;
			}
		}

		break;
	case LFViewContent:
		if (i->AttributeValues[LFAttrRating])
		{
			UCHAR Rating = *((UCHAR*)i->AttributeValues[LFAttrRating]);
			if (((i->Type & LFTypeMask)==LFTypeFile) || (Rating))
			{
				rectIcon.left += (m_IconSize[0].cx-88)/2;
				rectIcon.bottom--;
				rectIcon.top = rectIcon.bottom-18;

				PrepareBlend();
				Blend(dc, rectIcon, Rating, theApp.m_RatingBitmaps);
			}
		}

		rectIcon.left = rectLeft.left;
		rectIcon.right = rectIcon.left+m_IconSize[0].cx;
		rectIcon.top = rectLeft.top+1;
		rectIcon.bottom = rectIcon.top+m_IconSize[0].cy;
		DrawIcon(dc, rectIcon, i);

		if (IsEditing() && (idx==m_EditLabel))
			break;

		rectLeft.left += m_IconSize[0].cx+m_FontHeight[0]/2;
		rectLeft.top++;

		DrawProperty(dc, rectLeft, i, d, LFAttrFileName, Themed);
		DrawProperty(dc, rectLeft, i, d, LFAttrComments, Themed, FALSE);
		DrawProperty(dc, rectLeft, i, d, LFAttrDescription, Themed, FALSE);
		DrawProperty(dc, rectLeft, i, d, LFAttrArtist, Themed, FALSE);
		DrawProperty(dc, rectLeft, i, d, LFAttrTitle, Themed, FALSE);
		DrawProperty(dc, rectLeft, i, d, LFAttrAlbum, Themed, FALSE);
		DrawProperty(dc, rectLeft, i, d, LFAttrRecordingTime, Themed, FALSE);
		DrawProperty(dc, rectLeft, i, d, LFAttrRoll, Themed, FALSE);
		DrawProperty(dc, rectLeft, i, d, LFAttrDuration, Themed, FALSE);
		DrawProperty(dc, rectLeft, i, d, LFAttrTags, Themed, FALSE);
		DrawProperty(dc, rectLeft, i, d, LFAttrPages, Themed, FALSE);
		DrawProperty(dc, rectLeft, i, d, LFAttrWidth, Themed, FALSE);
		DrawProperty(dc, rectLeft, i, d, LFAttrHeight, Themed, FALSE);
		DrawProperty(dc, rectLeft, i, d, LFAttrRecordingEquipment, Themed, FALSE);
		DrawProperty(dc, rectLeft, i, d, LFAttrBitrate, Themed, FALSE);
		DrawProperty(dc, rectLeft, i, d, LFAttrCreationTime, Themed, FALSE);
		DrawProperty(dc, rectLeft, i, d, LFAttrFileTime, Themed, FALSE);
		DrawProperty(dc, rectLeft, i, d, LFAttrFileSize, Themed, FALSE);

		break;
	}
}

void CListView::DrawIcon(CDC& dc, CRect& rect, LFItemDescriptor* i)
{
	INT SysIconIndex = -1;
	CHAR Path[4];

#define JUMBOICON (m_ViewParameters.Mode==LFViewLargeIcons) || (m_ViewParameters.Mode==LFViewContent) || (m_ViewParameters.Mode==LFViewPreview)

	switch (i->Type & LFTypeMask)
	{
	case LFTypeFile:
		if ((m_ViewParameters.Mode==LFViewContent) || (m_ViewParameters.Mode==LFViewPreview))
			if (theApp.m_ThumbnailCache.DrawJumboThumbnail(dc, rect, i))
				return;
		if (JUMBOICON)
		{
			theApp.m_FileFormats.DrawJumboIcon(dc, rect, i->CoreAttributes.FileFormat, i->Type & LFTypeGhosted);
			return;
		}
		SysIconIndex = theApp.m_FileFormats.GetSysIconIndex(i->CoreAttributes.FileFormat);
		break;
	case LFTypeVolume:
		strcpy_s(Path, 4, " :\\");
		Path[0] = i->CoreAttributes.FileID[0];
		if (JUMBOICON)
		{
			theApp.m_FileFormats.DrawJumboIcon(dc, rect, Path, i->Type & LFTypeGhosted);
			return;
		}
		SysIconIndex = theApp.m_FileFormats.GetSysIconIndex(Path);
		break;
	}

	const UINT List = (SysIconIndex>=0) ? 1 : 0;
	const INT IconID = (List==1) ? SysIconIndex : i->IconID-1;
	if (IconID>=0)
	{
		rect.OffsetRect((rect.Width()-m_IconSize[List].cx)/2, (rect.Height()-m_IconSize[List].cy)/2);
		m_Icons[List]->DrawEx(&dc, IconID, rect.TopLeft(), m_IconSize[List], CLR_NONE, 0xFFFFFF, (i->Type & LFTypeGhosted) ? ILD_BLEND50 : ILD_TRANSPARENT);
	}
}

void CListView::AttributeToString(LFItemDescriptor* i, UINT Attr, WCHAR* tmpStr, size_t cCount)
{
	switch (Attr)
	{
	case LFAttrFileName:
		wcsncpy_s(tmpStr, cCount, GetLabel(i), 255);
		break;
	case LFAttrFileFormat:
		wcscpy_s(tmpStr, cCount, theApp.m_FileFormats.GetTypeName(i->CoreAttributes.FileFormat));
		break;
	default:
		LFAttributeToString(i, Attr, tmpStr, cCount);
	}
}

void CListView::DrawTileRows(CDC& dc, CRect& rect, LFItemDescriptor* i, GridItemData* d, INT* Rows, BOOL Themed)
{
	WCHAR tmpStr[4][256];
	UINT Cnt = 0;
	UINT Height = 0;

	for (UINT a=0; a<4; a++)
	{
		tmpStr[a][0] = L'\0';

		if (Rows[a]!=-1)
			if (Rows[a]==LFAttrRating)
			{
				Cnt++;
				Height += 18;
			}
			else
			{
				AttributeToString(i, Rows[a], tmpStr[a], 256);
				if (tmpStr[a][0]!=L'\0')
				{
					Cnt++;
					Height += m_FontHeight[0];
				}
			}
	}

	rect.top += (rect.Height()-Height)/2;
	rect.bottom = rect.top+max(m_FontHeight[0], 18);

	for (UINT a=0; a<4; a++)
	{
		if (Rows[a]==LFAttrRating)
		{
			PrepareBlend();
			Blend(dc, rect, i->CoreAttributes.Rating, theApp.m_RatingBitmaps);
			rect.OffsetRect(0, 18);
		}
		else
			if (tmpStr[a][0]!=L'\0')
			{
				dc.DrawText(tmpStr[a], rect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS);
				rect.OffsetRect(0, m_FontHeight[0]);
			}

		if (Rows[a]==LFAttrFileName)
			SwitchColor(dc, d);
	}
}

void CListView::DrawColumn(CDC& dc, CRect& rect, LFItemDescriptor* i, UINT Attr)
{
	if (theApp.m_Attributes[Attr]->Type==LFTypeRating)
	{
		if (i->AttributeValues[Attr])
		{
			UCHAR Rating = *((UCHAR*)i->AttributeValues[Attr]);
			if (((i->Type & LFTypeMask)==LFTypeFile) || (Rating))
			{
				PrepareBlend();
				if (Attr==LFAttrRating)
				{
					Blend(dc, rect, Rating, theApp.m_RatingBitmaps);
				}
				else
				{
					Blend(dc, rect, Rating, theApp.m_PriorityBitmaps);
				}
			}
		}

		return;
	}

	WCHAR tmpStr[256];
	AttributeToString(i, Attr, tmpStr, 256);
	if (tmpStr[0]!=L'\0')
	{
		CRect rectText(rect);
		dc.DrawText(tmpStr, rectText, (theApp.m_Attributes[Attr]->FormatRight ? DT_RIGHT : DT_LEFT) | DT_SINGLELINE | DT_END_ELLIPSIS);
	}
}

void CListView::DrawProperty(CDC& dc, CRect& rect, LFItemDescriptor* i, GridItemData* d, UINT Attr, BOOL Themed, BOOL AlwaysNewRow)
{
	CFont* pOldFont;

	switch (Attr)
	{
	case LFAttrFileName:
		pOldFont = dc.SelectObject(&theApp.m_LargeFont);
		DrawLabel(dc, rect, i, DT_LEFT | DT_SINGLELINE);
		dc.SelectObject(pOldFont);

		rect.top += m_FontHeight[1];
		break;
	case LFAttrRating:
	case LFAttrPriority:
		{
			PrepareBlend();
			if (Attr==LFAttrRating)
			{
				Blend(dc, rect, i->CoreAttributes.Rating, theApp.m_RatingBitmaps);
			}
			else
			{
				Blend(dc, rect, i->CoreAttributes.Priority, theApp.m_PriorityBitmaps);
			}
		}

		rect.top += 18;
		break;
	default:
		WCHAR tmpStr[256];
		AttributeToString(i, Attr, tmpStr, 256);
		if (tmpStr[0]!=L'\0')
		{
			if (rect.top>rect.bottom-m_FontHeight[0])
				return;

			COLORREF oldColor = dc.GetTextColor();

			CRect rectText(rect);
			if ((Attr!=LFAttrComments) && (Attr!=LFAttrDescription))
			{
				CString tmpCaption(theApp.m_Attributes[Attr]->Name);
				tmpCaption += _T(": ");
				dc.DrawText(tmpCaption, rectText, DT_LEFT | DT_SINGLELINE);
				rectText.left += dc.GetTextExtent(tmpCaption).cx;
			}

			SwitchColor(dc, d);
			dc.DrawText(tmpStr, rectText, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
			dc.SetTextColor(oldColor);
		}
		else
		{
			if (!AlwaysNewRow)
				return;
		}

		rect.top += m_FontHeight[0];
		break;
	}
}

INT CListView::GetMaxLabelWidth(INT Max)
{
	INT Width = 0;

	if (p_Result)
	{
		CDC* dc = GetWindowDC();
		CFont* pOldFont = dc->SelectObject(&theApp.m_DefaultFont);

		for (INT a=0; a<(INT)p_Result->m_ItemCount; a++)
		{
			CString label = GetLabel(p_Result->m_Items[a]);
			INT cx = dc->GetTextExtent(label).cx;

			if (cx>Width)
			{
				Width = cx;

				if (Width>=Max)
				{
					Width = Max;
					break;
				}
			}
		}
	
		dc->SelectObject(pOldFont);
		ReleaseDC(dc);
	}

	return Width;
}

INT CListView::GetMaxColumnWidth(UINT Col, INT Max)
{
	INT Width = 0;

	if (p_Result)
	{
		CDC* dc = GetWindowDC();
		CFont* pOldFont = dc->SelectObject(&theApp.m_DefaultFont);

		for (INT a=0; a<(INT)p_Result->m_ItemCount; a++)
		{
			WCHAR tmpStr[256];
			LFAttributeToString(p_Result->m_Items[a], Col, tmpStr, 256);
			INT cx = dc->GetTextExtent(tmpStr, (INT)wcslen(tmpStr)).cx;

			if (cx>Width)
			{
				Width = cx;

				if (Width>=Max)
				{
					Width = Max;
					break;
				}
			}
		}
	
		dc->SelectObject(pOldFont);
		ReleaseDC(dc);
	}

	return Width;
}

void CListView::AutosizeColumn(UINT Col)
{
	m_ViewParameters.ColumnWidth[Col] = p_ViewParameters->ColumnWidth[Col] = 3*PADDING +
		((Col==LFAttrFileName) ? m_IconSize[0].cx+PADDING+GetMaxLabelWidth(MAXAUTOWIDTH) : (theApp.m_Attributes[Col]->Type==LFTypeRating) ? RatingBitmapWidth+PADDING : GetMaxColumnWidth(Col, MAXAUTOWIDTH));

	if (m_ViewParameters.ColumnWidth[Col]<MINWIDTH)
		m_ViewParameters.ColumnWidth[Col] = MINWIDTH;
}

void CListView::SortCategories(LFSearchResult* Result)
{
	ASSERT(Result);
	DynArray<LFItemDescriptor*> Buckets[LFItemCategoryCount+18];

	for (UINT a=0; a<Result->m_ItemCount; a++)
		Buckets[Result->m_Items[a]->CategoryID].AddItem(Result->m_Items[a]);

	UINT Ptr = 0;
	for (UINT a=0; a<LFItemCategoryCount+18; a++)
		for (UINT b=0; b<Buckets[a].m_ItemCount; b++)
			Result->m_Items[Ptr++] = Buckets[a].m_Items[b];
}


BEGIN_MESSAGE_MAP(CListView, CGridView)
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_COMMAND_RANGE(IDM_DETAILS_TOGGLEATTRIBUTE, IDM_DETAILS_TOGGLEATTRIBUTE+LFAttributeCount-1, OnToggleAttribute)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_DETAILS_TOGGLEATTRIBUTE, IDM_DETAILS_TOGGLEATTRIBUTE+LFAttributeCount-1, OnUpdateToggleCommands)
	ON_COMMAND(IDM_DETAILS_AUTOSIZEALL, OnAutosizeAll)
	ON_COMMAND(IDM_DETAILS_AUTOSIZE, OnAutosize)
	ON_COMMAND(IDM_DETAILS_CHOOSE, OnChooseDetails)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_DETAILS_AUTOSIZEALL, IDM_DETAILS_CHOOSE, OnUpdateDetailsCommands)
	ON_NOTIFY(HDN_BEGINDRAG, 1, OnBeginDrag)
	ON_NOTIFY(HDN_BEGINTRACK, 1, OnBeginTrack)
	ON_NOTIFY(HDN_ENDDRAG, 1, OnEndDrag)
	ON_NOTIFY(HDN_ITEMCHANGING, 1, OnItemChanging)
	ON_NOTIFY(HDN_ITEMCLICK, 1, OnItemClick)
END_MESSAGE_MAP()

INT CListView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGridView::OnCreate(lpCreateStruct)==-1)
		return -1;

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | HDS_FLAT | HDS_HIDDEN | HDS_HORZ | HDS_FULLDRAG | HDS_BUTTONS | CCS_TOP | CCS_NOMOVEY | CCS_NODIVIDER;
	CRect rect;
	rect.SetRectEmpty();
	if (!m_wndHeader.Create(dwStyle, rect, this, 1))
		return -1;

	for (UINT a=0; a<LFAttributeCount; a++)
	{
		HDITEM HdItem;
		HdItem.mask = HDI_TEXT | HDI_FORMAT;
		HdItem.pszText = theApp.m_Attributes[a]->Name;
		HdItem.fmt = HDF_STRING;
		m_wndHeader.InsertItem(a, &HdItem);
	}

	return 0;
}

void CListView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (pWnd->GetSafeHwnd()==m_wndHeader)
	{
		CMenu menu;
		menu.LoadMenu(IDM_DETAILS);

		CMenu* pPopup = menu.GetSubMenu(0);
		ASSERT_VALID(pPopup);

		for (INT a=LFLastCoreAttribute-1; a>=0; a--)
			if ((a!=LFAttrStoreID) && (a!=LFAttrFileID) && (theApp.m_Contexts[m_Context]->AllowedAttributes->IsSet(a)))
				pPopup->InsertMenu(3, MF_BYPOSITION | MF_STRING, IDM_DETAILS_TOGGLEATTRIBUTE+a, theApp.m_Attributes[a]->Name);

		CPoint ptClient(point);
		ScreenToClient(&ptClient);

		HDHITTESTINFO htt;
		htt.pt = ptClient;
		m_HeaderItemClicked = m_wndHeader.HitTest(&htt);

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetOwner(), NULL);
		return;
	}

	CGridView::OnContextMenu(pWnd, point);
}

void CListView::OnToggleAttribute(UINT nID)
{
	UINT attr = nID-IDM_DETAILS_TOGGLEATTRIBUTE;
	ASSERT(attr<LFAttributeCount);

	p_ViewParameters->ColumnWidth[attr] = p_ViewParameters->ColumnWidth[attr] ? 0 : theApp.m_Attributes[attr]->RecommendedWidth;
	theApp.UpdateViewOptions(m_Context);
}

void CListView::OnUpdateToggleCommands(CCmdUI* pCmdUI)
{
	UINT attr = pCmdUI->m_nID-IDM_DETAILS_TOGGLEATTRIBUTE;
	ASSERT(attr<LFAttributeCount);

	pCmdUI->SetCheck(m_ViewParameters.ColumnWidth[attr]);
	pCmdUI->Enable(!theApp.m_Attributes[attr]->AlwaysVisible);
}

void CListView::OnAutosizeAll()
{
	for (UINT a=0; a<LFAttributeCount; a++)
		if (m_ViewParameters.ColumnWidth[a])
			AutosizeColumn(a);

	AdjustHeader(TRUE);
	AdjustLayout();
}

void CListView::OnAutosize()
{
	if (m_HeaderItemClicked!=-1)
	{
		AutosizeColumn(m_HeaderItemClicked);
		AdjustHeader(TRUE);
		AdjustLayout();
	}
}

void CListView::OnChooseDetails()
{
	ChooseDetailsDlg dlg(this, m_Context);
	if (dlg.DoModal()==IDOK)
		theApp.UpdateViewOptions(m_Context);
}

void CListView::OnUpdateDetailsCommands(CCmdUI* pCmdUI)
{
	BOOL b = (m_ViewParameters.Mode==LFViewDetails);

	if (pCmdUI->m_nID==IDM_DETAILS_AUTOSIZE)
		b &= (m_HeaderItemClicked!=-1);

	pCmdUI->Enable(b);
}

void CListView::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	*pResult = (pHdr->iItem==LFAttrFileName) || (m_ViewParameters.ColumnWidth[pHdr->iItem]==0);
}

void CListView::OnEndDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if (pHdr->pitem->mask & HDI_ORDER)
	{
		if (pHdr->pitem->iOrder==LFAttrFileName)
			pHdr->pitem->iOrder = 1;
		if (pHdr->iItem==LFAttrFileName)
			pHdr->pitem->iOrder = 0;

		// GetColumnOrderArray() enthält noch die alte Reihenfolge, daher:
		// 1. Spalte an der alten Stelle löschen
		for (UINT a=0; a<LFAttributeCount; a++)
			if (p_ViewParameters->ColumnOrder[a]==pHdr->iItem)
			{
				for (UINT b=a; b<LFAttributeCount-1; b++)
					p_ViewParameters->ColumnOrder[b] = p_ViewParameters->ColumnOrder[b+1];
				break;
			}

		// 2. Spalte an der neuen Stelle einfügen
		for (INT a=LFAttributeCount-1; a>pHdr->pitem->iOrder; a--)
			p_ViewParameters->ColumnOrder[a] = p_ViewParameters->ColumnOrder[a-1];

		p_ViewParameters->ColumnOrder[pHdr->pitem->iOrder] = pHdr->iItem;
		UpdateViewOptions(m_Context);

		*pResult = FALSE;
	}
}

void CListView::OnBeginTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if (pHdr->pitem->mask & HDI_WIDTH)
		*pResult = (theApp.m_Attributes[pHdr->iItem]->Type==LFTypeRating) || (m_ViewParameters.ColumnWidth[pHdr->iItem]==0);
}

void CListView::OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if ((pHdr->pitem->mask & HDI_WIDTH) && (!m_IgnoreHeaderItemChange))
	{
		if (pHdr->pitem->cxy<MINWIDTH)
			pHdr->pitem->cxy = (pHdr->iItem==LFAttrFileName) ? MINWIDTH : 0;

		m_ViewParameters.ColumnWidth[pHdr->iItem] = p_ViewParameters->ColumnWidth[pHdr->iItem] = pHdr->pitem->cxy;
		AdjustLayout();

		*pResult = FALSE;
	}
}

void CListView::OnItemClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;
	UINT attr = pHdr->iItem;

	if (!AttributeSortableInView(attr, m_ViewParameters.Mode))
	{
		CString msg;
		ENSURE(msg.LoadString(IDS_ATTRIBUTENOTSORTABLE));
		MessageBox(msg, theApp.m_Attributes[attr]->Name, MB_OK | MB_ICONWARNING);
	
		return;
	}

	if (p_ViewParameters->SortBy==attr)
	{
		p_ViewParameters->Descending = !p_ViewParameters->Descending;
	}
	else
	{
		p_ViewParameters->SortBy = attr;
		p_ViewParameters->Descending = theApp.m_Attributes[attr]->PreferDescendingSort;
	}
	theApp.UpdateSortOptions(m_Context);

	*pResult = NULL;
}
