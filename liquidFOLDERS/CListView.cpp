
// CListView.cpp: Implementierung der Klasse CListView
//

#include "stdafx.h"
#include "ChooseDetailsDlg.h"
#include "CListView.h"
#include "liquidFOLDERS.h"


// CListView
//

#define GetItemData(Index)                  ((GridItemData*)(m_ItemData+(Index)*m_DataSize))
#define PADDING                             2
#define DrawLabel(dc, rect, pItemDescriptor, format)      dc.DrawText(GetLabel(pItemDescriptor), rect, DT_END_ELLIPSIS | DT_NOPREFIX | format);
#define SwitchColor(dc, d)                  if ((Themed) && (!(pItemDescriptor->CoreAttributes.Flags & LFFlagMissing)) && !pData->Hdr.Selected) dc.SetTextColor(0x808080);
#define PrepareBlend()                      INT w = min(rect.Width(), RatingBitmapWidth); \
                                            INT h = min(rect.Height(), RatingBitmapHeight);
#define Blend(dc, rect, level, bitmaps)     { HDC hdcMem = CreateCompatibleDC(dc); \
                                            HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, bitmaps[level>LFMaxRating ? 0 : level]); \
                                            AlphaBlend(dc, rect.left, rect.top+1, w, h, hdcMem, 0, 0, w, h, BF); \
                                            SelectObject(hdcMem, hOldBitmap); \
                                            DeleteDC(hdcMem); }
#define MAXAUTOWIDTH                        400
#define MINWIDTH                            50


/*	if (m_HeaderHeight>0)
		if (Themed)
		{
			Bitmap* pDivider = theApp.GetCachedResourceImage(IDB_DIVUP);

			Graphics g(dc);
			g.DrawImage(pDivider, (rect.Width()-(INT)pDivider->GetWidth())/2+GetScrollPos(SB_HORZ)+BACKSTAGEBORDER-1, m_HeaderHeight-(INT)pDivider->GetHeight());
		}
		else
		{
			dc.FillSolidRect(0, 0, rect.Width(), m_HeaderHeight, GetSysColor(COLOR_3DFACE));
		}*/


CListView::CListView(UINT DataSize)
	: CGridView(DataSize)
{
	m_Icons[0] = m_Icons[1] = NULL;
	m_HeaderItemClicked = -1;
	m_IgnoreHeaderItemChange = FALSE;
}

