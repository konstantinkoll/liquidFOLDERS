
// LFChooseStoreDlg.cpp: Implementierung der Klasse LFChooseStoreDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CStoreList
//

void CStoreList::AddColumn(INT ID, UINT Attr)
{
	CExplorerList::AddColumn(ID, LFGetApp()->m_Attributes[Attr].Name, LFGetApp()->m_Attributes[Attr].RecommendedWidth, LFGetApp()->m_Attributes[Attr].FormatRight);
}

void CStoreList::AddStoreColumns()
{
	AddColumn(0, LFAttrFileName);
	AddColumn(1, LFAttrComments);
	AddColumn(2, LFAttrDescription);
	AddColumn(3, LFAttrCreationTime);
	AddColumn(4, LFAttrStoreID);
}

void CStoreList::AddItemCategories()
{
	for (UINT a=0; a<LFItemCategoryCount; a++)
		AddCategory(a, LFGetApp()->m_ItemCategories[a].Caption, LFGetApp()->m_ItemCategories[a].Hint);
}

void CStoreList::SetSearchResult(LFSearchResult* pSearchResult)
{
	DeleteAllItems();

	if (pSearchResult)
	{
		LFSortSearchResult(pSearchResult, LFAttrFileName, FALSE);
		LFErrorBox(this, pSearchResult->m_LastError);

		static UINT puColumns[2] = { 1, 2 };

		LVITEM lvi;
		ZeroMemory(&lvi, sizeof(lvi));

		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_GROUPID | LVIF_COLUMNS | LVIF_STATE;
		lvi.stateMask = LVIS_CUT | LVIS_OVERLAYMASK;
		lvi.cColumns = 2;
		lvi.puColumns = puColumns;

		for (UINT a=0; a<pSearchResult->m_ItemCount; a++)
		{
			lvi.iItem = a;
			lvi.pszText = (*pSearchResult)[a]->CoreAttributes.FileName;
			lvi.iImage = (*pSearchResult)[a]->IconID-1;
			lvi.iGroupId = (*pSearchResult)[a]->CategoryID;
			lvi.state = (((*pSearchResult)[a]->Type & LFTypeGhosted) ? LVIS_CUT : 0) | ((*pSearchResult)[a]->Type & LFTypeBadgeMask);
			INT Index = InsertItem(&lvi);

			WCHAR tmpStr[256];
			SetItemText(Index, 1, (*pSearchResult)[a]->CoreAttributes.Comments);
			SetItemText(Index, 2, (*pSearchResult)[a]->Description);

			LFAttributeToString((*pSearchResult)[a], LFAttrCreationTime, tmpStr, 256);
			SetItemText(Index, 3, tmpStr);

			LFAttributeToString((*pSearchResult)[a], LFAttrStoreID, tmpStr, 256);
			SetItemText(Index, 4, tmpStr);
		}
	}

	if (GetView()==LV_VIEW_DETAILS)
		for (UINT a=0; a<5; a++)
			SetColumnWidth(a, LVSCW_AUTOSIZE);
}


BEGIN_MESSAGE_MAP(CStoreList, CExplorerList)
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

void CStoreList::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
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

	CExplorerList::OnKeyDown(nChar, nRepCnt, nFlags);
}


// LFChooseStoreDlg
//

#define GetSelectedStore() m_wndStoreList.GetNextItem(-1, LVIS_SELECTED)

LFChooseStoreDlg::LFChooseStoreDlg(CWnd* pParentWnd, BOOL Writeable)
	: LFDialog(IDD_CHOOSESTORE, pParentWnd)
{
	m_StoreID[0] = '\0';
	m_pSearchResult = NULL;
	m_Writeable = Writeable;
}

void LFChooseStoreDlg::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
	{
		INT Index = GetSelectedStore();
		strcpy_s(m_StoreID, LFKeySize, Index!=-1 ? (*m_pSearchResult)[Index]->StoreID : "");
	}
}

void LFChooseStoreDlg::AdjustLayout(const CRect& rectLayout, UINT nFlags)
{
	LFDialog::AdjustLayout(rectLayout, nFlags);

	UINT ExplorerHeight = 0;
	if (IsWindow(m_wndHeaderArea))
	{
		ExplorerHeight = m_wndHeaderArea.GetPreferredHeight();
		m_wndHeaderArea.SetWindowPos(NULL, rectLayout.left, rectLayout.top, rectLayout.Width(), ExplorerHeight, nFlags);
	}

	if (IsWindow(m_wndStoreList))
	{
		const INT BorderLeft = (LFGetApp()->OSVersion==OS_XP) ? m_wndStoreList.IsGroupViewEnabled() ? BACKSTAGEBORDER-15 : BACKSTAGEBORDER+1 : BACKSTAGEBORDER-2;
		m_wndStoreList.SetWindowPos(NULL, rectLayout.left+BorderLeft, rectLayout.top+ExplorerHeight, rectLayout.Width()-BorderLeft, m_BottomDivider-rectLayout.top-ExplorerHeight, nFlags);
	}
}

