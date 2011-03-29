
// CTreeView.cpp: Implementierung der Klasse CTreeView
//

#include "stdafx.h"
#include "CTreeView.h"
#include "Migrate.h"
#include "ChoosePropertyDlg.h"
#include "LFCore.h"
#include "Resource.h"


// CTreeView
//

#define MINWIDTH          75
#define MAXWIDTH          350

#define BORDER            3
#define MARGIN            4
#define GUTTER            13

#define MAKEPOS(r, c)     (r)*MaxColumns+(c)
#define MAKEPOSI(p)       MAKEPOS(p.y, p.x)

CTreeView::CTreeView()
{
	m_Tree = NULL;
	p_Edit = NULL;
	m_Allocated = m_Rows = m_Cols = 0;
	hThemeList = hThemeButton = hThemeTree = NULL;
	m_SelectedItem.x = m_SelectedItem.y = m_HotItem.x = m_HotItem.y = m_HotExpando.x = m_HotExpando.y = m_EditLabel.x = m_EditLabel.y = -1;
	m_CheckboxHot = m_CheckboxPressed = m_Hover = m_SpacePressed = FALSE;
	m_pContextMenu2 = NULL;

	for (UINT a=0; a<MaxColumns; a++)
		m_ColumnMapping[a] = -1;

	pDesktop = NULL;
	SHGetDesktopFolder(&pDesktop);
}

CTreeView::~CTreeView()
{
	DestroyEdit();

	if (pDesktop)
		pDesktop->Release();
}

BOOL CTreeView::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

LRESULT CTreeView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITMENUPOPUP:
	case WM_DRAWITEM:
	case WM_MEASUREITEM:
		if (m_pContextMenu2)
		{
			m_pContextMenu2->HandleMenuMsg(message, wParam, lParam);
			return 0;
		}
	}

	return CWnd::WindowProc(message, wParam, lParam);
}

