
// CTimelineView.cpp: Implementierung der Klasse CTimelineView
//

#include "stdafx.h"
#include "CTimelineView.h"
#include "StoreManager.h"


WCHAR* GetAttribute(TimelineItemData* d, LFItemDescriptor* i, UINT Attr, UINT Mask)
{
	ASSERT(theApp.m_Attributes[Attr]->Type==LFTypeUnicodeString);

	if (i->AttributeValues[Attr])
		if (*((WCHAR*)i->AttributeValues[Attr]))
		{
			d->Preview |= Mask;
			return (WCHAR*)i->AttributeValues[Attr];
		}

	return NULL;
}


// CTimelineView
//

#define ARROWSIZE     6
#define BORDER        6
#define GUTTER        14
#define MIDDLE        24
#define WHITE         100

#define GetItemData(idx)     ((TimelineItemData*)(m_ItemData+(idx)*m_DataSize))
#define UsePreview(i)        ((i->CoreAttributes.DomainID>=LFDomainPhotos) && (i->CoreAttributes.DomainID<=LFDomainVideos))

CTimelineView::CTimelineView()
	: CFileView(sizeof(TimelineItemData), TRUE, TRUE, TRUE, TRUE, TRUE, FALSE)
{
}

void CTimelineView::SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data)
{
	CFileView::SetSearchResult(pRawFiles, pCookedFiles, Data);

	if (p_CookedFiles)
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			TimelineItemData* d = GetItemData(a);
			LFItemDescriptor* i = p_CookedFiles->m_Items[a];

			LFVariantData v;
			v.Attr = m_ViewParameters.SortBy;
			LFGetAttributeVariantData(i, &v);

			d->Hdr.Valid = !LFIsNullVariantData(&v);
			if (d->Hdr.Valid)
			{
				SYSTEMTIME stUTC;
				SYSTEMTIME stLocal;
				FileTimeToSystemTime(&v.Time, &stUTC);
				SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

				d->Year = stLocal.wYear;

				switch (i->Type & LFTypeMask)
				{
				case LFTypeFile:
					d->pComments = GetAttribute(d, i, LFAttrComments, PRV_COMMENTS);

					if (i->CoreAttributes.DomainID==LFDomainAudio)
					{
						d->pArtist = GetAttribute(d, i, LFAttrArtist, PRV_AUDIOTITLE);
						d->pTitle = GetAttribute(d, i, LFAttrTitle, PRV_AUDIOTITLE);
						d->pAlbum = GetAttribute(d, i, LFAttrAlbum, PRV_AUDIOALBUM);
					}

					break;
				case LFTypeVirtual:
					d->Preview |= PRV_COMMENTS;
					d->pComments = NULL;

					for (INT b=i->FirstAggregate; b<=i->LastAggregate; b++)
					{
						LFItemDescriptor* i = p_RawFiles->m_Items[b];
						if (UsePreview(i))
							d->Preview |= PRV_THUMBS;

						ASSERT(theApp.m_Attributes[LFAttrRoll]->Type==LFTypeUnicodeString);
						if (d->Preview & PRV_COMMENTS)
							if (!i->AttributeValues[LFAttrRoll])
							{
								d->Preview &= ~PRV_COMMENTS;
							}
							else
								if (d->pComments)
								{
									if (wcscmp(d->pComments, (WCHAR*)i->AttributeValues[LFAttrRoll])!=0)
										d->Preview &= ~PRV_COMMENTS;
								}
								else
								{
									d->pComments = (WCHAR*)i->AttributeValues[LFAttrRoll];
									if (*d->pComments==L'\0')
										d->Preview &= ~PRV_COMMENTS;
								}
					}
				}
			}
		}
}

