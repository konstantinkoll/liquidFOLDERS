
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

void CStoreList::SetSearchResult(LFSearchResult* pResult)
{
	DeleteAllItems();

	if (pResult)
	{
		LFSortSearchResult(pResult, LFAttrFileName, FALSE);
		LFErrorBox(pResult->m_LastError, GetParent()->GetSafeHwnd());

		static UINT puColumns[2] = { 1, 2 };

		LVITEM lvi;
		ZeroMemory(&lvi, sizeof(lvi));

		lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_GROUPID | LVIF_COLUMNS | LVIF_STATE;
		lvi.stateMask = LVIS_CUT | LVIS_OVERLAYMASK;
		lvi.cColumns = 2;
		lvi.puColumns = puColumns;

		for (UINT a=0; a<pResult->m_ItemCount; a++)
		{
			lvi.iItem = a;
			lvi.pszText = pResult->m_Items[a]->CoreAttributes.FileName;
			lvi.iImage = pResult->m_Items[a]->IconID-1;
			lvi.iGroupId = pResult->m_Items[a]->CategoryID;
			lvi.state = ((pResult->m_Items[a]->Type & LFTypeGhosted) ? LVIS_CUT : 0) | (pResult->m_Items[a]->Type & LFTypeDefault ? INDEXTOOVERLAYMASK(1) : 0);
			INT Index = InsertItem(&lvi);

			WCHAR tmpStr[256];
			SetItemText(Index, 1, pResult->m_Items[a]->CoreAttributes.Comments);
			SetItemText(Index, 2, pResult->m_Items[a]->Description);

			LFAttributeToString(pResult->m_Items[a], LFAttrCreationTime, tmpStr, 256);
			SetItemText(Index, 3, tmpStr);

			LFAttributeToString(pResult->m_Items[a], LFAttrStoreID, tmpStr, 256);
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
	if (!IsWindow(m_wndStoreList))
		return;

	CRect rect;
	GetLayoutRect(rect);

	UINT ExplorerHeight = 0;
	if (IsWindow(m_wndHeaderArea))
	{
		ExplorerHeight = m_wndHeaderArea.GetPreferredHeight();
		m_wndHeaderArea.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	}

	INT BorderLeft = (LFGetApp()->OSVersion==OS_XP) ? m_wndStoreList.IsGroupViewEnabled() ? 2 : 15 : 4;
	m_wndStoreList.SetWindowPos(NULL, rect.left+BorderLeft, rect.top+ExplorerHeight, rect.Width()-BorderLeft, rect.Height()-ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);
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
	ON_NOTIFY(REQUEST_TOOLTIP_DATA, IDC_STORELIST, OnRequestTooltipData)
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
	m_wndStoreList.Create(WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_EDITLABELS, rect, this, IDC_STORELIST);

	m_wndStoreList.SetImageList(&LFGetApp()->m_CoreImageListSmall, LVSIL_SMALL);
	m_wndStoreList.SetImageList(&LFGetApp()->m_CoreImageListExtraLarge, LVSIL_NORMAL);

	m_wndStoreList.AddStoreColumns();
	m_wndStoreList.AddItemCategories();
	m_wndStoreList.SetMenus(IDM_STORE, FALSE, IDM_STORES);
	m_wndStoreList.EnableGroupView(LFGetApp()->OSVersion>OS_XP);
	m_wndStoreList.SetView(LV_VIEW_TILE);
	m_wndStoreList.SetItemsPerRow(3);
	m_wndStoreList.SetFocus();

	SendMessage(LFGetApp()->p_MessageIDs->StoresChanged);

	AdjustLayout();

	return FALSE;  // TRUE zur�ckgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
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
		INT Index = m_wndStoreList.GetNextItem(-1, LVIS_SELECTED);
		if (Index!=-1)
			strcpy_s(StoreID, LFKeySize, m_pResult->m_Items[Index]->StoreID);

		LFFreeSearchResult(m_pResult);
	}

	LFFilter* filter = LFAllocFilter();
	m_pResult = LFQuery(filter);
	LFFreeFilter(filter);

	m_wndStoreList.SetSearchResult(m_pResult);

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

	m_wndStoreList.SetItemState(Index, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

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

void LFChooseStoreDlg::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	if (pTooltipData->Item!=-1)
	{
		CString tmpStr;
		GetHintForStore(m_pResult->m_Items[pTooltipData->Item], tmpStr);

		wcscpy_s(pTooltipData->Text, 4096, tmpStr);
		pTooltipData->hIcon = LFGetApp()->m_CoreImageListExtraLarge.ExtractIcon(m_pResult->m_Items[pTooltipData->Item]->IconID-1);

		pTooltipData->Show = TRUE;
	}

	*pResult = 0;
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
			b = (m_wndStoreList.GetEditControl()==NULL);
			break;
		}
	}

	pCmdUI->Enable(b);
}
