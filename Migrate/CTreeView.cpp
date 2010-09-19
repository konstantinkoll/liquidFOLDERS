
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

#define BORDER       3
#define MARGIN       4
#define GUTTER       9

CTreeView::CTreeView()
{
	m_Tree = NULL;
	m_Allocated = m_Rows = m_Cols = 0;
	hThemeList = hThemeButton = NULL;
	m_Selected.x = m_Selected.y = m_Hot.x = m_Hot.y = -1;
	m_CheckboxHot = m_CheckboxPressed = FALSE;

	pDesktop = NULL;
	SHGetDesktopFolder(&pDesktop);
}

CTreeView::~CTreeView()
{
	if (pDesktop)
		pDesktop->Release();
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
	m_Selected.x = m_Selected.y = m_Hot.x = m_Hot.y = -1;
	m_CheckboxHot = m_CheckboxPressed = FALSE;

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

	HRESULT hr;
	IShellFolder* pParentFolder = NULL;
	LPCITEMIDLIST pidlRel = NULL;
	if (theApp.GetShellManager()->GetItemSize(pidl)==2)
	{
		//pParentFolder = pDesktop;
		pidlRel = theApp.GetShellManager()->CopyItem(pidl);
		pDesktop->AddRef();
		hr = S_OK;
	}
	else
	{
		hr = SHBindToParent(pidl, IID_IShellFolder, (void**)&pParentFolder, &pidlRel);
	}

	if (SUCCEEDED(hr))
	{
		InsertItem(0, 0, pParentFolder, theApp.GetShellManager()->CopyItem(pidlRel), theApp.GetShellManager()->CopyItem(pidl), CF_CHECKED);
		if (pParentFolder)
			pParentFolder->Release();

		if (!Update)
		{
			// TODO
			m_Selected.x = m_Selected.y = 0;
		}
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
		for (UINT a=m_Rows; a>=Row; a--)
			memcpy(&m_Tree[(a+1)*MaxColumns], &m_Tree[a*MaxColumns], MaxColumns*sizeof(Cell));

	ZeroMemory(&m_Tree[Row*MaxColumns], MaxColumns*sizeof(Cell));
	m_Rows++;

	return TRUE;
}

void CTreeView::SetItem(UINT row, UINT col, IShellFolder* pParentFolder, LPITEMIDLIST pidlRel, LPITEMIDLIST pidlFQ, UINT Flags)
{
	ASSERT(row<m_Rows);
	ASSERT(col<MaxColumns);

	Cell* cell = &m_Tree[row*MaxColumns+col];
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

	cell->Flags = Flags;
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

UINT CTreeView::InsertItem(UINT row, UINT col, IShellFolder* pParentFolder, LPITEMIDLIST pidlRel, LPITEMIDLIST pidlFQ, UINT Flags)
{
	UINT Inserted = 0;

	SetItem(row, col, pParentFolder, pidlRel, pidlFQ, Flags);

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
					DWORD dwAttribs = SFGAO_HASSUBFOLDER | SFGAO_FOLDER | SFGAO_FILESYSANCESTOR | SFGAO_FILESYSTEM;
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
							m_Tree[a*MaxColumns+col+1].Flags |= CF_HASSIBLINGS;
							if (m_Tree[a*MaxColumns+col].Flags & CF_HASCHILDREN)
								break;
							m_Tree[a*MaxColumns+col+1].Flags |= CF_ISSIBLING;
						}

						Inserted++;
						InsertRow(row+Inserted);
						Flags |= CF_ISSIBLING;
					}
					else
					{
						m_Tree[row*MaxColumns+col].Flags |= CF_HASCHILDREN;
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

void CTreeView::SetCheckboxSize()
{
	if (hThemeButton)
	{
		CDC dc;
		dc.CreateCompatibleDC(NULL);

		theApp.zGetThemePartSize(hThemeButton, dc, BP_CHECKBOX, CBS_UNCHECKEDDISABLED, NULL, TS_DRAW, &m_CheckboxSize);
	}
	else
	{
		m_CheckboxSize.cx = GetSystemMetrics(SM_CXMENUCHECK);
		m_CheckboxSize.cy = GetSystemMetrics(SM_CYMENUCHECK);
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

	HDITEM HdItem;
	HdItem.mask = HDI_TEXT | HDI_WIDTH | HDI_FORMAT;
	HdItem.fmt = HDF_STRING | HDF_CENTER;

	for (UINT a=0; a<MaxColumns; a++)
	{
		HdItem.cxy = m_ColumnWidth[a];
		HdItem.pszText = a ? L"Ignore" : L"";
		m_wndHeader.InsertItem(a, &HdItem);
	}

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

	int y = m_HeaderHeight+1;
	Cell* curCell = m_Tree;
	for (UINT row=0; row<m_Rows; row++)
	{
		int x = 1;
		for (UINT col=0; col<MaxColumns; col++)
		{
			if (curCell->pItem)
			{
				CRect rectItem(x+GUTTER, y, x+m_ColumnWidth[col], y+m_RowHeight);
				BOOL Hot = (m_Hot.x==(int)row) && (m_Hot.y==(int)col);
				BOOL Selected = (m_Selected.x==(int)row) && (m_Selected.y==(int)col);

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
						dc.FillSolidRect(rectItem, GetSysColor(COLOR_HIGHLIGHT));
						dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
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
						uiStyle = m_CheckboxPressed ? CBS_UNCHECKEDPRESSED : m_CheckboxHot ? CBS_UNCHECKEDHOT : CBS_UNCHECKEDNORMAL;
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
						uiStyle |= (curCell->Flags & CF_CHECKED ? DFCS_CHECKED : 0) | (m_CheckboxPressed ? DFCS_PUSHED : 0);
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
					dc.MoveTo(x+GUTTER+BORDER, y+m_RowHeight/2);
					dc.LineTo(x+m_ColumnWidth[col], y+m_RowHeight/2);
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

		m_ColumnWidth[pHdr->iItem] = pHdr->pitem->cxy;
		Invalidate();

		*pResult = FALSE;
	}
}
