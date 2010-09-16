
// CTreeView.cpp: Implementierung der Klasse CTreeView
//

#include "stdafx.h"
#include "CTreeView.h"
#include "Migrate.h"
#include "LFCore.h"


// CTreeView
//

#define MINWIDTH     75
#define MAXWIDTH     350

CTreeView::CTreeView()
{
}

BOOL CTreeView::Create(CWnd* _pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T("Tree"), dwStyle, rect, _pParentWnd, nID);
}

void CTreeView::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	WINDOWPOS wp;
	HDLAYOUT HdLayout;
	HdLayout.prc = &rect;
	HdLayout.pwpos = &wp;
	m_wndHeader.Layout(&HdLayout);

	m_wndHeader.SetWindowPos(NULL, wp.x, wp.y, wp.cx, wp.cy, wp.flags | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CTreeView::ClearRoot()
{
	m_wndHeader.ModifyStyle(0, HDS_HIDDEN);
	AdjustLayout();


	Invalidate();
}

void CTreeView::SetRoot(LPITEMIDLIST pidl, BOOL Update)
{
	m_wndHeader.ModifyStyle(HDS_HIDDEN, 0);
	AdjustLayout();


	Invalidate();
}


BEGIN_MESSAGE_MAP(CTreeView, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_NOTIFY(HDN_BEGINDRAG, 1, OnBeginDrag)
	ON_NOTIFY(HDN_ITEMCHANGING, 1, OnItemChanging)
END_MESSAGE_MAP()

int CTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | HDS_FLAT | HDS_HIDDEN | HDS_HORZ | HDS_FULLDRAG | HDS_BUTTONS | CCS_TOP | CCS_NOMOVEY | CCS_NODIVIDER;
	CRect rect;
	rect.SetRectEmpty();
	if (!m_wndHeader.Create(dwStyle, rect, this, 1))
		return -1;

	m_wndHeader.SetFont(&theApp.m_DefaultFont);

	HDITEM HdItem;
	HdItem.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
	HdItem.cxy = MINWIDTH;
	HdItem.fmt = HDF_STRING | HDF_CENTER;

	for (UINT a=0; a<10; a++)
	{
		HdItem.pszText = a ? L"Ignore" : L"";
		m_wndHeader.InsertItem(a, &HdItem);
	}

	return 0;
}

BOOL CTreeView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CTreeView::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);
	pDC.FillSolidRect(rect, 0xFFFFFF);
}

void CTreeView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

BOOL CTreeView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	return TRUE;
}

void CTreeView::OnBeginDrag(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	*pResult = TRUE;
}

void CTreeView::OnItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if (pHdr->pitem->mask & HDI_WIDTH)
	{
		if (pHdr->pitem->cxy<MINWIDTH)
			pHdr->pitem->cxy = MINWIDTH;

		if (pHdr->pitem->cxy>MAXWIDTH)
			pHdr->pitem->cxy = MAXWIDTH;

		*pResult = FALSE;
	}
}
