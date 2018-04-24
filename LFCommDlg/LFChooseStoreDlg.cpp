
// LFChooseStoreDlg.cpp: Implementierung der Klasse LFChooseStoreDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CStoreList
//

CStoreList::CStoreList()
	: CAbstractFileView(FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLEFOCUSITEM | FRONTSTAGE_ENABLELABELEDIT)
{
}

BOOL CStoreList::GetContextMenu(CMenu& Menu, INT Index)
{
	if (Index>=0)
		Menu.LoadMenu(IDM_STORE);

	return FALSE;
}

void CStoreList::AdjustLayout()
{
	AdjustLayoutColumns(2, BACKSTAGEBORDER);
}

INT CStoreList::GetTileRows(const LFItemDescriptor* pItemDescriptor)
{
	INT Rows = 2;

	// Comments
	if (pItemDescriptor->CoreAttributes.Comments[0])
		Rows++;

	// Description
	if (pItemDescriptor->Description[0])
		Rows++;

	return Rows;
}

void CStoreList::DrawTileRow(CDC& dc, CRect& rectText, LPCWSTR pStr) const
{
	ASSERT(pStr);

	if (pStr[0])
	{
		dc.DrawText(pStr, -1, rectText, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_SINGLELINE);
		rectText.top += m_DefaultFontHeight;
	}
}

void CStoreList::DrawItem(CDC& dc, Graphics& /*g*/, LPCRECT rectItem, INT Index, BOOL Themed)
{
	const LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

	CRect rect(rectItem);
	rect.DeflateRect(ITEMVIEWPADDING, ITEMVIEWPADDING);

	// Icon
	LFGetApp()->m_CoreImageListExtraLarge.DrawEx(&dc, pItemDescriptor->IconID-1, 
		CPoint(rect.left, rect.top+(rect.Height()-m_IconSize)/2), CSize(m_IconSize, m_IconSize), CLR_NONE, CLR_NONE,
		((pItemDescriptor->Type & LFTypeGhosted) ? ILD_BLEND50 : ILD_TRANSPARENT) | (pItemDescriptor->Type & LFTypeBadgeMask));

	// Text
	CRect rectText(rect);
	rectText.left += m_IconSize+ITEMVIEWPADDING;
	rectText.top += (rect.Height()-GetTileRows(pItemDescriptor)*m_DefaultFontHeight)/2;

	DrawTileRow(dc, rectText, pItemDescriptor->CoreAttributes.FileName);

	SetDarkTextColor(dc, Index, Themed);

	DrawTileRow(dc, rectText, pItemDescriptor->CoreAttributes.Comments);
	DrawTileRow(dc, rectText, pItemDescriptor->Description);
	DrawTileRow(dc, rectText, LFGetApp()->GetFreeBytesAvailable(pItemDescriptor->StoreDescriptor.FreeBytesAvailable.QuadPart));
}

RECT CStoreList::GetLabelRect(INT Index) const
{
	RECT rect = GetItemRect(Index);

	rect.bottom = (rect.top+=(rect.bottom-rect.top-GetTileRows(Index)*m_DefaultFontHeight)/2-2)+m_DefaultFontHeight+4;
	rect.left += m_IconSize+2*ITEMVIEWPADDING-5;
	rect.right -= ITEMVIEWPADDING-2;

	return rect;
}


BEGIN_MESSAGE_MAP(CStoreList, CAbstractFileView)
	ON_WM_CREATE()
END_MESSAGE_MAP()

INT CStoreList::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CAbstractFileView::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Item
	SetItemHeight(max(m_IconSize=LFGetApp()->m_ExtraLargeIconSize, 4*m_DefaultFontHeight)+2*ITEMVIEWPADDING);

	return 0;
}


// LFChooseStoreDlg
//

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
		const INT Index = GetSelectedStore();
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
		m_wndStoreList.SetWindowPos(NULL, rectLayout.left, rectLayout.top+ExplorerHeight, rectLayout.Width(), m_BottomDivider-rectLayout.top-ExplorerHeight, nFlags);
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
	m_wndHeaderArea.SetHeader(LFGetApp()->m_Contexts[LFContextStores].Name, Hint, NULL, CPoint(0, 0), FALSE);

	m_wndStoreList.Create(this, IDC_STORELIST);
	m_wndStoreList.SetFocus();

	OnUpdateStores(NULL, NULL);

	return FALSE;
}


