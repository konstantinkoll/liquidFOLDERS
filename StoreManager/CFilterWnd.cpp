
#include "stdafx.h"
#include "CFilterWnd.h"
#include "EditConditionDlg.h"
#include "SaveFilterDlg.h"
#include "StoreManager.h"


// CFilterWnd
//

#define GUTTER     4
#define IDLIST     7

CFilterWnd::CFilterWnd()
	: CGlasPane()
{
	m_FontHeight = 0;
	m_StoreID[0]= '\0';
}

CFilterWnd::~CFilterWnd()
{
}

void CFilterWnd::SetOwner(CWnd* pOwnerWnd)
{
	CGlasPane::SetOwner(pOwnerWnd);

	m_wndList.SetOwner(pOwnerWnd);
}

void CFilterWnd::AdjustLayout()
{
	if (!IsWindow(m_wndBottomArea))
		return;

	CRect rectClient;
	GetClientRect(rectClient);

	CRect rectButton;
	m_wndBottomArea.GetDlgItem(IDM_FILTER_SEARCH)->GetWindowRect(&rectButton);
	m_wndBottomArea.ScreenToClient(&rectButton);

	const INT heightLabel = m_wndLabel1.GetPreferredHeight();
	const INT heightButton = rectButton.Height();
	const INT heightRadio = max(m_FontHeight+2, 16);
	const INT heightText = m_FontHeight+7;

	const UINT BottomHeight = MulDiv(45, LOWORD(GetDialogBaseUnits()), 8);
	const INT widthButton1 = m_FontHeight*7;
	const INT widthButton2 = m_FontHeight*15;

	INT cy = -1;

	m_wndLabel1.SetWindowPos(NULL, rectClient.left, cy, rectClient.Width(), heightLabel, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightLabel+GUTTER;

	m_wndAllStores.SetWindowPos(NULL, rectClient.left+GUTTER+1, cy, rectClient.Width()-2*GUTTER-1, heightRadio, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightRadio+GUTTER;

	m_wndThisStore.SetWindowPos(NULL, rectClient.left+GUTTER+1, cy, rectClient.Width()-2*GUTTER-1, heightRadio, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightRadio+GUTTER;

	m_wndLabel2.SetWindowPos(NULL, rectClient.left, cy, rectClient.Width(), heightLabel+GUTTER, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightLabel+GUTTER;

	m_wndSearchterm.SetWindowPos(NULL, rectClient.left+GUTTER+1, cy, rectClient.Width()-2*GUTTER-1, heightText, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightText+2*GUTTER;

	m_wndLabel3.SetWindowPos(NULL, rectClient.left, cy, rectClient.Width(), heightLabel, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightLabel+GUTTER;

	m_wndAddCondition.SetWindowPos(NULL, rectClient.left+GUTTER, cy, min(rectClient.Width(), widthButton2)-2*GUTTER, heightButton, SWP_NOACTIVATE | SWP_NOZORDER);
	cy += heightButton+GUTTER;

	m_wndList.SetWindowPos(NULL, rectClient.left+GUTTER, cy, rectClient.Width()-GUTTER, rectClient.Height()-cy-BottomHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	m_wndBottomArea.SetWindowPos(NULL, rectClient.left, rectClient.bottom-BottomHeight, rectClient.Width(), BottomHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndBottomArea.GetDlgItem(IDM_FILTER_SEARCH)->SetWindowPos(NULL, rectButton.left, rectButton.top, min(rectClient.Width()/2-3*rectButton.left/2, widthButton1), rectButton.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndBottomArea.GetDlgItem(IDM_FILTER_SAVE)->SetWindowPos(NULL, min(widthButton1+2*rectButton.left, rectClient.Width()-(rectClient.Width()/2-rectButton.left/2))+1, rectButton.top, min(rectClient.Width()/2-3*rectButton.left/2, widthButton1), rectButton.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFilterWnd::SetStoreID(CHAR* StoreID, BOOL SetCheck)
{
	strcpy_s(m_StoreID, LFKeySize, StoreID ? StoreID : "");

	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_FILTER_THISSTORE));

	BOOL InStore = FALSE;
	if (m_StoreID[0]!='\0')
	{
		LFStoreDescriptor s;
		if (LFGetStoreSettings(m_StoreID, &s)==LFOk)
		{
			InStore = TRUE;

			tmpStr.Append(_T(" ("));
			tmpStr.Append(s.StoreName);
			tmpStr.Append(_T(")"));
		}
	}

	m_wndThisStore.EnableWindow(InStore);
	m_wndThisStore.SetWindowText(tmpStr);

	if (!InStore)
	{
		m_wndAllStores.SetCheck(TRUE);
		m_wndThisStore.SetCheck(FALSE);
	}
	else
		if (SetCheck)
		{
			m_wndAllStores.SetCheck(FALSE);
			m_wndThisStore.SetCheck(TRUE);
		}
}

void CFilterWnd::SetFilter(LFFilter* f)
{
	SetStoreID(f->StoreID, TRUE);
	m_wndSearchterm.SetWindowText(f->Searchterm);

	m_Conditions.m_ItemCount = 0;
	m_wndList.DeleteAllItems();

	LFFilterCondition* c = f->ConditionList;
	while (c)
	{
		m_Conditions.AddItem(*c);
		m_wndList.InsertItem(c);
		c = c->Next;
	}
}

LFFilter* CFilterWnd::CreateFilter()
{
	LFFilter* f = LFAllocFilter();
	f->Mode = LFFilterModeSearch;
	f->Options.IsSearch = true;
	strcpy_s(f->StoreID, LFKeySize, m_wndAllStores.GetCheck() ? "" : m_StoreID);
	m_wndSearchterm.GetWindowText(f->Searchterm, 256);

	for (INT a=m_Conditions.m_ItemCount-1; a>=0; a--)
	{
		LFFilterCondition* c = LFAllocFilterCondition();
		*c = m_Conditions.m_Items[a];
		c->Next = f->ConditionList;
		f->ConditionList = c;
	}

	return f;
}


BEGIN_MESSAGE_MAP(CFilterWnd, CGlasPane)
	ON_WM_CREATE()
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(NM_DBLCLK, IDLIST, OnDoubleClick)
	ON_COMMAND(IDOK, OnSearch)
	ON_COMMAND(IDM_FILTER_SAVE, OnSave)
	ON_COMMAND(IDM_FILTER_SEARCH, OnSearch)
	ON_COMMAND(IDM_CONDITIONLIST_ADD, OnAddCondition)
	ON_COMMAND(IDM_CONDITION_EDIT, OnEditCondition)
	ON_COMMAND(IDM_CONDITION_DELETE, OnDeleteCondition)
	ON_UPDATE_COMMAND_UI(IDOK, OnUpdateCommands)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILTER_SAVE, IDM_FILTER_SEARCH, OnUpdateCommands)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_CONDITION_EDIT, IDM_CONDITION_DELETE, OnUpdateCommands)
END_MESSAGE_MAP()

INT CFilterWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlasPane::OnCreate(lpCreateStruct)==-1)
		return -1;

	CDC* dc = GetWindowDC();
	HGDIOBJ hOldFont = dc->SelectStockObject(DEFAULT_GUI_FONT);
	m_FontHeight = dc->GetTextExtent(_T("Wy")).cy;
	dc->SelectObject(hOldFont);
	ReleaseDC(dc);

	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_TABSTOP;
	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_FILTER_SEARCHIN));
	if (!m_wndLabel1.Create(this, 1, tmpStr))
		return -1;
	ENSURE(tmpStr.LoadString(IDS_FILTER_ALLSTORES));
	if (!m_wndAllStores.Create(tmpStr, dwViewStyle | BS_AUTORADIOBUTTON | WS_GROUP, CRect(0, 0, 0, 0), this, 2))
		return -1;
	ENSURE(tmpStr.LoadString(IDS_FILTER_THISSTORE));
	if (!m_wndThisStore.Create(tmpStr, dwViewStyle | BS_AUTORADIOBUTTON, CRect(0, 0, 0, 0), this, 3))
		return -1;
	ENSURE(tmpStr.LoadString(IDS_FILTER_SEARCHTERM));
	if (!m_wndLabel2.Create(this, 4, tmpStr))
		return -1;
	if (!m_wndSearchterm.CreateEx(WS_EX_CLIENTEDGE, _T("EDIT"), _T(""), dwViewStyle | ES_AUTOHSCROLL, CRect(0, 0, 0, 0), this, 5))
		return -1;
	ENSURE(tmpStr.LoadString(IDS_FILTER_OTHERCONDITIONS));
	if (!m_wndLabel3.Create(this, 6, tmpStr))
		return -1;
	ENSURE(tmpStr.LoadString(IDS_FILTER_ADDCONDITION));
	if (!m_wndAddCondition.Create(tmpStr, dwViewStyle | BS_PUSHBUTTON, CRect(0, 0, 0, 0), this, IDM_CONDITIONLIST_ADD))
		return -1;
	if (!m_wndList.Create(dwViewStyle | LVS_NOCOLUMNHEADER | LVS_SHAREIMAGELISTS | LVS_SINGLESEL, CRect(0, 0, 0, 0), this, IDLIST))
		return -1;
	if (!m_wndBottomArea.Create(this, MAKEINTRESOURCE(IDD_FILTER), CBRS_BOTTOM, 8))
		return -1;

	m_wndAllStores.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
	m_wndThisStore.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
	m_wndSearchterm.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));
	m_wndAddCondition.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));

	m_wndAllStores.SetCheck(BST_CHECKED);

	m_wndList.SetView(LV_VIEW_TILE);
	m_wndList.SetMenus(IDM_CONDITIONLIST);