BOOL CTreeView::PreTranslateMessage(MSG* pMsg)
{
	switch (pMsg->message)
	{
	case WM_KEYDOWN:
		if (p_Edit)
			switch (pMsg->wParam)
			{
			case VK_EXECUTE:
			case VK_RETURN:
				DestroyEdit(TRUE);
				return TRUE;
			case VK_ESCAPE:
				DestroyEdit(FALSE);
				return TRUE;
			}
		break;
	case WM_MOUSEWHEEL:
	case WM_MOUSEHWHEEL:
		if (p_Edit)
			return TRUE;
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_NCLBUTTONDOWN:
	case WM_NCRBUTTONDOWN:
	case WM_NCMBUTTONDOWN:
	case WM_NCLBUTTONUP:
	case WM_NCRBUTTONUP:
	case WM_NCMBUTTONUP:
		m_TooltipCtrl.Deactivate();
		break;
	}

	return CWnd::PreTranslateMessage(pMsg);
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

	AdjustScrollbars();

	m_wndHeader.SetWindowPos(NULL, wp.x-m_HScrollPos, wp.y, wp.cx+m_HScrollMax+GetSystemMetrics(SM_CXVSCROLL), m_HeaderHeight, wp.flags | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CTreeView::ClearRoot()
{
	DestroyEdit();
	FreeTree();
	m_SelectedItem.x = m_SelectedItem.y = m_HotItem.x = m_HotItem.y = -1;
	m_CheckboxHot = m_CheckboxPressed = m_SpacePressed = FALSE;

	m_wndHeader.ModifyStyle(0, HDS_HIDDEN);
	ResetScrollbars();
	AdjustLayout();
	Invalidate();
}

void CTreeView::SetRoot(LPITEMIDLIST pidl, BOOL Update, BOOL ExpandAll)
{
	DestroyEdit();

	if (!Update)
	{
		FreeTree();
		InsertRow(0);
		m_Tree->Flags |= CF_CHECKED;

		for (INT col=m_wndHeader.GetItemCount()-1; col>=0; col--)
		{
			m_wndHeader.DeleteItem(col);
			m_ColumnWidth[col] = MINWIDTH;
		}
	}

	IShellFolder* pParentFolder = NULL;
	LPCITEMIDLIST pidlRel = NULL;
	if (SUCCEEDED(SHBindToParent(pidl, IID_IShellFolder, (void**)&pParentFolder, &pidlRel)))
	{
		SetItem(0, 0, theApp.GetShellManager()->CopyItem(pidlRel), theApp.GetShellManager()->CopyItem(pidl), m_Tree->Flags);

		if (!Update)
		{
			EnumObjects(0, 0, ExpandAll);
			AutosizeColumns();
			m_SelectedItem.x = m_SelectedItem.y = 0;
		}
		else
		{
			UpdateChildPIDLs(0, 0);
		}

		pParentFolder->Release();
	}

	m_wndHeader.ModifyStyle(HDS_HIDDEN, 0);
	AdjustLayout();
	Invalidate();
}

void CTreeView::SetBranchCheck(BOOL Check, CPoint item)
{
	if ((item.x==-1) || (item.y==-1))
		item = m_SelectedItem;
	if ((item.x==-1) || (item.y==-1) || (item.x>=(INT)m_Cols) || (item.y>=(INT)m_Rows))
		return;

	UINT LastRow = GetChildRect(item);
	if (Check)
	{
		m_Tree[MAKEPOSI(item)].Flags |= CF_CHECKED;
	}
	else
	{
		m_Tree[MAKEPOSI(item)].Flags &= ~CF_CHECKED;
	}

	for (UINT Row=item.y; Row<=LastRow; Row++)
		for (UINT Col=item.x+1; Col<m_Cols; Col++)
			if (Check)
			{
				m_Tree[MAKEPOS(Row, Col)].Flags |= CF_CHECKED;
			}
			else
			{
				m_Tree[MAKEPOS(Row, Col)].Flags &= ~CF_CHECKED;
			}

	Invalidate();
}

void CTreeView::ExpandFolder(CPoint item, BOOL ExpandAll)
{
	if ((item.x==-1) || (item.y==-1))
		item = m_SelectedItem;
	if ((item.x==-1) || (item.y==-1) || (item.x>=(INT)m_Cols) || (item.y>=(INT)m_Rows))
		return;

	if (m_Tree[MAKEPOSI(item)].Flags & CF_CANEXPAND)
		Expand(item.y, item.x, ExpandAll);
}

void CTreeView::OpenFolder(CPoint item)
{
	ExecuteContextMenu(item, "open");
}

void CTreeView::DeleteFolder(CPoint item)
{
	if (ExecuteContextMenu(item, "delete"))
		RemoveItem(item.y, item.x);
}

void CTreeView::ShowProperties(CPoint item)
{
	ExecuteContextMenu(item, "properties");
}

void CTreeView::AutosizeColumns()
{
	m_wndHeader.SetRedraw(FALSE);

	for (UINT col=0; col<m_Cols; col++)
		AutosizeColumn(col);

	m_wndHeader.SetRedraw(TRUE);
	m_wndHeader.Invalidate();

	AdjustScrollbars();
	Invalidate();
}

void CTreeView::EditLabel(CPoint item)
{
	if ((item.x==-1) || (item.y==-1))
		item = m_SelectedItem;
	if ((item.x==-1) || (item.y==-1) || (item.x>=(INT)m_Cols) || (item.y>=(INT)m_Rows))
	{
		m_EditLabel.x = m_EditLabel.y = -1;
		return;
	}
	if (!(m_Tree[MAKEPOSI(item)].Flags & CF_CANRENAME))
	{
		m_EditLabel.x = m_EditLabel.y = -1;
		return;
	}

	EnsureVisible(item);

	INT y = m_HeaderHeight+item.y*m_RowHeight;
	INT x = 0;
	for (INT a=0; a<item.x; a++)
		x += m_ColumnWidth[a];

	WCHAR Name[MAX_PATH];
	wcscpy_s(Name, MAX_PATH, m_Tree[MAKEPOSI(item)].pItem->Name);

	IShellFolder* pParentFolder = NULL;
	if (SUCCEEDED(SHBindToParent(m_Tree[MAKEPOSI(item)].pItem->pidlFQ, IID_IShellFolder, (void**)&pParentFolder, NULL)))
	{
		STRRET strret;
		if (SUCCEEDED(pParentFolder->GetDisplayNameOf(m_Tree[MAKEPOSI(item)].pItem->pidlRel, SHGDN_FOREDITING, &strret)))
			if (strret.uType==STRRET_WSTR)
				wcscpy_s(Name, MAX_PATH, strret.pOleStr);

		pParentFolder->Release();
	}

	CRect rect(x+m_CheckboxSize.cx+m_IconSize.cx+GUTTER+BORDER+2*MARGIN-5, y, x+m_ColumnWidth[item.x], y+m_RowHeight);

	p_Edit = new CEdit();
	p_Edit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_AUTOHSCROLL, rect, this, 2);
	p_Edit->SetWindowText(Name);
	p_Edit->SetSel(0, (INT)wcslen(Name));
	p_Edit->SetFont(&theApp.m_DefaultFont);
	p_Edit->SetFocus();
}

void CTreeView::EnsureVisible(CPoint item)
{
	if ((item.x==-1) || (item.y==-1))
		item = m_SelectedItem;
	if ((item.x==-1) || (item.y==-1) || (item.x>=(INT)m_Cols) || (item.y>=(INT)m_Rows))
		return;

	CRect rect;
	GetClientRect(&rect);

	SCROLLINFO si;
	INT nInc;

	// Vertikal
	nInc = 0;
	if ((INT)((item.y+1)*m_RowHeight)>m_VScrollPos+rect.Height()-(INT)m_HeaderHeight)
		nInc = (item.y+1)*m_RowHeight-rect.Height()+(INT)m_HeaderHeight-m_VScrollPos;
	if ((INT)(item.y*m_RowHeight)<m_VScrollPos+nInc)
		nInc = item.y*m_RowHeight-m_VScrollPos;

	nInc = max(-m_VScrollPos, min(nInc, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindowEx(0, -nInc, NULL, NULL, NULL, NULL, SW_INVALIDATE);

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_VScrollPos;
		SetScrollInfo(SB_VERT, &si);
	}

	// Horizontal
	INT x = 0;
	for (INT a=0; a<item.x; a++)
		x += m_ColumnWidth[a];

	nInc = 0;
	if (x+m_ColumnWidth[item.x]>m_HScrollPos+rect.Width())
		nInc = x+m_ColumnWidth[item.x]-rect.Width()-m_HScrollPos;
	if (x<m_HScrollPos+nInc)
		nInc = x-m_HScrollPos;

	nInc = max(-m_HScrollPos, min(nInc, m_HScrollMax-m_HScrollPos));
	if (nInc)
	{
		m_HScrollPos += nInc;
		ScrollWindowEx(-nInc, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE);

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_HScrollPos;
		SetScrollInfo(SB_HORZ, &si);
	}
}

void CTreeView::PopulateMigrationList(CMigrationList* ml, LFItemDescriptor* it, UINT row, UINT col)
{
	if ((row>=m_Rows) || (col>=m_Cols))
		return;

	Cell* cell = &m_Tree[MAKEPOS(row, col)];
	if (!cell->pItem)
		return;

	// Template ergänzen
	LFItemDescriptor* it2 = it;
	if (m_ColumnMapping[col]!=-1)
	{
		LFVariantData v;
		v.Attr = m_ColumnMapping[col];
		LFVariantDataFromString(&v, cell->pItem->Name);

		if (!v.IsNull)
		{
			it2 = LFAllocItemDescriptor(it);
			LFSetAttributeVariantData(it2, &v);
		}
	}

	// Ordner hinzufügen
	if ((cell->Flags & CF_CHECKED) && (cell->pItem->Path[0]!='\0'))
		ml->AddFolder(cell->pItem->Name, cell->pItem->Path, it2, cell->pItem->IconIDNormal, !(cell->Flags & CF_HASCHILDREN));

	// Unterordner
	col++;
	if ((cell->Flags & CF_HASCHILDREN) && (col<m_Cols))
	{
		UINT LastRow = GetChildRect(CPoint(col-1, row));
		for (UINT r=row; r<=LastRow; r++)
			if (m_Tree[MAKEPOS(r, col)].pItem)
				PopulateMigrationList(ml, it2, r, col);
	}

	// Template freigeben
	if (it2!=it)
		LFFreeItemDescriptor(it2);
}

void CTreeView::UncheckMigrated(CReportList* rl)
{
	for (UINT row=0; row<m_Rows; row++)
		for (UINT col=0; col<m_Cols; col++)
		{
			Cell* cell = &m_Tree[MAKEPOS(row, col)];

			if (cell->pItem)
			{
				for (UINT a=0; a<rl->m_ItemCount; a++)
					if (wcscmp(cell->pItem->Path, rl->m_Items[a]->Path)==0)
						cell->Flags &= ~CF_CHECKED;
			}
		}

	Invalidate();
}

BOOL CTreeView::FoldersChecked()
{
	for (UINT row=0; row<m_Rows; row++)
		for (UINT col=0; col<m_Cols; col++)
		{
			Cell* cell = &m_Tree[MAKEPOS(row, col)];

			if ((cell->pItem) && (cell->Flags & CF_CHECKED))
				return TRUE;
		}

	return FALSE;
}

void CTreeView::ResetScrollbars()
{
	ScrollWindowEx(0, m_VScrollPos, NULL, NULL, NULL, NULL, SW_INVALIDATE);
	ScrollWindowEx(m_HScrollPos, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE | SW_SCROLLCHILDREN);
	m_VScrollPos = m_HScrollPos = 0;
	SetScrollPos(SB_VERT, m_VScrollPos, TRUE);
	SetScrollPos(SB_HORZ, m_HScrollPos, TRUE);
}

void CTreeView::AdjustScrollbars()
{
	CRect rect;
	GetWindowRect(&rect);

	INT ScrollHeight = m_Rows*m_RowHeight;
	INT ScrollWidth = (m_Cols<MaxColumns) ? GUTTER : 0;
	for (UINT col=0; col<m_Cols; col++)
		ScrollWidth += m_ColumnWidth[col];

	BOOL HScroll = FALSE;
	if (ScrollWidth>rect.Width())
	{
		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);
		HScroll = TRUE;
	}
	if (ScrollHeight>rect.Height()-(INT)m_HeaderHeight)
		rect.right -= GetSystemMetrics(SM_CXVSCROLL);
	if ((ScrollWidth>rect.Width()) && (!HScroll))
		rect.bottom -= GetSystemMetrics(SM_CYHSCROLL);

	INT oldVScrollPos = m_VScrollPos;
	m_VScrollMax = max(0, ScrollHeight-rect.Height()+(INT)m_HeaderHeight);
	m_VScrollPos = min(m_VScrollPos, m_VScrollMax);

	SCROLLINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Height()-m_HeaderHeight;
	si.nMin = 0;
	si.nMax = ScrollHeight-1;
	si.nPos = m_VScrollPos;
	SetScrollInfo(SB_VERT, &si);

	INT oldHScrollPos = m_HScrollPos;
	m_HScrollMax = max(0, ScrollWidth-rect.Width());
	m_HScrollPos = min(m_HScrollPos, m_HScrollMax);

	ZeroMemory(&si, sizeof(si));
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_PAGE | SIF_RANGE | SIF_POS;
	si.nPage = rect.Width();
	si.nMin = 0;
	si.nMax = ScrollWidth-1;
	si.nPos = m_HScrollPos;
	SetScrollInfo(SB_HORZ, &si);

	if ((oldVScrollPos!=m_VScrollPos) || (oldHScrollPos!=m_HScrollPos))
		Invalidate();
}

BOOL CTreeView::InsertRow(UINT row)
{
	ASSERT(row<=m_Rows);

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

	if (row<m_Rows)
		for (UINT a=m_Rows-1; a>=row; a--)
			memcpy(&m_Tree[MAKEPOS(a+1, 0)], &m_Tree[MAKEPOS(a, 0)], MaxColumns*sizeof(Cell));

	ZeroMemory(&m_Tree[MAKEPOS(row, 0)], MaxColumns*sizeof(Cell));
	m_Rows++;
	m_HotItem.x = m_HotItem.y = -1;
	if (m_SelectedItem.y>=(INT)row)
		m_SelectedItem.y++;

	return TRUE;
}

void CTreeView::RemoveRows(UINT first, UINT last)
{
	if (first>last)
		return;

	if ((m_HotItem.y>=(INT)first) && (m_HotItem.y<=(INT)last))
		m_TooltipCtrl.Deactivate();

	for (UINT row=first; row<=last; row++)
		for (UINT col=0; col<m_Cols; col++)
			FreeItem(&m_Tree[MAKEPOS(row, col)]);

	memcpy(&m_Tree[MAKEPOS(first, 0)], &m_Tree[MAKEPOS(last+1, 0)], (m_Rows-last)*MaxColumns*sizeof(Cell));

	if (m_SelectedItem.y>(INT)last)
		m_SelectedItem.y -= last-first+1;
	m_HotItem.x = m_HotItem.y = -1;
	m_Rows -= last-first+1;
	ZeroMemory(&m_Tree[MAKEPOS(m_Rows, 0)], (last-first+1)*MaxColumns*sizeof(Cell));
}

void CTreeView::UpdateChildPIDLs(UINT row, UINT col)
{
	if (col>=m_Cols-1)
		return;
	if (!(m_Tree[MAKEPOS(row, col)].Flags & CF_HASCHILDREN))
		return;

	UINT LastRow = GetChildRect(CPoint(col, row));
	LPITEMIDLIST pidlParent = m_Tree[MAKEPOS(row, col)].pItem->pidlFQ;

	while (row<=LastRow)
	{
		ItemData* pItem = m_Tree[MAKEPOS(row, col+1)].pItem;
		if (pItem)
		{
			theApp.GetShellManager()->FreeItem(pItem->pidlFQ);
			pItem->pidlFQ = theApp.GetShellManager()->ConcatenateItem(pidlParent, pItem->pidlRel);

			if (!SHGetPathFromIDList(pItem->pidlFQ, pItem->Path))
				pItem->Path[0] = L'\0';

			UpdateChildPIDLs(row, col+1);
		}

		row++;
	}
}

void CTreeView::SetItem(UINT row, UINT col, LPITEMIDLIST pidlRel, LPITEMIDLIST pidlFQ, UINT Flags)
{
	ASSERT(row<m_Rows);
	ASSERT(col<MaxColumns);

	if (!pidlRel)
		return;

	if (col>=m_Cols)
		m_Cols = col+1;

	Cell* cell = &m_Tree[MAKEPOS(row, col)];
	if (!cell->pItem)
	{
		cell->pItem = (ItemData*)malloc(sizeof(ItemData));
	}
	else
	{
		theApp.GetShellManager()->FreeItem(cell->pItem->pidlFQ);
		theApp.GetShellManager()->FreeItem(cell->pItem->pidlRel);
	}

	cell->Flags = Flags;
	cell->pItem->pidlFQ = pidlFQ ? pidlFQ : theApp.GetShellManager()->CopyItem(pidlRel);
	cell->pItem->pidlRel = pidlRel;

	SHFILEINFO sfi;
	if (SUCCEEDED(SHGetFileInfo((LPCTSTR)pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_LINKOVERLAY | SHGFI_DISPLAYNAME | SHGFI_ATTRIBUTES)))
	{
		wcscpy_s(cell->pItem->Name, 256, sfi.szDisplayName);
		cell->pItem->IconIDNormal = sfi.iIcon;

		if ((!(cell->Flags & CF_HASCHILDREN)) && (sfi.dwAttributes & SFGAO_HASSUBFOLDER))
			cell->Flags |= CF_CANEXPAND;
		if (sfi.dwAttributes & SFGAO_HASPROPSHEET)
			cell->Flags |= CF_HASPROPSHEET;
		if (sfi.dwAttributes & SFGAO_CANRENAME)
			cell->Flags |= CF_CANRENAME;
		if (sfi.dwAttributes & SFGAO_CANDELETE)
			cell->Flags |= CF_CANDELETE;
	}
	else
	{
		wcscpy_s(cell->pItem->Name, 256, L"?");
		cell->pItem->IconIDNormal = -1;
		cell->Flags &= ~(CF_CANEXPAND | CF_HASPROPSHEET | CF_CANRENAME | CF_CANDELETE);
	}

	CDC* pDC = GetWindowDC();
	CFont* pOldFont = pDC->SelectObject(&theApp.m_DefaultFont);
	CSize sz = pDC->GetTextExtent(cell->pItem->Name, (INT)wcslen(cell->pItem->Name));
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

	cell->pItem->Width = sz.cx;

	cell->pItem->IconIDSelected =
		SUCCEEDED(SHGetFileInfo((LPCTSTR)pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_OPENICON))
		? sfi.iIcon : -1;

	if (!SHGetPathFromIDList(pidlFQ, cell->pItem->Path))
		cell->pItem->Path[0] = L'\0';
}

void CTreeView::RemoveItem(UINT row, UINT col)
{
	ASSERT(row<m_Rows);
	ASSERT(col<MaxColumns);

	if (!col)
	{
		ClearRoot();
		return;
	}

	UINT LastRow = GetChildRect(CPoint(col, row));
	if (((m_SelectedItem.y>=(INT)row+1) && (m_SelectedItem.y<=(INT)LastRow)) || ((m_SelectedItem.y==(INT)row) && (m_SelectedItem.x>=(INT)col)))
	{
		CPoint item(m_SelectedItem);

		item.x = col-1;
		for (INT r=row; r>=0; r--)
			if (m_Tree[MAKEPOS(r, item.x)].pItem)
			{
				item.y = r;
				break;
			}

		SelectItem(item);
	}

	BOOL HasSiblings = (m_Tree[MAKEPOS(row, col)].Flags & CF_HASSIBLINGS);
	if (m_Tree[MAKEPOS(row, col)].Flags & CF_ISSIBLING)
	{
		if (!HasSiblings)
			m_Tree[MAKEPOS(row-1, col)].Flags &= ~CF_HASSIBLINGS;

		RemoveRows(row, LastRow);
	}
	else
		if (HasSiblings)
		{
			for (UINT c=col; c<m_Cols; c++)
				std::swap(m_Tree[MAKEPOS(row, c)], m_Tree[MAKEPOS(LastRow+1, c)]);

			RemoveRows(row+1, LastRow+1);

			m_Tree[MAKEPOS(row, col)].Flags &= ~CF_ISSIBLING;
		}
		else
		{
			for (UINT c=col; c<m_Cols; c++)
				FreeItem(&m_Tree[MAKEPOS(row, c)]);

			m_Tree[MAKEPOS(row, col-1)].Flags &= ~(CF_CANCOLLAPSE | CF_CANEXPAND | CF_HASCHILDREN);
		}

	m_HotItem.x = m_HotItem.y = -1;

	AdjustScrollbars();
	Invalidate();
}

UINT CTreeView::EnumObjects(UINT row, UINT col, BOOL ExpandAll, BOOL FirstInstance)
{
	if (!(m_Tree[MAKEPOS(row, col)].Flags & CF_CANEXPAND))
		return 0;
	if (col>=MaxColumns-1)
		return 0;

	Cell* cell = &m_Tree[MAKEPOS(row, col)];
	cell->Flags &= ~CF_CANEXPAND;

	IShellFolder* pDesktop = NULL;
	if (FAILED(SHGetDesktopFolder(&pDesktop)))
		return 0;

	IShellFolder* pParentFolder = NULL;
	if (theApp.GetShellManager()->GetItemSize(cell->pItem->pidlFQ)==2)
	{
		pParentFolder = pDesktop;
		pDesktop->AddRef();
	}
	else
		if (FAILED(pDesktop->BindToObject(cell->pItem->pidlFQ, NULL, IID_IShellFolder, (void**)&pParentFolder)))
		{
			pDesktop->Release();
			return 0;
		}

	UINT Inserted = 0;

	IEnumIDList* pEnum;
	if (SUCCEEDED(pParentFolder->EnumObjects(NULL, SHCONTF_FOLDERS, &pEnum)))
	{
		CWaitCursor wait;

		BOOL NewRow = FALSE;
		UINT Flags = ExpandAll ? (cell->Flags & CF_CHECKED) : 0;

		LPITEMIDLIST pidlTemp;
		while (pEnum->Next(1, &pidlTemp, NULL)==S_OK)
		{
			DWORD dwAttributes = SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM;
			pParentFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlTemp, &dwAttributes);

			// Don't include virtual branches
			if (!(dwAttributes & (SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM)))
				continue;

			// Don't include liquidFOLDERS
			SHDESCRIPTIONID did;
			if (SUCCEEDED(SHGetDataFromIDList(pParentFolder, pidlTemp, SHGDFIL_DESCRIPTIONID, &did, sizeof(SHDESCRIPTIONID))))
			{
				const CLSID LFNE = { 0x3F2D914F, 0xFE57, 0x414F, { 0x9F, 0x88, 0xA3, 0x77, 0xC7, 0x84, 0x1D, 0xA4 } };
				if (did.clsid==LFNE)
					continue;
			}

			// Don't include file junctions
			LPITEMIDLIST pidlFQ = theApp.GetShellManager()->ConcatenateItem(m_Tree[MAKEPOS(row, col)].pItem->pidlFQ, pidlTemp);

			WCHAR Path[MAX_PATH];
			if (SUCCEEDED(SHGetPathFromIDListW(pidlFQ, Path)))
			{
				DWORD attr = GetFileAttributesW(Path);
				if ((attr!=INVALID_FILE_ATTRIBUTES) && (!(attr & FILE_ATTRIBUTE_DIRECTORY)))
				{
					theApp.GetShellManager()->FreeItem(pidlFQ);
					continue;
				}
			}

			if (NewRow)
			{
				m_Tree[MAKEPOS(row+Inserted, col+1)].Flags |= CF_HASSIBLINGS;

				Inserted++;
				InsertRow(row+Inserted);
				Flags |= CF_ISSIBLING;
			}
			else
			{
				m_Tree[MAKEPOS(row, col)].Flags |= (CF_HASCHILDREN | CF_CANCOLLAPSE);
			}

			SetItem(row+Inserted, col+1, pidlTemp, pidlFQ, Flags);
			if (ExpandAll)
				Inserted += EnumObjects(row+Inserted, col+1, TRUE, FALSE);

			NewRow = TRUE;
		}

		pEnum->Release();
	}

	pParentFolder->Release();
	pDesktop->Release();

	if (FirstInstance)
		for (UINT a=min(row+Inserted, m_Rows-2); a>row; a--)
			for (UINT b=1; b<m_Cols; b++)
				if (m_Tree[MAKEPOS(a+1, b)].Flags & CF_ISSIBLING)
				{
					m_Tree[MAKEPOS(a, b)].Flags |= CF_HASSIBLINGS;
					if (!m_Tree[MAKEPOS(a, b-1)].pItem)
						m_Tree[MAKEPOS(a, b)].Flags |= CF_ISSIBLING;
				}

	return Inserted;
}

