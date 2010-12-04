
// CListView.cpp: Implementierung der Klasse CListView
//

#include "stdafx.h"
#include "CListView.h"
#include "StoreManager.h"
#include <string>


// CListView
//

#define GetItemData(idx)                   ((FVItemData*)(m_ItemData+idx*m_DataSize))
#define PADDING                            2
#define DrawLabel(dc, rect, i, format)     dc.DrawText(GetLabel(i), -1, rect, DT_END_ELLIPSIS | format);
#define SwitchColor(dc, d)                 if ((Themed) && ((hThemeList) || (!d->Selected))) dc.SetTextColor(0x808080);
#define PrepareBlend()                     INT w = min(rect.Width(), RatingBitmapWidth); \
                                           INT h = min(rect.Height(), RatingBitmapHeight); \
                                           BLENDFUNCTION BF = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };
#define Blend(dc, rect, level, bitmaps)    { HDC hdcMem = CreateCompatibleDC(dc); \
                                           HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, bitmaps[level>LFMaxRating ? 0 : level]); \
                                           AlphaBlend(dc, rect.left, rect.top+1, w, h, hdcMem, 0, 0, w, h, BF); \
                                           SelectObject(hdcMem, hbmOld); \
                                           DeleteDC(hdcMem); }
#define RIGHTCOLUMN                        215

CListView::CListView(UINT DataSize)
	: CGridView(DataSize)
{
	m_Icons[0] = m_Icons[1] = NULL;
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
		case LFViewSearchResult:
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
}

void CListView::SetSearchResult(LFSearchResult* Result)
{
	m_Extensions.empty();

	if (Result)
		for (UINT a=0; a<Result->m_ItemCount; a++)
		{
			LFItemDescriptor* i = Result->m_Items[a];
			if ((i->Type & LFTypeMask)==LFTypeFile)
				if (m_Extensions.count(i->CoreAttributes.FileFormat)==0)
				{
					CString Ext = _T("*.");
					Ext += i->CoreAttributes.FileFormat;

					SHFILEINFO sfi;
					if (SUCCEEDED(SHGetFileInfo(Ext, 0, &sfi, sizeof(sfi), SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES)))
						m_Extensions[i->CoreAttributes.FileFormat] = sfi.szTypeName;
				}
		}
}

void CListView::AdjustLayout()
{
	GVArrange gva = { 0, 0, 17, 2, PADDING, 1, -1 };

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
		ArrangeVertical(gva);
		break;
	case LFViewDetails:
	case LFViewCalendarDay:
		gva.cx = 140;
		gva.cy = max(m_IconSize[0].cy, m_FontHeight[0]);
		ArrangeHorizontal(gva, FALSE, TRUE);
		break;
	case LFViewTiles:
		gva.cx = 240;
		gva.cy = max(m_IconSize[0].cy, m_FontHeight[0]*3+max(m_FontHeight[0], 18));
		gva.gutterx = gva.guttery = 3;
		ArrangeHorizontal(gva, FALSE);
		break;
	case LFViewSearchResult:
		gva.cy = 2+max(m_IconSize[0].cy, max(m_FontHeight[0]*3+m_FontHeight[1], m_FontHeight[0]*2+max(m_FontHeight[0], 18)*2+1));
		ArrangeHorizontal(gva, FALSE, TRUE, TRUE);
		break;
	}
}

