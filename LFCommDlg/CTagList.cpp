
// CTagList.cpp: Implementierung der Klasse CTagList
//

#include "stdafx.h"
#include "CTagList.h"


// CTagList
//

CTagList::CTagList()
	: CListCtrl()
{
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
	CRect rect;
	int H;
	COLORREF oldCol;
	COLORREF bkCol = GetBkColor();

	// Background
	CRect rectBounds;
	GetItemRect(nID, rectBounds, LVIR_BOUNDS);
	int oldMode = pDC->SetBkMode(TRANSPARENT);

	UINT State = GetItemState(nID, LVIS_SELECTED | LVIS_FOCUSED | LVIS_CUT);
	if ((State & (LVIS_SELECTED | LVIS_FOCUSED)) && ((State & LVIS_SELECTED) || ((GetFocus()==this)) || (GetStyle() & LVS_SHOWSELALWAYS)))
	{
		pDC->FillSolidRect(rectBounds, GetSysColor(COLOR_HIGHLIGHT));
		//((MyDrawManager*)dm)->DrawItem(pDC, rectBounds, State & LVIS_SELECTED, ((GetFocus()==this) && (State & LVIS_FOCUSED)));
	}
	else
	{
		pDC->FillSolidRect(rectBounds, bkCol);
	}

	TCHAR text[260];
	UINT columns[2];
	LVITEM item;
	ZeroMemory(&item, sizeof(item));
	item.iItem = nID;
	item.pszText = text;
	item.cchTextMax = sizeof(text)/sizeof(TCHAR);
	item.puColumns = columns;
	item.cColumns = 1;
	item.mask = LVIF_TEXT | LVIF_COLUMNS;
	GetItem(&item);

	// Label
	H = pDC->GetTextExtent(item.pszText).cy;
	GetItemRect(nID, &rect, LVIR_LABEL);

	UINT nFormat = DT_SINGLELINE;
	pDC->DrawText(item.pszText, -1, rect, DT_NOPREFIX | DT_END_ELLIPSIS | nFormat);

	item.mask = LVIF_TEXT;

	oldCol = (COLORREF)-1;
	if ((!State) && (pDC->GetTextColor()==0x000000))
		oldCol = pDC->SetTextColor((bkCol>>1) & 0x7F7F7F);

	for (UINT a=0; a<item.cColumns; a++)
	{
		rect.top += pDC->GetTextExtent(item.pszText).cy;

		item.iSubItem = item.puColumns[a];
		item.pszText = text;
		GetItem(&item);

		pDC->DrawText(item.pszText, -1, rect, DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS);
	}

	if (oldCol!=(COLORREF)-1)
		pDC->SetTextColor(oldCol);

	pDC->SetBkMode(oldMode);
}
