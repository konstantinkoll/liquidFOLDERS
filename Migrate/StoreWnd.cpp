
#include "stdafx.h"
#include "StoreWnd.h"
#include "Resource.h"
#include "liquidFOLDERS.h"
#include "LFCore.h"
#include "MainFrm.h"
#include "Migrate.h"


// CStoreWnd
//

CStoreWnd::CStoreWnd()
{
	result = NULL;
}

CStoreWnd::~CStoreWnd()
{
	if (result)
	{
		LFFreeSearchResult(result);
		result = NULL;
	}
}

void CStoreWnd::AddStoreItem(LFItemDescriptor* i)
{
	UINT puColumns[] = { 1, 2 };
	LVITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_COLUMNS | LVIF_GROUPID | LVIF_STATE;
	lvi.cColumns = 2;
	lvi.puColumns = puColumns;
	lvi.iGroupId = i->CategoryID;
	lvi.iItem = m_wndList.GetItemCount();
	lvi.pszText = i->CoreAttributes.FileName;
	lvi.iImage = i->IconID-1;
	lvi.stateMask = LVIS_CUT;
	lvi.state = (i->Type & LFTypeGhosted) ? LVIS_CUT : 0;
	int idx = m_wndList.InsertItem(&lvi);

	wchar_t tmpStr[256];
	LFAttributeToString(i, LFAttrCreationTime, tmpStr, 256);

	m_wndList.SetItemText(idx, 1, i->CoreAttributes.Comment);
	m_wndList.SetItemText(idx, 2, tmpStr);
}

void CStoreWnd::UpdateStores(BOOL FocusDefaultStore)
{
	char StoreID[LFKeySize] = "";
	int idx = m_wndList.GetNextItem(-1, LVNI_SELECTED);

	if (result)
	{
		if ((idx!=-1) && (!FocusDefaultStore))
			strcpy_s(StoreID, LFKeySize, result->m_Items[idx]->StoreID);

		LFFreeSearchResult(result);
		result = NULL;
	}

	result = LFQuery(NULL);

	LFSortSearchResult(result, LFAttrFileName, false, true);

	m_wndList.DeleteAllItems();

	idx = -1;
	for (UINT a=0; a<result->m_ItemCount; a++)
	{
		AddStoreItem(result->m_Items[a]);

		if (((idx==-1) && (result->m_Items[a]->Type & LFTypeDefaultStore)) || (strcmp(StoreID, result->m_Items[a]->StoreID)==0))
			idx = a;
	}

	m_wndList.SetItemState(idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
}

int CStoreWnd::GetSelectedItem()
{
	return m_wndList.GetNextItem(-1, LVNI_SELECTED | LVNI_FOCUSED);
}


BEGIN_MESSAGE_MAP(CStoreWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_PAINT()
	ON_NOTIFY(LVN_ENDLABELEDIT, ID_STORELIST, OnEndLabelEdit)
	ON_COMMAND(ID_STORE_NEW, OnStoreNew)
	ON_COMMAND(ID_STORE_DELETE, OnStoreDelete)
	ON_COMMAND(ID_STORE_RENAME, OnStoreRename)
	ON_COMMAND(ID_STORE_MAKEDEFAULT, OnStoreMakeDefault)
	ON_COMMAND(ID_STORE_MAKEHYBRID, OnStoreMakeHybrid)
	ON_COMMAND(ID_STORE_PROPERTIES, OnStoreProperties)
	ON_COMMAND(ID_STORE_SHOWCATEGORIES, OnStoreShowCategories)
	ON_UPDATE_COMMAND_UI_RANGE(ID_STORE_DELETE, ID_STORE_SHOWCATEGORIES, OnUpdateCommands)
END_MESSAGE_MAP()

int CStoreWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct)==-1)
		return -1;

	theApp.SetRegistryBase(_T("Stores"));
	m_Categories = theApp.GetInt(_T("Categories"), TRUE);
	theApp.SetRegistryBase(_T("Workspace"));

	if (m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE) == -1)
		return -1;

	m_wndToolBar.LoadToolBar(ID_PANE_STOREWND, 0, 0, TRUE);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | LVS_NOCOLUMNHEADER | LVS_SHOWSELALWAYS | LVS_SINGLESEL | LVS_EDITLABELS;
	if (!m_wndList.Create(dwViewStyle, CRect(0, 0, 0, 0), this, ID_STORELIST))
		return -1;

	const DWORD dwExStyle = LVS_EX_DOUBLEBUFFER | LVS_EX_LABELTIP;
	m_wndList.SetExtendedStyle(dwExStyle);
	m_wndList.EnableGroupView(m_Categories);
	m_wndList.SetContextMenu(IDM_STORE);

	LVGROUP lvg;
	ZeroMemory(&lvg, sizeof(lvg));
	lvg.cbSize = sizeof(lvg);
	lvg.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN;
	lvg.uAlign = LVGA_HEADER_LEFT;
	if (theApp.osInfo.dwMajorVersion>=6)
	{
		lvg.mask |= LVGF_STATE;
		lvg.state = LVGS_COLLAPSIBLE;
		lvg.stateMask = 0;
	}

	for (UINT a=0; a<LFItemCategoryCount; a++)
	{
		lvg.iGroupId = a;
		lvg.pszHeader = theApp.m_ItemCategories[a]->Name;
		m_wndList.InsertGroup(a, &lvg);
	}

	LV_COLUMN lvc;
	ZeroMemory(&lvc, sizeof(lvc));

	lvc.mask = LVCF_FMT | LVCF_TEXT | LVCF_SUBITEM;
	for (UINT a=0; a<3; a++)
	{
		lvc.fmt = LVCFMT_LEFT;
		lvc.iSubItem = a;
		m_wndList.InsertColumn(a, &lvc);
	}

	m_wndList.SetView(LV_VIEW_TILE);

	// Load icons
	HINSTANCE hModIcons = LoadLibrary(_T("LFCORE.DLL"));
	if (hModIcons!=NULL)
	{
		theApp.ExtractCoreIcons(hModIcons, 32, &m_Icons);
		FreeLibrary(hModIcons);

		m_wndList.SetImageList(&m_Icons, LVSIL_NORMAL);
	}

	UpdateStores(TRUE);
	LFErrorBox(result->m_LastError);

	return 0;
}

void CStoreWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	CRect rectClient;
	GetClientRect(rectClient);

	int heightTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), heightTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndList.SetWindowPos(NULL, rectClient.left, rectClient.top+heightTlb, rectClient.Width(), rectClient.Height()-heightTlb, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CStoreWnd::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);
	m_wndList.SetFocus();
}

void CStoreWnd::OnPaint()
{
	m_wndList.SetBkColor(afxGlobalData.clrBarFace);
	m_wndList.SetTextBkColor(afxGlobalData.clrBarFace);

	CDockablePane::OnPaint();
}

void CStoreWnd::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;
	CString name = pDispInfo->item.pszText;

	if (name!="")
		if (result)
			LFErrorBox(LFSetStoreAttributes(result->m_Items[pDispInfo->item.iItem]->StoreID, name.GetBuffer(), NULL));

	*pResult = FALSE;
}

void CStoreWnd::OnStoreNew()
{
	LFStoreDescriptor* s = LFAllocStoreDescriptor();

	LFStoreNewDlg dlg(this, IDD_STORENEW, '\0', s);
	if (dlg.DoModal()==IDOK)
		LFErrorBox(LFCreateStore(s, dlg.makeDefault));

	LFFreeStoreDescriptor(s);
}

void CStoreWnd::OnStoreDelete()
{
	int i = GetSelectedItem();

	if (i!=-1)
	{
		LFItemDescriptor* store = LFAllocItemDescriptor(result->m_Items[i]);
		LFErrorBox(theApp.DeleteStore(store));
		LFFreeItemDescriptor(store);
	}
}

void CStoreWnd::OnStoreRename()
{
	int i = GetSelectedItem();

	if (i!=-1)
	{
		m_wndList.SetFocus();
		m_wndList.EditLabel(i);
	}
}

void CStoreWnd::OnStoreMakeDefault()
{
	int i = GetSelectedItem();

	if (i!=-1)
		LFErrorBox(LFMakeDefaultStore(result->m_Items[i]->StoreID));
}

void CStoreWnd::OnStoreMakeHybrid()
{
	int i = GetSelectedItem();

	if (i!=-1)
		LFErrorBox(LFMakeHybridStore(result->m_Items[i]->StoreID));
}

void CStoreWnd::OnStoreProperties()
{
	int i = GetSelectedItem();

	if (i!=-1)
	{
		LFStorePropertiesDlg dlg(result->m_Items[i]->StoreID, this);
		dlg.DoModal();
	}
}

void CStoreWnd::OnStoreShowCategories()
{
	m_Categories = !m_Categories;
	m_wndList.EnableGroupView(m_Categories);

	theApp.SetRegistryBase(_T("Stores"));
	theApp.WriteInt(_T("Categories"), m_Categories);
	theApp.SetRegistryBase(_T("Workspace"));
}

void CStoreWnd::OnUpdateCommands(CCmdUI* pCmdUI)
{
	int i = GetSelectedItem();
	BOOL b = FALSE;
	switch (pCmdUI->m_nID)
	{
	case ID_STORE_NEW:
	case ID_STORE_SHOWCATEGORIES:
		b = TRUE;
		break;
	case ID_STORE_DELETE:
	case ID_STORE_RENAME:
	case ID_STORE_PROPERTIES:
		b = (i!=-1);
		break;
	case ID_STORE_MAKEDEFAULT:
		if ((i!=-1) && (result))
			b = (result->m_Items[i]->CategoryID==LFCategoryInternalStores) &&
				((result->m_Items[i]->Type & LFTypeDefaultStore)==0);
		break;
	case ID_STORE_MAKEHYBRID:
		if ((i!=-1) && (result))
			b = (result->m_Items[i]->CategoryID==LFCategoryExternalStores);
	}

	pCmdUI->Enable(b);
}