void CListView::DrawItem(CDC& dc, LPRECT rectItem, INT idx, BOOL Themed)
{
	PrepareSysIcon(idx);

	LFItemDescriptor* i = p_Result->m_Items[idx];
	FVItemData* d = GetItemData(idx);
	INT Rows[4];
	BOOL Right = FALSE;

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
		DrawIcon(dc, rectIcon, i, d);
		rectLabel.top += m_IconSize[0].cy+PADDING;
		DrawLabel(dc, rectLabel, i, DT_CENTER | DT_WORDBREAK);
		break;
	case LFViewDetails:
	case LFViewCalendarDay:
		rectIcon.right = rectIcon.left+m_IconSize[0].cx;
		DrawIcon(dc, rectIcon, i, d);
		break;
	case LFViewList:
		rectIcon.right = rectIcon.left+m_IconSize[0].cx;
		DrawIcon(dc, rectIcon, i, d);
		rectLabel.left += m_IconSize[0].cx+PADDING;
		DrawLabel(dc, rectLabel, i, DT_LEFT | DT_SINGLELINE);
		break;
	case LFViewTiles:
		rectIcon.right = rectIcon.left+m_IconSize[0].cx;
		DrawIcon(dc, rectIcon, i, d);
		rectLabel.left += m_IconSize[0].cx+m_FontHeight[0]/2;

		Rows[0] = LFAttrFileName;
		switch (i->Type & LFTypeMask)
		{
		case LFTypeStore:
		case LFTypeDrive:
			Rows[1] = LFAttrComment;
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
				Rows[1] = LFAttrComment;
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
	case LFViewSearchResult:
		rectIcon.right = rectIcon.left+m_IconSize[0].cx;
		DrawIcon(dc, rectIcon, i, d);

		rectLeft.left += m_IconSize[0].cx+m_FontHeight[0]/2;
		rectLeft.top++;
		Right = (rect.Width()>600) && (((i->Type & LFTypeMask)==LFTypeStore) || ((i->Type & LFTypeMask)==LFTypeFile));
		if (Right)
			rectLeft.right -= RIGHTCOLUMN+2*PADDING;

		DrawProperty(dc, rectLeft, i, d, LFAttrFileName, Themed);
		switch (i->Type & LFTypeMask)
		{
		case LFTypeStore:
		case LFTypeDrive:
			DrawProperty(dc, rectLeft, i, d, LFAttrComment, Themed);
			DrawProperty(dc, rectLeft, i, d, LFAttrDescription, Themed);
			break;
		case LFTypeFile:
			DrawProperty(dc, rectLeft, i, d, LFAttrComment, Themed);
			DrawProperty(dc, rectLeft, i, d, LFAttrTags, Themed);
			DrawProperty(dc, rectLeft, i, d, LFAttrFileFormat, Themed);
			break;
		case LFTypeVirtual:
			if (m_Context==LFContextStoreHome)
			{
				DrawProperty(dc, rectLeft, i, d, LFAttrComment, Themed);
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
	}
}

void CListView::DrawIcon(CDC& dc, CRect& rect, LFItemDescriptor* i, FVItemData* d)
{
	const INT List = (d->SysIconIndex>=0) ? 1 : 0;
	rect.OffsetRect((rect.Width()-m_IconSize[List].cx)/2, (rect.Height()-m_IconSize[List].cy)/2);
	m_Icons[List]->DrawEx(&dc, (d->SysIconIndex>=0) ? d->SysIconIndex : i->IconID-1, rect.TopLeft(), m_IconSize[List], CLR_NONE, 0xFFFFFF, (i->Type & LFTypeGhosted) ? ILD_BLEND50 : ILD_TRANSPARENT);
}

void CListView::AttributeToString(LFItemDescriptor* i, UINT Attr, WCHAR* tmpStr, size_t cCount)
{
	switch (Attr)
	{
	case LFAttrFileName:
		wcscpy_s(tmpStr, cCount, GetLabel(i));
		break;
	case LFAttrFileFormat:
		wcscpy_s(tmpStr, cCount, m_Extensions[i->CoreAttributes.FileFormat].c_str());
		break;
	default:
		LFAttributeToString(i, Attr, tmpStr, cCount);
	}
}

void CListView::DrawTileRows(CDC& dc, CRect& rect, LFItemDescriptor* i, FVItemData* d, INT* Rows, BOOL Themed)
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
	rect.bottom = rect.top+m_FontHeight[0];

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
				dc.DrawText(tmpStr[a], -1, rect, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
				rect.OffsetRect(0, m_FontHeight[0]);
			}

		if (Rows[a]==LFAttrFileName)
			SwitchColor(dc, d);
	}
}

void CListView::DrawProperty(CDC& dc, CRect& rect, LFItemDescriptor* i, FVItemData* d, UINT Attr, BOOL Themed)
{
	CFont* pOldFont;
	WCHAR tmpStr[256];

	switch (Attr)
	{
	case LFAttrFileName:
		pOldFont = dc.SelectObject(&theApp.m_LargeFont);
		DrawLabel(dc, rect, i, DT_LEFT | DT_SINGLELINE);
		dc.SelectObject(pOldFont);

		rect.OffsetRect(0, m_FontHeight[1]);
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

		rect.OffsetRect(0, 18);
		break;
	default:
		AttributeToString(i, Attr, tmpStr, 256);
		if (tmpStr[0]!=L'\0')
		{
			COLORREF oldColor = dc.GetTextColor();

			CRect rectText(rect);
			if ((Attr!=LFAttrComment) && (Attr!=LFAttrDescription))
			{
				CString tmpCaption(theApp.m_Attributes[Attr]->Name);
				tmpCaption += _T(": ");
				dc.DrawText(tmpCaption, -1, rectText, DT_LEFT | DT_SINGLELINE);
				rectText.left += dc.GetTextExtent(tmpCaption, tmpCaption.GetLength()).cx;
			}

			SwitchColor(dc, d);
			dc.DrawText(tmpStr, -1, rectText, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS);
			dc.SetTextColor(oldColor);
		}

		rect.OffsetRect(0, m_FontHeight[0]);
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
			INT cx = dc->GetTextExtent(label, label.GetLength()).cx;

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