BEGIN_MESSAGE_MAP(LFChooseStoreDlg, LFDialog)
	ON_WM_DESTROY()
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(IVN_SELECTIONCHANGED, IDC_STORELIST, OnSelectionChanged)
	ON_MESSAGE(WM_RENAMEITEM, OnRenameItem)

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

void LFChooseStoreDlg::OnSelectionChanged(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	UpdateOkButton();

	*pResult = 0;
}

LRESULT LFChooseStoreDlg::OnRenameItem(WPARAM wParam, LPARAM lParam)
{
	CWaitCursor csr;

	LFTransactionList* pTransactionList = LFAllocTransactionList();
	LFAddTransactionItem(pTransactionList, (*m_pSearchResult)[(UINT)wParam]);

	LFVariantData VData;
	LFInitVariantData(VData, LFAttrFileName);

	wcsncpy_s(VData.UnicodeString, 256, (LPCWSTR)lParam, _TRUNCATE);
	VData.IsNull = FALSE;

	LFDoTransaction(pTransactionList, LFTransactionTypeUpdate, NULL, NULL, &VData);

	LFErrorBox(this, pTransactionList->m_LastError);
	LFFreeTransactionList(pTransactionList);

	return NULL;
}


LRESULT LFChooseStoreDlg::OnUpdateStores(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// Save ID of selected store
	CHAR StoreID[LFKeySize] = "";

	if (m_pSearchResult)
	{
		const INT Index = GetSelectedStore();
		if (Index!=-1)
			strcpy_s(StoreID, LFKeySize, (*m_pSearchResult)[Index]->StoreID);

		LFFreeSearchResult(m_pSearchResult);
	}

	// Filter
	LFFilter* pFilter = LFAllocFilter(LFFilterModeStores);

	// Query
	LFSortSearchResult(m_pSearchResult=LFQuery(pFilter), LFAttrFileName);
	LFFreeFilter(pFilter);

	LFErrorBox(this, m_pSearchResult->m_LastError);

	// Update search result
	m_wndStoreList.UpdateSearchResult(m_pSearchResult);

	// Set previously selected store or default store
	INT Index = -1;

	for (UINT a=0; a<m_pSearchResult->m_ItemCount; a++)
		if (((Index==-1) && ((*m_pSearchResult)[a]->Type & LFTypeDefault)) || !strcmp(StoreID, (*m_pSearchResult)[a]->StoreID))
			Index = a;

	m_wndStoreList.SetFocusItem(Index);

	// Update Ok button
	UpdateOkButton();

	return NULL;
}


void LFChooseStoreDlg::OnStoreMakeDefault()
{
	const INT Index = GetSelectedStore();
	if (Index!=-1)
		LFErrorBox(this, LFSetDefaultStore((*m_pSearchResult)[Index]->StoreID));
}

void LFChooseStoreDlg::OnStoreShortcut()
{
	const INT Index = GetSelectedStore();
	if (Index!=-1)
		LFErrorBox(this, LFCreateDesktopShortcutForItem((*m_pSearchResult)[Index]));
}

void LFChooseStoreDlg::OnStoreDelete()
{
	const INT Index = GetSelectedStore();
	if (Index!=-1)
		LFDeleteStore((*m_pSearchResult)[Index]->StoreID, this);
}

void LFChooseStoreDlg::OnStoreRename()
{
	const INT Index = GetSelectedStore();
	if (Index!=-1)
	{
		if (GetFocus()!=&m_wndStoreList)
			m_wndStoreList.SetFocus();

		m_wndStoreList.EditLabel(Index);
	}
}

void LFChooseStoreDlg::OnStoreProperties()
{
	const INT Index = GetSelectedStore();
	if (Index!=-1)
		LFStorePropertiesDlg((*m_pSearchResult)[Index]->StoreID, this).DoModal();
}

void LFChooseStoreDlg::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;

	const INT Index = GetSelectedStore();
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
			bEnable = FALSE;
			break;

		case IDM_STORE_SHORTCUT:
			bEnable = (pItemDescriptor->Type & LFTypeShortcutAllowed);
			break;

		case IDM_STORE_DELETE:
			bEnable = (pItemDescriptor->Type & LFTypeWriteable);
			break;

		case IDM_STORE_RENAME:
			bEnable = (pItemDescriptor->Type & LFTypeWriteable) && !m_wndStoreList.IsEditing();
			break;
		}
	}

	pCmdUI->Enable(bEnable);
}