void LFChooseStoreDlg::UpdateOkButton()
{
	const INT Index = GetSelectedStore();
	BOOL bEnable = (Index!=-1);

	if (m_Writeable && bEnable)
		bEnable &= ((*m_pSearchResult)[Index]->Type & (LFTypeMounted | LFTypeWriteable))==(LFTypeMounted | LFTypeWriteable);

	GetDlgItem(IDOK)->EnableWindow(bEnable);
}

BOOL LFChooseStoreDlg::InitDialog()
{
	CString Hint;
	if (m_Writeable)
		ENSURE(Hint.LoadString(IDS_CHOOSESTORE_HINT));

	m_wndHeaderArea.Create(this, IDC_HEADERAREA);
	m_wndHeaderArea.SetText(LFGetApp()->m_Contexts[LFContextStores].Name, Hint, FALSE);

	m_wndStoreList.Create(WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_EDITLABELS, CRect(0, 0, 0, 0), this, IDC_STORELIST);

	m_wndStoreList.SetImageList(&LFGetApp()->m_CoreImageListSmall, LVSIL_SMALL);
	m_wndStoreList.SetImageList(&LFGetApp()->m_CoreImageListExtraLarge, LVSIL_NORMAL);

	m_wndStoreList.AddStoreColumns();
	m_wndStoreList.AddItemCategories();
	m_wndStoreList.SetMenus(IDM_STORE);
	m_wndStoreList.EnableGroupView(LFGetApp()->OSVersion>OS_XP);
	m_wndStoreList.SetView(LV_VIEW_TILE);
	m_wndStoreList.SetItemsPerRow(3);
	m_wndStoreList.SetFocus();

	OnUpdateStores(NULL, NULL);

	return FALSE;
}


BEGIN_MESSAGE_MAP(LFChooseStoreDlg, LFDialog)
	ON_WM_DESTROY()
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(NM_DBLCLK, IDC_STORELIST, OnDoubleClick)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_STORELIST, OnItemChanged)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_STORELIST, OnEndLabelEdit)
	ON_NOTIFY(REQUEST_TOOLTIP_DATA, IDC_STORELIST, OnRequestTooltipData)

	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoresChanged, OnUpdateStores)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoreAttributesChanged, OnUpdateStores)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->DefaultStoreChanged, OnUpdateStores)

	ON_COMMAND(IDM_STORE_MAKEDEFAULT, OnStoreMakeDefault)
	ON_COMMAND(IDM_STORE_SHORTCUT, OnStoreShortcut)
	ON_COMMAND(IDM_STORE_DELETE, OnStoreDelete)
	ON_COMMAND(IDM_STORE_RENAME, OnStoreRename)
	ON_COMMAND(IDM_STORE_PROPERTIES, OnStoreProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORE_SYNCHRONIZE, IDM_STORE_PROPERTIES, OnUpdateStoreCommands)
END_MESSAGE_MAP()

void LFChooseStoreDlg::OnDestroy()
{
	LFFreeSearchResult(m_pSearchResult);

	LFDialog::OnDestroy();
}

void LFChooseStoreDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	LFDialog::OnGetMinMaxInfo(lpMMI);

	if (IsWindowVisible())
	{
		CRect rect;
		GetWindowRect(rect);

		lpMMI->ptMinTrackSize.x = lpMMI->ptMaxTrackSize.x = rect.Width();
	}

	lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, 300);
}

LRESULT LFChooseStoreDlg::OnUpdateStores(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// Save selected store
	CHAR StoreID[LFKeySize] = "";

	if (m_pSearchResult)
	{
		INT Index = m_wndStoreList.GetNextItem(-1, LVIS_SELECTED);
		if (Index!=-1)
			strcpy_s(StoreID, LFKeySize, (*m_pSearchResult)[Index]->StoreID);

		LFFreeSearchResult(m_pSearchResult);
	}

	// Query
	LFFilter* pFilter = LFAllocFilter();
	m_pSearchResult = LFQuery(pFilter);
	LFFreeFilter(pFilter);

	m_wndStoreList.SetSearchResult(m_pSearchResult);

	// Header
	if (!m_Writeable)
	{
		CString Hint;
		Hint.Format(m_pSearchResult->m_StoreCount==1 ? IDS_STORES_SINGULAR : IDS_STORES_PLURAL, m_pSearchResult->m_StoreCount);

		m_wndHeaderArea.SetText(LFGetApp()->m_Contexts[LFContextStores].Name, Hint);
	}

	// Set previously selected store
	INT Index = -1;
	for (UINT a=0; a<m_pSearchResult->m_ItemCount; a++)
		if (((Index==-1) && ((*m_pSearchResult)[a]->Type & LFTypeDefault)) || (!strcmp(StoreID, (*m_pSearchResult)[a]->StoreID)))
			Index = a;

	m_wndStoreList.SetItemState(Index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

	return NULL;
}

void LFChooseStoreDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pSearchResult*/)
{
	if ((GetSelectedStore()!=-1) && (GetDlgItem(IDOK)->IsWindowEnabled()))
		PostMessage(WM_COMMAND, (WPARAM)IDOK);
}