void CListView::SetViewSettings(BOOL Force)
{
	if ((p_ContextViewSettings->View!=m_ContextViewSettings.View) || (Force))
	{
		INT cx;
		INT cy;

		switch (p_ContextViewSettings->View)
		{
		case LFViewDetails:
			m_Icons[0] = &theApp.m_CoreImageListJumbo;
			m_Icons[1] = (theApp.OSVersion<OS_Vista) ? &theApp.m_SystemImageListExtraLarge : &theApp.m_SystemImageListJumbo;

			cx = cy = 128;

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

	if ((p_ContextViewSettings->View==LFViewList) && (p_CookedFiles))
		for (UINT a=0; a<LFAttributeCount; a++)
			if (p_ContextViewSettings->ColumnWidth[a]!=m_ContextViewSettings.ColumnWidth[a])
			{
				m_ContextViewSettings = *p_ContextViewSettings;
				AdjustLayout();

				break;
			}

	AdjustHeader((p_ContextViewSettings->View==LFViewList) && (p_CookedFiles));
}

void CListView::SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData)
{
	CGridView::SetSearchResult(pRawFiles, pCookedFiles, pPersistentData);

	ValidateAllItems();

	AdjustHeader(m_ContextViewSettings.View==LFViewList);
}

void CListView::AdjustHeader(BOOL bShow)
{
	if (bShow)
	{
		m_wndHeader.SetRedraw(FALSE);
		m_IgnoreHeaderItemChange = TRUE;

		VERIFY(m_wndHeader.SetOrderArray(LFAttributeCount, p_ContextViewSettings->ColumnOrder));

		for (UINT a=0; a<LFAttributeCount; a++)
		{
			HDITEM hdi;
			hdi.mask = HDI_WIDTH | HDI_FORMAT;
			hdi.cxy = p_ContextViewSettings->ColumnWidth[a];
			hdi.fmt = theApp.m_Attributes[a].TypeProperties.FormatRight ? HDF_RIGHT : HDF_LEFT;

			if (hdi.cxy)
				if (theApp.m_Attributes[a].AttrProperties.Type==LFTypeRating)
				{
					hdi.cxy = p_ContextViewSettings->ColumnWidth[a] = RatingBitmapWidth+4*PADDING;
				}
				else
					if (hdi.cxy<MINWIDTH)
						p_ContextViewSettings->ColumnWidth[a] = hdi.cxy = MINWIDTH;

			if (p_ContextViewSettings->SortBy==a)
				hdi.fmt |= p_ContextViewSettings->Descending ? HDF_SORTDOWN : HDF_SORTUP;

			m_wndHeader.SetItem(a, &hdi);
		}

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

void CListView::AdjustLayout()
{
	// Header
	CRect rect;
	GetWindowRect(rect);

	WINDOWPOS wp;
	HDLAYOUT HdLayout;
	HdLayout.prc = &rect;
	HdLayout.pwpos = &wp;
	m_wndHeader.Layout(&HdLayout);

	wp.x = BACKSTAGEBORDER-1;
	wp.y = 0;
	m_HeaderHeight = wp.cy;

	// Items
	CSize szItem;

	switch (m_ContextViewSettings.View)
	{
	case LFViewList:
		szItem.cx = -2*PADDING;

		for (UINT a=0; a<LFAttributeCount; a++)
			szItem.cx += m_ContextViewSettings.ColumnWidth[a];

		szItem.cy = max(m_IconSize[0].cy, m_DefaultFontHeight)+1;

		Arrange(szItem, PADDING, CSize(1, -1), TRUE);

		break;

	case LFViewDetails:
		szItem.cx = 0;
		szItem.cy = m_IconSize[0].cy+18+3+8+1;

		Arrange(szItem, PADDING, CSize(1, -1), TRUE);

		break;
	}

	// Header
	m_wndHeader.SetWindowPos(NULL, wp.x-m_HScrollPos, wp.y, wp.cx+m_HScrollMax+GetSystemMetrics(SM_CXVSCROLL), m_HeaderHeight, wp.flags | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
	m_wndHeader.Invalidate();
}

RECT CListView::GetLabelRect(INT Index) const
{
	RECT rect = GetItemRect(Index);

	switch (m_ContextViewSettings.View)
	{
	case LFViewList:
		rect.right = rect.left+m_ContextViewSettings.ColumnWidth[0]-3*PADDING;

	case LFViewDetails:
		rect.left += m_IconSize[0].cx+2*PADDING;
		break;
	}

	return rect;
}

void CListView::DrawItem(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed)
{
	LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];
	GridItemData* pData = GetItemData(Index);

	CRect rectClient;

	CRect rect(rectItem);
	rect.DeflateRect(PADDING, PADDING);

	CRect rectIcon(rect);
	CRect rectLabel(rect);

	switch (m_ContextViewSettings.View)
	{
	case LFViewList:
		rectIcon.right = rectIcon.left+m_IconSize[0].cx;

		rectLabel.right = rectLabel.left+m_ContextViewSettings.ColumnWidth[0]-3*PADDING;
		rectLabel.left = rectIcon.right+PADDING;

		GetClientRect(rectClient);

		for (UINT a=0; a<LFAttributeCount; a++)
		{
			UINT Attr = m_ContextViewSettings.ColumnOrder[a];
			if (m_ContextViewSettings.ColumnWidth[Attr])
			{
				if (Attr)
				{
					rectLabel.left = rectLabel.right+3*PADDING;
					rectLabel.right = rectLabel.left+m_ContextViewSettings.ColumnWidth[Attr]-3*PADDING;
				}

				switch (Attr)
				{
				case LFAttrFileName:
					if ((IsEditing()) && (Index==m_EditLabel))
						continue;

					break;
				}

				if ((rectLabel.left<=rectClient.right) && (rectLabel.right>=rectClient.left))
					DrawColumn(dc, rectLabel, pItemDescriptor, Attr);
			}
		}

		break;

	case LFViewDetails:
		if (pItemDescriptor->AttributeValues[LFAttrRating])
		{
			UCHAR Rating = *((UCHAR*)pItemDescriptor->AttributeValues[LFAttrRating]);
			if (((pItemDescriptor->Type & LFTypeMask)==LFTypeFile) || (Rating))
			{
				rectIcon.left += (m_IconSize[0].cx-88)/2;
				rectIcon.bottom -= 9;
				rectIcon.top = rectIcon.bottom-18;

				PrepareBlend();
				Blend(dc, rectIcon, Rating, theApp.hRatingBitmaps);
			}
		}

		rectIcon.left = rectLabel.left;
		rectIcon.right = rectIcon.left+m_IconSize[0].cx;
		rectIcon.top = rectLabel.top+1;
		rectIcon.bottom = rectIcon.top+m_IconSize[0].cy;

		if (IsEditing() && (Index==m_EditLabel))
			break;

		rectLabel.left += m_IconSize[0].cx+m_DefaultFontHeight/2;
		rectLabel.top++;

		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrFileName, Themed);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrComments, Themed, FALSE);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrDescription, Themed, FALSE);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrArtist, Themed, FALSE);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrTitle, Themed, FALSE);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrAlbum, Themed, FALSE);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrRecordingTime, Themed, FALSE);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrRoll, Themed, FALSE);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrDuration, Themed, FALSE);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrHashtags, Themed, FALSE);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrPages, Themed, FALSE);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrWidth, Themed, FALSE);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrHeight, Themed, FALSE);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrEquipment, Themed, FALSE);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrBitrate, Themed, FALSE);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrCreationTime, Themed, FALSE);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrFileTime, Themed, FALSE);
		DrawProperty(dc, rectLabel, pItemDescriptor, pData, LFAttrFileSize, Themed, FALSE);

		break;
	}

	DrawIcon(dc, rectIcon, pItemDescriptor);
}