void CTimelineView::AdjustLayout()
{
	if (!p_CookedFiles)
	{
		m_ScrollWidth = m_ScrollHeight = 0;
		return;
	}

	CRect rect;
	GetWindowRect(&rect);

	BOOL HasScrollbars = FALSE;

Restart:
	SYSTEMTIME st;
	GetLocalTime(&st);

	WORD Year = st.wYear;

	m_Categories.m_ItemCount = 0;
	m_TwoColumns = rect.Width()-2*GUTTER-MIDDLE-7*BORDER>=512;
	m_ItemWidth = m_TwoColumns ? (rect.Width()-MIDDLE)/2-GUTTER : rect.Width()-2*GUTTER;
	m_PreviewCount = (m_ItemWidth-BORDER)/(128+BORDER);

	INT CurRow[2] = { GUTTER, GUTTER };
	INT LastRow = 0;

	for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
	{
		TimelineItemData* d = GetItemData(a);

		if (d->Hdr.Valid)
		{
			LFItemDescriptor* i = p_CookedFiles->m_Items[a];

			if (m_ItemWidth<2*BORDER+128)
			{
				d->Preview &= ~PRV_THUMBS;
			}
			else
			{
				switch (i->Type & LFTypeMask)
				{
				case LFTypeFile:
					if (UsePreview(i))
						d->Preview |= PRV_THUMBS;
					break;
				case LFTypeVirtual:
					for (INT b=i->FirstAggregate; b<=i->LastAggregate; b++)
					{
						LFItemDescriptor* i = p_RawFiles->m_Items[b];
						if (UsePreview(i))
						{
							d->Preview |= PRV_THUMBS;
							break;
						}
					}
				}
			}

			INT h = 2*BORDER+m_CaptionHeight;
			if (d->Preview)
				h += BORDER/2+BORDER;
			if (d->Preview & PRV_AUDIOTITLE)
				h += m_FontHeight[0];
			if (d->Preview & PRV_AUDIOALBUM)
				h += m_FontHeight[0];
			if (d->Preview & PRV_COMMENTS)
				h += m_FontHeight[0];
			if (d->Preview & PRV_THUMBS)
				h += 128;

			if (d->Year!=Year)
			{
				Year = d->Year;

				ItemCategory ic;
				ZeroMemory(&ic, sizeof(ic));

				swprintf_s(ic.Caption, 256, L"%d", Year);

				ic.Rect.left = rect.Width()/2-m_LabelWidth/2;
				ic.Rect.right = ic.Rect.left+m_LabelWidth;
				ic.Rect.top = max(CurRow[0], CurRow[1]);
				ic.Rect.bottom = ic.Rect.top+2*BORDER+m_FontHeight[0];
				m_Categories.AddItem(ic);

				CurRow[0] = CurRow[1] = ic.Rect.bottom+GUTTER;
			}

			INT c = m_TwoColumns ? CurRow[0]<=CurRow[1] ? 0 : 1 : 0;
			d->Arrow = m_TwoColumns ? 1-(BYTE)c*2 : 0;
			d->ArrowOffs = 0;
			d->Hdr.RectInflate = d->Arrow ? ARROWSIZE+1 : 0;

			if (abs(CurRow[c]-LastRow)<2*ARROWSIZE)
				if (h>(m_CaptionHeight+BORDER)/2+ARROWSIZE+BORDER+2*GUTTER)
				{
					d->ArrowOffs = 2*GUTTER;
				}
				else
				{
					CurRow[c] += GUTTER;
				}

			d->Hdr.Rect.left = (c==0) ? GUTTER : GUTTER+m_ItemWidth+MIDDLE;
			d->Hdr.Rect.top = CurRow[c];
			d->Hdr.Rect.right = d->Hdr.Rect.left+m_ItemWidth;
			d->Hdr.Rect.bottom = d->Hdr.Rect.top+h;

			LastRow = CurRow[c];
			CurRow[c] += h+GUTTER;

			if ((CurRow[c]>rect.Height()) && (!HasScrollbars))
			{
				HasScrollbars = TRUE;
				rect.right -= GetSystemMetrics(SM_CXVSCROLL);
				goto Restart;
			}
		}
	}

	m_ScrollHeight = m_TwoColumns ? max(CurRow[0], CurRow[1]) : CurRow[0];
	m_ScrollWidth = rect.Width();

	CFileView::AdjustLayout();
}

