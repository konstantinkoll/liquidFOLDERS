
#include "stdafx.h"
#include "CFilterWnd.h"
#include "EditConditionDlg.h"
#include "StoreManager.h"


// CFilterWnd
//

CFilterWnd::CFilterWnd()
	: CGlasPane()
{
}

CFilterWnd::~CFilterWnd()
{
}

void CFilterWnd::AdjustLayout()
{
	CRect rectClient;
	GetClientRect(rectClient);

	const INT borderBtn = 4;

	INT heightTxt = m_wndLabel1.GetPreferredHeight();
	INT heightBtn = 2*borderBtn+heightTxt+15;

	CRect rectCombo;
	m_wndStoreCombo.GetWindowRect(&rectCombo);

	INT cy = -1;

	m_wndLabel1.SetWindowPos(NULL, rectClient.left, cy, rectClient.Width(), heightTxt, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightTxt+borderBtn;

	m_wndFreetext.SetWindowPos(NULL, rectClient.left+borderBtn+1, cy, rectClient.Width()-2*borderBtn-1, rectCombo.Height()-1, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += rectCombo.Height()-1+borderBtn;

	m_wndLabel2.SetWindowPos(NULL, rectClient.left, cy, rectClient.Width(), heightTxt+borderBtn, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightTxt+borderBtn;

	m_wndStoreCombo.SetWindowPos(NULL, rectClient.left+borderBtn+1, cy, rectClient.Width()-2*borderBtn-1, rectCombo.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	cy += rectCombo.Height()+borderBtn;

	m_wndAddCondition.SetWindowPos(NULL, rectClient.left+borderBtn, cy+borderBtn, rectClient.Width()/2-3*borderBtn/2, heightBtn-2*borderBtn, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndStartSearch.SetWindowPos(NULL, rectClient.Width()-(rectClient.Width()/2-borderBtn/2)+1, cy+borderBtn, rectClient.Width()/2-3*borderBtn/2, heightBtn-2*borderBtn, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightBtn;

	m_wndLabel3.SetWindowPos(NULL, rectClient.left, cy, rectClient.Width(), 20, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndList.SetWindowPos(NULL, rectClient.left+borderBtn, cy+20, rectClient.Width()-borderBtn, rectClient.Height()-cy-20, SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(CFilterWnd, CGlasPane)
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_DBLCLK, 4, OnDoubleClick)
//	ON_UPDATE_COMMAND_UI_RANGE(ID_FILTER_CLEAR, ID_FILTER_SAVEAS, OnUpdateCommands)
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateCommands)
END_MESSAGE_MAP()

INT CFilterWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlasPane::OnCreate(lpCreateStruct)==-1)
		return -1;

	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP;
	if (!m_wndFreetext.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), dwViewStyle | ES_AUTOHSCROLL, CRect(0, 0, 0, 0), this, 4))
		return -1;
	if (!m_wndStoreCombo.Create(dwViewStyle | CBS_DROPDOWNLIST | CBS_SORT, CRect(0, 0, 0, 0), this, 3))
		return -1;
	if (!m_wndAddCondition.Create(L"&Add...", dwViewStyle | BS_PUSHBUTTON, CRect(0, 0, 0, 0), this, IDCANCEL))
		return -1;
	if (!m_wndStartSearch.Create(L"&Search", dwViewStyle | BS_DEFPUSHBUTTON, CRect(0, 0, 0, 0), this, IDOK))
		return -1;

	m_wndFreetext.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
	m_wndStoreCombo.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
	m_wndStartSearch.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
	m_wndAddCondition.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));

	m_wndStoreCombo.AddString(L"All stores");
	m_wndStoreCombo.AddString(L"This store");
	m_wndStoreCombo.SetCurSel(0);

	if (!m_wndLabel1.Create(this, 5, L"Global search term"))
		return -1;
	if (!m_wndLabel2.Create(this, 6, L"Search in"))
		return -1;
	if (!m_wndLabel3.Create(this, 7, L"Other conditions"))
		return -1;

	if (!m_wndList.Create(dwViewStyle | LVS_NOCOLUMNHEADER | LVS_SHAREIMAGELISTS, CRect(0, 0, 0, 0), this, 4))
		return -1;

	m_wndList.SetMenus(IDM_CONDITION, TRUE, IDM_CONDITIONLIST);

	LFFilterCondition c;
	c.AttrData.Attr = LFAttrArtist;
	LFGetNullVariantData(&c.AttrData);
	c.AttrData.IsNull = false;
	wcscpy_s(c.AttrData.UnicodeString, 256, L"Madonna");
	m_wndList.InsertItem(&c);

	c.AttrData.Attr = LFAttrRating;
	LFGetNullVariantData(&c.AttrData);
	c.AttrData.IsNull = false;
	c.AttrData.Rating = 7;
	m_wndList.InsertItem(&c);

	c.AttrData.Attr = LFAttrAlbum;
	LFGetNullVariantData(&c.AttrData);
	c.AttrData.IsNull = false;
	wcscpy_s(c.AttrData.UnicodeString, 256, L"True Blue");
	m_wndList.InsertItem(&c);

	m_wndList.SetView(LV_VIEW_TILE);

	return 0;
}

void CFilterWnd::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if (pWnd!=this)
		return;

	if ((point.x<0) || (point.y<0))
	{
		CRect rect;
		GetClientRect(rect);

		point.x = (rect.left+rect.right)/2;
		point.y = (rect.top+rect.bottom)/2;
		ClientToScreen(&point);
	}

	CMenu menu;
	ENSURE(menu.LoadMenu(IDM_FILTER));

	CMenu* pPopup = menu.GetSubMenu(0);
	ASSERT_VALID(pPopup);

	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetOwner(), NULL);
}

void CFilterWnd::OnDoubleClick(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
//	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	EditConditionDlg dlg(GetParent());
	if (dlg.DoModal()==IDOK)
	{
	}

	m_wndList.SetFocus();
}


void CFilterWnd::OnUpdateCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}
