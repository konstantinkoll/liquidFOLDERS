
#include "stdafx.h"
#include "CFilterWnd.h"
#include "EditConditionDlg.h"
#include "StoreManager.h"


// CFilterWnd
//

#define     GUTTER 4

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

	INT heightLabel = m_wndLabel1.GetPreferredHeight();
	INT heightButton = heightLabel+8;
	INT heightRadio = 16;
	INT heightText = 20;

	INT cy = -1;

	m_wndLabel1.SetWindowPos(NULL, rectClient.left, cy, rectClient.Width(), heightLabel, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightLabel+GUTTER;

	m_wndAllStores.SetWindowPos(NULL, rectClient.left+GUTTER+1, cy, rectClient.Width()-2*GUTTER-1, heightRadio, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightRadio+GUTTER;

	m_wndThisStore.SetWindowPos(NULL, rectClient.left+GUTTER+1, cy, rectClient.Width()-2*GUTTER-1, heightRadio, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightRadio+GUTTER;

	m_wndSaveFilter.SetWindowPos(NULL, rectClient.left+GUTTER, cy+GUTTER, rectClient.Width()/2-3*GUTTER/2, heightButton, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndStartSearch.SetWindowPos(NULL, rectClient.Width()-(rectClient.Width()/2-GUTTER/2)+1, cy+GUTTER, rectClient.Width()/2-3*GUTTER/2, heightButton, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightButton+2*GUTTER;

	m_wndLabel2.SetWindowPos(NULL, rectClient.left, cy, rectClient.Width(), heightLabel+GUTTER, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightLabel+GUTTER;

	m_wndFreetext.SetWindowPos(NULL, rectClient.left+GUTTER+1, cy, rectClient.Width()-2*GUTTER-1, heightText, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightText+2*GUTTER;

	m_wndLabel3.SetWindowPos(NULL, rectClient.left, cy, rectClient.Width(), heightLabel, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightLabel+GUTTER;

	m_wndList.SetWindowPos(NULL, rectClient.left+GUTTER, cy, rectClient.Width()-GUTTER, rectClient.Height()-cy, SWP_NOACTIVATE | SWP_NOZORDER);
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
	if (!m_wndAllStores.Create(L"&All stores", dwViewStyle | BS_AUTORADIOBUTTON | WS_GROUP, CRect(0, 0, 0, 0), this, 80))
		return -1;
	if (!m_wndThisStore.Create(L"&This store", dwViewStyle | BS_AUTORADIOBUTTON, CRect(0, 0, 0, 0), this, 81))
		return -1;
	if (!m_wndSaveFilter.Create(L"Sa&ve", dwViewStyle | BS_PUSHBUTTON, CRect(0, 0, 0, 0), this, IDCANCEL))
		return -1;
	if (!m_wndStartSearch.Create(L"&Search", dwViewStyle | BS_DEFPUSHBUTTON, CRect(0, 0, 0, 0), this, IDOK))
		return -1;
	if (!m_wndFreetext.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), dwViewStyle | ES_AUTOHSCROLL, CRect(0, 0, 0, 0), this, 2))
		return -1;
	if (!m_wndAddCondition.Create(L"A&dd condition...", dwViewStyle | BS_PUSHBUTTON, CRect(0, 0, 0, 0), this, IDCANCEL))
		return -1;

	m_wndAllStores.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
	m_wndThisStore.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
	m_wndSaveFilter.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
	m_wndStartSearch.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
	m_wndFreetext.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
	m_wndAddCondition.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));

	m_wndAllStores.SetCheck(BST_CHECKED);

	if (!m_wndLabel1.Create(this, 5, L"Search in"))
		return -1;
	if (!m_wndLabel2.Create(this, 6, L"Global search term"))
		return -1;
	if (!m_wndLabel3.Create(this, 7, L"Other conditions"))
		return -1;

	if (!m_wndList.Create(dwViewStyle | LVS_NOCOLUMNHEADER | LVS_SHAREIMAGELISTS, CRect(0, 0, 0, 0), this, 4))
		return -1;

	m_wndList.SetView(LV_VIEW_TILE);
	m_wndList.SetMenus(IDM_CONDITION, TRUE, IDM_CONDITIONLIST);

	LFFilterCondition c;
	c.AttrData.Attr = LFAttrArtist;
	LFGetNullVariantData(&c.AttrData);
	c.AttrData.IsNull = false;
	c.Compare = LFFilterCompareIsEqual;
	wcscpy_s(c.AttrData.UnicodeString, 256, L"Madonna");
	m_wndList.InsertItem(&c);

	c.AttrData.Attr = LFAttrRating;
	LFGetNullVariantData(&c.AttrData);
	c.AttrData.IsNull = false;
	c.Compare = LFFilterCompareIsAboveOrEqual;
	c.AttrData.Rating = 7;
	m_wndList.InsertItem(&c);

	c.AttrData.Attr = LFAttrAlbum;
	LFGetNullVariantData(&c.AttrData);
	c.AttrData.IsNull = false;
	c.Compare = LFFilterCompareIsEqual;
	wcscpy_s(c.AttrData.UnicodeString, 256, L"True Blue");
	m_wndList.InsertItem(&c);

	return 0;
}

void CFilterWnd::OnContextMenu(CWnd* pWnd, CPoint point)
{
	if ((pWnd!=this) && (pWnd!=&m_wndAllStores) && (pWnd!=&m_wndThisStore))
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
