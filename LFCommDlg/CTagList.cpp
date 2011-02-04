
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
	ZeroMemory(&m_BgBitmaps, sizeof(m_BgBitmaps));
}

CTagList::~CTagList()
{
	for (UINT a=0; a<2; a++)
		if (m_BgBitmaps[a])
			delete m_BgBitmaps[a];
}

void CTagList::CreateRoundRectangle(CRect rect, INT rad, GraphicsPath& path)
{
	path.Reset();

	INT l = rect.left;
	INT t = rect.top;
	INT w = rect.Width()-1;
	INT h = rect.Height()-1;
	INT d = rad<<1;

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
	ON_WM_SYSCOLORCHANGE()
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
		DrawItem((INT)lplvcd->nmcd.dwItemSpec, CDC::FromHandle(lplvcd->nmcd.hdc));
		*pResult = CDRF_SKIPDEFAULT;
		break;
	default:
		*pResult = CDRF_DODEFAULT;
	}
}

void CTagList::DrawItem(INT nID, CDC* pDC)
{
	COLORREF bkCol = GetBkColor();
	COLORREF selCol;
	COLORREF texCol;

	UINT State = GetItemState(nID, LVIS_SELECTED | LVIS_FOCUSED);
	UINT bgBitmap;
	if ((State & LVIS_SELECTED) && ((GetFocus()==this) || (GetStyle() & LVS_SHOWSELALWAYS)))
	{
		selCol = GetSysColor(COLOR_HIGHLIGHT);
		texCol = GetSysColor(COLOR_HIGHLIGHTTEXT);
		bgBitmap = 0;
	}
	else
	{
		selCol = GetSysColor(COLOR_HIGHLIGHT);
		selCol = (((((selCol>>16) & 0xFF) >> 2) + ((((bkCol>>16) & 0xFF)*3) >> 2))<<16) |
				(((((selCol>>8) & 0xFF) >> 2) + ((((bkCol>>8) & 0xFF)*3) >> 2))<<8) |
				(((selCol & 0xFF) >> 2) + (((bkCol & 0xFF)*3) >> 2));
		texCol = GetSysColor(COLOR_WINDOWTEXT);
		bgBitmap = 1;
	}

	// Background
	CRect rectBounds;
	GetItemRect(nID, rectBounds, LVIR_BOUNDS);

	CRect rect(rectBounds);
	rect.MoveToXY(0, 0);

	CDC dc;
	dc.CreateCompatibleDC(pDC);
	CBitmap* pOldBitmap;

	if (!m_BgBitmaps[bgBitmap])
	{
		dc.SetBkMode(TRANSPARENT);

		m_BgBitmaps[bgBitmap] = new CBitmap();
		m_BgBitmaps[bgBitmap]->CreateCompatibleBitmap(pDC, rectBounds.Width(), rectBounds.Height());
		pOldBitmap = dc.SelectObject(m_BgBitmaps[bgBitmap]);

		dc.FillSolidRect(rect, bkCol);

		// Border
		Graphics g(dc);
		g.SetCompositingMode(CompositingModeSourceOver);
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		if (!m_Path.GetPointCount())
			CreateRoundRectangle(rect, 9, m_Path);

		// Inner border
		SolidBrush sBr(Color(State & LVIS_SELECTED ? 0xC0 : 0x80, selCol & 0xFF, (selCol>>8) & 0xFF, (selCol>>16) & 0xFF));
		g.FillPath(&sBr, &m_Path);

		// Outer border
		Pen pen(Color(selCol & 0xFF, (selCol>>8) & 0xFF, (selCol>>16) & 0xFF));
		g.DrawPath(&pen, &m_Path);
	}
	else
	{
		pOldBitmap = dc.SelectObject(m_BgBitmaps[bgBitmap]);
	}

	pDC->BitBlt(rectBounds.left, rectBounds.top, rectBounds.Width(), rectBounds.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);

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

	pDC->SetBkMode(TRANSPARENT);

	LFApplication* pApp = (LFApplication*)AfxGetApp();

	// Count
	CFont* pOldFont = pDC->SelectObject(&pApp->m_SmallFont);
	rectBounds.DeflateRect(5, 0);
	INT L = pDC->GetTextExtent(item.pszText).cx;
	pDC->SetTextColor((State & LVIS_SELECTED) ? texCol : ((texCol>>1) & 0x7F7F7F) + ((pDC->GetPixel(rectBounds.right, rectBounds.top+10)>>1) & 0x7F7F7F));
	pDC->DrawText(item.pszText, rectBounds, DT_NOPREFIX | DT_END_ELLIPSIS | DT_SINGLELINE | DT_RIGHT | DT_VCENTER);
	pDC->SelectObject(pOldFont);

	// Label
	item.iSubItem = 0;
	item.pszText = text;
	GetItem(&item);

	pOldFont = pDC->SelectObject(&pApp->m_DefaultFont);
	rectBounds.right -= L+5;
	pDC->SetTextColor(texCol);
	pDC->DrawText(item.pszText, rectBounds, DT_NOPREFIX | DT_END_ELLIPSIS | DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	pDC->SelectObject(pOldFont);

	// FocusRect
	if ((State & LVIS_FOCUSED) && (GetFocus()==this))
	{
		rectBounds.DeflateRect(0, 1);
		pDC->SetBkColor(0x000000);
		pDC->DrawFocusRect(rectBounds);
	}
}

void CTagList::OnSysColorChange()
{
	for (UINT a=0; a<2; a++)
		if (m_BgBitmaps[a])
		{
			delete m_BgBitmaps[a];
			m_BgBitmaps[a] = NULL;
		}
}
