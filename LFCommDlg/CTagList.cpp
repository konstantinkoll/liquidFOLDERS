
// CTagList.cpp: Implementierung der Klasse CTagList
//

#include "stdafx.h"
#include "CTagList.h"
#include "LFApplication.h"


// CTagList
//

CTagList::CTagList()
	: CListCtrl()
{
	CString face = ((LFApplication*)AfxGetApp())->GetDefaultFontFace();

	m_FontLarge.CreateFont(-14, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
	m_FontSmall.CreateFont(-10, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
}

CTagList::~CTagList()
{
}

void CTagList::CreateRoundRectangle(CRect rect, int rad, GraphicsPath& path)
{
	path.Reset();

	int l = rect.left;
	int t = rect.top;
	int w = rect.Width()-1;
	int h = rect.Height()-1;
	int d = rad<<1;

	path.AddArc(l, t, d, d, 180, 90);
	path.AddLine(l+rad, t, l+w-rad, t);
	path.AddArc(l+w-d, t, d, d, 270, 90);
	path.AddLine(l+w, t+rad, l+w, t+h-rad);
	path.AddArc(l+w-d, t+h-d, d, d, 0, 90);
	path.AddLine(l+w-rad, t+h, l+rad, t+h);
	path.AddArc(l, t+h-d, d, d, 90, 90);
	path.AddLine(l, t+h-rad, l, t+rad);
	path.CloseFigure();
}


BEGIN_MESSAGE_MAP(CTagList, CListCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
END_MESSAGE_MAP()

void CTagList::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMLVCUSTOMDRAW lplvcd = (LPNMLVCUSTOMDRAW)pNMHDR;

	switch(lplvcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		break;
	case CDDS_ITEMPREPAINT:
		DrawItem((int)lplvcd->nmcd.dwItemSpec, CDC::FromHandle(lplvcd->nmcd.hdc));
		*pResult = CDRF_SKIPDEFAULT;
		break;
	default:
		*pResult = CDRF_DODEFAULT;
	}
}

void CTagList::DrawItem(int nID, CDC* pDC)
{
	COLORREF bkCol = GetBkColor();
	COLORREF selCol;
	COLORREF texCol;

	// Background
	CRect rectBounds;
	GetItemRect(nID, rectBounds, LVIR_BOUNDS);
	CRect rect(rectBounds);
	rect.MoveToXY(0, 0);

	UINT State = GetItemState(nID, LVIS_SELECTED | LVIS_FOCUSED);
	if ((State & LVIS_SELECTED) && ((GetFocus()==this) || (GetStyle() & LVS_SHOWSELALWAYS)))
	{
		selCol = GetSysColor(COLOR_HIGHLIGHT);
		texCol = GetSysColor(COLOR_HIGHLIGHTTEXT);
	}
	else
	{
		selCol = GetSysColor(COLOR_HIGHLIGHT);
		selCol = (((((selCol>>16) & 0xFF) >> 2) + ((((bkCol>>16) & 0xFF)*3) >> 2))<<16) |
				(((((selCol>>8) & 0xFF) >> 2) + ((((bkCol>>8) & 0xFF)*3) >> 2))<<8) |
				(((selCol & 0xFF) >> 2) + (((bkCol & 0xFF)*3) >> 2));
		texCol = GetSysColor(COLOR_WINDOWTEXT);
	}

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(pDC, rectBounds.Width(), rectBounds.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	dc.FillSolidRect(rect, bkCol);

	// Border
	Graphics g(dc.m_hDC);
	g.SetCompositingMode(CompositingModeSourceOver);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	GraphicsPath path;
	CreateRoundRectangle(rect, 9, path);

	// Inner border
	SolidBrush sBr(Color(State & LVIS_SELECTED ? 0xE0 : 0x80, selCol & 0xFF, (selCol>>8) & 0xFF, (selCol>>16) & 0xFF));
	g.FillPath(&sBr, &path);

	// Outer border
	Pen pen(Color(selCol & 0xFF, (selCol>>8) & 0xFF, (selCol>>16) & 0xFF));
	g.DrawPath(&pen, &path);

	// Item
	TCHAR text[260];
	UINT columns[2];
	LVITEM item;
	ZeroMemory(&item, sizeof(item));
	item.iItem = nID;
	item.iSubItem = 1;
	item.pszText = text;
	item.cchTextMax = sizeof(text)/sizeof(TCHAR);
	item.puColumns = columns;
	item.cColumns = 1;
	item.mask = LVIF_TEXT | LVIF_COLUMNS;
	GetItem(&item);

	// Count
	CFont* pOldFont = dc.SelectObject(&m_FontSmall);
	rect.DeflateRect(5, 0);
	int L = dc.GetTextExtent(item.pszText).cx;
	dc.SetTextColor(texCol);
	dc.DrawText(item.pszText, -1, rect, DT_NOPREFIX | DT_END_ELLIPSIS | DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
	dc.SelectObject(pOldFont);

	// Label
	item.iSubItem = 0;
	item.pszText = text;
	GetItem(&item);

	pOldFont = dc.SelectObject(&m_FontLarge);
	rect.right -= L+5;
	SetTextColor(texCol);
	dc.DrawText(item.pszText, -1, rect, DT_NOPREFIX | DT_END_ELLIPSIS | DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	dc.SelectObject(pOldFont);

	pDC->BitBlt(rectBounds.left, rectBounds.top, rectBounds.Width(), rectBounds.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);

	// FocusRect
	if ((State & LVIS_FOCUSED) && (GetFocus()==this))
	{
		rect.DeflateRect(0, 2);
		rect.MoveToXY(rectBounds.left+5, rectBounds.top+2);
		pDC->DrawFocusRect(rect);
	}
}
