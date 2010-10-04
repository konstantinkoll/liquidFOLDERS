
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
#define GUTTER            9

#define MAKEPOS(r, c)     (r)*MaxColumns+(c)
#define MAKEPOSI(p)       MAKEPOS(p.y, p.x)

CTreeView::CTreeView()
{
	m_Tree = NULL;
	p_Edit = NULL;
	m_Allocated = m_Rows = m_Cols = 0;
	hThemeList = hThemeButton = NULL;
	m_Selected.x = m_Selected.y = m_Hot.x = m_Hot.y = -1;
	m_CheckboxHot = m_CheckboxPressed = m_Hover = FALSE;
	m_pContextMenu2 = NULL;
	m_EditLabel = CPoint(-1, -1);

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

BOOL CTreeView::Create(CWnd* _pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP | WS_HSCROLL | WS_VSCROLL;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), dwStyle, rect, _pParentWnd, nID);
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
			case VK_RETURN:
				DestroyEdit(TRUE);
			case VK_ESCAPE:
				DestroyEdit();
				return TRUE;
			}
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
	case WM_MOUSEWHEEL:
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
	m_wndHeader.SetWindowPos(NULL, wp.x, wp.y, wp.cx, m_HeaderHeight, wp.flags | SWP_NOZORDER | SWP_NOACTIVATE);
}

void CTreeView::ClearRoot()
{
	DestroyEdit();
	FreeTree();
	m_Selected.x = m_Selected.y = m_Hot.x = m_Hot.y = -1;
	m_CheckboxHot = m_CheckboxPressed = FALSE;

	m_wndHeader.ModifyStyle(0, HDS_HIDDEN);
	AdjustLayout();
	Invalidate();

	NotifyOwner();
}

void CTreeView::SetRoot(LPITEMIDLIST pidl, BOOL Update)
{
	DestroyEdit();

	if (!Update)
	{
		FreeTree();
		InsertRow(0);

		for (int col=m_wndHeader.GetItemCount()-1; col>=0; col--)
			m_wndHeader.DeleteItem(col);
	}

	HRESULT hr;
	IShellFolder* pParentFolder = NULL;
	LPCITEMIDLIST pidlRel = NULL;
	if (theApp.GetShellManager()->GetItemSize(pidl)==2)
	{
		pidlRel = theApp.GetShellManager()->CopyItem(pidl);
		hr = S_OK;
	}
	else
	{
		hr = SHBindToParent(pidl, IID_IShellFolder, (void**)&pParentFolder, &pidlRel);
	}

	if (SUCCEEDED(hr))
	{
		if (!Update)
		{
			InsertItem(0, 0, pParentFolder, theApp.GetShellManager()->CopyItem(pidlRel), theApp.GetShellManager()->CopyItem(pidl), CF_CHECKED);

			for (UINT a=0; a<m_Cols; a++)
				AutosizeColumn(a);

			m_Selected.x = m_Selected.y = 0;
		}
		else
		{
			SetItem(0, 0, theApp.GetShellManager()->CopyItem(pidlRel), theApp.GetShellManager()->CopyItem(pidl), m_Tree->Flags);
		}

		if (pParentFolder)
		pParentFolder->Release();
	}

	m_wndHeader.ModifyStyle(HDS_HIDDEN, 0);
	AdjustLayout();
	SetFocus();
	Invalidate();

	NotifyOwner();
}