__forceinline void CListView::DrawIcon(CDC& dc, const CRect& rect, LFItemDescriptor* pItemDescriptor)
{
	INT SysIconIndex = -1;

	if (!pItemDescriptor->IconID)
	{
		ASSERT((pItemDescriptor->Type & LFTypeMask)==LFTypeFile);

		if (m_ContextViewSettings.View==LFViewDetails)
		{
			if (!theApp.m_ThumbnailCache.DrawJumboThumbnail(dc, rect, pItemDescriptor))
				theApp.m_FileFormats.DrawJumboIcon(dc, rect, pItemDescriptor->CoreAttributes.FileFormat, pItemDescriptor->Type & LFTypeGhosted);

			return;
		}

		SysIconIndex = theApp.m_FileFormats.GetSysIconIndex(pItemDescriptor->CoreAttributes.FileFormat);
	}

	const UINT List = (SysIconIndex>=0) ? 1 : 0;
	const INT IconID = (List==1) ? SysIconIndex : pItemDescriptor->IconID-1;

	if (IconID>=0)
		m_Icons[List]->DrawEx(&dc, IconID, CPoint(rect.left+(rect.Width()-m_IconSize[List].cx)/2, rect.top+(rect.Height()-m_IconSize[List].cy)/2), m_IconSize[List], CLR_NONE, 0xFFFFFF, ((pItemDescriptor->Type & LFTypeGhosted) ? ILD_BLEND50 : ILD_TRANSPARENT) | (pItemDescriptor->Type & LFTypeBadgeMask));
}