void CTreeView::Expand(UINT row, UINT col, BOOL ExpandAll, BOOL AutosizeHeader)
{
	ASSERT(m_Tree[MAKEPOS(row, col)].Flags & CF_CANEXPAND);

	EnumObjects(row, col, ExpandAll);

	if (AutosizeHeader)
		for (UINT a=(INT)col+1; a<m_Cols; a++)
			AutosizeColumn(a, TRUE);

	AdjustScrollbars();
	Invalidate();
}

void CTreeView::Collapse(UINT row, UINT col)
{
	ASSERT(m_Tree[MAKEPOS(row, col)].Flags & CF_CANCOLLAPSE);

	m_Tree[MAKEPOS(row, col)].Flags &= ~CF_CANCOLLAPSE;
	m_Tree[MAKEPOS(row, col)].Flags |= CF_CANEXPAND;

	UINT LastRow = GetChildRect(CPoint(col, row));
	if (((m_SelectedItem.y>=(INT)row+1) && (m_SelectedItem.y<=(INT)LastRow)) || ((m_SelectedItem.y==(INT)row) && (m_SelectedItem.x>(INT)col)))
		m_SelectedItem = CPoint(col, row);

	RemoveRows(row+1, LastRow);

	for (UINT c=col+1; c<m_Cols; c++)
		FreeItem(&m_Tree[MAKEPOS(row, c)]);

	AdjustScrollbars();
	Invalidate();
}

