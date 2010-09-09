
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
	p_FooterHandler = NULL;
	hTheme = NULL;
	m_ItemMenuID = m_BackgroundMenuID = 0;
}

void CExplorerList::EnableTheming()
{
	if ((p_App->m_ThemeLibLoaded) && (p_App->OSVersion>=OS_Vista))
	{
		p_App->zSetWindowTheme(GetSafeHwnd(), L"explorer", NULL);
		hTheme = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
	}
}

void CExplorerList::AddCategory(int ID, CString name, CString hint)
{
	LVGROUP lvg;
	ZeroMemory(&lvg, sizeof(lvg));

	lvg.cbSize = sizeof(lvg);
	lvg.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN;
	lvg.uAlign = LVGA_HEADER_LEFT;
	lvg.iGroupId = ID;
	lvg.pszHeader = name.GetBuffer();
	if ((!hint.IsEmpty()) && (p_App->OSVersion>=OS_Vista))
	{
		lvg.pszSubtitle = hint.GetBuffer();
		lvg.mask |= LVGF_SUBTITLE;
	}

	InsertGroup(ID, &lvg);
}

void CExplorerList::AddItemCategories()
{
	for (UINT a=0; a<LFItemCategoryCount; a++)
		AddCategory(a, p_App->m_ItemCategories[a]->Name, p_App->m_ItemCategories[a]->Hint);
}

void CExplorerList::AddColumn(int ID, CString name)
{
	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));

	lvc.mask = LVCF_TEXT | LVCF_SUBITEM;
	lvc.pszText = name.GetBuffer();
	lvc.iSubItem = ID;
	
	InsertColumn(ID, &lvc);
}

void CExplorerList::AddColumn(int ID, UINT attr)
{
	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));

	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvc.pszText = p_App->m_Attributes[attr]->Name;
	lvc.cx = p_App->m_Attributes[attr]->RecommendedWidth;
	lvc.fmt = p_App->m_Attributes[attr]->FormatRight ? LVCFMT_RIGHT : LVCFMT_LEFT;
	lvc.iSubItem = ID;

	InsertColumn(ID, &lvc);
}

void CExplorerList::AddStoreColumns()
{
	AddColumn(LFAttrFileName, 0);
	AddColumn(LFAttrComment, 1);
	AddColumn(LFAttrCreationTime, 2);
	AddColumn(LFAttrStoreID, 3);
	AddColumn(LFAttrDescription, 4);
}

void CExplorerList::SetSearchResult(LFSearchResult* result)
{
	DeleteAllItems();

	if (result)
	{
		LFSortSearchResult(result, LFAttrFileName, false, IsGroupViewEnabled()==TRUE);
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
			int idx = InsertItem(&lvi);

			wchar_t tmpStr[256];
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

void CExplorerList::SetMenus(UINT _ItemMenuID, UINT _BackgroundMenuID)
{
	m_ItemMenuID = _ItemMenuID;
	m_BackgroundMenuID = _BackgroundMenuID;
}

BOOL CExplorerList::SupportsFooter()
{
	return (p_FooterHandler!=NULL);
}

void CExplorerList::ShowFooter(IListViewFooterCallback* pCallbackObject)
{
	if (p_FooterHandler)
		p_FooterHandler->Show(pCallbackObject);
}

void CExplorerList::RemoveFooter()
{
	if (p_FooterHandler)
	{
		BOOL Visible;
		p_FooterHandler->IsVisible(&Visible);

		if (Visible)
			p_FooterHandler->RemoveAllButtons();
	}
}

void CExplorerList::SetFooterText(LPCWSTR pText)
{
	if (p_FooterHandler)
		p_FooterHandler->SetIntroText(pText);
}

void CExplorerList::InsertFooterButton(int insertAt, LPCWSTR pText, LPCWSTR pUnknown, UINT iconIndex, LONG lParam)
{
	if (p_FooterHandler)
		p_FooterHandler->InsertButton(insertAt, pText, pUnknown, iconIndex, lParam);
}


BEGIN_MESSAGE_MAP(CExplorerList, CListCtrl)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

int CExplorerList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CListCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);
	EnableTheming();

	LVTILEVIEWINFO tvi;
	ZeroMemory(&tvi, sizeof(tvi));
	tvi.cbSize = sizeof(LVTILEVIEWINFO);
	tvi.cLines = 2;
	tvi.dwFlags = LVTVIF_FIXEDWIDTH;
	tvi.dwMask = LVTVIM_COLUMNS | LVTVIM_TILESIZE;
	tvi.sizeTile.cx = 220;
	SetTileViewInfo(&tvi);

	if (p_App->OSVersion>=OS_Vista)
		SendMessage(0x10BD, (WPARAM)&IID_IListViewFooter, (LPARAM)&p_FooterHandler);

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
		pInfo.pt.x = r.left;
		pInfo.pt.y = r.top;
		pos = pInfo.pt;
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

		CMenu* PopupMenu = Menu.GetSubMenu(0);
		ASSERT_VALID(PopupMenu);

		if (pInfo.iItem!=-1)
			PopupMenu->SetDefaultItem(0, TRUE);

		TrackPopupMenu(PopupMenu->GetSafeHmenu(), TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, 0, GetOwner()->GetSafeHwnd(), NULL);
	}
}
