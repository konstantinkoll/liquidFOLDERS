
// LFChooseStoreDlg.cpp: Implementierung der Klasse LFChooseStoreDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFChooseStoreDlg
//

#define GetSelectedStore() m_wndExplorerList.GetNextItem(-1, LVIS_SELECTED)

LFChooseStoreDlg::LFChooseStoreDlg(CWnd* pParentWnd, BOOL Mounted)
	: LFDialog(IDD_CHOOSESTORE, pParentWnd)
{
	m_StoreID[0] = '\0';
	m_pResult = NULL;
	m_Mounted = Mounted;
}

LFChooseStoreDlg::~LFChooseStoreDlg()
{
	if (m_pResult)
		LFFreeSearchResult(m_pResult);
}

void LFChooseStoreDlg::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
	{
		INT Index = GetSelectedStore();
		strcpy_s(m_StoreID, LFKeySize, Index!=-1 ? m_pResult->m_Items[Index]->StoreID : "");
	}
}

void LFChooseStoreDlg::AdjustLayout()
{
	if (!IsWindow(m_wndExplorerList))
		return;

	CRect rect;
	GetLayoutRect(rect);

	UINT ExplorerHeight = 0;
	if (IsWindow(m_wndHeaderArea))
	{
		ExplorerHeight = m_wndHeaderArea.GetPreferredHeight();
		m_wndHeaderArea.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	}

	CRect borders(0, 0, 7, 7);
	MapDialogRect(&borders);

	INT borderLeft = (LFGetApp()->OSVersion==OS_XP) ? 15 : borders.Width()/2;
	m_wndExplorerList.SetWindowPos(NULL, rect.left+borderLeft, rect.top+ExplorerHeight, rect.Width()-borderLeft, rect.Height()-ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}

void LFChooseStoreDlg::UpdateOkButton()
{
	INT Index = GetSelectedStore();
	BOOL b = (Index!=-1);

	if (m_Mounted && b)
		b &= !(m_pResult->m_Items[Index]->Type & LFTypeNotMounted);

	GetDlgItem(IDOK)->EnableWindow(b);
}


BEGIN_MESSAGE_MAP(LFChooseStoreDlg, LFDialog)
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(NM_DBLCLK, IDC_STORELIST, OnDoubleClick)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_STORELIST, OnItemChanged)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_STORELIST, OnEndLabelEdit)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoresChanged, OnUpdateStores)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoreAttributesChanged, OnUpdateStores)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->DefaultStoreChanged, OnUpdateStores)
	ON_COMMAND(IDM_STORE_MAKEDEFAULT, OnStoreMakeDefault)
	ON_COMMAND(IDM_STORE_SHORTCUT, OnStoreShortcut)
	ON_COMMAND(IDM_STORE_DELETE, OnStoreDelete)
	ON_COMMAND(IDM_STORE_RENAME, OnStoreRename)
	ON_COMMAND(IDM_STORE_PROPERTIES, OnStoreProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORE_MAKEDEFAULT, IDM_STORE_PROPERTIES, OnUpdateStoreCommands)
END_MESSAGE_MAP()

BOOL LFChooseStoreDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	CString Hint;
	if (m_Mounted)
		ENSURE(Hint.LoadString(IDS_CHOOSESTORE_HINT));

	m_wndHeaderArea.Create(this, IDC_HEADERAREA);
	m_wndHeaderArea.SetText(LFGetApp()->m_Contexts[LFContextStores].Name, Hint, FALSE);

	CRect rect;
	rect.SetRectEmpty();
	m_wndExplorerList.Create(WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_SHOWSELALWAYS | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS | LVS_ALIGNTOP | LVS_EDITLABELS | LVS_SINGLESEL, rect, this, IDC_STORELIST);

	m_wndExplorerList.SetImageList(&LFGetApp()->m_CoreImageListSmall, LVSIL_SMALL);
	m_wndExplorerList.SetImageList(&LFGetApp()->m_CoreImageListExtraLarge, LVSIL_NORMAL);

	m_wndExplorerList.AddStoreColumns();
	m_wndExplorerList.AddItemCategories();
	m_wndExplorerList.SetMenus(IDM_STORE, FALSE, IDM_STORES);
	m_wndExplorerList.EnableGroupView(LFGetApp()->OSVersion>OS_XP);
	m_wndExplorerList.SetView(LV_VIEW_TILE);
	m_wndExplorerList.SetFocus();

	SendMessage(LFGetApp()->p_MessageIDs->StoresChanged);

	AdjustLayout();

	return FALSE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFChooseStoreDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	LFDialog::OnGetMinMaxInfo(lpMMI);

	CRect rect;
	GetWindowRect(rect);
	if (rect.Width())
		lpMMI->ptMinTrackSize.x = lpMMI->ptMaxTrackSize.x = rect.Width();

	lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, 300);
}