void CTreeView::FreeItem(Cell* cell)
{
	if (cell->pItem)
	{
		theApp.GetShellManager()->FreeItem(cell->pItem->pidlFQ);
		theApp.GetShellManager()->FreeItem(cell->pItem->pidlRel);

		free(cell->pItem);
		cell->pItem = NULL;
	}

	cell->Flags = 0;
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

BOOL CTreeView::HitTest(CPoint point, CPoint* item, BOOL* cbhot, CPoint* exphot)
{
	BOOL res = FALSE;
	BOOL onitem = FALSE;
	BOOL onexpando = FALSE;

	point.y -= m_HeaderHeight-m_VScrollPos;
	INT row = (point.y>=0) ? point.y/m_RowHeight : -1;
	INT col = -1;
	INT x = -m_HScrollPos;

	if (row!=-1)
	{
		for (UINT a=0; a<min(m_Cols+1, MaxColumns); a++)
		{
			if ((point.x>=x) && (point.x<x+m_ColumnWidth[a]))
			{
				col = a;
				onitem = (point.x>=x+GUTTER) && (col<(INT)m_Cols);
				break;
			}

			x += m_ColumnWidth[a];
		}
	}

	if ((row>=0) && (row<(INT)m_Rows) && (col!=-1))
	{
		if (onitem)
		{
			res = (m_Tree[MAKEPOS(row, col)].pItem!=NULL);
			if ((res) && (cbhot))
			{
				CRect rectButton(x+GUTTER+BORDER, row*m_RowHeight+(m_RowHeight-m_CheckboxSize.cy)/2, x+GUTTER+BORDER+m_CheckboxSize.cx, row*m_RowHeight+(m_RowHeight-m_CheckboxSize.cy)/2+m_CheckboxSize.cy);
				*cbhot = rectButton.PtInRect(point);
			}
		}
		else
			if (col)
				if (m_Tree[MAKEPOS(row, col-1)].Flags & (CF_CANEXPAND | CF_CANCOLLAPSE))
				{
					CRect rectGlyph(x, row*m_RowHeight+(m_RowHeight-m_GlyphSize.cy-2)/2, x+GUTTER, row*m_RowHeight+(m_RowHeight-m_GlyphSize.cy-2)/2+m_GlyphSize.cy+2);
					onexpando = rectGlyph.PtInRect(point);
				}
	}

	if (item)
	{
		item->x = (res && onitem) ? col : -1;
		item->y = (res && onitem) ? row : -1;
	}

	if (exphot)
	{
		exphot->x = onexpando ? col : -1;
		exphot->y = onexpando ? row : -1;
	}

	return res;
}

void CTreeView::InvalidateItem(CPoint Item)
{
	if ((Item.x!=-1) && (Item.y!=-1))
	{
		INT x = -m_HScrollPos;
		for (UINT a=0; a<(UINT)Item.x; a++)
			x += m_ColumnWidth[a];

		CRect rect(x, m_HeaderHeight+Item.y*m_RowHeight-m_VScrollPos, x+m_ColumnWidth[Item.x], m_HeaderHeight-m_VScrollPos+(Item.y+1)*m_RowHeight);
		InvalidateRect(rect);
	}
}

void CTreeView::TrackMenu(UINT nID, CPoint point, INT col)
{
	CMenu menu;
	ENSURE(menu.LoadMenu(nID));

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	if (!col)
		pPopup->EnableMenuItem(IDD_CHOOSEPROPERTY, MF_GRAYED | MF_DISABLED);

	if ((!col) || (m_ColumnMapping[col]==-1))
		pPopup->EnableMenuItem(IDM_TREE_RESETPROPERTY, MF_GRAYED | MF_DISABLED);

	BOOL Enable = FALSE;
	for (UINT row=0; row<m_Rows; row++)
		if (m_Tree[MAKEPOS(row, col)].Flags & CF_CANEXPAND)
		{
			Enable = TRUE;
			break;
		}

	if (!Enable)
		pPopup->EnableMenuItem(IDM_TREE_EXPANDCOLUMN, MF_GRAYED | MF_DISABLED);

	switch (pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD, point.x, point.y, this))
	{
	case IDM_TREE_AUTOSIZE:
		AutosizeColumn(col);
		AdjustScrollbars();
		Invalidate();
		break;
	case IDM_VIEW_AUTOSIZEALL:
		AutosizeColumns();
		break;
	case IDM_TREE_EXPANDCOLUMN:
		ExpandColumn(col);
		break;
	case IDD_CHOOSEPROPERTY:
		PostMessage(IDD_CHOOSEPROPERTY, (WPARAM)col);
		break;
	case IDM_TREE_RESETPROPERTY:
		m_ColumnMapping[col] = -1;
		UpdateColumnCaption(col);
		break;
	}
}

void CTreeView::SetWidgetSize()
{
	if (hThemeButton)
	{
		CDC* dc = GetDC();
		theApp.zGetThemePartSize(hThemeButton, *dc, BP_CHECKBOX, CBS_UNCHECKEDDISABLED, NULL, TS_DRAW, &m_CheckboxSize);
		theApp.zGetThemePartSize(hThemeTree, *dc, TVP_GLYPH, GLPS_CLOSED, NULL, TS_DRAW, &m_GlyphSize);
		ReleaseDC(dc);
	}
	else
	{
		m_CheckboxSize.cx = GetSystemMetrics(SM_CXMENUCHECK);
		m_CheckboxSize.cy = GetSystemMetrics(SM_CYMENUCHECK);
		m_GlyphSize.cx = m_GlyphSize.cy = 11;
	}
}

UINT CTreeView::GetChildRect(CPoint item)
{
	UINT row = item.y;

	while (row<m_Rows)
	{
		for (INT col=0; col<=item.x; col++)
			if (m_Tree[MAKEPOS(row+1, col)].pItem)
				return row;

		row++;
	}

	return row-1;
}

void CTreeView::SelectItem(CPoint Item)
{
	if (Item==m_SelectedItem)
		return;

	if (!m_Tree[MAKEPOSI(Item)].pItem)
		return;

	InvalidateItem(m_SelectedItem);
	m_SelectedItem = Item;
	EnsureVisible(Item);
	InvalidateItem(Item);
	m_EditLabel = CPoint(-1, -1);

	ReleaseCapture();
}

void CTreeView::DeletePath(LPWSTR Path)
{
	CPoint item(0, 0);
	while ((item.x<(INT)m_Cols) && (item.y<(INT)m_Rows))
	{
		if (m_Tree[MAKEPOSI(item)].pItem)
			if (wcscmp(m_Tree[MAKEPOSI(item)].pItem->Path, Path)==0)
				RemoveItem(item.y, item.x);

		item.y++;
		if (item.y>=(INT)m_Rows)
		{
			item.x++;
			item.y = 0;
		}
	}
}

void CTreeView::AddPath(LPWSTR Path, LPWSTR Parent)
{
	CPoint item(0, 0);
	while ((item.x<min((INT)m_Cols, MaxColumns-1)) && (item.y<(INT)m_Rows))
	{
		if (m_Tree[MAKEPOSI(item)].pItem)
			if (wcscmp(m_Tree[MAKEPOSI(item)].pItem->Path, Parent)==0)
			{
				ULONG chEaten;
				LPITEMIDLIST pidlFQ = NULL;
				if (SUCCEEDED(pDesktop->ParseDisplayName(NULL, NULL, Path, &chEaten, &pidlFQ, NULL)))
				{
					IShellFolder* pParentFolder = NULL;
					LPCITEMIDLIST pidlRel = NULL;
					if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder, (void**)&pParentFolder, &pidlRel)))
					{
						CPoint add(item.x+1, item.y);
						if ((INT)m_Cols<=item.x)
							m_Cols = item.x+1;

						UINT Flags = 0;
						if (!(m_Tree[MAKEPOSI(item)].Flags & CF_HASCHILDREN))
						{
							m_Tree[MAKEPOSI(item)].Flags |= (CF_HASCHILDREN | CF_CANCOLLAPSE);
						}
						else
						{
							UINT LastRow = GetChildRect(item);
							BOOL Insert = TRUE;
							while (add.y<=(INT)LastRow)
							{
								if (m_Tree[MAKEPOSI(add)].pItem)
									if (wcscmp(m_Tree[MAKEPOSI(add)].pItem->Path, Path)==0)
									{
										Insert = FALSE;
										break;
									}

								add.y++;
							}

							if (Insert)
							{
								InsertRow(add.y);

								Flags = CF_ISSIBLING;
								for (INT row=add.y-1; row>item.y; row--)
									m_Tree[MAKEPOS(row, add.x)].Flags |= CF_HASSIBLINGS | CF_ISSIBLING;

								if (add.y<(INT)m_Rows-1)
									for (UINT col=1; (INT)col<add.x; col++)
									if (m_Tree[MAKEPOS(add.y+1, col)].Flags & CF_ISSIBLING)
									{
										m_Tree[MAKEPOS(add.y, col)].Flags |= CF_HASSIBLINGS;
										if (!m_Tree[MAKEPOS(add.y, col-1)].pItem)
											m_Tree[MAKEPOS(add.y, col)].Flags |= CF_ISSIBLING;
									}
							}
							else
							{
								Flags = m_Tree[MAKEPOSI(add)].Flags;
							}
						}

						SetItem(add.y, add.x, theApp.GetShellManager()->CopyItem(pidlRel), pidlFQ, Flags);

						AutosizeColumn(add.x, TRUE);
						AdjustScrollbars();
						InvalidateItem(add);
						InvalidateItem(item);

						pParentFolder->Release();
					}
				}
		}

		item.y++;
		if (item.y>=(INT)m_Rows)
		{
			item.x++;
			item.y = 0;
		}
	}
}

void CTreeView::UpdatePath(LPWSTR Path1, LPWSTR Path2, IShellFolder* pDesktop)
{
	CPoint item(0, 0);
	while ((item.x<(INT)m_Cols) && (item.y<(INT)m_Rows))
	{
		if (m_Tree[MAKEPOSI(item)].pItem)
			if (wcscmp(m_Tree[MAKEPOSI(item)].pItem->Path, Path1)==0)
			{
				ULONG chEaten;
				LPITEMIDLIST pidlFQ = NULL;
				if (SUCCEEDED(pDesktop->ParseDisplayName(NULL, NULL, Path2, &chEaten, &pidlFQ, NULL)))
				{
					IShellFolder* pParentFolder = NULL;
					LPCITEMIDLIST pidlRel = NULL;
					if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder, (void**)&pParentFolder, &pidlRel)))
					{
						SetItem(item.y, item.x, theApp.GetShellManager()->CopyItem(pidlRel), pidlFQ, m_Tree[MAKEPOSI(item)].Flags);
						InvalidateItem(item);
						UpdateChildPIDLs(item.y, item.x);

						pParentFolder->Release();
					}
				}
			}

		item.y++;
		if (item.y>=(INT)m_Rows)
		{
			item.x++;
			item.y = 0;
		}
	}
}

BOOL CTreeView::ExecuteContextMenu(CPoint& item, LPCSTR verb)
{
	if ((item.x==-1) || (item.y==-1))
		item = m_SelectedItem;
	if ((item.x==-1) || (item.y==-1) || (item.x>=(INT)m_Cols) || (item.y>=(INT)m_Rows))
		return FALSE;

	DestroyEdit();

	Cell* cell = &m_Tree[MAKEPOSI(item)];
	if (!cell->pItem)
		return FALSE;

	IShellFolder* pParentFolder = NULL;
	if (FAILED(SHBindToParent(cell->pItem->pidlFQ, IID_IShellFolder, (void**)&pParentFolder, NULL)))
		return FALSE;

	IContextMenu* pcm = NULL;
	if (SUCCEEDED(pParentFolder->GetUIObjectOf(GetSafeHwnd(), 1, (LPCITEMIDLIST*)&cell->pItem->pidlRel, IID_IContextMenu, NULL, (void**)&pcm)))
	{
		HMENU hPopup = CreatePopupMenu();
		if (hPopup)
		{
			UINT uFlags = CMF_NORMAL | CMF_EXPLORE;
			if (SUCCEEDED(pcm->QueryContextMenu(hPopup, 0, 1, 0x6FFF, uFlags)))
			{
				CWaitCursor wait;

				CMINVOKECOMMANDINFO cmi;
				cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
				cmi.fMask = 0;
				cmi.hwnd = GetSafeHwnd();
				cmi.lpVerb = verb;
				cmi.lpParameters = NULL;
				cmi.lpDirectory = NULL;
				cmi.nShow = SW_SHOWNORMAL;
				cmi.dwHotKey = 0;
				cmi.hIcon = NULL;

				return SUCCEEDED(pcm->InvokeCommand(&cmi));
			}
		}
	}

	return FALSE;
}