void CTreeView::SetBranchCheck(BOOL Check, CPoint item)
{
	if ((item.x==-1) || (item.y==-1))
		item = m_Selected;
	if ((item.x==-1) || (item.y==-1) || (item.x>=(int)m_Cols) || (item.y>=(int)m_Rows))
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

void CTreeView::OpenFolder(CPoint item)
{
	ExecuteContextMenu(item, "open");
}

void CTreeView::DeleteFolder(CPoint item)
{
	ExecuteContextMenu(item, "delete");
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

	Invalidate();

	m_wndHeader.SetRedraw(TRUE);
	m_wndHeader.Invalidate();
}

void CTreeView::EditLabel(CPoint item)
{
	if ((item.x==-1) || (item.y==-1))
		item = m_Selected;
	if ((item.x==-1) || (item.y==-1) || (item.x>=(int)m_Cols) || (item.y>=(int)m_Rows))
		return;

	int y = m_HeaderHeight+item.y*m_RowHeight;
	int x = 0;
	for (int a=0; a<item.x; a++)
		x += m_ColumnWidth[a];

	wchar_t* Name = m_Tree[MAKEPOSI(item)].pItem->Name;

	CRect rect(x+m_CheckboxSize.cx+m_IconSize.cx+GUTTER+BORDER+2*MARGIN-5, y, x+m_ColumnWidth[item.x], y+m_RowHeight);

	p_Edit = new CEdit();
	p_Edit->Create(WS_CHILD | WS_VISIBLE | WS_BORDER | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | ES_AUTOHSCROLL, rect, this, 2);
	p_Edit->SetWindowText(Name);
	p_Edit->SetSel(0, wcslen(Name));
	p_Edit->SetFont(&theApp.m_DefaultFont);
	p_Edit->SetFocus();
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
		for (UINT a=m_Rows; a>=Row; a--)
			memcpy(&m_Tree[MAKEPOS(a+1, 0)], &m_Tree[MAKEPOS(a, 0)], MaxColumns*sizeof(Cell));

	ZeroMemory(&m_Tree[MAKEPOS(Row, 0)], MaxColumns*sizeof(Cell));
	m_Rows++;

	return TRUE;
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
	if (SUCCEEDED(SHGetFileInfo((LPCTSTR)pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_LINKOVERLAY | SHGFI_DISPLAYNAME)))
	{
		wcscpy_s(cell->pItem->Name, 256, sfi.szDisplayName);
		cell->pItem->IconIDNormal = sfi.iIcon;
	}
	else
	{
		wcscpy_s(cell->pItem->Name, 256, L"?");
		cell->pItem->IconIDNormal = -1;
	}

	CDC* pDC = GetWindowDC();
	CFont* pOldFont = pDC->SelectObject(&theApp.m_DefaultFont);
	CSize sz = pDC->GetTextExtent(cell->pItem->Name, wcslen(cell->pItem->Name));
	pDC->SelectObject(pOldFont);
	ReleaseDC(pDC);

	cell->pItem->Width = sz.cx;

	cell->pItem->IconIDSelected =
		SUCCEEDED(SHGetFileInfo((LPCTSTR)pidlFQ, 0, &sfi, sizeof(sfi), SHGFI_PIDL | SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_OPENICON))
		? sfi.iIcon : -1;

	if (!SHGetPathFromIDList(pidlFQ, cell->pItem->Path))
		cell->pItem->Path[0] = L'\0';
}

UINT CTreeView::InsertItem(UINT row, UINT col, IShellFolder* pParentFolder, LPITEMIDLIST pidlRel, LPITEMIDLIST pidlFQ, UINT Flags)
{
	UINT Inserted = 0;

	SetItem(row, col, pidlRel, pidlFQ, Flags);

	if (col<MaxColumns-1)
	{
		IShellFolder* pFolder;
		HRESULT hr;
		if (!pParentFolder)
		{
			hr = SHGetDesktopFolder(&pFolder);
		}
		else
		{
			hr = pDesktop->BindToObject(pidlFQ, NULL, IID_IShellFolder, (void**)&pFolder);
		}

		if (SUCCEEDED(hr))
		{
			IEnumIDList* pEnum;
			if (SUCCEEDED(pFolder->EnumObjects(NULL, SHCONTF_FOLDERS, &pEnum)))
			{
				BOOL NewRow = FALSE;
				Flags &= CF_CHECKED;

				LPITEMIDLIST pidlTemp;
				while (pEnum->Next(1, &pidlTemp, NULL)==S_OK)
				{
					DWORD dwAttribs = SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM;
					pFolder->GetAttributesOf(1, (LPCITEMIDLIST*)&pidlTemp, &dwAttribs);

					if (!(dwAttribs & (SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM)))
						continue;

					SHDESCRIPTIONID did;
					if (SUCCEEDED(SHGetDataFromIDList(pParentFolder, pidlTemp, SHGDFIL_DESCRIPTIONID, &did, sizeof(SHDESCRIPTIONID))))
					{
						const CLSID LFNE = { 0x3F2D914F, 0xFE57, 0x414F, { 0x9F, 0x88, 0xA3, 0x77, 0xC7, 0x84, 0x1D, 0xA4 } };
						if (did.clsid==LFNE)
							continue;
					}

					if (NewRow)
					{
						for (int a=row+Inserted; a>=0; a--)
						{
							m_Tree[MAKEPOS(a, col+1)].Flags |= CF_HASSIBLINGS;
							if (m_Tree[MAKEPOS(a, col)].Flags & CF_HASCHILDREN)
								break;
							m_Tree[MAKEPOS(a, col+1)].Flags |= CF_ISSIBLING;
						}

						Inserted++;
						InsertRow(row+Inserted);
						Flags |= CF_ISSIBLING;
					}
					else
					{
						m_Tree[MAKEPOS(row, col)].Flags |= CF_HASCHILDREN;
					}

					Inserted += InsertItem(row+Inserted, col+1, pFolder, pidlTemp, theApp.GetShellManager()->ConcatenateItem(pidlFQ, pidlTemp), Flags);
					NewRow = TRUE;
				}
				pEnum->Release();
			}
			pFolder->Release();
		}
	}

	return Inserted;
}

void CTreeView::FreeItem(Cell* cell)
{
	if (cell->pItem)
	{
		theApp.GetShellManager()->FreeItem(cell->pItem->pidlFQ);
		theApp.GetShellManager()->FreeItem(cell->pItem->pidlRel);

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

BOOL CTreeView::HitTest(CPoint point, CPoint* item, BOOL* cbhot)
{
	BOOL res = FALSE;

	point.y -= m_HeaderHeight;
	int row = (point.y>=0) ? point.y/m_RowHeight : -1;
	int col = -1;
	int x = 1;
	if (row!=-1)
	{
		for (UINT a=0; a<m_Cols; a++)
		{
			if ((point.x>=x+GUTTER) && (point.x<x+m_ColumnWidth[a]))
			{
				col = a;
				break;
			}

			x += m_ColumnWidth[a];
		}
	}

	if ((row>=0) && (row<(int)m_Rows) && (col!=-1))
	{
		res = (m_Tree[MAKEPOS(row, col)].pItem!=NULL);
		if ((res) && (cbhot))
		{
			CRect rectButton(x+GUTTER+BORDER, row*m_RowHeight+(m_RowHeight-m_CheckboxSize.cy)/2, x+GUTTER+BORDER+m_CheckboxSize.cx, row*m_RowHeight+(m_RowHeight-m_CheckboxSize.cy)/2+m_CheckboxSize.cy);
			*cbhot = rectButton.PtInRect(point);
		}
	}

	if (item)
	{
		item->x = res ? col : -1;
		item->y = res ? row : -1;
	}

	return res;
}

void CTreeView::InvalidateItem(CPoint item)
{
	if ((item.x!=-1) && (item.y!=-1))
	{
		int x = 0;
		for (UINT a=0; a<(UINT)item.x; a++)
			x += m_ColumnWidth[a];

		CRect rect(x, m_HeaderHeight+item.y*m_RowHeight, x+m_ColumnWidth[item.x], m_HeaderHeight+(item.y+1)*m_RowHeight);
		InvalidateRect(rect);
	}
}

void CTreeView::TrackMenu(UINT nID, CPoint point, int col)
{
	CMenu menu;
	ENSURE(menu.LoadMenu(nID));

	CMenu* popup = menu.GetSubMenu(0);
	ASSERT(popup);

	if (!col)
	{
		popup->EnableMenuItem(IDD_CHOOSEPROPERTY, MF_GRAYED | MF_DISABLED);
		popup->EnableMenuItem(ID_VIEW_RESETPROPERTY, MF_GRAYED | MF_DISABLED);
	}

	switch (popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RETURNCMD | TPM_RIGHTBUTTON, point.x, point.y, this, NULL))
	{
	case ID_VIEW_AUTOSIZE:
		AutosizeColumn(col);
		Invalidate();
		break;
	case ID_VIEW_AUTOSIZEALL:
		AutosizeColumns();
		break;
	case IDD_CHOOSEPROPERTY:
		PostMessage(IDD_CHOOSEPROPERTY, (WPARAM)col);
		break;
	case ID_VIEW_RESETPROPERTY:
		m_ColumnMapping[col] = -1;
		UpdateColumnCaption(col);
		break;
	}
}

void CTreeView::SetCheckboxSize()
{
	if (hThemeButton)
	{
		CDC* dc = GetDC();
		theApp.zGetThemePartSize(hThemeButton, *dc, BP_CHECKBOX, CBS_UNCHECKEDDISABLED, NULL, TS_DRAW, &m_CheckboxSize);
		ReleaseDC(dc);
	}
	else
	{
		m_CheckboxSize.cx = GetSystemMetrics(SM_CXMENUCHECK);
		m_CheckboxSize.cy = GetSystemMetrics(SM_CYMENUCHECK);
	}
}

UINT CTreeView::GetChildRect(CPoint item)
{
	UINT row = item.y;

	while (row<m_Rows)
	{
		if (m_Tree[MAKEPOS(row+1, item.x)].pItem)
			return row;

		row++;
	}

	return row-1;
}

void CTreeView::NotifyOwner()
{
	tagTreeView tag;
	ZeroMemory(&tag, sizeof(tag));
	tag.hdr.hwndFrom = m_hWnd;
	tag.hdr.idFrom = GetDlgCtrlID();
	tag.hdr.code = TVN_SELCHANGED;
	tag.pCell = ((m_Selected.x==-1) || (m_Selected.y==-1)) ? NULL : &m_Tree[MAKEPOSI(m_Selected)];

	GetOwner()->SendMessage(WM_NOTIFY, tag.hdr.idFrom, LPARAM(&tag));
}

void CTreeView::SelectItem(CPoint Item)
{
	if (Item==m_Selected)
		return;

	if (!m_Tree[MAKEPOSI(Item)].pItem)
		return;

	InvalidateItem(m_Selected);
	InvalidateItem(Item);
	m_Selected = Item;
	m_EditLabel = CPoint(-1, -1);

	NotifyOwner();
}

void CTreeView::ExecuteContextMenu(CPoint item, LPCSTR verb)
{
	if ((item.x==-1) || (item.y==-1))
		item = m_Selected;
	if ((item.x==-1) || (item.y==-1) || (item.x>=(int)m_Cols) || (item.y>=(int)m_Rows))
		return;

	DestroyEdit();

	Cell* cell = &m_Tree[MAKEPOSI(item)];
	if (!cell->pItem)
		return;

	IShellFolder* pParentFolder = NULL;
	if (FAILED(SHBindToParent(cell->pItem->pidlFQ, IID_IShellFolder, (void**)&pParentFolder, NULL)))
		return;

	IContextMenu* pcm = NULL;
	if (SUCCEEDED(pParentFolder->GetUIObjectOf(theApp.m_pMainWnd->GetSafeHwnd(), 1, (LPCITEMIDLIST*)&cell->pItem->pidlRel, IID_IContextMenu, NULL, (void**)&pcm)))
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
				cmi.hwnd = theApp.m_pMainWnd->GetSafeHwnd();
				cmi.lpVerb = verb;
				cmi.lpParameters = NULL;
				cmi.lpDirectory = NULL;
				cmi.nShow = SW_SHOWNORMAL;
				cmi.dwHotKey = 0;
				cmi.hIcon = NULL;

				pcm->InvokeCommand(&cmi);
			}
		}
	}
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

