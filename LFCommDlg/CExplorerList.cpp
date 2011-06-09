
// CExplorerList.cpp: Implementierung der Klasse CExplorerList
//

#include "stdafx.h"
#include "CExplorerList.h"


// CExplorerList
//

CExplorerList::CExplorerList()
	: CListCtrl()
{
	p_App = (LFApplication*)AfxGetApp();
	p_Result = NULL;
	hTheme = NULL;
	m_ItemMenuID = m_BackgroundMenuID = 0;
}

void CExplorerList::PreSubclassWindow()
{
	CListCtrl::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (!pThreadState->m_pWndInit)
		Init();
}

void CExplorerList::Init()
{
	ModifyStyle(0, LVS_SHAREIMAGELISTS);
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	if ((p_App->m_ThemeLibLoaded) && (p_App->OSVersion>=OS_Vista))
	{
		p_App->zSetWindowTheme(GetSafeHwnd(), L"EXPLORER", NULL);
		hTheme = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
	}

	LVTILEVIEWINFO tvi;
	ZeroMemory(&tvi, sizeof(tvi));
	tvi.cbSize = sizeof(LVTILEVIEWINFO);
	tvi.cLines = 2;
	tvi.dwFlags = LVTVIF_FIXEDWIDTH;
	tvi.dwMask = LVTVIM_COLUMNS | LVTVIM_TILESIZE;
	tvi.sizeTile.cx = 218;
	SetTileViewInfo(&tvi);
}

void CExplorerList::AddCategory(INT ID, CString Name, CString Hint, BOOL Collapsable)
{
	LVGROUP lvg;
	ZeroMemory(&lvg, sizeof(lvg));

	lvg.cbSize = sizeof(lvg);
	lvg.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN;
	lvg.uAlign = LVGA_HEADER_LEFT;
	lvg.iGroupId = ID;
	lvg.pszHeader = Name.GetBuffer();
	if (p_App->OSVersion>=OS_Vista)
	{
		if (!Hint.IsEmpty())
		{
			lvg.pszSubtitle = Hint.GetBuffer();
			lvg.mask |= LVGF_SUBTITLE;
		}
		if (Collapsable)
		{
			lvg.stateMask = LVGS_COLLAPSIBLE;
			lvg.mask |= LVGF_STATE;
		}
	}

	InsertGroup(ID, &lvg);
}

void CExplorerList::AddItemCategories()
{
	for (UINT a=0; a<LFItemCategoryCount; a++)
		AddCategory(a, p_App->m_ItemCategories[a]->Caption, p_App->m_ItemCategories[a]->Hint);
}

void CExplorerList::AddColumn(INT ID, CString Name)
{
	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));

	lvc.mask = LVCF_TEXT | LVCF_SUBITEM;
	lvc.pszText = Name.GetBuffer();
	lvc.iSubItem = ID;
	
	InsertColumn(ID, &lvc);
}

void CExplorerList::AddColumn(INT ID, UINT Attr)
{
	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.pszText = p_App->m_Attributes[Attr]->Name;
	lvc.cx = p_App->m_Attributes[Attr]->RecommendedWidth;
	lvc.fmt = p_App->m_Attributes[Attr]->FormatRight ? LVCFMT_RIGHT : LVCFMT_LEFT;
	lvc.iSubItem = ID;

	InsertColumn(ID, &lvc);
}

void CExplorerList::AddStoreColumns()
{
	AddColumn(LFAttrFileName, 0);
	AddColumn(LFAttrComments, 1);
	AddColumn(LFAttrCreationTime, 2);
	AddColumn(LFAttrStoreID, 3);
	AddColumn(LFAttrDescription, 4);
}

void CExplorerList::SetSearchResult(LFSearchResult* result)
{
	DeleteAllItems();

	p_Result = result;
	if (result)
	{
		LFSortSearchResult(result, LFAttrFileName, false);
		LFErrorBox(result->m_LastError, GetParent()->GetSafeHwnd());

		UINT puColumns[2];
		puColumns[0] = 1;
		puColumns[1] = 4;

		LVITEM lvi;
		ZeroMemory(&lvi, sizeof(lvi));
		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_GROUPID | LVIF_COLUMNS | LVIF_STATE;
		lvi.puColumns = puColumns;

		for (UINT a=0; a<result->m_ItemCount; a++)
		{
			lvi.iItem = a;
			lvi.cColumns = 2;
			lvi.pszText = (LPWSTR)result->m_Items[a]->CoreAttributes.FileName;
			lvi.iImage = result->m_Items[a]->IconID-1;
			lvi.iGroupId = result->m_Items[a]->CategoryID;
			lvi.state = (result->m_Items[a]->Type & LFTypeGhosted) ? LVIS_CUT : 0;
			lvi.stateMask = LVIS_CUT;
			INT idx = InsertItem(&lvi);

			WCHAR tmpStr[256];
			SetItemText(idx, 1, result->m_Items[a]->CoreAttributes.Comment);
			LFAttributeToString(result->m_Items[a], LFAttrCreationTime, tmpStr, 256);
			SetItemText(idx, 2, tmpStr);
			LFAttributeToString(result->m_Items[a], LFAttrStoreID, tmpStr, 256);
			SetItemText(idx, 3, tmpStr);
			SetItemText(idx, 4, result->m_Items[a]->Description);
		}
	}

	if (GetView()==LV_VIEW_DETAILS)
		for (UINT a=0; a<5; a++)
			SetColumnWidth(a, LVSCW_AUTOSIZE_USEHEADER);
}

void CExplorerList::SetMenus(UINT ItemMenuID, BOOL HighlightFirst, UINT BackgroundMenuID)
{
	m_ItemMenuID = ItemMenuID;
	m_HighlightFirst = HighlightFirst;
	m_BackgroundMenuID = BackgroundMenuID;
}


BEGIN_MESSAGE_MAP(CExplorerList, CListCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
	ON_WM_CONTEXTMENU()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

INT CExplorerList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct)==-1)
		return -1;

	Init();

	return 0;
}

void CExplorerList::OnDestroy()
{
	if (hTheme)
		p_App->zCloseThemeData(hTheme);

	CListCtrl::OnDestroy();
}

LRESULT CExplorerList::OnThemeChanged()
{
	if ((p_App->m_ThemeLibLoaded) && (p_App->OSVersion>=OS_Vista))
	{
		if (hTheme)
			p_App->zCloseThemeData(hTheme);

		hTheme = p_App->zOpenThemeData(m_hWnd, VSCLASS_LISTVIEW);
	}

	return TRUE;
}

void CExplorerList::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
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

void CExplorerList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_F2:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
		{
			EditLabel(GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED));
			return;
		}
		break;
	case VK_DELETE:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
		{
			GetOwner()->SendMessage(WM_COMMAND, IDM_STORE_DELETE);
			return;
		}
		break;
	}

	CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
}