RECT CTimelineView::GetLabelRect(INT idx)
{
	RECT rect = GetItemRect(idx);

	rect.left += 2*BORDER+m_IconSize.cx-5;
	rect.top += BORDER-2;
	rect.right -= BORDER-2;
	rect.bottom = rect.top+m_FontHeight[0]+4;

	return rect;
}

void CTimelineView::DrawCategory(CDC& dc, Graphics& g, LPRECT rectCategory, ItemCategory* ic, COLORREF tlCol, BOOL Themed)
{
	if (Themed && (theApp.OSVersion!=OS_Eight))
	{
		GraphicsPath path;
		CreateRoundRectangle(rectCategory, 4, path);
	
		Matrix m;
		m.Translate(0.5, 0.5);
		path.Transform(&m);

		SolidBrush brush(Color(0xFF, tlCol & 0xFF, (tlCol>>8) & 0xFF, (tlCol>>16) & 0xFF));
		g.FillPath(&brush, &path);
	}
	else
	{
		dc.FillSolidRect(rectCategory, tlCol);
	}

	CRect rectText(rectCategory);
	if (Themed)
		rectText.OffsetRect(1, 1);

	dc.DrawText(ic->Caption, -1, rectText, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
}

void CTimelineView::DrawItem(CDC& dc, Graphics& g, LPRECT rectItem, INT idx, BOOL Themed)
{
	TimelineItemData* d = GetItemData(idx);
	LFItemDescriptor* i = p_CookedFiles->m_Items[idx];

	BOOL Hot = (m_HotItem==idx);
	BOOL Selected = d->Hdr.Selected;

	COLORREF brCol = Hot ? Themed ? 0xCDBBB4 : GetSysColor(COLOR_WINDOWTEXT) : Themed ? 0xE0CDC4 : GetSysColor(COLOR_3DSHADOW);
	COLORREF bkCol = hThemeList ? 0xFFFFFF : Selected ? GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHT : COLOR_3DFACE) : Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
	COLORREF cpCol = Themed ? 0x98593B : GetSysColor(COLOR_WINDOWTEXT);
	COLORREF txCol = Themed ? 0x808080 : GetSysColor(COLOR_3DSHADOW);
	COLORREF atCol = Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT);

	// Shadow
	GraphicsPath path;
	Color sCol(0x12, 0x00, 0x00, 0x00);
	if (Themed)
	{
		CRect rectBorder(rectItem);
		rectBorder.left++;
		rectBorder.top++;

		CreateRoundRectangle(rectBorder, (theApp.OSVersion!=OS_Eight) ? 2 : 0, path);

		Pen pen(sCol);
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
			cpCol = txCol = atCol = GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHTTEXT : COLOR_BTNTEXT);
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
	if ((!hThemeList) || (!(Hot || Selected)))
		if (Themed && (theApp.OSVersion!=OS_Eight))
		{
			CRect rectBorder(rectItem);
			rectBorder.right--;
			rectBorder.bottom--;

			Matrix m;
			m.Translate(-1.0, -1.0);
			path.Transform(&m);

			Pen pen(Color(brCol & 0xFF, (brCol>>8) & 0xFF, (brCol>>16) & 0xFF));
			g.DrawPath(&pen, &path);
		}
		else
		{
			dc.Draw3dRect(rectItem, brCol, brCol);
		}

	// Arrows
	if (d->Arrow)
	{
		INT Base = (d->Arrow==1) ? rectItem->right-1 : rectItem->left;
		INT Start = rectItem->top+(m_CaptionHeight+BORDER)/2-ARROWSIZE+d->ArrowOffs;
		INT y = Start;

		COLORREF ptCol = hThemeList ? dc.GetPixel(Base, y) : brCol;

		for (INT a=-ARROWSIZE; a<=ARROWSIZE; a++)
		{
			CPen pen(PS_SOLID, 1, dc.GetPixel(Base-2*d->Arrow, y));
			CPen* pPen = dc.SelectObject(&pen);

			const INT x = Base+(ARROWSIZE-abs(a)+1)*d->Arrow;
			dc.MoveTo(Base, y);
			dc.LineTo(x, y);
			dc.SetPixel(x, y++, ptCol);

			dc.SelectObject(pPen);
		}

		if (Themed)
		{
			INT Offs = d->Arrow>0 ? 0 : 1;

			g.SetSmoothingMode(SmoothingModeNone);

			Pen pen(sCol);
			g.DrawLine(&pen, Base+(ARROWSIZE+1)*d->Arrow, Start+ARROWSIZE+1, Base+d->Arrow+1-Offs, Start+2*ARROWSIZE+Offs);

			g.SetSmoothingMode(SmoothingModeAntiAlias);
		}
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

		// Attributes
		if (d->Preview & (PRV_COMMENTS | PRV_AUDIOTITLE | PRV_AUDIOALBUM))
		{
			CRect rectAttr(rectItem->left+BORDER+2, 0, rectItem->right-BORDER, 0);
			rectAttr.top = rectText.bottom+BORDER+BORDER/2;
			rectAttr.bottom = rectAttr.top+m_FontHeight[0];

			dc.SetTextColor(atCol);

			if (d->Preview & PRV_AUDIOTITLE)
			{
				WCHAR tmpStr[513] = L"";

				if (d->pArtist)
				{
					wcscpy_s(tmpStr, 513, d->pArtist);
					if (d->pTitle)
						wcscat_s(tmpStr, 513, L": ");
				}

				if (d->pTitle)
					wcscat_s(tmpStr, 513, d->pTitle);

				dc.DrawText(tmpStr, -1, rectAttr, DT_SINGLELINE | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
				rectAttr.OffsetRect(0, m_FontHeight[0]);
			}

			if (d->Preview & PRV_AUDIOALBUM)
			{
				ASSERT(d->pAlbum);

				dc.DrawText(d->pAlbum, -1, rectAttr, DT_SINGLELINE | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
				rectAttr.OffsetRect(0, m_FontHeight[0]);
			}

			if (d->Preview & PRV_COMMENTS)
			{
				ASSERT(d->pComments);

				dc.DrawText(d->pComments, -1, rectAttr, DT_SINGLELINE | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
			}
		}

		// Thumbs
		if (d->Preview & PRV_THUMBS)
		{
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
}

void CTimelineView::ScrollWindow(INT dx, INT dy)
{
	CFileView::ScrollWindow(dx, dy);

	if (IsCtrlThemed())
	{
		CRect rect;
		GetClientRect(rect);

		rect.bottom = rect.top+(dy<0) ? WHITE : WHITE+dy;
		InvalidateRect(rect);
	}
}


BEGIN_MESSAGE_MAP(CTimelineView, CFileView)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
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

	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&theApp.m_BoldFont);
	m_LabelWidth = dc->GetTextExtent(_T("8888")).cx+2*BORDER;
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

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
	{
		RECT rectIntersect;

		// Timeline
		COLORREF tlCol = Themed ? 0xD3B5A9 : GetSysColor(COLOR_3DSHADOW);
		if (m_TwoColumns)
			dc.FillSolidRect(rect.Width()/2-1, -m_VScrollPos, 2, m_ScrollHeight, tlCol);

		CFont* pOldFont = dc.SelectObject(&theApp.m_BoldFont);

		dc.SetTextColor(Themed ? 0xFFFFFF : GetSysColor(COLOR_HIGHLIGHTTEXT));

		for (UINT a=0; a<m_Categories.m_ItemCount; a++)
		{
			CRect rect(m_Categories.m_Items[a].Rect);
			rect.OffsetRect(0, -m_VScrollPos);
			if (IntersectRect(&rectIntersect, rect, rectUpdate))
				DrawCategory(dc, g, rect, &m_Categories.m_Items[a], tlCol, Themed);
		}

		dc.SelectObject(pOldFont);

		// Items
		if (p_CookedFiles)
			if (p_CookedFiles->m_ItemCount)
				for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
				{
					TimelineItemData* d = GetItemData(a);
					if (d->Hdr.Valid)
					{
						CRect rect(d->Hdr.Rect);
						rect.OffsetRect(0, -m_VScrollPos);
						if (IntersectRect(&rectIntersect, rect, rectUpdate))
							DrawItem(dc, g, rect, a, Themed);
					}
				}
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}

void CTimelineView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CFileView::OnKeyDown(nChar, nRepCnt, nFlags);

	if (p_CookedFiles)
	{
		CRect rect;
		GetClientRect(&rect);

		INT item = m_FocusItem;
		INT left = GetItemData(item)->Hdr.Rect.left;
		INT right = GetItemData(item)->Hdr.Rect.right;
		INT top = GetItemData(item)->Hdr.Rect.top;
		INT bottom = GetItemData(item)->Hdr.Rect.bottom;

		switch (nChar)
		{
		case VK_LEFT:
			for (INT a=item-1; a>=0; a--)
			{
				TimelineItemData* d = GetItemData(a);
				if ((d->Hdr.Rect.right<left) && d->Hdr.Valid)
				{
					item = a;
					break;
				}
			}

			break;
		case VK_RIGHT:
			for (INT a=item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				TimelineItemData* d = GetItemData(a);
				if ((d->Hdr.Rect.left>right) && d->Hdr.Valid)
				{
					item = a;
					break;
				}
			}
			for (INT a=item; a>=0; a--)
			{
				TimelineItemData* d = GetItemData(a);
				if ((d->Hdr.Rect.left>right) && d->Hdr.Valid)
				{
					item = a;
					break;
				}
			}

			break;
		case VK_UP:
			for (INT a=item-1; a>=0; a--)
			{
				TimelineItemData* d = GetItemData(a);
				if ((d->Hdr.Rect.left==left) && d->Hdr.Valid)
				{
					item = a;
					break;
				}
			}

			break;
		case VK_PRIOR:
			for (INT a=item-1; a>=0; a--)
			{
				TimelineItemData* d = GetItemData(a);
				if ((d->Hdr.Rect.left<=left) && d->Hdr.Valid)
				{
					item = a;
					if (d->Hdr.Rect.top<=bottom-rect.Height())
						break;
				}
			}

			break;
		case VK_DOWN:
			for (INT a=item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				TimelineItemData* d = GetItemData(a);
				if ((d->Hdr.Rect.left==left) && d->Hdr.Valid)
				{
					item = a;
					break;
				}
			}

			break;
		case VK_NEXT:
			for (INT a=item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				TimelineItemData* d = GetItemData(a);
				if ((d->Hdr.Rect.right>=right) && d->Hdr.Valid)
				{
					item = a;
					if (d->Hdr.Rect.bottom>=top+rect.Height())
						break;
				}
			}

			break;
		case VK_HOME:
			if (GetKeyState(VK_CONTROL)<0)
			{
				for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
					if (GetItemData(a)->Hdr.Valid)
					{
						item = a;
						break;
					}
			}
			else
				for (INT a=item-1; a>=0; a--)
				{
					TimelineItemData* d = GetItemData(a);
					if (d->Hdr.Valid)
						if (d->Hdr.Rect.right<left)
						{
							item = a;
							break;
						}
				}

			break;
		case VK_END:
			if (GetKeyState(VK_CONTROL)<0)
			{
				for (INT a=(INT)p_CookedFiles->m_ItemCount-1; a>=0; a--)
					if (GetItemData(a)->Hdr.Valid)
					{
						item = a;
						break;
					}
			}
			else
				for (INT a=item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
				{
					TimelineItemData* d = GetItemData(a);
					if (d->Hdr.Valid)
						if (d->Hdr.Rect.left>right)
						{
							item = a;
							break;
						}
				}

			break;
		}

		if (item!=m_FocusItem)
		{
			SetFocusItem(item, GetKeyState(VK_SHIFT)<0);

			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);
			OnMouseMove(0, pt);
		}
	}
}
