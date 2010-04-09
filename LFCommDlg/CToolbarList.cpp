
// CToolbarList.cpp: Implementierung der Klasse CToolbarList
//

#include "stdafx.h"
#include "CToolbarList.h"


// CToolbarList
//

CToolbarList::CToolbarList()
	: CPaneList()
{
	m_Width = 32767;
}

CToolbarList::~CToolbarList()
{
}

BEGIN_MESSAGE_MAP(CToolbarList, CPaneList)
	ON_WM_PAINT()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_NOTIFY_REFLECT(LVN_ITEMCHANGED, OnItemChanged)
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

void CToolbarList::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(&rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	CFont* pOldFont = dc.SelectObject(GetFont());

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	// Background
	CDrawingManager dm(dc);
	dm.FillGradient(rect, afxGlobalData.clrBarFace, RGB(255, 255, 255));

	for (int a=0; a<GetItemCount(); a++)
		DrawItem(a, &dc, CMFCVisualManager::GetInstance());

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}

void CToolbarList::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CPaneList::OnHScroll(nSBCode, nPos, pScrollBar);
	RedrawWindow(0, 0, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
}

void CToolbarList::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CPaneList::OnVScroll(nSBCode, nPos, pScrollBar);
	RedrawWindow(0, 0, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
}

void CToolbarList::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && (pNMListView->uOldState & LVIS_CUT) && (pNMListView->uNewState & LVIS_SELECTED))
	{
		*pResult = FALSE;
		return;
	}

	RedrawWindow(0, 0, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
}

BOOL CToolbarList::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CPaneList::OnMouseWheel(nFlags, zDelta, pt);
	RedrawWindow(0, 0, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);

	return TRUE;
}

void CToolbarList::SetSpacing(int cx)
{
	int nHorzSpacing;
	int nVertSpacing;
	GetItemSpacing(FALSE, &nHorzSpacing, &nVertSpacing);
	int n = GetItemCount();
	int l = n*nHorzSpacing;
	int diff = max(cx-l, 0);

	POINT pt;
	pt.y = 0;
	for (int a=0; a<n; a++)
	{
		pt.x = 96+a*nHorzSpacing+(diff*(a+1))/(n+1);
		SetItemPosition(a, pt);
	}
}

BOOL CToolbarList::SetWindowPos(const CWnd* pWndInsertAfter, int x, int y, int cx, int cy, UINT nFlags)
{
	SetRedraw(FALSE);

	if (m_Width>cx)
		SetSpacing(cx);

	BOOL res = CPaneList::SetWindowPos(pWndInsertAfter, x, y, cx, cy, nFlags);

	if (m_Width<cx)
		SetSpacing(cx);

	m_Width = cx;
	SetRedraw(TRUE);

	RedrawWindow(0, 0, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
	return res;
}