CString CTreeView::GetColumnCaption(UINT col)
{
	CString tmpStr;

	if (col)
		if (m_ColumnMapping[col]!=-1)
		{
			tmpStr = theApp.m_Attributes[m_ColumnMapping[col]]->Name;
		}
		else
		{
			ENSURE(tmpStr.LoadString(IDS_NOPROPERTY));
		}

	return tmpStr;
}

void CTreeView::UpdateColumnCaption(UINT col)
{
	ASSERT(col<m_Cols);

	CString caption = GetColumnCaption(col);

	HDITEM HdItem;
	HdItem.mask = HDI_TEXT;
	HdItem.pszText = caption.GetBuffer();
	m_wndHeader.SetItem(col, &HdItem);
}

void CTreeView::ExpandColumn(UINT col)
{
	SetRedraw(FALSE);

	for (UINT row=0; row<m_Rows; row++)
		if (m_Tree[MAKEPOS(row, col)].Flags & CF_CANEXPAND)
			Expand(row, col, FALSE, FALSE);


	for (UINT a=(INT)col+1; a<m_Cols; a++)
		AutosizeColumn(a, TRUE);

	SetRedraw(TRUE);
	m_wndHeader.Invalidate();

	AdjustScrollbars();
	Invalidate();
}

void CTreeView::AutosizeColumn(UINT col, BOOL OnlyEnlarge)
{
	DestroyEdit();

	INT Width = 0;
	for (UINT row=0; row<m_Rows; row++)
		if (m_Tree[MAKEPOS(row, col)].pItem)
			Width = max(Width, m_Tree[MAKEPOS(row, col)].pItem->Width);

	Width += GUTTER+2*BORDER+m_CheckboxSize.cx+m_IconSize.cx+3*MARGIN;
	m_ColumnWidth[col] = min(OnlyEnlarge ? max(Width, m_ColumnWidth[col]) : Width, MAXWIDTH);

	if (m_wndHeader.GetItemCount()>(INT)col)
	{
		HDITEM HdItem;
		HdItem.mask = HDI_WIDTH;
		HdItem.cxy = m_ColumnWidth[col];
		m_wndHeader.SetItem(col, &HdItem);
	}
	else
		while (m_wndHeader.GetItemCount()<=(INT)col)
		{
			INT idx = m_wndHeader.GetItemCount();
			CString caption = GetColumnCaption(idx);

			HDITEM HdItem;
			HdItem.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
			HdItem.fmt = HDF_STRING | HDF_CENTER;
			HdItem.cxy = m_ColumnWidth[idx];
			HdItem.pszText = caption.GetBuffer();
			m_wndHeader.InsertItem(idx, &HdItem);
		}
}

void CTreeView::DestroyEdit(BOOL Accept)
{
	if (p_Edit)
	{
		CPoint item = m_EditLabel;
		if ((item.x==-1) || (item.y==-1))
			item = m_SelectedItem;

		CEdit* victim = p_Edit;
		p_Edit = NULL;

		CString Name;
		victim->GetWindowText(Name);
		victim->DestroyWindow();
		delete victim;

		if ((Accept) && (!Name.IsEmpty()) && (item.x!=-1) && (item.y!=-1))
		{
			Cell* cell = &m_Tree[MAKEPOSI(item)];
			if (cell->pItem)
				if (Name!=cell->pItem->Name)
				{
					IShellFolder* pParentFolder = NULL;
					if (SUCCEEDED(SHBindToParent(cell->pItem->pidlFQ, IID_IShellFolder, (void**)&pParentFolder, NULL)))
					{
						LPITEMIDLIST pidlRel = NULL;
						if (SUCCEEDED(pParentFolder->SetNameOf(m_hWnd, cell->pItem->pidlRel, Name, SHGDN_NORMAL, &pidlRel)))
						{
							LPITEMIDLIST pidlParent = NULL;
							theApp.GetShellManager()->GetParentItem(cell->pItem->pidlFQ, pidlParent);

							SetItem(item.y, item.x, pidlRel, theApp.GetShellManager()->ConcatenateItem(pidlParent, pidlRel), cell->Flags);
							UpdateChildPIDLs(item.y, item.x);
							InvalidateItem(item);

							theApp.GetShellManager()->FreeItem(pidlParent);
						}

						pParentFolder->Release();
					}
				}
		}
	}

	m_EditLabel.x = m_EditLabel.y = -1;
}


BEGIN_MESSAGE_MAP(CTreeView, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_VSCROLL()
	ON_WM_HSCROLL()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
	ON_WM_MOUSEWHEEL()
	ON_WM_MOUSEHWHEEL()
	ON_WM_KEYDOWN()
	ON_WM_KEYUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_RBUTTONDOWN()
	ON_WM_CONTEXTMENU()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	ON_NOTIFY(HDN_BEGINDRAG, 1, OnBeginDrag)
	ON_NOTIFY(HDN_ITEMCHANGING, 1, OnItemChanging)
	ON_NOTIFY(HDN_ITEMCLICK, 1, OnItemClick)
	ON_EN_KILLFOCUS(2, OnDestroyEdit)
	ON_MESSAGE(IDD_CHOOSEPROPERTY, OnChooseProperty)
	ON_COMMAND(IDM_VIEW_OPEN, OnOpen)
	ON_COMMAND(IDM_VIEW_DELETE, OnDelete)
	ON_COMMAND(IDM_VIEW_RENAME, OnRename)
	ON_COMMAND(IDM_VIEW_PROPERTIES, OnProperties)
	ON_COMMAND(IDM_VIEW_AUTOSIZEALL, OnAutosizeAll)
	ON_COMMAND(IDM_VIEW_EXPAND, OnExpand)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_VIEW_OPEN, IDM_VIEW_EXPAND, OnUpdateCommands)
	ON_MESSAGE(WM_SHELLCHANGE, OnShellChange)
END_MESSAGE_MAP()

INT CTreeView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | HDS_FLAT | HDS_HIDDEN | HDS_HORZ | HDS_FULLDRAG | HDS_BUTTONS | CCS_TOP | CCS_NOMOVEY | CCS_NODIVIDER;
	CRect rect;
	rect.SetRectEmpty();
	if (!m_wndHeader.Create(dwStyle, rect, this, 1))
		return -1;

	m_TooltipCtrl.Create(this);

	if (theApp.m_ThemeLibLoaded)
	{
		hThemeButton = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
		if (theApp.OSVersion>=OS_Vista)
		{
			theApp.zSetWindowTheme(GetSafeHwnd(), L"explorer", NULL);
			hThemeList = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
		}
		hThemeTree = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_TREEVIEW);
	}

	SetWidgetSize();

	IMAGEINFO ii;
	theApp.m_SystemImageListSmall.GetImageInfo(0, &ii);
	m_IconSize.cx = ii.rcImage.right-ii.rcImage.left;
	m_IconSize.cy = ii.rcImage.bottom-ii.rcImage.top;

	m_DefaultGlyphs.Create(IDB_TREEWIDGETS, 11, 1, 0xFFFFFF);

	LOGFONT lf;
	theApp.m_DefaultFont.GetLogFont(&lf);
	m_RowHeight = (4+max(abs(lf.lfHeight), m_IconSize.cy)) & ~1;

	for (UINT a=0; a<MaxColumns; a++)
		m_ColumnWidth[a] = MINWIDTH;

	// Benachrichtigung, wenn sich Items ändern
	SHChangeNotifyEntry shCNE = { NULL, TRUE };
	m_ulSHChangeNotifyRegister = SHChangeNotifyRegister(m_hWnd, SHCNRF_InterruptLevel | SHCNRF_ShellLevel,
		SHCNE_DRIVEREMOVED | SHCNE_MEDIAREMOVED | SHCNE_MKDIR | SHCNE_RMDIR |
		SHCNE_RENAMEFOLDER | SHCNE_UPDATEITEM | SHCNE_INTERRUPT,
		WM_SHELLCHANGE, 1, &shCNE);

	ResetScrollbars();

	return 0;
}

void CTreeView::OnDestroy()
{
	if (m_ulSHChangeNotifyRegister)
		VERIFY(SHChangeNotifyDeregister(m_ulSHChangeNotifyRegister));

	if (hThemeButton)
		theApp.zCloseThemeData(hThemeButton);
	if (hThemeList)
		theApp.zCloseThemeData(hThemeList);
	if (hThemeTree)
		theApp.zCloseThemeData(hThemeTree);

	FreeTree();

	CWnd::OnDestroy();
}

LRESULT CTreeView::OnThemeChanged()
{
	if (theApp.m_ThemeLibLoaded)
	{
		if (hThemeButton)
			theApp.zCloseThemeData(hThemeButton);
		if (hThemeList)
			theApp.zCloseThemeData(hThemeList);
		if (hThemeTree)
			theApp.zCloseThemeData(hThemeTree);

		hThemeButton = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
		if (theApp.OSVersion>=OS_Vista)
			hThemeList = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
	}

	SetWidgetSize();
	return TRUE;
}

