
// CTimelineView.cpp: Implementierung der Klasse CTimelineView
//

#include "stdafx.h"
#include "CTimelineView.h"
#include "StoreManager.h"


// CTimelineView
//

#define BORDER     6
#define GUTTER     10
#define MIDDLE     24
#define WHITE      100

#define GetItemData(idx)     ((TimelineItemData*)(m_ItemData+(idx)*m_DataSize))
#define UsePreview(i)        ((i->CoreAttributes.DomainID>=(theApp.OSVersion>OS_XP ? LFDomainAudio : LFDomainPhotos)) && (i->CoreAttributes.DomainID<=LFDomainVideos))

CTimelineView::CTimelineView()
	: CFileView(sizeof(TimelineItemData), TRUE, TRUE, TRUE, TRUE, TRUE, TRUE)
{
}

void CTimelineView::SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data)
{
	CFileView::SetSearchResult(pRawFiles, pCookedFiles, Data);

	if (p_CookedFiles)
	{
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			TimelineItemData* d = GetItemData(a);
			d->Hdr.Valid = TRUE;
		}
	}
}

void CTimelineView::AdjustLayout()
{
	if (!p_CookedFiles)
		return;

	CRect rect;
	GetWindowRect(&rect);

	BOOL HasScrollbars = FALSE;

Restart:
	m_TwoColumns = rect.Width()-2*GUTTER-MIDDLE-10*BORDER>=512;
	m_ItemWidth = m_TwoColumns ? (rect.Width()-MIDDLE)/2-2*GUTTER : rect.Width()-2*GUTTER;
	ASSERT(m_ItemWidth>0);
	m_PreviewCount = (m_ItemWidth-BORDER)/(128+BORDER);

	INT CurRow[2] = { GUTTER, GUTTER };

	for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
	{
		TimelineItemData* d = GetItemData(a);
		if (m_ItemWidth<2*BORDER+128)
		{
			d->Preview = FALSE;
		}
		else
			switch (p_CookedFiles->m_Items[a]->Type & LFTypeMask)
			{
			case LFTypeFile:
				d->Preview = UsePreview(p_CookedFiles->m_Items[a]);
				break;
			case LFTypeVirtual:
				for (INT b=p_CookedFiles->m_Items[a]->FirstAggregate; b<=p_CookedFiles->m_Items[a]->LastAggregate; b++)
				{
					LFItemDescriptor* i = p_RawFiles->m_Items[b];
					if (UsePreview(i))
					{
						d->Preview = TRUE;
						break;
					}
				}
			}

		INT h = 2*BORDER+m_CaptionHeight;
		if (d->Preview)
			h += 128+BORDER+BORDER/2;

		INT c = m_TwoColumns ? CurRow[0]<=CurRow[1] ? 0 : 1 : 0;

		d->Hdr.Rect.left = (c==0) ? GUTTER : rect.Width()-GUTTER-m_ItemWidth;
		d->Hdr.Rect.top = CurRow[c];
		d->Hdr.Rect.right = d->Hdr.Rect.left+m_ItemWidth;
		d->Hdr.Rect.bottom = d->Hdr.Rect.top+h;

		CurRow[c] += h+GUTTER;

		if ((CurRow[c]>rect.Height()) && (!HasScrollbars))
		{
			HasScrollbars = TRUE;
			rect.right -= GetSystemMetrics(SM_CXVSCROLL);
			goto Restart;
		}
	}

	m_ScrollHeight = m_TwoColumns ? max(CurRow[0], CurRow[1]) : CurRow[0];
	m_ScrollWidth = rect.Width();

	CFileView::AdjustLayout();
}

