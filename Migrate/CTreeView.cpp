
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
	m_Tree = NULL;
	m_Allocated = m_Rows = m_Cols = 0;
}

BOOL CTreeView::Create(CWnd* _pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), dwStyle, rect, _pParentWnd, nID);
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

	m_HeaderHeight = wp.cy + (wp.cy ? 4 : 0);
	m_wndHeader.SetWindowPos(NULL, wp.x, wp.y, wp.cx, m_HeaderHeight, wp.flags | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CTreeView::ClearRoot()
{
	FreeTree();

	m_wndHeader.ModifyStyle(0, HDS_HIDDEN);
	AdjustLayout();
	Invalidate();
}

void CTreeView::SetRoot(LPITEMIDLIST pidl, BOOL Update)
{
	if (!Update)
	{
		FreeTree();
		InsertRow(0);
	}

//	SetItem(m_Tree, theApp.GetShellManager()->CopyItem(pidl));

	if (!Update)
	{
		// TODO
	}

	m_wndHeader.ModifyStyle(HDS_HIDDEN, 0);
	AdjustLayout();
	Invalidate();
}

BOOL CTreeView::InsertRow(UINT Row)
{
	ASSERT(Row<=m_Rows);

	if (!m_Tree)
	{
		m_Tree = (Cell*)_aligned_malloc(FirstAlloc*MaxColumns*sizeof(Cell), MemoryAlignment);
		if (!m_Tree)
			return FALSE;

		m_Allocated = FirstAlloc;
	}

	if (m_Rows==m_Allocated)
	{
		m_Tree = (Cell*)_aligned_realloc(m_Tree, (m_Allocated+SubsequentAlloc)*MaxColumns*sizeof(Cell), MemoryAlignment);
		if (!m_Tree)
			return FALSE;

		m_Allocated += SubsequentAlloc;
	}

	if (Row<m_Rows)
		for (UINT a=m_Rows; a>Row; a--)
			memcpy(&m_Tree[(a+1)*MaxColumns], &m_Tree[a*MaxColumns], MaxColumns*sizeof(Cell));

	ZeroMemory(&m_Tree[Row*MaxColumns], MaxColumns*sizeof(Cell));
	m_Rows++;

	return TRUE;
}

void CTreeView::SetItem(Cell* cell, IShellFolder* pParentFolder, LPITEMIDLIST pidlRel, LPITEMIDLIST pidlFQ)
{
	if (!cell->pItem)
	{
		cell->pItem = (ItemData*)malloc(sizeof(ItemData));
	}
	else
	{
		theApp.GetShellManager()->FreeItem(cell->pItem->pidlFQ);
		theApp.GetShellManager()->FreeItem(cell->pItem->pidlRel);
		if (cell->pItem->pParentFolder)
			cell->pItem->pParentFolder->Release();
	}

	if (!pidlRel)
		return;

	if (pParentFolder)
		pParentFolder->AddRef();

	cell->pItem->pidlFQ = pidlFQ ? pidlFQ : theApp.GetShellManager()->CopyItem(pidlRel);
	cell->pItem->pidlRel = pidlRel;
	cell->pItem->pParentFolder = pParentFolder;

	SHFILEINFO sfi;
	if (SUCCEEDED(SHGetFileInfo((LPCTSTR)pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_DISPLAYNAME)))
	{
		wcscpy_s(cell->pItem->Name, 256, sfi.szDisplayName);
	}
	else
	{
		wcscpy_s(cell->pItem->Name, 256, L"?");
	}

	cell->pItem->IconIDNormal =
		SUCCEEDED(SHGetFileInfo((LPCTSTR)pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_LINKOVERLAY))
		? sfi.iIcon : -1;

	cell->pItem->IconIDSelected =
		SUCCEEDED(SHGetFileInfo((LPCTSTR)pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_OPENICON))
		? sfi.iIcon : -1;

	if (!SHGetPathFromIDList(pidlFQ, cell->pItem->Path))
		cell->pItem->Path[0] = L'\0';
}

void CTreeView::FreeItem(Cell* cell)
{
	if (cell->pItem)
	{
		theApp.GetShellManager()->FreeItem(cell->pItem->pidlFQ);
		theApp.GetShellManager()->FreeItem(cell->pItem->pidlRel);
		if (cell->pItem->pParentFolder)
			cell->pItem->pParentFolder->Release();

		free(cell->pItem);
	}
}

void CTreeView::FreeTree()
{
	if (m_Tree)
	{
		Cell* cell = m_Tree;
		for (UINT a=0; a<m_Rows*MaxColumns; a++)
			FreeItem(cell++);

		_aligned_free(m_Tree);
		m_Tree = NULL;
	}

	m_Allocated = m_Rows = m_Cols = 0;
}


BEGIN_MESSAGE_MAP(CTreeView, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_LBUTTONDOWN()
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

	HDITEM HdItem;
	HdItem.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
	HdItem.cxy = MINWIDTH;
	HdItem.fmt = HDF_STRING | HDF_CENTER;

	for (UINT a=0; a<MaxColumns; a++)
	{
		HdItem.pszText = a ? L"Ignore" : L"";
		m_wndHeader.InsertItem(a, &HdItem);
	}

	return 0;
}

void CTreeView::OnDestroy()
{
	CWnd::OnDestroy();

	FreeTree();
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

void CTreeView::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();
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