BOOL CTreeView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CTreeView::OnPaint()
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

	BOOL Themed = IsCtrlThemed();
	COLORREF bkCol = Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
	dc.FillSolidRect(rect, bkCol);

	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	LOGBRUSH brsh;
	brsh.lbColor = 0x808080;
	brsh.lbStyle = PS_SOLID;
	CPen pen(PS_COSMETIC | PS_ALTERNATE, 1, &brsh);
	CPen* pOldPen = dc.SelectObject(&pen);

	INT start = m_VScrollPos/m_RowHeight;
	INT y = m_HeaderHeight-(m_VScrollPos % m_RowHeight);
	Cell* curCell = &m_Tree[MAKEPOS(start, 0)];
	for (UINT row=start; row<m_Rows; row++)
	{
		INT x = -m_HScrollPos;
		for (UINT col=0; col<MaxColumns; col++)
		{
			CRect rectItem(x, y, x+m_ColumnWidth[col], y+m_RowHeight);
			CRect rectIntersect;
			if (rectIntersect.IntersectRect(rectItem, rectUpdate))
			{
				rectItem.left += GUTTER;

				BOOL Hot = (m_HotItem.x==(INT)col) && (m_HotItem.y==(INT)row);
				BOOL Selected = (m_SelectedItem.x==(INT)col) && (m_SelectedItem.y==(INT)row);

				if (curCell->pItem)
				{
					if (hThemeList)
					{
						if (Hot | Selected)
						{
							const INT StateIDs[4] = { LISS_NORMAL, LISS_HOT, GetFocus()!=this ? LISS_SELECTEDNOTFOCUS : LISS_SELECTED, LISS_HOTSELECTED };
							UINT State = 0;
							if (Hot)
								State |= 1;
							if (Selected)
								State |= 2;
								theApp.zDrawThemeBackground(hThemeList, dc, LVP_LISTITEM, StateIDs[State], rectItem, rectItem);
						}

						dc.SetTextColor(curCell->pItem->Path[0] ? 0x000000 : 0x808080);
					}
					else
						if (Selected)
						{
							dc.FillSolidRect(rectItem, GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHT : COLOR_3DFACE));
							dc.SetTextColor(GetSysColor(GetFocus()==this ? COLOR_HIGHLIGHTTEXT : COLOR_WINDOWTEXT));

							if (GetFocus()==this)
							{
								dc.SetBkColor(0x000000);
								dc.DrawFocusRect(rectItem);
							}
						}
						else
						{
							dc.SetTextColor(curCell->pItem->Path[0] ? Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT) : Themed ? 0x808080 : GetSysColor(COLOR_GRAYTEXT));
						}

					rectItem.left += m_CheckboxSize.cx+BORDER+MARGIN;
					theApp.m_SystemImageListSmall.Draw(&dc, Selected ? curCell->pItem->IconIDSelected : curCell->pItem->IconIDNormal, CPoint(rectItem.left, y+(m_RowHeight-m_IconSize.cy)/2), ILD_TRANSPARENT);
					rectItem.left += m_IconSize.cx+MARGIN;
					rectItem.right -= BORDER;

					CRect rectButton(x+GUTTER+BORDER, y+(m_RowHeight-m_CheckboxSize.cy)/2, x+GUTTER+BORDER+m_CheckboxSize.cx, y+(m_RowHeight-m_CheckboxSize.cy)/2+m_CheckboxSize.cy);
					if (hThemeButton)
					{
						INT uiStyle;
						if (curCell->pItem->Path[0])
						{
							uiStyle = (Selected && (m_SpacePressed || m_CheckboxPressed)) ? CBS_UNCHECKEDPRESSED : (Hot && m_CheckboxHot) ? CBS_UNCHECKEDHOT : CBS_UNCHECKEDNORMAL;
							if (curCell->Flags & CF_CHECKED)
								uiStyle += 4;
						}
						else
						{
							uiStyle = CBS_UNCHECKEDDISABLED;
						}
						theApp.zDrawThemeBackground(hThemeButton, dc, BP_CHECKBOX, uiStyle, rectButton, rectButton);
					}
					else
					{
						UINT uiStyle = DFCS_BUTTONCHECK;
						if (curCell->pItem->Path[0])
						{
							uiStyle |= (curCell->Flags & CF_CHECKED ? DFCS_CHECKED : 0) | ((Selected && (m_SpacePressed || m_CheckboxPressed)) ? DFCS_PUSHED : 0);
						}
						else
						{
							uiStyle |= DFCS_INACTIVE;
						}
						dc.DrawFrameControl(rectButton, DFC_BUTTON, uiStyle);
					}

					dc.DrawText(curCell->pItem->Name, -1, rectItem, DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

					dc.MoveTo(x+((curCell->Flags & CF_ISSIBLING) ? GUTTER/2 : 0), y+m_RowHeight/2);
					dc.LineTo(x+GUTTER+BORDER-1, y+m_RowHeight/2);

					if (curCell->Flags & (CF_HASCHILDREN | CF_CANEXPAND))
					{
						INT right = x+GUTTER+BORDER+m_CheckboxSize.cx+m_IconSize.cx+2*MARGIN+curCell->pItem->Width+1;
						if (right<x+m_ColumnWidth[col])
						{
							dc.MoveTo(x+m_ColumnWidth[col], y+m_RowHeight/2);
							dc.LineTo(right, y+m_RowHeight/2);
						}
					}
				}

				if (curCell->Flags & CF_HASSIBLINGS)
				{
					dc.MoveTo(x+GUTTER/2, y+m_RowHeight/2);
					dc.LineTo(x+GUTTER/2, y+m_RowHeight);
				}

				if (curCell->Flags & CF_ISSIBLING)
				{
					dc.MoveTo(x+GUTTER/2, y);
					dc.LineTo(x+GUTTER/2, y+m_RowHeight/2);
				}

				if (col)
					if ((curCell-1)->Flags & (CF_CANEXPAND | CF_CANCOLLAPSE))
					{
						dc.MoveTo(x, y+m_RowHeight/2);
						dc.LineTo(x+2, y+m_RowHeight/2);

						CRect rectGlyph(x, y+(m_RowHeight-m_GlyphSize.cy)/2, x+m_GlyphSize.cx, y+(m_RowHeight-m_GlyphSize.cy)/2+m_GlyphSize.cy);
						if (hThemeTree)
						{
							if (theApp.OSVersion==OS_XP)
							{
								rectGlyph.OffsetRect(2, 1);
							}
							else
							{
								rectGlyph.OffsetRect(1-m_GlyphSize.cx/4, 0);
							}

							BOOL Hot = (m_HotExpando.x==(INT)col) && (m_HotExpando.y==(INT)row) && (theApp.OSVersion>OS_XP);
							theApp.zDrawThemeBackground(hThemeTree, dc, Hot ? TVP_HOTGLYPH : TVP_GLYPH, (curCell-1)->Flags & CF_CANEXPAND ? GLPS_CLOSED : GLPS_OPENED, rectGlyph, rectGlyph);
						}
						else
						{
							rectGlyph.OffsetRect(1, 0);
							m_DefaultGlyphs.Draw(&dc, (curCell-1)->Flags & CF_CANEXPAND ? 0 : 1, rectGlyph.TopLeft(), 0);
						}
					}
			}

			x += m_ColumnWidth[col];
			curCell++;
		}

		y += m_RowHeight;
		if (y>rect.Height())
			break;
	}

	if (!m_Rows)
	{
		CRect rectText(rect);
		rectText.top += m_HeaderHeight+6;

		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_NOTHINGTODISPLAY));

		dc.SetTextColor(Themed ? 0x6D6D6D : GetSysColor(COLOR_3DFACE));
		dc.DrawText(tmpStr, -1, rectText, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldPen);
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}

void CTreeView::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CTreeView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	CRect rect;
	GetClientRect(&rect);

	SCROLLINFO si;

	INT nInc = 0;
	switch (nSBCode)
	{
	case SB_TOP:
		nInc = -m_VScrollPos;
		break;
	case SB_BOTTOM:
		nInc = m_VScrollMax-m_VScrollPos;
		break;
	case SB_LINEUP:
		nInc = -((INT)m_RowHeight);
		break;
	case SB_LINEDOWN:
		nInc = m_RowHeight;
		break;
	case SB_PAGEUP:
		nInc = min(-1, -(rect.Height()-(INT)m_HeaderHeight));
		break;
	case SB_PAGEDOWN:
		nInc = max(1, rect.Height()-(INT)m_HeaderHeight);
		break;
	case SB_THUMBTRACK:
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_VERT, &si);

		nInc = si.nTrackPos-m_VScrollPos;
		break;
	}

	nInc = max(-m_VScrollPos, min(nInc, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_VScrollPos += nInc;
		ScrollWindowEx(0, -nInc, NULL, NULL, NULL, NULL, SW_INVALIDATE);

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_VScrollPos;
		SetScrollInfo(SB_VERT, &si);

		if (p_Edit)
		{
			CRect rect;
			p_Edit->GetWindowRect(&rect);
			ScreenToClient(&rect);

			rect.OffsetRect(0, -nInc);
			p_Edit->SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

void CTreeView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO si;

	INT nInc = 0;
	switch (nSBCode)
	{
	case SB_TOP:
		nInc = -m_HScrollPos;
		break;
	case SB_BOTTOM:
		nInc = m_HScrollMax-m_HScrollPos;
		break;
	case SB_PAGEUP:
	case SB_LINEUP:
		nInc = -64;
		break;
	case SB_PAGEDOWN:
	case SB_LINEDOWN:
		nInc = 64;
		break;
	case SB_THUMBTRACK:
		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_TRACKPOS;
		GetScrollInfo(SB_HORZ, &si);

		nInc = si.nTrackPos-m_HScrollPos;
		break;
	}

	nInc = max(-m_HScrollPos, min(nInc, m_HScrollMax-m_HScrollPos));
	if (nInc)
	{
		m_HScrollPos += nInc;
		ScrollWindowEx(-nInc, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE | SW_SCROLLCHILDREN);

		ZeroMemory(&si, sizeof(si));
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_POS;
		si.nPos = m_HScrollPos;
		SetScrollInfo(SB_HORZ, &si);

		UpdateWindow();
	}

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}

BOOL CTreeView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	return TRUE;
}

void CTreeView::OnMouseMove(UINT nFlags, CPoint point)
{
	BOOL Dragging = (GetCapture()==this);
	BOOL Pressed = FALSE;
	CPoint Item(-1, -1);
	CPoint Expando(-1, -1);
	BOOL OnItem = HitTest(point, &Item, Dragging ? &Pressed : &m_CheckboxHot, &Expando);

	if (!m_Hover)
	{
		m_Hover = TRUE;

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = LFHOVERTIME;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
	else
		if ((m_TooltipCtrl.IsWindowVisible()) && (Item!=m_HotItem))
			m_TooltipCtrl.Deactivate();

	if (!Dragging)
	{
		if (m_HotItem!=Item)
		{
			InvalidateItem(m_HotItem);
			m_HotItem = Item;
			InvalidateItem(m_HotItem);
		}
		if (m_HotExpando!=Expando)
		{
			InvalidateItem(m_HotExpando);
			m_HotExpando = Expando;
			InvalidateItem(m_HotExpando);
		}

		if ((OnItem) && (nFlags & MK_RBUTTON))
		{
			SetFocus();
			m_SelectedItem = m_HotItem;
		}
	}

	m_CheckboxPressed = (Item==m_SelectedItem) && Pressed && Dragging;
}

void CTreeView::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	InvalidateItem(m_HotItem);
	InvalidateItem(m_HotExpando);

	m_Hover = FALSE;
	m_HotItem.x = m_HotItem.y = -1;
}

void CTreeView::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if ((m_HotItem.x!=-1) && (m_HotItem.y!=-1) && (!m_CheckboxHot) && (!p_Edit))
			if (m_HotItem==m_EditLabel)
			{
				m_TooltipCtrl.Deactivate();
				EditLabel(m_EditLabel);
			}
			else
				if (!m_TooltipCtrl.IsWindowVisible())
				{
					HICON hIcon = NULL;
					CSize size(0, 0);
					CString caption;
					CString hint;
					TooltipDataFromPIDL(m_Tree[MAKEPOSI(m_HotItem)].pItem->pidlFQ, &theApp.m_SystemImageListExtraLarge, hIcon, size, caption, hint);

					ClientToScreen(&point);
					m_TooltipCtrl.Track(point, hIcon, size, caption, hint);
				}
	}
	else
	{
		m_TooltipCtrl.Deactivate();
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE | TME_HOVER;
	tme.dwHoverTime = LFHOVERTIME;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
}

