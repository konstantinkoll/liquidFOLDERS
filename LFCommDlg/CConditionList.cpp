
// CConditionList.cpp: Implementierung der Klasse CConditionList
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CConditionList
//

static UINT puColumns[] = { 1, 2 };

CConditionList::CConditionList()
	: CListCtrl()
{
	hTheme = NULL;
	m_BackgroundMenuID = 0;
	m_LastWidth = -1;

	for (UINT a=0; a<LFFilterCompareCount; a++)
		ENSURE(m_Compare[a].LoadString(IDS_COMPARE_FIRST+a));
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
	ModifyStyle(0, LVS_SHAREIMAGELISTS);
	SetExtendedStyle(GetExtendedStyle() | LVS_EX_FULLROWSELECT | LVS_EX_DOUBLEBUFFER);

	if ((LFGetApp()->m_ThemeLibLoaded) && (LFGetApp()->OSVersion>=OS_Vista))
	{
		LFGetApp()->zSetWindowTheme(GetSafeHwnd(), L"EXPLORER", NULL);
		hTheme = LFGetApp()->zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
	}

	m_AttributeIcons16.Create(IDB_ATTRIBUTEICONS_16);
	SetImageList(&m_AttributeIcons16, LVSIL_SMALL);

	m_AttributeIcons32.Create(IDB_ATTRIBUTEICONS_32, 32, 32);
	SetImageList(&m_AttributeIcons32, LVSIL_NORMAL);

	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));
	lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	lvc.pszText = L"";
	lvc.fmt = LVCFMT_LEFT;
	for (UINT a=0; a<3; a++)
	{
		lvc.iSubItem = a;
		InsertColumn(a, &lvc);
	}

	LVTILEVIEWINFO tvi;
	ZeroMemory(&tvi, sizeof(tvi));
	tvi.cbSize = sizeof(LVTILEVIEWINFO);
	tvi.cLines = 1;
	tvi.dwFlags = LVTVIF_FIXEDWIDTH;
	tvi.dwMask = LVTVIM_COLUMNS;
	SetTileViewInfo(&tvi);
}

BOOL CConditionList::SetWindowPos(const CWnd* pWndInsertAfter, INT x, INT y, INT cx, INT cy, UINT nFlags)
{
	if (cx<m_LastWidth)
	{
		SetTileSize(cx);

		return CListCtrl::SetWindowPos(pWndInsertAfter, x, y, cx, cy, nFlags);
	}
	else
	{
		BOOL Result = CListCtrl::SetWindowPos(pWndInsertAfter, x, y, cx, cy, nFlags);
		SetTileSize(cx);

		return Result;
	}
}

void CConditionList::SetTileSize(INT cx)
{
	if (cx==-1)
	{
		cx = m_LastWidth;
	}
	else
	{
		m_LastWidth = cx;
	}

	if (GetStyle() & WS_VSCROLL)
		cx -= GetSystemMetrics(SM_CXVSCROLL);

	LVTILEVIEWINFO tvi;
	ZeroMemory(&tvi, sizeof(tvi));
	tvi.cbSize = sizeof(LVTILEVIEWINFO);
	tvi.dwFlags = LVTVIF_FIXEDWIDTH;
	tvi.dwMask = LVTVIM_TILESIZE;
	tvi.sizeTile.cx = (IsGroupViewEnabled() && (LFGetApp()->OSVersion==OS_XP)) ? cx-16 : cx;
	SetTileViewInfo(&tvi);
}

void CConditionList::ConditionToItem(LFFilterCondition* c, LVITEM& lvi)
{
	ZeroMemory(&lvi, sizeof(lvi));

	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_COLUMNS;
	lvi.cColumns = 2;
	lvi.puColumns = puColumns;
	lvi.pszText = LFGetApp()->m_Attributes[c->AttrData.Attr].Name;
	lvi.iImage = GetAttributeIconIndex(c->AttrData.Attr);
}

void CConditionList::FinishItem(INT nItem, LFFilterCondition* c)
{
	ASSERT(c->Compare<LFFilterCompareCount);

	WCHAR tmpStr[512];
	wcscpy_s(tmpStr, 512, m_Compare[c->Compare]);

	if (c->Compare)
	{
		wcscat_s(tmpStr, 512, L" ");
		LFVariantDataToString(&c->AttrData, &tmpStr[wcslen(tmpStr)], 512-wcslen(tmpStr));
	}

	SetItemText(nItem, 1, tmpStr);
}

void CConditionList::InsertItem(LFFilterCondition* c)
{
	LVITEM lvi;
	ConditionToItem(c, lvi);
	lvi.iItem = GetItemCount();

	FinishItem(CListCtrl::InsertItem(&lvi), c);
}

void CConditionList::SetItem(INT nItem, LFFilterCondition* c)
{
	LVITEM lvi;
	ConditionToItem(c, lvi);
	lvi.iItem = nItem;

	if (CListCtrl::SetItem(&lvi))
		FinishItem(nItem, c);
}

void CConditionList::SetMenus(UINT BackgroundMenuID)
{
	m_BackgroundMenuID = BackgroundMenuID;
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
		LFGetApp()->zCloseThemeData(hTheme);

	CListCtrl::OnDestroy();
}

LRESULT CConditionList::OnThemeChanged()
{
	if ((LFGetApp()->m_ThemeLibLoaded) && (LFGetApp()->OSVersion>=OS_Vista))
	{
		if (hTheme)
			LFGetApp()->zCloseThemeData(hTheme);

		hTheme = LFGetApp()->zOpenThemeData(GetSafeHwnd(), VSCLASS_LISTVIEW);
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
			MenuID = IDM_CONDITION;

	if (MenuID)
	{
		ClientToScreen(&pos);

		CMenu Menu;
		Menu.LoadMenu(MenuID);
		ASSERT_VALID(&Menu);

		CMenu* pPopup = Menu.GetSubMenu(0);
		ASSERT_VALID(pPopup);

		if (pInfo.iItem!=-1)
			pPopup->SetDefaultItem(0, TRUE);

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, GetOwner());
	}
}

void CConditionList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch(nChar)
	{
	case VK_EXECUTE:
	case VK_RETURN:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			GetOwner()->PostMessage(WM_COMMAND, IDM_CONDITION_EDIT);
		break;
	case VK_DELETE:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
			GetOwner()->PostMessage(WM_COMMAND, IDM_CONDITION_DELETE);
		break;
	default:
		CListCtrl::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}