void CTimelineView::DrawItem(CDC& dc, Graphics& g, LPRECT rectItem, INT idx, BOOL Themed)
{
	TimelineItemData* d = GetItemData(idx);
	LFItemDescriptor* i = p_CookedFiles->m_Items[idx];

	BOOL Hot = (m_HotItem==idx);
	BOOL Selected = d->Hdr.Selected;

	COLORREF brCol = Hot ? Themed ? 0xCDBBB4 : GetSysColor(COLOR_WINDOWTEXT) : Themed ? 0xE0CDC4 : GetSysColor(COLOR_3DSHADOW);;
	COLORREF bkCol = hThemeList ? 0xFFFFFF : Selected ? GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHT : COLOR_3DFACE) : Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
	COLORREF cpCol = Themed ? 0x98593B : GetSysColor(COLOR_WINDOWTEXT);
	COLORREF txCol = Themed ? 0x808080 : GetSysColor(COLOR_3DSHADOW);

	// Shadow
	GraphicsPath path;
	if (Themed)
	{
		CRect rectBorder(rectItem);
		rectBorder.left++;
		rectBorder.top++;

		CreateRoundRectangle(rectBorder, (theApp.OSVersion!=OS_Eight) ? 2 : 0, path);

		Pen pen(Color(0x10, 0x00, 0x00, 0x00));
		g.DrawPath(&pen, &path);
	}

	// Background
	CRect rect(rectItem);
	rect.DeflateRect(1, 1);
	dc.FillSolidRect(rect, bkCol);
	if (hThemeList)
	{
		rect.InflateRect(1, 1);
		DrawItemBackground(dc, rect, idx, Themed);
	}
	else
	{
		if (Selected)
		{
			cpCol = txCol = GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT);
			dc.SetTextColor(txCol);
			dc.SetBkColor(0x000000);
		}
		else
		{
			dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
			dc.SetBkColor(0xFFFFFF);
		}

		if ((idx==m_FocusItem) && (GetFocus()==this))
			dc.DrawFocusRect(rect);
	}

	// Border
	if ((!hThemeList) || (!(Hot | Selected)))
		if (Themed)
		{
			CRect rectBorder(rectItem);
			rectBorder.right--;
			rectBorder.bottom--;

			Matrix m;
			m.Translate(-1.0, -1.0);
			path.Transform(&m);

			Pen pen(Color(0xFF, brCol & 0xFF, (brCol>>8) & 0xFF, (brCol>>16) & 0xFF));
			g.DrawPath(&pen, &path);
		}
		else
		{
			dc.Draw3dRect(rectItem, brCol, brCol);
		}

	// Icon
	CRect rectText(rectItem);
	rectText.DeflateRect(BORDER, BORDER);

	const UINT nStyle = ((i->Type & LFTypeGhosted) ? ILD_BLEND50 : ILD_TRANSPARENT) | (i->Type & LFTypeDefault ? INDEXTOOVERLAYMASK(1) : 0);
	if ((i->Type & LFTypeMask)==LFTypeFile)
	{
		theApp.m_SystemImageListSmall.DrawEx(&dc, theApp.m_FileFormats.GetSysIconIndex(i->CoreAttributes.FileFormat), rectText.TopLeft(), m_IconSize, CLR_NONE, 0xFFFFFF, nStyle);
	}
	else
	{
		theApp.m_CoreImageListSmall.DrawEx(&dc, i->IconID-1, rectText.TopLeft(), m_IconSize, CLR_NONE, 0xFFFFFF, nStyle);
	}

	// Caption

	rectText.left += m_IconSize.cx+BORDER;
	rectText.bottom = rectText.top+m_FontHeight[0];

	dc.SetTextColor(cpCol);
	if ((i->Type & LFTypeMask)!=LFTypeVirtual)
	{
		dc.DrawText(GetLabel(i), rectText, DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);
	}
	else
	{
		dc.DrawText(i->Description, -1, rectText, DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);
	}

	rectText.top = rectText.bottom+BORDER/2;
	rectText.bottom = rectText.top+m_FontHeight[3];

	WCHAR tmpBuf[256];
	LFAttributeToString(i, ((i->Type & LFTypeMask)==LFTypeFile) ? m_ViewParameters.SortBy : LFAttrFileName, tmpBuf, 256);

	CFont* pOldFont = dc.SelectObject(&theApp.m_SmallFont);
	dc.SetTextColor(txCol);
	dc.DrawText(tmpBuf, -1, rectText, DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);
	dc.SelectObject(pOldFont);

	// Preview
	if (d->Preview)
	{
		if (!Themed || !Selected)
			dc.FillSolidRect(rectItem->left+BORDER+1, rectText.bottom+BORDER/2, m_ItemWidth-2*BORDER-2, 1, Themed ? 0xE5E5E5 : GetSysColor(COLOR_3DFACE));

		CRect rectPreview(rectItem);
		rectPreview.DeflateRect(BORDER, BORDER);
		rectPreview.top = rectPreview.bottom-128;
		rectPreview.left++;
		rectPreview.right = rectPreview.left+128;

		if ((i->Type & LFTypeMask)==LFTypeVirtual)
		{
			for (INT a=LFMaxRating; a>=0; a--)
				for (INT b=i->FirstAggregate; b<=i->LastAggregate; b++)
				{
					LFItemDescriptor* ri = p_RawFiles->m_Items[b];
					if (UsePreview(ri) && (ri->CoreAttributes.Rating==a))
					{
						CRect rect(rectPreview);
						theApp.m_ThumbnailCache.DrawJumboThumbnail(dc, rect, ri);

						rectPreview.OffsetRect(128+BORDER, 0);
						if (rectPreview.right>rectItem->right-BORDER-1)
							return;
					}
				}
		}
		else
		{
			theApp.m_ThumbnailCache.DrawJumboThumbnail(dc, rectPreview, i);
		}
	}
}