BOOL CTreeView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(&rect);
	if (!rect.PtInRect(pt))
		return FALSE;

	INT nInc = max(-m_VScrollPos, min(-zDelta*(INT)m_RowHeight/WHEEL_DELTA, m_VScrollMax-m_VScrollPos));
	if (nInc)
	{
		m_TooltipCtrl.Deactivate();

		m_VScrollPos += nInc;
		ScrollWindowEx(0, -nInc, NULL, NULL, NULL, NULL, SW_INVALIDATE);
		SetScrollPos(SB_VERT, m_VScrollPos, TRUE);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}

	return TRUE;
}

void CTreeView::OnMouseHWheel(UINT nFlags, short zDelta, CPoint pt)
{
	CRect rect;
	GetWindowRect(&rect);
	if (!rect.PtInRect(pt))
		return;

	INT nInc = max(-m_HScrollPos, min(zDelta*64/WHEEL_DELTA, m_HScrollMax-m_HScrollPos));
	if (nInc)
	{
		m_TooltipCtrl.Deactivate();

		m_HScrollPos += nInc;
		ScrollWindowEx(-nInc, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE);
		SetScrollPos(SB_HORZ, m_HScrollPos, TRUE);

		ScreenToClient(&pt);
		OnMouseMove(nFlags, pt);
	}
}

void CTreeView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_Rows)
	{
		CRect rect;
		GetClientRect(&rect);

		CPoint item(m_SelectedItem);

		switch (nChar)
		{
		case VK_F2:
			EditLabel();
			break;
		case VK_EXECUTE:
		case VK_RETURN:
			if (m_Tree[MAKEPOSI(item)].Flags & CF_CANEXPAND)
			{
				ExpandFolder();
			}
			else
			{
				OpenFolder();
			}

			break;
		case VK_DELETE:
			if (m_Tree[MAKEPOSI(item)].Flags & CF_CANDELETE)
				DeleteFolder();

			break;
		case VK_SPACE:
			if (m_Tree[MAKEPOSI(item)].pItem->Path[0])
			{
				m_SpacePressed = TRUE;
				InvalidateItem(item);
			}

			break;
		case VK_LEFT:
			if (GetKeyState(VK_CONTROL)<0)
			{
				if (m_Tree[MAKEPOSI(item)].Flags & CF_CANCOLLAPSE)
					Collapse(item.y, item.x);
			}
			else
				if (item.x)
				{
					item.x--;
					for (INT row=item.y; row>=0; row--)
						if (m_Tree[MAKEPOS(row, item.x)].pItem)
						{
							item.y = row;
							break;
						}
				}

			break;
		case VK_RIGHT:
			if (m_Tree[MAKEPOSI(item)].Flags & CF_CANEXPAND)
				Expand(item.y, item.x, FALSE);

			if ((item.x<(INT)m_Cols-1) && (GetKeyState(VK_CONTROL)>=0))
				for (INT row=item.y; row<(INT)m_Rows; row++)
					if (m_Tree[MAKEPOS(row, item.x+1)].pItem)
					{
						item.x++;
						item.y = row;
						break;
					}

			break;
		case VK_UP:
			for (INT row=item.y-1; row>=0; row--)
				if (m_Tree[MAKEPOS(row, item.x)].pItem)
				{
					item.y = row;
					break;
				}

			break;
		case VK_PRIOR:
			for (INT row=item.y-1; row>=0; row--)
				if (m_Tree[MAKEPOS(row, item.x)].pItem)
				{
					item.y = row;
					if (row<=m_SelectedItem.y-rect.Height()/(INT)m_RowHeight)
						break;
				}

			break;
		case VK_DOWN:
			for (INT row=item.y+1; row<(INT)m_Rows; row++)
				if (m_Tree[MAKEPOS(row, item.x)].pItem)
				{
					item.y = row;
					break;
				}

			break;
		case VK_NEXT:
			for (INT row=item.y+1; row<(INT)m_Rows; row++)
				if (m_Tree[MAKEPOS(row, item.x)].pItem)
				{
					item.y = row;
					if (row>=m_SelectedItem.y+rect.Height()/(INT)m_RowHeight)
						break;
				}

			break;
		case VK_HOME:
			if (GetKeyState(VK_CONTROL)<0)
			{
				item.x = item.y = 0;
			}
			else
				for (INT col=item.x; col>=0; col--)
					if (m_Tree[MAKEPOS(item.y, col)].pItem)
					{
						item.x = col;
					}
					else
						break;

			break;
		case VK_END:
			if (GetKeyState(VK_CONTROL)<0)
			{
				item.x = m_Cols-1;
				item.y = m_Rows-1;
				while ((item.x>0) || (item.y>0))
				{
					if (m_Tree[MAKEPOS(item.y, item.x)].pItem)
						break;

					item.y--;
					if (item.y<0)
					{
						item.y = m_Rows-1;
						item.x--;
					}
				}
			}
			else
				for (INT col=item.x; col<(INT)m_Cols; col++)
					if (m_Tree[MAKEPOS(item.y, col)].pItem)
					{
						item.x = col;
					}
					else
						break;

			break;
		default:
			CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
			return;
		}

		SelectItem(item);
	}
}

void CTreeView::OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (m_Rows)
		switch (nChar)
		{
		case VK_SPACE:
			if (m_SpacePressed)
			{
				if (m_Tree[MAKEPOSI(m_SelectedItem)].pItem->Path[0])
				{
					m_Tree[MAKEPOSI(m_SelectedItem)].Flags ^= CF_CHECKED;
					InvalidateItem(m_SelectedItem);
				}

				m_SpacePressed = FALSE;
			}

			break;
		default:
			CWnd::OnKeyUp(nChar, nRepCnt, nFlags);
		}
}

void CTreeView::OnLButtonDown(UINT nFlags, CPoint point)
{
	SetFocus();

	CPoint Item;
	CPoint Expando;
	if (HitTest(point, &Item, &m_CheckboxHot, &Expando))
	{
		if ((Item==m_SelectedItem) && (!m_CheckboxHot))
		{
			m_EditLabel = m_SelectedItem;
		}
		else
		{
			SelectItem(Item);
		}

		if (m_CheckboxHot)
		{
			m_CheckboxPressed = TRUE;
			SetCapture();

			InvalidateItem(Item);
		}
	}
	else
		if ((Expando.x!=-1) && (Expando.y!=-1))
			if (m_Tree[MAKEPOS(Expando.y, Expando.x-1)].Flags & CF_CANEXPAND)
			{
				Expand(Expando.y, Expando.x-1, nFlags & MK_CONTROL);
			}
			else
			{
				Collapse(Expando.y, Expando.x-1);
			}
}

void CTreeView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (GetCapture()==this)
	{
		CPoint Item;
		if (HitTest(point, &Item, &m_CheckboxPressed, &m_HotExpando))
			if ((Item==m_SelectedItem) && (m_CheckboxPressed))
				if (nFlags & MK_CONTROL)
				{
					m_Tree[MAKEPOSI(Item)].Flags ^= CF_CHECKED;
					InvalidateItem(Item);
				}
				else
				{
					SetBranchCheck(!(m_Tree[MAKEPOSI(Item)].Flags & CF_CHECKED), Item);
				}

		m_CheckboxPressed = FALSE;
		ReleaseCapture();
	}
}

void CTreeView::OnLButtonDblClk(UINT /*nFlags*/, CPoint point)
{
	m_CheckboxPressed = FALSE;
	ReleaseCapture();

	CPoint Item;
	BOOL Checkbox;
	if (HitTest(point, &Item, &Checkbox, &m_HotExpando))
		if ((Item==m_SelectedItem) && (!Checkbox))
			if (m_Tree[MAKEPOSI(Item)].Flags & CF_CANEXPAND)
			{
				ExpandFolder();
			}
			else
			{
				OpenFolder();
			}
}

void CTreeView::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	SetFocus();

	CPoint Item;
	if (HitTest(point, &Item, &m_CheckboxHot, &m_HotExpando))
		SelectItem(Item);
}