#ifdef DEBUG
	LFFilterCondition c;
	c.AttrData.Attr = LFAttrArtist;
	LFGetNullVariantData(&c.AttrData);
	c.AttrData.IsNull = false;
	c.Compare = LFFilterCompareIsEqual;
	wcscpy_s(c.AttrData.UnicodeString, 256, L"Madonna");
	m_Conditions.AddItem(c);
	m_wndList.InsertItem(&c);

	c.AttrData.Attr = LFAttrRating;
	LFGetNullVariantData(&c.AttrData);
	c.AttrData.IsNull = false;
	c.Compare = LFFilterCompareIsAboveOrEqual;
	c.AttrData.Rating = 7;
	m_Conditions.AddItem(c);
	m_wndList.InsertItem(&c);

	c.AttrData.Attr = LFAttrAlbum;
	LFGetNullVariantData(&c.AttrData);
	c.AttrData.IsNull = false;
	c.Compare = LFFilterCompareIsEqual;
	wcscpy_s(c.AttrData.UnicodeString, 256, L"True Blue");
	m_Conditions.AddItem(c);
	m_wndList.InsertItem(&c);
#endif

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

void CFilterWnd::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	OnEditCondition();
}


void CFilterWnd::OnSave()
{
	SaveFilterDlg dlg(this, m_StoreID, TRUE);
	if (dlg.DoModal()==IDOK)
	{
		CWaitCursor csr;

		LFErrorBox(LFSaveFilter(dlg.m_StoreID, CreateFilter(), dlg.m_FileName, dlg.m_Comments, NULL), GetSafeHwnd());
		GetOwner()->PostMessage(WM_COMMAND, ID_NAV_RELOAD);
	}
}

