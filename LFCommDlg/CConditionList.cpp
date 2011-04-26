
// CConditionList.cpp: Implementierung der Klasse CConditionList
//

#include "stdafx.h"
#include "CConditionList.h"


// CConditionList
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;
extern INT GetAttributeIconIndex(UINT Attr);

CConditionList::CConditionList()
	: CListCtrl()
{
	p_App = (LFApplication*)AfxGetApp();
	hTheme = NULL;
	m_ItemMenuID = m_BackgroundMenuID = 0;
}

void CConditionList::PreSubclassWindow()
{
	CListCtrl::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (!pThreadState->m_pWndInit)
		Init();
}

void CConditionList::Init()
{
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	if ((p_App->m_ThemeLibLoaded) && (p_App->OSVersion>=OS_Vista))
	{
		p_App->zSetWindowTheme(GetSafeHwnd(), L"EXPLORER", NULL);
		hTheme = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
	}

	m_AttributeIcons16.Create(IDB_ATTRIBUTEICONS_16, LFCommDlgDLL.hResource, 0, -1, 16, 16);
	m_AttributeIcons32.Create(IDB_ATTRIBUTEICONS_16, LFCommDlgDLL.hResource, 0, -1, 32, 32);

	SetImageList(&m_AttributeIcons16, LVSIL_SMALL);
	SetImageList(&m_AttributeIcons32, LVSIL_NORMAL);

	/*LVTILEVIEWINFO tvi;
	ZeroMemory(&tvi, sizeof(tvi));
	tvi.cbSize = sizeof(LVTILEVIEWINFO);
	tvi.cLines = 2;
	tvi.dwFlags = LVTVIF_FIXEDWIDTH;
	tvi.dwMask = LVTVIM_COLUMNS | LVTVIM_TILESIZE;
	tvi.sizeTile.cx = 218;
	SetTileViewInfo(&tvi);*/
}

void CConditionList::SetMenus(UINT _ItemMenuID, BOOL _HighlightFirst, UINT _BackgroundMenuID)
{
	m_ItemMenuID = _ItemMenuID;
	m_HighlightFirst = _HighlightFirst;
	m_BackgroundMenuID = _BackgroundMenuID;
}


BEGIN_MESSAGE_MAP(CConditionList, CListCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
	ON_WM_CONTEXTMENU()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

INT CConditionList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct)==-1)
		return -1;

	Init();

	return 0;
}

void CConditionList::OnDestroy()
{
	if (hTheme)
		p_App->zCloseThemeData(hTheme);

	CListCtrl::OnDestroy();
}

LRESULT CConditionList::OnThemeChanged()
{
	if ((p_App->m_ThemeLibLoaded) && (p_App->OSVersion>=OS_Vista))
	{
		if (hTheme)
			p_App->zCloseThemeData(hTheme);

		hTheme = p_App->zOpenThemeData(m_hWnd, VSCLASS_LISTVIEW);
	}

	return TRUE;
}

void CConditionList::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
{
	LVHITTESTINFO pInfo;
	if ((pos.x<0) || (pos.y<0))
	{
		CRect r;
		GetItemRect(GetNextItem(-1, LVNI_FOCUSED), r, LVIR_ICON);
		pInfo.pt.x = pos.x = r.left;
		pInfo.pt.y = r.top;

		GetItemRect(GetNextItem(-1, LVNI_FOCUSED), r, LVIR_LABEL);
		pos.y = r.bottom;
	}
	else
	{
		ScreenToClient(&pos);
		pInfo.pt = pos;
	}

	SubItemHitTest(&pInfo);

	UINT MenuID = m_BackgroundMenuID;
	if (pInfo.iItem!=-1)
		if (GetNextItem(pInfo.iItem-1, LVNI_FOCUSED | LVNI_SELECTED)==pInfo.iItem)
			MenuID = m_ItemMenuID;

	if (MenuID)
	{
		ClientToScreen(&pos);

		CMenu Menu;
		Menu.LoadMenu(MenuID);
		ASSERT_VALID(&Menu);

		CMenu* pPopup = Menu.GetSubMenu(0);
		ASSERT_VALID(pPopup);

		if ((pInfo.iItem!=-1) && (m_HighlightFirst))
			pPopup->SetDefaultItem(0, TRUE);

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, GetOwner());
	}
}

void CConditionList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
/*	switch(nChar)
	{
	}*/

	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}