void CTreeView::AutosizeColumn(UINT col)
{
	DestroyEdit();

	int Width = 0;
	for (UINT row=0; row<m_Rows; row++)
		if (m_Tree[MAKEPOS(row, col)].pItem)
			Width = max(Width, m_Tree[MAKEPOS(row, col)].pItem->Width);

	m_ColumnWidth[col] = min(Width+GUTTER+2*BORDER+m_CheckboxSize.cx+m_IconSize.cx+3*MARGIN, MAXWIDTH);

	if (m_wndHeader.GetItemCount()>(int)col)
	{
		HDITEM HdItem;
		HdItem.mask = HDI_WIDTH;
		HdItem.cxy = m_ColumnWidth[col];
		m_wndHeader.SetItem(col, &HdItem);
	}
	else
		while (m_wndHeader.GetItemCount()<=(int)col)
		{
			int idx = m_wndHeader.GetItemCount();
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
			item = m_Selected;

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
							InvalidateItem(item);

							theApp.GetShellManager()->FreeItem(pidlParent);
						}

						pParentFolder->Release();
					}
				}
		}

		m_EditLabel.x = m_EditLabel.y = -1;
	}
}


BEGIN_MESSAGE_MAP(CTreeView, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_SETCURSOR()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEHOVER()
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

	m_TooltipCtrl.Create(this);

	if (theApp.m_ThemeLibLoaded)
	{
		hThemeButton = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
		if (theApp.OSVersion>=OS_Vista)
		{
			theApp.zSetWindowTheme(GetSafeHwnd(), L"explorer", NULL);
			hThemeList = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
		}
	}

	SetCheckboxSize();

	IMAGEINFO ii;
	theApp.m_SystemImageListSmall.GetImageInfo(0, &ii);
	m_IconSize.cx = ii.rcImage.right-ii.rcImage.left;
	m_IconSize.cy = ii.rcImage.bottom-ii.rcImage.top;

	LOGFONT lf;
	theApp.m_DefaultFont.GetLogFont(&lf);
	m_RowHeight = (4+max(abs(lf.lfHeight), m_IconSize.cy)) & ~1;

	for (UINT a=0; a<MaxColumns; a++)
		m_ColumnWidth[a] = MINWIDTH;

	return 0;
}