void LFChooseStoreDlg::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pSearchResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && ((pNMListView->uOldState & LVIS_SELECTED) || (pNMListView->uNewState & LVIS_SELECTED)))
		UpdateOkButton();
}

void LFChooseStoreDlg::OnEndLabelEdit(NMHDR* pNMHDR, LRESULT* pResult)
{
	LV_DISPINFO* pDispInfo = (LV_DISPINFO*)pNMHDR;

	*pResult = FALSE;

	if ((m_pSearchResult) && (pDispInfo->item.pszText))
		if (pDispInfo->item.pszText[0]!=L'\0')
		{
			CWaitCursor csr;

			LFTransactionList* pTransactionList = LFAllocTransactionList();
			LFAddTransactionItem(pTransactionList, (*m_pSearchResult)[pDispInfo->item.iItem]);

			LFVariantData Value;
			Value.Attr = LFAttrFileName;
			Value.Type = LFTypeUnicodeString;
			Value.IsNull = FALSE;

			wcsncpy_s(Value.UnicodeString, 256, pDispInfo->item.pszText, 255);

			LFDoTransaction(pTransactionList, LFTransactionTypeUpdate, NULL, NULL, &Value);
			LFErrorBox(this, pTransactionList->m_LastError);

			LFFreeTransactionList(pTransactionList);
			*pResult = TRUE;
		}
}

void LFChooseStoreDlg::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	if (pTooltipData->Item!=-1)
	{
		CString tmpStr;
		GetHintForStore(tmpStr, (*m_pSearchResult)[pTooltipData->Item]);

		wcscpy_s(pTooltipData->Hint, 4096, tmpStr);
		pTooltipData->hIcon = LFGetApp()->m_CoreImageListExtraLarge.ExtractIcon((*m_pSearchResult)[pTooltipData->Item]->IconID-1);

		*pResult = TRUE;
	}
	else
	{
		*pResult = FALSE;
	}
}


void LFChooseStoreDlg::OnStoreMakeDefault()
{
	INT Index = GetSelectedStore();
	if (Index!=-1)
		LFErrorBox(this, LFSetDefaultStore((*m_pSearchResult)[Index]->StoreID));
}

void LFChooseStoreDlg::OnStoreShortcut()
{
	INT Index = GetSelectedStore();
	if (Index!=-1)
		LFCreateDesktopShortcutForStore((*m_pSearchResult)[Index]);
}

void LFChooseStoreDlg::OnStoreDelete()
{
	INT Index = GetSelectedStore();
	if (Index!=-1)
		LFDeleteStore((*m_pSearchResult)[Index]->StoreID, this);
}

void LFChooseStoreDlg::OnStoreRename()
{
	INT Index = GetSelectedStore();
	if (Index!=-1)
	{
		if (GetFocus()!=&m_wndStoreList)
			m_wndStoreList.SetFocus();

		m_wndStoreList.EditLabel(Index);
	}
}

void LFChooseStoreDlg::OnStoreProperties()
{
	INT Index = GetSelectedStore();
	if (Index!=-1)
	{
		LFStorePropertiesDlg dlg((*m_pSearchResult)[Index]->StoreID, this);
		dlg.DoModal();
	}
}

void LFChooseStoreDlg::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;

	INT Index = GetSelectedStore();
	if (Index!=-1)
	{
		LFItemDescriptor* pItemDescriptor = (*m_pSearchResult)[Index];
		bEnable = ((pItemDescriptor->Type & LFTypeMask)==LFTypeStore);

		switch (pCmdUI->m_nID)
		{
		case IDM_STORE_MAKEDEFAULT:
			bEnable = !(pItemDescriptor->Type & LFTypeDefault);
			break;

		case IDM_STORE_SYNCHRONIZE:
		case IDM_STORE_IMPORTFOLDER:
			bEnable = FALSE;
			break;

		case IDM_STORE_SHORTCUT:
			bEnable = (pItemDescriptor->Type & LFTypeShortcutAllowed);
			break;

		case IDM_STORE_DELETE:
			bEnable = (pItemDescriptor->Type & LFTypeWriteable);
			break;

		case IDM_STORE_RENAME:
			bEnable = (pItemDescriptor->Type & LFTypeWriteable) && (m_wndStoreList.GetEditControl()==NULL);
			break;
		}
	}

	pCmdUI->Enable(bEnable);
}