BEGIN_MESSAGE_MAP(CTimelineView, CFileView)
	ON_WM_CREATE()
	ON_WM_PAINT()
END_MESSAGE_MAP()

INT CTimelineView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct)==-1)
		return -1;

	INT cx = 16;
	INT cy = 16;

	ImageList_GetIconSize(theApp.m_CoreImageListSmall, &cx, &cy);
	m_IconSize.cx = cx;
	m_IconSize.cy = cy;

	m_CaptionHeight = max(cy, m_FontHeight[1]+m_FontHeight[3]+BORDER/2);

	return 0;
}

void CTimelineView::OnPaint()
{
	CRect rectUpdate;
	GetUpdateRect(rectUpdate);

	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	Graphics g(dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	// Background
	BOOL Themed = IsCtrlThemed();
	COLORREF bkCol = Themed ? 0xFDF7F4 : GetSysColor(COLOR_3DFACE);
	dc.FillSolidRect(rect, bkCol);

	if (Themed)
	{
		LinearGradientBrush brush(Point(0, 0), Point(0, WHITE+1), Color(0xFF, 0xFF, 0xFF, 0xFF), Color(0x00, 0xFF, 0xFF, 0xFF));
		g.FillRectangle(&brush, Rect(0, 0, rect.Width(), WHITE));
	}

	// Items
	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	if (m_Nothing)
	{
		CRect rectText(rect);
		rectText.top += m_HeaderHeight+6;

		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_NOTHINGTODISPLAY));

		dc.SetTextColor(Themed ? 0x6D6D6D : GetSysColor(COLOR_3DFACE));
		dc.DrawText(tmpStr, rectText, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	}
	else
		if (p_CookedFiles)
			if (p_CookedFiles->m_ItemCount)
			{
				RECT rectIntersect;

				for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
				{
					TimelineItemData* d = GetItemData(a);
					if (d->Hdr.Valid)
					{
						CRect rect(d->Hdr.Rect);
						rect.OffsetRect(-m_HScrollPos, -m_VScrollPos);
						if (IntersectRect(&rectIntersect, rect, rectUpdate))
							DrawItem(dc, g, rect, a, Themed);
					}
				}

				/*if (m_HasCategories)
					for (UINT a=0; a<m_Categories.m_ItemCount; a++)
					{
						CRect rect(m_Categories.m_Items[a].Rect);
						rect.OffsetRect(-m_HScrollPos, -m_VScrollPos+(INT)m_HeaderHeight);
						if (IntersectRect(&rectIntersect, rect, rectUpdate))
							DrawCategory(dc, rect, &m_Categories.m_Items[a], Themed);
					}*/
			}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}
