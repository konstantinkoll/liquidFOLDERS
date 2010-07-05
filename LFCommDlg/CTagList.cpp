
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
	m_FontSmall.CreateFont(-8, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
		OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
		face);
}

CTagList::~CTagList()
{
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
	COLORREF oldCol = (COLORREF)-1;
	COLORREF bkCol = GetBkColor();
	COLORREF selCol;
	COLORREF texCol;

	// Background
	CRect rectBounds;
	GetItemRect(nID, rectBounds, LVIR_BOUNDS);
	int oldMode = pDC->SetBkMode(TRANSPARENT);

	pDC->FillSolidRect(rectBounds, bkCol);

	UINT State = GetItemState(nID, LVIS_SELECTED | LVIS_FOCUSED | LVIS_CUT);
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
	pDC->FillSolidRect(rectBounds, selCol);

	rectBounds.DeflateRect(4, 0);

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

	// Anzahl
	CFont* pOldFont = pDC->SelectObject(&m_FontSmall);
	int L = pDC->GetTextExtent(item.pszText).cx;

	oldCol = pDC->SetTextColor(texCol);
	pDC->DrawText(item.pszText, -1, rectBounds, DT_NOPREFIX | DT_END_ELLIPSIS | DT_SINGLELINE | DT_RIGHT | DT_VCENTER);

	pDC->SelectObject(pOldFont);

	// Label
	item.iSubItem = 0;
	item.pszText = text;
	GetItem(&item);

	pOldFont = pDC->SelectObject(&m_FontLarge);
	rectBounds.right -= L+4;

	SetTextColor(texCol);
	pDC->DrawText(item.pszText, -1, rectBounds, DT_NOPREFIX | DT_END_ELLIPSIS | DT_SINGLELINE | DT_CENTER | DT_VCENTER);

	pDC->SelectObject(pOldFont);
	pDC->SetTextColor(oldCol);
	pDC->SetBkMode(oldMode);
}