void CTreeView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (m_pContextMenu2)
		return;

	CPoint item(-1, -1);
	if ((point.x<0) || (point.y<0))
	{
		if ((m_SelectedItem.x==-1) || (m_SelectedItem.y==-1))
		{
			GetParent()->SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(point.x, point.y));
			return;
		}

		item = m_SelectedItem;

		point.x = GUTTER;
		for (INT a=0; a<item.x; a++)
			point.x += m_ColumnWidth[a];
		point.y = (item.y+1)*m_RowHeight+m_HeaderHeight+1;
		ClientToScreen(&point);
	}
	else
	{
		CPoint ptClient(point);
		ScreenToClient(&ptClient);

		if (pWnd->GetSafeHwnd()==m_wndHeader)
		{
			HDHITTESTINFO htt;
			htt.pt = ptClient;
			TrackMenu(IDM_HEADER, point, m_wndHeader.HitTest(&htt));
			return;
		}

		if (!HitTest(ptClient, &item, NULL, NULL))
		{
			GetParent()->SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(point.x, point.y));
			return;
		}
	}

	if ((item.x==-1) || (item.y==-1) || (item.x>=(INT)m_Cols) || (item.y>=(INT)m_Rows))
		return;

	Cell* cell = &m_Tree[MAKEPOSI(item)];
	if (!cell->pItem)
		return;

	IShellFolder* pParentFolder = NULL;
	if (FAILED(SHBindToParent(cell->pItem->pidlFQ, IID_IShellFolder, (void**)&pParentFolder, NULL)))
		return;

	IContextMenu* pcm = NULL;
	if (SUCCEEDED(pParentFolder->GetUIObjectOf(GetSafeHwnd(), 1, (LPCITEMIDLIST*)&cell->pItem->pidlRel, IID_IContextMenu, NULL, (void**)&pcm)))
	{
		HMENU hPopup = CreatePopupMenu();
		if (hPopup)
		{
			UINT uFlags = CMF_NORMAL | CMF_CANRENAME;
			if (SUCCEEDED(pcm->QueryContextMenu(hPopup, 0, 1, 0x6FFF, uFlags)))
			{
				InsertMenu(hPopup, 0, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);

				CString tmpStr;
				ENSURE(tmpStr.LoadString(IDS_INCLUDEBRANCH));
				InsertMenu(hPopup, 0, MF_BYPOSITION, 0x7001, tmpStr);

				ENSURE(tmpStr.LoadString(IDS_EXCLUDEBRANCH));
				InsertMenu(hPopup, 1, MF_BYPOSITION, 0x7002, tmpStr);

				if (cell->Flags & CF_CANEXPAND)
				{
					ENSURE(tmpStr.LoadString(IDS_EXPAND));
					InsertMenu(hPopup, 0, MF_BYPOSITION, 0x7003, tmpStr);

					ENSURE(tmpStr.LoadString(IDS_EXPANDBRANCH));
					InsertMenu(hPopup, 1, MF_BYPOSITION, 0x7004, tmpStr);

					SetMenuDefaultItem(hPopup, 0x7003, FALSE);
				}

				if (cell->Flags & CF_CANCOLLAPSE)
				{
					ENSURE(tmpStr.LoadString(IDS_COLLAPSE));
					InsertMenu(hPopup, 0, MF_BYPOSITION, 0x7005, tmpStr);
				}

				if (item.x)
				{
					CString tmpStr;
					ENSURE(tmpStr.LoadString(IDS_CHOOSEPROPERTY));
					InsertMenu(hPopup, 0, MF_BYPOSITION, 0x7000, tmpStr);

					InsertMenu(hPopup, 1, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
				}

				pcm->QueryInterface(IID_IContextMenu2, (void**)&m_pContextMenu2);
				UINT idCmd = TrackPopupMenu(hPopup, TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, 0, GetSafeHwnd(), NULL);
				if (m_pContextMenu2)
				{
					m_pContextMenu2->Release();
					m_pContextMenu2 = NULL;
				}

				switch (idCmd)
				{
				case 0x7000:
					OnChooseProperty((WPARAM)item.x, NULL);
					break;
				case 0x7001:
					SetBranchCheck(TRUE, item);
					break;
				case 0x7002:
					SetBranchCheck(FALSE, item);
					break;
				case 0x7003:
					ExpandFolder(item, FALSE);
					break;
				case 0x7004:
					ExpandFolder(item, TRUE);
					break;
				case 0x7005:
					Collapse(item.y, item.x);
					break;
				case 0:
					break;
				default:
					{
						CHAR Verb[256] = "";
						pcm->GetCommandString(idCmd-1, GCS_VERBA, NULL, Verb, 256);

						if (strcmp(Verb, "rename")==0)
						{
							EditLabel(item);
						}
						else
						{
							CWaitCursor wait;

							CMINVOKECOMMANDINFO cmi;
							cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
							cmi.fMask = 0;
							cmi.hwnd = GetSafeHwnd();
							cmi.lpVerb = (LPCSTR)(INT_PTR)(idCmd-1);
							cmi.lpParameters = NULL;
							cmi.lpDirectory = NULL;
							cmi.nShow = SW_SHOWNORMAL;
							cmi.dwHotKey = 0;
							cmi.hIcon = NULL;

							pcm->InvokeCommand(&cmi);

							SetFocus();
						}
					}
				}
			}
		}

		pcm->Release();
	}

	pParentFolder->Release();
}

void CTreeView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	InvalidateItem(m_SelectedItem);
}

void CTreeView::OnKillFocus(CWnd* /*pNewWnd*/)
{
	m_SpacePressed = FALSE;
	InvalidateItem(m_SelectedItem);
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

		m_ColumnWidth[pHdr->iItem] = pHdr->pitem->cxy;
		AdjustLayout();
		Invalidate();

		*pResult = FALSE;
	}
}

void CTreeView::OnItemClick(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMHEADER pHdr = (LPNMHEADER)pNMHDR;

	if (pHdr->iItem)
		PostMessage(IDD_CHOOSEPROPERTY, (WPARAM)pHdr->iItem);

	*pResult = NULL;
}

void CTreeView::OnDestroyEdit()
{
	DestroyEdit(TRUE);
}

LRESULT CTreeView::OnChooseProperty(WPARAM wParam, LPARAM /*lParam*/)
{
	ASSERT((INT)wParam>0);
	ASSERT((INT)wParam<MaxColumns);

	ChoosePropertyDlg dlg(theApp.m_pMainWnd, m_ColumnMapping[(INT)wParam]);
	if (dlg.DoModal()!=IDCANCEL)
	{
		for (UINT a=0; a<MaxColumns; a++)
			if (a==(UINT)wParam)
			{
				m_ColumnMapping[a] = dlg.m_Attr;
				UpdateColumnCaption(a);
			}
			else
				if ((m_ColumnMapping[a]==dlg.m_Attr) && (theApp.m_Attributes[dlg.m_Attr]->Type!=LFTypeUnicodeArray))
				{
					m_ColumnMapping[a] = -1;
					UpdateColumnCaption(a);
				}
	}

	return NULL;
}

void CTreeView::OnOpen()
{
	OpenFolder();
}

void CTreeView::OnDelete()
{
	DeleteFolder();
}

void CTreeView::OnRename()
{
	EditLabel();
}

void CTreeView::OnProperties()
{
	ShowProperties();
}

void CTreeView::OnAutosizeAll()
{
	AutosizeColumns();
}

void CTreeView::OnExpand()
{
	ExpandFolder();
}

void CTreeView::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	if ((m_SelectedItem.x!=-1) && (m_SelectedItem.y!=-1))
	{
		Cell* cell = &m_Tree[MAKEPOSI(m_SelectedItem)];

		switch (pCmdUI->m_nID)
		{
		case IDM_VIEW_OPEN:
			b = TRUE;
			break;
		case IDM_VIEW_DELETE:
			b = (cell->Flags & CF_CANDELETE);
			break;
		case IDM_VIEW_RENAME:
			b = (cell->Flags & CF_CANRENAME);
			break;
		case IDM_VIEW_PROPERTIES:
			b = (cell->Flags & CF_HASPROPSHEET);
			break;
		case IDM_VIEW_AUTOSIZEALL:
			b = TRUE;
			break;
		case IDM_VIEW_EXPAND:
			b = (cell->Flags & CF_CANEXPAND);
			break;
		}
	}

	pCmdUI->Enable(b);
}

LRESULT CTreeView::OnShellChange(WPARAM wParam, LPARAM lParam)
{
	LPITEMIDLIST* pidls = (LPITEMIDLIST*)wParam;

	WCHAR Path1[MAX_PATH] = L"";
	WCHAR Path2[MAX_PATH] = L"";
	WCHAR Parent1[MAX_PATH] = L"";
	WCHAR Parent2[MAX_PATH] = L"";

	IShellFolder* pDesktop = NULL;
	if (FAILED(SHGetDesktopFolder(&pDesktop)))
		return NULL;

	SHGetPathFromIDList(pidls[0], Path1);

	wcscpy_s(Parent1, MAX_PATH, Path1);
	WCHAR* last = wcsrchr(Parent1, L'\\');
	if (last<=&Parent1[2])
		last = &Parent1[3];
	*last = '\0';

	if (pidls[1])
	{
		SHGetPathFromIDList(pidls[1], Path2);

		wcscpy_s(Parent2, MAX_PATH, Path2);
		last = wcsrchr(Parent2, L'\\');
		if (last<=&Parent2[2])
			last = &Parent2[3];
		*last = '\0';
	}

	switch (lParam)
	{
	case SHCNE_MKDIR:
		if ((Path1[0]!='\0') && (Parent1[0]!='\0') && (wcscmp(Path1, Parent1)!=0))
			AddPath(Path1, Parent1);
		break;
	case SHCNE_DRIVEREMOVED:
	case SHCNE_MEDIAREMOVED:
	case SHCNE_RMDIR:
		if (Path1[0]!='\0')
			DeletePath(Path1);
		break;
	case SHCNE_RENAMEFOLDER:
		if ((Path1[0]!='\0') && (Path2[0]!='\0'))
			if (wcscmp(Parent1, Parent2)==0)
			{
				UpdatePath(Path1, Path2, pDesktop);
			}
			else
			{
				DeletePath(Path1);
				if ((Parent2[0]!='\0') && (wcscmp(Path2, Parent2)!=0))
					AddPath(Path2, Parent2);
			}
		break;
	case SHCNE_UPDATEITEM:
		wcscpy_s(Path2, MAX_PATH, Parent1);
		wcscat_s(Path2, MAX_PATH, L"\\desktop.ini");
		if (wcscmp(Path1, Path2)==0)
			UpdatePath(Parent1, Parent1, pDesktop);
		break;
	}

	pDesktop->Release();

	return NULL;
}