void CTreeView::OnDestroy()
{
	if (hThemeButton)
		theApp.zCloseThemeData(hThemeButton);
	if (hThemeList)
		theApp.zCloseThemeData(hThemeList);

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

		hThemeButton = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
		if (theApp.OSVersion>=OS_Vista)
			hThemeList = theApp.zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
	}

	SetCheckboxSize();
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
	dc.FillSolidRect(rect, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	LOGBRUSH brsh;
	brsh.lbColor = 0x808080;
	brsh.lbStyle = PS_SOLID;
	CPen pen(PS_COSMETIC | PS_ALTERNATE, 1, &brsh);
	CPen* pOldPen = dc.SelectObject(&pen);

	int y = m_HeaderHeight;
	Cell* curCell = m_Tree;
	for (UINT row=0; row<m_Rows; row++)
	{
		int x = 0;
		for (UINT col=0; col<MaxColumns; col++)
		{
			if (curCell->pItem)
			{
				CRect rectItem(x+GUTTER, y, x+m_ColumnWidth[col], y+m_RowHeight);
				CRect rectIntersect;
				if (rectIntersect.IntersectRect(rectItem, rectUpdate))
				{
					BOOL Hot = (m_Hot.x==(int)col) && (m_Hot.y==(int)row);
					BOOL Selected = (m_Selected.x==(int)col) && (m_Selected.y==(int)row);

					if (hThemeList)
					{
						if (Hot | Selected)
						{
							const int StateIDs[4] = { LISS_NORMAL, LISS_HOT, GetFocus()!=this ? LISS_SELECTEDNOTFOCUS : LISS_SELECTED, LISS_HOTSELECTED };
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
								dc.DrawFocusRect(rectItem);
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
						int uiStyle;
						if (curCell->pItem->Path[0])
						{
							uiStyle = (Selected && m_CheckboxPressed) ? CBS_UNCHECKEDPRESSED : (Hot && m_CheckboxHot) ? CBS_UNCHECKEDHOT : CBS_UNCHECKEDNORMAL;
							if (curCell->Flags & CF_CHECKED)
								uiStyle += 4;
						}
						else
						{
							uiStyle = CBS_UNCHECKEDDISABLED;
						}
						theApp.zDrawThemeBackground(hThemeButton, dc.m_hDC, BP_CHECKBOX, uiStyle, rectButton, rectButton);
					}
					else
					{
						UINT uiStyle = DFCS_BUTTONCHECK;
						if (curCell->pItem->Path[0])
						{
							uiStyle |= (curCell->Flags & CF_CHECKED ? DFCS_CHECKED : 0) | ((Selected && m_CheckboxPressed) ? DFCS_PUSHED : 0);
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

					if (curCell->Flags & CF_HASCHILDREN)
					{
						int right = x+GUTTER+BORDER+m_CheckboxSize.cx+m_IconSize.cx+2*MARGIN+curCell->pItem->Width+1;
						if (right<x+m_ColumnWidth[col])
						{
							dc.MoveTo(x+m_ColumnWidth[col], y+m_RowHeight/2);
							dc.LineTo(right, y+m_RowHeight/2);
						}
					}
				}
			}

			if (curCell->Flags & CF_ISSIBLING)
			{
				dc.MoveTo(x+GUTTER/2, y);
				dc.LineTo(x+GUTTER/2, y+m_RowHeight/2);
			}

			if (curCell->Flags & CF_HASSIBLINGS)
			{
				dc.MoveTo(x+GUTTER/2, y+m_RowHeight/2);
				dc.LineTo(x+GUTTER/2, y+m_RowHeight);
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

void CTreeView::OnMouseMove(UINT nFlags, CPoint point)
{
	BOOL Dragging = (GetCapture()==this);
	BOOL Pressed = FALSE;
	CPoint Item(-1, -1);
	BOOL OnItem = HitTest(point, &Item, Dragging ? &Pressed : &m_CheckboxHot);

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
		if ((m_TooltipCtrl.IsWindowVisible()) && (Item!=m_Hot))
			m_TooltipCtrl.Deactivate();

	InvalidateItem(m_Hot);

	if (!Dragging)
	{
		m_Hot = Item;

		if ((OnItem) && (nFlags & MK_RBUTTON))
		{
			SetFocus();
			InvalidateItem(m_Selected);
			m_Selected = m_Hot;
		}
	}
	m_CheckboxPressed = (Item==m_Selected) && Pressed && Dragging;

	InvalidateItem(m_Hot);
}

void CTreeView::OnMouseLeave()
{
	m_TooltipCtrl.Deactivate();
	m_Hover = FALSE;
	m_Hot.x = m_Hot.y = -1;

	Invalidate();
}

void CTreeView::OnMouseHover(UINT nFlags, CPoint point)
{
	if ((nFlags & (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON | MK_XBUTTON1 | MK_XBUTTON2))==0)
	{
		if ((m_Hot.x!=-1) && (m_Hot.y!=-1) && (!p_Edit))
			if (m_Hot==m_EditLabel)
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
					TooltipDataFromPIDL(m_Tree[MAKEPOSI(m_Hot)].pItem->pidlFQ, &theApp.m_SystemImageListLarge, hIcon, size, caption, hint);

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

void CTreeView::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	SetFocus();

	CPoint Item;
	if (HitTest(point, &Item, &m_CheckboxHot))
	{
		if (Item==m_Selected)
		{
			m_EditLabel = m_Selected;
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
}

void CTreeView::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (GetCapture()==this)
	{
		CPoint Item;
		if (HitTest(point, &Item, &m_CheckboxPressed))
			if ((Item==m_Selected) && (m_CheckboxPressed))
				if (nFlags & MK_CONTROL)
				{
					SetBranchCheck(!(m_Tree[MAKEPOSI(Item)].Flags & CF_CHECKED), Item);
				}
				else
				{
					m_Tree[MAKEPOSI(Item)].Flags ^= CF_CHECKED;
					InvalidateItem(Item);
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
	if (HitTest(point, &Item, &Checkbox))
		if ((Item==m_Selected) && (!Checkbox))
			OpenFolder();
}

void CTreeView::OnRButtonDown(UINT /*nFlags*/, CPoint point)
{
	SetFocus();

	CPoint Item;
	if (HitTest(point, &Item, &m_CheckboxHot))
		SelectItem(Item);
}

void CTreeView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (m_pContextMenu2)
		return;

	CPoint item(-1, -1);
	if ((point.x==-1) && (point.y==-1))
	{
		if ((m_Selected.x==-1) || (m_Selected.y==-1))
		{
			GetParent()->SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(point.x, point.y));
			return;
		}

		item = m_Selected;

		point.x = GUTTER;
		for (int a=0; a<item.x; a++)
			point.x += m_ColumnWidth[a];
		point.y = (item.y+1)*m_RowHeight+m_HeaderHeight+1;
		ClientToScreen(&point);
	}
	else
	{
		CPoint ptClient(point);
		ScreenToClient(&ptClient);

		if (pWnd->GetSafeHwnd()==m_wndHeader.GetSafeHwnd())
		{
			HDHITTESTINFO htt;
			htt.pt = ptClient;
			TrackMenu(IDM_HEADER, point, m_wndHeader.HitTest(&htt));
			return;
		}

		if (!HitTest(ptClient, &item, NULL))
		{
			GetParent()->SendMessage(WM_CONTEXTMENU, (WPARAM)m_hWnd, MAKELPARAM(point.x, point.y));
			return;
		}
	}

	if ((item.x==-1) || (item.y==-1) || (item.x>=(int)m_Cols) || (item.y>=(int)m_Rows))
		return;

	Cell* cell = &m_Tree[MAKEPOSI(item)];
	if (!cell->pItem)
		return;

	IShellFolder* pParentFolder = NULL;
	if (FAILED(SHBindToParent(cell->pItem->pidlFQ, IID_IShellFolder, (void**)&pParentFolder, NULL)))
		return;

	IContextMenu* pcm = NULL;
	if (SUCCEEDED(pParentFolder->GetUIObjectOf(theApp.m_pMainWnd->GetSafeHwnd(), 1, (LPCITEMIDLIST*)&cell->pItem->pidlRel, IID_IContextMenu, NULL, (void**)&pcm)))
	{
		HMENU hPopup = CreatePopupMenu();
		if (hPopup)
		{
			UINT uFlags = CMF_NORMAL | CMF_CANRENAME;
			if (SUCCEEDED(pcm->QueryContextMenu(hPopup, 0, 1, 0x6FFF, uFlags)))
			{
				if (cell->Flags & CF_HASCHILDREN)
				{
					CString tmpStr = theApp.GetCommandName(ID_VIEW_INCLUDEBRANCH);
					tmpStr.Insert(0, _T("&"));
					InsertMenu(hPopup, 0, MF_BYPOSITION, 0x7001, tmpStr);

					tmpStr = theApp.GetCommandName(ID_VIEW_EXCLUDEBRANCH);
					tmpStr.Insert(0, _T("&"));
					InsertMenu(hPopup, 1, MF_BYPOSITION, 0x7002, tmpStr);

					InsertMenu(hPopup, 2, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
				}

				if (item.x)
				{
					CString tmpStr;
					tmpStr.LoadString(IDS_CHOOSEPROPERTY);
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
				case 0:
					break;
				default:
					{
						char Verb[256] = "";
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
							cmi.hwnd = theApp.m_pMainWnd->GetSafeHwnd();
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
	InvalidateItem(m_Selected);
}

void CTreeView::OnKillFocus(CWnd* /*pNewWnd*/)
{
	InvalidateItem(m_Selected);
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
	ASSERT((int)wParam>0);
	ASSERT((int)wParam<MaxColumns);

	ChoosePropertyDlg dlg(theApp.m_pMainWnd);
	if (dlg.DoModal()!=IDCANCEL)
	{
		m_ColumnMapping[(int)wParam] = dlg.m_Attr;
		UpdateColumnCaption((UINT)wParam);
	}

	return NULL;
}
