
#include "stdafx.h"
#include "CFilterWnd.h"
#include "Resource.h"
#include "liquidFOLDERS.h"
#include "LFCore.h"
#include "StoreManager.h"


// CFilterWnd
//

CFilterWnd::CFilterWnd()
	: CGlasPane()
{
	m_Icons = NULL;
}

CFilterWnd::~CFilterWnd()
{
	if (m_Icons)
		delete m_Icons;
}

void CFilterWnd::AddConditionItem(BOOL focus)
{
	UINT puColumns[] = { 1, 2 };
	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_COLUMNS;
	lvi.cColumns = 2;
	lvi.puColumns = puColumns;
	lvi.iItem = m_wndList.GetItemCount();
	lvi.pszText = L"Property";
	lvi.iImage = -1;
	INT idx = m_wndList.InsertItem(&lvi);

	m_wndList.SetItemText(idx, 1, L"Condition");
	m_wndList.SetItemText(idx, 2, L"Value");

	if (focus)
		m_wndList.SetItemState(idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

void CFilterWnd::UpdateList()
{
	m_wndList.SetRedraw(FALSE);
	m_wndList.DeleteAllItems();

	AddConditionItem(FALSE);
	AddConditionItem(FALSE);
	AddConditionItem(FALSE);
	AddConditionItem(FALSE);
	AddConditionItem(FALSE);

	m_wndList.SetRedraw(TRUE);
}


BEGIN_MESSAGE_MAP(CFilterWnd, CGlasPane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR()
	ON_UPDATE_COMMAND_UI_RANGE(ID_FILTER_CLEAR, ID_FILTER_SAVEAS, OnUpdateCommands)
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateCommands)
END_MESSAGE_MAP()

INT CFilterWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlasPane::OnCreate(lpCreateStruct)==-1)
		return -1;

	if (m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE)==-1)
		return -1;

	m_wndToolBar.LoadToolBar(ID_PANE_FILTERWND, 0, 0, TRUE);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP;
	if (!m_wndFreetext.Create(dwViewStyle, CRect(0, 0, 0, 0), this, 4))
		return -1;
	if (!m_wndStoreCombo.Create(dwViewStyle | CBS_DROPDOWNLIST | CBS_SORT, CRect(0, 0, 0, 0), this, 3))
		return -1;
	if (!m_wndAddCondition.Create(L"&Add...", dwViewStyle | BS_PUSHBUTTON, CRect(0, 0, 0, 0), this, IDCANCEL))
		return -1;
	if (!m_wndStartSearch.Create(L"&Search", dwViewStyle | BS_DEFPUSHBUTTON, CRect(0, 0, 0, 0), this, IDOK))
		return -1;

	m_wndFreetext.ModifyStyleEx(0, WS_EX_STATICEDGE, 0);

	CFont* fnt = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));
	m_wndFreetext.SetFont(fnt);
	m_wndStoreCombo.SetFont(fnt);
	m_wndStartSearch.SetFont(fnt);
	m_wndAddCondition.SetFont(fnt);

	m_wndStoreCombo.AddString(L"Search all stores");
	m_wndStoreCombo.AddString(L"Search current store");
	m_wndStoreCombo.SetCurSel(0);

	if (!m_wndText1.Create(this, 5, L"Search for:"))
		return -1;
	if (!m_wndText2.Create(this, 6, L"Scope:"))
		return -1;
	if (!m_wndText3.Create(this, 7, L"Files must also meet these conditions:"))
		return -1;

	if (!m_wndList.Create(dwViewStyle | LVS_NOCOLUMNHEADER, CRect(0, 0, 0, 0), this, ID_FILTERLIST))
		return -1;

	const DWORD dwExStyle = LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP;
	m_wndList.SetExtendedStyle(dwExStyle);

	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));

	lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	for (UINT a=0; a<3; a++)
	{
		lvc.pszText = L"";
		lvc.fmt = LVCFMT_LEFT;
		lvc.iSubItem = a;
		m_wndList.InsertColumn(a, &lvc);
	}

	m_wndList.SetView(LV_VIEW_TILE);

	CMFCToolBarImages tmp;
	tmp.SetImageSize(CSize(32, 32));
	tmp.Load(IDB_TASKS);
	m_Icons = new CImageList();
	m_Icons->Create(32, 32, ILC_COLOR32, 2, 1);
		for (INT a=0; a<tmp.GetCount(); a++)
		{
			HICON h = tmp.ExtractIcon(a);
			m_Icons->Add(h);
			DestroyIcon(h);
		}
	tmp.Clear();

	m_wndList.SetImageList(m_Icons, LVSIL_NORMAL);

	return 0;
}

void CFilterWnd::OnSize(UINT nType, INT cx, INT cy)
{
	CGlasPane::OnSize(nType, cx, cy);

	CRect rectClient;
	GetClientRect(rectClient);

	const INT borderBtn = 4;
	LOGFONT lFont;
	CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT))->GetLogFont(&lFont);

	INT heightTxt = abs(lFont.lfHeight);
	INT heightTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;
	INT heightBtn = 2*borderBtn+heightTxt+15;

	CRect rectCombo;
	m_wndStoreCombo.GetWindowRect(&rectCombo);

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), heightTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	cy = rectClient.top+heightTlb;

	m_wndText1.SetWindowPos(NULL, rectClient.left+borderBtn, cy, rectClient.Width()-borderBtn, heightTxt+borderBtn, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightTxt+borderBtn+1;

	m_wndFreetext.SetWindowPos(NULL, rectClient.left+borderBtn+1, cy, rectClient.Width()-2*borderBtn-1, rectCombo.Height()-3, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += rectCombo.Height()-3+borderBtn;

	m_wndText2.SetWindowPos(NULL, rectClient.left+borderBtn, cy, rectClient.Width()-borderBtn, heightTxt+borderBtn, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightTxt+borderBtn+1;

	m_wndStoreCombo.SetWindowPos(NULL, rectClient.left+borderBtn+1, cy, rectClient.Width()-2*borderBtn-1, rectCombo.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	cy += rectCombo.Height()+borderBtn;

	m_wndAddCondition.SetWindowPos(NULL, rectClient.left+borderBtn, cy+borderBtn, rectClient.Width()/2-3*borderBtn/2, heightBtn-2*borderBtn, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndStartSearch.SetWindowPos(NULL, rectClient.Width()-(rectClient.Width()/2-borderBtn/2)+1, cy+borderBtn, rectClient.Width()/2-3*borderBtn/2, heightBtn-2*borderBtn, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightBtn;

	m_wndText3.SetWindowPos(NULL, rectClient.left+borderBtn, cy, rectClient.Width()-borderBtn, 20, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndList.SetWindowPos(NULL, rectClient.left+borderBtn, cy+20, rectClient.Width()-borderBtn, rectClient.Height()-cy-20, SWP_NOACTIVATE | SWP_NOZORDER);
}

BOOL CFilterWnd::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	pDC->FillSolidRect(rect, afxGlobalData.clrBarFace);

	return TRUE;
}

HBRUSH CFilterWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	if ((nCtlColor==CTLCOLOR_BTN) || (nCtlColor==CTLCOLOR_STATIC))
	{
		pDC->SetBkMode(TRANSPARENT);
		hbr = CreateSolidBrush(afxGlobalData.clrBarFace);
	}

	return hbr;
}

void CFilterWnd::OnUpdateCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}