void CFilterWnd::OnSearch()
{
	theApp.ShowNagScreen(NAG_EXPIRED | NAG_FORCE, this);

	GetOwner()->SendMessage(WM_NAVIGATETO, (WPARAM)CreateFilter());
}

void CFilterWnd::OnAddCondition()
{
	EditConditionDlg dlg(this);
	if (dlg.DoModal()==IDOK)
	{
		m_Conditions.AddItem(dlg.m_Condition);
		m_wndList.InsertItem(&dlg.m_Condition);
	}

	m_wndList.SetFocus();
}

void CFilterWnd::OnEditCondition()
{
	INT idx = m_wndList.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
	if (idx!=-1)
	{
		EditConditionDlg dlg(this, &m_Conditions.m_Items[idx]);
		if (dlg.DoModal()==IDOK)
		{
			m_Conditions.m_Items[idx] = dlg.m_Condition;
			m_wndList.SetItem(idx, &dlg.m_Condition);
		}

		m_wndList.SetFocus();
	}
}

void CFilterWnd::OnDeleteCondition()
{
	INT idx = m_wndList.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
	if (idx!=-1)
	{
		m_Conditions.m_ItemCount--;
		for (INT a=idx; a<(INT)m_Conditions.m_ItemCount; a++)
			m_Conditions.m_Items[a] = m_Conditions.m_Items[a+1];

		m_wndList.DeleteItem(idx);
		m_wndList.Arrange(LVA_ALIGNTOP);
	}
}

void CFilterWnd::OnUpdateCommands(CCmdUI* pCmdUI)
{
	BOOL b = TRUE;

	switch (pCmdUI->m_nID)
	{
	case IDM_CONDITION_EDIT:
	case IDM_CONDITION_DELETE:
		b = (m_wndList.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED)!=-1);
		break;
	}

	pCmdUI->Enable(b);
}