void CListView::AttributeToString(LFItemDescriptor* pItemDescriptor, UINT Attr, WCHAR* tmpStr, SIZE_T cCount)
{
	switch (Attr)
	{
	case LFAttrFileName:
		wcsncpy_s(tmpStr, cCount, GetLabel(pItemDescriptor), _TRUNCATE);
		break;

	case LFAttrFileFormat:
		wcscpy_s(tmpStr, cCount, theApp.m_FileFormats.GetTypeName(pItemDescriptor->CoreAttributes.FileFormat));
		break;

	default:
		LFAttributeToString(pItemDescriptor, Attr, tmpStr, cCount);
	}
}

__forceinline void CListView::DrawColumn(CDC& dc, CRect& rect, LFItemDescriptor* pItemDescriptor, UINT Attr)
{
	if (theApp.m_Attributes[Attr].AttrProperties.Type==LFTypeRating)
	{
		if (pItemDescriptor->AttributeValues[Attr])
		{
			const UCHAR Rating = *((UCHAR*)pItemDescriptor->AttributeValues[Attr]);
			if (((pItemDescriptor->Type & LFTypeMask)==LFTypeFile) || (Rating))
			{
				PrepareBlend();
				if (Attr==LFAttrRating)
				{
					Blend(dc, rect, Rating, theApp.hRatingBitmaps);
				}
				else
				{
					Blend(dc, rect, Rating, theApp.hPriorityBitmaps);
				}
			}
		}
	}
	else
	{
		WCHAR tmpStr[256];
		AttributeToString(pItemDescriptor, Attr, tmpStr, 256);

		if (tmpStr[0]!=L'\0')
			dc.DrawText(tmpStr, rect, (theApp.m_Attributes[Attr].TypeProperties.FormatRight ? DT_RIGHT : DT_LEFT) | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
	}
}

void CListView::DrawProperty(CDC& dc, CRect& rect, LFItemDescriptor* pItemDescriptor, GridItemData* pData, UINT Attr, BOOL Themed, BOOL AlwaysNewRow)
{
	CFont* pOldFont;

	switch (Attr)
	{
	case LFAttrFileName:
		pOldFont = dc.SelectObject(&theApp.m_LargeFont);
		DrawLabel(dc, rect, pItemDescriptor, DT_LEFT | DT_SINGLELINE);
		dc.SelectObject(pOldFont);

		rect.top += m_LargeFontHeight;

		break;

	case LFAttrRating:
	case LFAttrPriority:
		{
			PrepareBlend();
			if (Attr==LFAttrRating)
			{
				Blend(dc, rect, pItemDescriptor->CoreAttributes.Rating, theApp.hRatingBitmaps);
			}
			else
			{
				Blend(dc, rect, pItemDescriptor->CoreAttributes.Priority, theApp.hPriorityBitmaps);
			}
		}

		rect.top += 18;

		break;

	default:
		WCHAR tmpStr[256];
		AttributeToString(pItemDescriptor, Attr, tmpStr, 256);

		if (tmpStr[0]!=L'\0')
		{
			if (rect.top>rect.bottom-m_DefaultFontHeight)
				return;

			COLORREF oldColor = dc.GetTextColor();

			CRect rectText(rect);
			if ((Attr!=LFAttrComments) && (Attr!=LFAttrDescription))
			{
				CString tmpCaption(theApp.m_Attributes[Attr].Name);
				tmpCaption += _T(": ");

				dc.DrawText(tmpCaption, rectText, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

				rectText.left += dc.GetTextExtent(tmpCaption).cx;
			}

			SwitchColor(dc, d);
			dc.DrawText(tmpStr, rectText, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
			dc.SetTextColor(oldColor);
		}
		else
		{
			if (!AlwaysNewRow)
				return;
		}

		rect.top += m_DefaultFontHeight;

		break;
	}
}

INT CListView::GetMaxLabelWidth(INT Max)
{
	INT Width = 0;

	if (p_CookedFiles)
	{
		CDC* pDC = GetWindowDC();
		CFont* pOldFont = pDC->SelectObject(&theApp.m_DefaultFont);

		for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
		{
			INT cx = pDC->GetTextExtent(GetLabel((*p_CookedFiles)[a])).cx;

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
	
		pDC->SelectObject(pOldFont);
		ReleaseDC(pDC);
	}

	return Width;
}

INT CListView::GetMaxColumnWidth(UINT Col, INT Max)
{
	INT Width = 0;

	if (p_CookedFiles)
	{
		CDC dc;
		dc.CreateCompatibleDC(NULL);

		CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

		for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
		{
			WCHAR tmpStr[256];
			LFAttributeToString((*p_CookedFiles)[a], Col, tmpStr, 256);

			const INT cx = dc.GetTextExtent(tmpStr, (INT)wcslen(tmpStr)).cx;
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
	
		dc.SelectObject(pOldFont);
	}

	return Width;
}

void CListView::AutosizeColumn(UINT Col)
{
	p_ContextViewSettings->ColumnWidth[Col] = p_ContextViewSettings->ColumnWidth[Col] = 3*PADDING +
		((Col==LFAttrFileName) ? m_IconSize[0].cx+PADDING+GetMaxLabelWidth(MAXAUTOWIDTH) : (theApp.m_Attributes[Col].AttrProperties.Type==LFTypeRating) ? RatingBitmapWidth+PADDING : GetMaxColumnWidth(Col, MAXAUTOWIDTH));

	if (theApp.m_Attributes[Col].TypeProperties.FormatRight)
		p_ContextViewSettings->ColumnWidth[Col] += 3;

	if (p_ContextViewSettings->ColumnWidth[Col]<MINWIDTH)
		p_ContextViewSettings->ColumnWidth[Col] = MINWIDTH;

	m_ContextViewSettings.ColumnWidth[Col] = p_ContextViewSettings->ColumnWidth[Col];
}

void CListView::ScrollWindow(INT dx, INT dy, LPCRECT lpRect, LPCRECT lpClipRect)
{
	if (IsWindow(m_wndHeader) && (dx!=0))
	{
		CRect rectWindow;
		GetWindowRect(rectWindow);

		WINDOWPOS wp;
		HDLAYOUT HdLayout;
		HdLayout.prc = &rectWindow;
		HdLayout.pwpos = &wp;
		m_wndHeader.Layout(&HdLayout);

		wp.x = BACKSTAGEBORDER-1;
		wp.y = 0;

		m_wndHeader.SetWindowPos(NULL, wp.x-m_HScrollPos, wp.y, wp.cx+m_HScrollMax+GetSystemMetrics(SM_CXVSCROLL), m_HeaderHeight, wp.flags | SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOCOPYBITS);
		m_wndHeader.RedrawWindow(NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);

		CRect rect;
		GetClientRect(rect);

		InvalidateRect(CRect(rect.left, 0, rect.right, m_HeaderHeight));
	}

	CGridView::ScrollWindow(dx, dy, lpRect, lpClipRect);
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

	if (!m_wndHeader.Create(this, 1))
		return -1;

	for (UINT a=0; a<LFAttributeCount; a++)
	{
		HDITEM HdItem;
		HdItem.mask = HDI_TEXT | HDI_FORMAT;
		HdItem.pszText = theApp.m_Attributes[a].Name;
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

		for (INT a=LFAttributeCount-1; a>=0; a--)
			if (theApp.IsAttributeAdvertised(m_Context, a))
				pPopup->InsertMenu(3, MF_BYPOSITION | MF_STRING, IDM_DETAILS_TOGGLEATTRIBUTE+a, theApp.m_Attributes[a].Name);

		CPoint pt(point);
		ScreenToClient(&pt);

		HDHITTESTINFO htt;
		htt.pt = pt;
		m_HeaderItemClicked = m_wndHeader.HitTest(&htt);

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetOwner(), NULL);

		return;
	}

	CGridView::OnContextMenu(pWnd, point);
}

void CListView::OnToggleAttribute(UINT nID)
{
	UINT Attr = nID-IDM_DETAILS_TOGGLEATTRIBUTE;
	ASSERT(Attr<LFAttributeCount);

	p_ContextViewSettings->ColumnWidth[Attr] = p_ContextViewSettings->ColumnWidth[Attr] ? 0 : theApp.m_Attributes[Attr].TypeProperties.DefaultColumnWidth;
	theApp.UpdateViewSettings(m_Context);
}

void CListView::OnUpdateToggleCommands(CCmdUI* pCmdUI)
{
	UINT Attr = pCmdUI->m_nID-IDM_DETAILS_TOGGLEATTRIBUTE;
	ASSERT(Attr<LFAttributeCount);

	pCmdUI->SetCheck(m_ContextViewSettings.ColumnWidth[Attr]);
	pCmdUI->Enable(!theApp.m_Attributes[Attr].AttrProperties.AlwaysShow);
}

void CListView::OnAutosizeAll()
{
	for (UINT a=0; a<LFAttributeCount; a++)
		if (m_ContextViewSettings.ColumnWidth[a])
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
	ChooseDetailsDlg dlg(m_Context, this);
	if (dlg.DoModal()==IDOK)
		theApp.UpdateViewSettings(m_Context);
}

void CListView::OnUpdateDetailsCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = (m_ContextViewSettings.View==LFViewList);

	if (pCmdUI->m_nID==IDM_DETAILS_AUTOSIZE)
		bEnable &= (m_HeaderItemClicked!=-1);

	pCmdUI->Enable(bEnable);
}

void CListView::OnBeginDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	*pResult = (m_ContextViewSettings.ColumnWidth[pHdr->iItem]==0);
}

void CListView::OnEndDrag(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if (pHdr->pitem->mask & HDI_ORDER)
	{
		if (pHdr->pitem->iOrder==-1)
		{
			p_ContextViewSettings->ColumnWidth[pHdr->iItem] = 0;
		}
		else
		{
			// GetColumnOrderArray() enthält noch die alte Reihenfolge, daher:
			// 1. Spalte an der alten Stelle löschen
			for (UINT a=0; a<LFAttributeCount; a++)
				if (p_ContextViewSettings->ColumnOrder[a]==pHdr->iItem)
				{
					for (UINT b=a; b<LFAttributeCount-1; b++)
						p_ContextViewSettings->ColumnOrder[b] = p_ContextViewSettings->ColumnOrder[b+1];

					break;
				}

			// 2. Spalte an der neuen Stelle einfügen
			for (INT a=LFAttributeCount-1; a>pHdr->pitem->iOrder; a--)
				p_ContextViewSettings->ColumnOrder[a] = p_ContextViewSettings->ColumnOrder[a-1];

			p_ContextViewSettings->ColumnOrder[pHdr->pitem->iOrder] = pHdr->iItem;
		}

		UpdateViewSettings(m_Context);

		*pResult = FALSE;
	}
}

void CListView::OnBeginTrack(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if (pHdr->pitem->mask & HDI_WIDTH)
		*pResult = (theApp.m_Attributes[pHdr->iItem].AttrProperties.Type==LFTypeRating) || (m_ContextViewSettings.ColumnWidth[pHdr->iItem]==0);
}

void CListView::OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if ((pHdr->pitem->mask & HDI_WIDTH) && (!m_IgnoreHeaderItemChange))
	{
		if (pHdr->pitem->cxy<MINWIDTH)
			pHdr->pitem->cxy = theApp.m_Attributes[pHdr->iItem].AttrProperties.AlwaysShow ? MINWIDTH : 0;

		m_ContextViewSettings.ColumnWidth[pHdr->iItem] = p_ContextViewSettings->ColumnWidth[pHdr->iItem] = pHdr->pitem->cxy;
		AdjustLayout();

		*pResult = FALSE;
	}
}

void CListView::OnItemClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	const UINT Attr = pHdr->iItem;
	theApp.SetContextSort(m_Context, Attr, p_ContextViewSettings->SortBy==Attr ? !p_ContextViewSettings->Descending : theApp.m_Attributes[Attr].AttrProperties.DefaultDescending);

	*pResult = FALSE;
}