LRESULT LFChooseStoreDlg::OnUpdateStores(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	CHAR StoreID[LFKeySize] = "";
	if (m_pResult)
	{
		INT Index = m_wndExplorerList.GetNextItem(-1, LVIS_SELECTED);
		if (Index!=-1)
			strcpy_s(StoreID, LFKeySize, m_pResult->m_Items[Index]->StoreID);

		LFFreeSearchResult(m_pResult);
	}

	LFFilter* filter = LFAllocFilter();
	m_pResult = LFQuery(filter);
	LFFreeFilter(filter);

	m_wndExplorerList.SetSearchResult(m_pResult);

	if (!m_Mounted)
	{
		CString Hint;
		Hint.Format(m_pResult->m_StoreCount==1 ? IDS_STORES_SINGULAR : IDS_STORES_PLURAL, m_pResult->m_StoreCount);

		m_wndHeaderArea.SetText(LFGetApp()->m_Contexts[LFContextStores].Name, Hint);
	}

	INT Index = -1;
	for (UINT a=0; a<m_pResult->m_ItemCount; a++)
		if (((Index==-1) && (m_pResult->m_Items[a]->Type & LFTypeDefault)) || (!strcmp(StoreID, m_pResult->m_Items[a]->StoreID)))
			Index = a;

	m_wndExplorerList.SetItemState(Index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	return NULL;
}

void LFChooseStoreDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	if ((GetSelectedStore()!=-1) && (GetDlgItem(IDOK)->IsWindowEnabled()))
		PostMessage(WM_COMMAND, (WPARAM)IDOK);
}

void LFChooseStoreDlg::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && ((pNMListView->uOldState & LVIS_SELECTED) || (pNMListView->uNewState & LVIS_SELECTED)))
		UpdateOkButton();
}

void LFChooseStoreDlg::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	*pResult = FALSE;

	if ((m_pResult) && (pDispInfo->item.pszText))
		if (pDispInfo->item.pszText[0]!=L'\0')
		{
			LFTransactionList* pTransactionList = LFAllocTransactionList();
			LFAddTransactionItem(pTransactionList, m_pResult->m_Items[pDispInfo->item.iItem]);

			LFVariantData value;
			value.Attr = LFAttrFileName;
			value.Type = LFTypeUnicodeString;
			value.IsNull = FALSE;

			wcsncpy_s(value.UnicodeString, 256, pDispInfo->item.pszText, 255);

			LFTransactionUpdate(pTransactionList, &value);
			LFErrorBox(pTransactionList->m_LastError, GetSafeHwnd());

			LFFreeTransactionList(pTransactionList);
			*pResult = TRUE;
		}
}


void LFChooseStoreDlg::OnStoreMakeDefault()
{
	INT Index = GetSelectedStore();
	if (Index!=-1)
		LFErrorBox(LFMakeDefaultStore(m_pResult->m_Items[Index]->StoreID), GetSafeHwnd());
}

void LFChooseStoreDlg::OnStoreShortcut()
{
	INT Index = GetSelectedStore();
	if (Index!=-1)
		if (LFAskCreateShortcut(GetSafeHwnd()))
			LFCreateDesktopShortcutForStore(m_pResult->m_Items[Index]);
}

void LFChooseStoreDlg::OnStoreDelete()
{
	INT Index = GetSelectedStore();
	if (Index!=-1)
		LFDeleteStore(m_pResult->m_Items[Index]->StoreID, this);
}

void LFChooseStoreDlg::OnStoreRename()
{
	INT Index = GetSelectedStore();
	if (Index!=-1)
	{
		if (GetFocus()!=&m_wndExplorerList)
			m_wndExplorerList.SetFocus();

		m_wndExplorerList.EditLabel(Index);
	}
}

void LFChooseStoreDlg::OnStoreProperties()
{
	INT Index = GetSelectedStore();
	if (Index!=-1)
	{
		LFStorePropertiesDlg dlg(m_pResult->m_Items[Index]->StoreID, this);
		dlg.DoModal();
	}
}

void LFChooseStoreDlg::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	INT Index = GetSelectedStore();
	if (Index!=-1)
	{
		LFItemDescriptor* Item = m_pResult->m_Items[Index];
		b = ((Item->Type & LFTypeMask)==LFTypeStore);

		switch (pCmdUI->m_nID)
		{
		case IDM_STORE_MAKEDEFAULT:
			b = !(Item->Type & LFTypeDefault);
			break;

		case IDM_STORE_IMPORTFOLDER:
			b = FALSE;
			break;

		case IDM_STORE_SHORTCUT:
			b = (Item->Type & LFTypeShortcutAllowed);
			break;

		case IDM_STORE_RENAME:
			b = (m_wndExplorerList.GetEditControl()==NULL);
			break;
		}
	}

	pCmdUI->Enable(b);
}
