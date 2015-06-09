
// LFChooseStoreDlg.cpp: Implementierung der Klasse LFChooseStoreDlg
//

#include "StdAfx.h"
#include "LFCommDlg.h"
#include "Resource.h"


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
		INT idx = GetSelectedStore();
		strcpy_s(m_StoreID, LFKeySize, idx!=-1 ? m_pResult->m_Items[idx]->StoreID : "");
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

	INT borderLeft = (p_App->OSVersion==OS_XP) ? 15 : borders.Width()/2;
	m_wndExplorerList.SetWindowPos(NULL, rect.left+borderLeft, rect.top+ExplorerHeight, rect.Width()-borderLeft, rect.Height()-ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}

void LFChooseStoreDlg::UpdateOkButton()
{
	INT idx = GetSelectedStore();
	BOOL b = (idx!=-1);

	if (m_Mounted)
		b &= !(m_pResult->m_Items[idx]->Type & LFTypeNotMounted);

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
	m_wndHeaderArea.SetText(p_App->m_Contexts[LFContextStores].Name, Hint, FALSE);

	const UINT dwStyle = WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_SHOWSELALWAYS | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS | LVS_ALIGNTOP | LVS_EDITLABELS;
	CRect rect;
	rect.SetRectEmpty();
	m_wndExplorerList.Create(dwStyle, rect, this, IDC_STORELIST);

	m_wndExplorerList.SetImageList(&p_App->m_CoreImageListSmall, LVSIL_SMALL);
	m_wndExplorerList.SetImageList(&p_App->m_CoreImageListLarge, LVSIL_NORMAL);

	IMAGEINFO ii;
	p_App->m_CoreImageListLarge.GetImageInfo(0, &ii);
	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&p_App->m_DefaultFont);
	m_wndExplorerList.SetIconSpacing(GetSystemMetrics(SM_CXICONSPACING), ii.rcImage.bottom-ii.rcImage.top+dc->GetTextExtent(_T("Wy")).cy*2+4);
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	m_wndExplorerList.AddStoreColumns();
	m_wndExplorerList.AddItemCategories();
	m_wndExplorerList.SetMenus(IDM_STORE, FALSE, IDM_STORES);
	m_wndExplorerList.EnableGroupView(p_App->OSVersion>OS_XP);
	m_wndExplorerList.SetView(LV_VIEW_TILE);
	m_wndExplorerList.SetFocus();

	SendMessage(p_App->p_MessageIDs->StoresChanged);

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
		INT idx = m_wndExplorerList.GetNextItem(-1, LVIS_SELECTED);
		if (idx!=-1)
			strcpy_s(StoreID, LFKeySize, m_pResult->m_Items[idx]->StoreID);

		LFFreeSearchResult(m_pResult);
	}

	LFFilter* filter = LFAllocFilter();
	m_pResult = LFQuery(filter);
	LFFreeFilter(filter);

	m_wndExplorerList.SetSearchResult(m_pResult);

	if (!m_Mounted)
	{
		CString Hint;
		CString Mask;
		ENSURE(Mask.LoadString(m_pResult->m_StoreCount==1 ? IDS_STORES_SINGULAR : IDS_STORES_PLURAL));
		Hint.Format(Mask, m_pResult->m_StoreCount);

		m_wndHeaderArea.SetText(p_App->m_Contexts[LFContextStores].Name, Hint);
	}

	INT idx = -1;
	for (UINT a=0; a<m_pResult->m_ItemCount; a++)
		if (((idx==-1) && (m_pResult->m_Items[a]->Type & LFTypeDefault)) || (!strcmp(StoreID, m_pResult->m_Items[a]->StoreID)))
			idx = a;

	m_wndExplorerList.SetItemState(idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);

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
			LFTransactionList* tl = LFAllocTransactionList();
			LFAddItemDescriptor(tl, m_pResult->m_Items[pDispInfo->item.iItem]);

			LFVariantData value;
			value.Attr = LFAttrFileName;
			value.Type = LFTypeUnicodeString;
			value.IsNull = false;

			wcsncpy_s(value.UnicodeString, 256, pDispInfo->item.pszText, 255);

			LFTransactionUpdate(tl, &value);
			LFErrorBox(tl->m_LastError, GetSafeHwnd());

			LFFreeTransactionList(tl);
			*pResult = TRUE;
		}
}


void LFChooseStoreDlg::OnStoreMakeDefault()
{
	INT idx = GetSelectedStore();
	if (idx!=-1)
		LFErrorBox(LFMakeDefaultStore(m_pResult->m_Items[idx]->StoreID), GetSafeHwnd());
}

void LFChooseStoreDlg::OnStoreShortcut()
{
	INT idx = GetSelectedStore();
	if (idx!=-1)
		if (LFAskCreateShortcut(GetSafeHwnd()))
			LFCreateDesktopShortcutForStore(m_pResult->m_Items[idx]);
}

void LFChooseStoreDlg::OnStoreDelete()
{
	INT idx = GetSelectedStore();
	if (idx!=-1)
		LFDeleteStore(m_pResult->m_Items[idx]->StoreID, this);
}

void LFChooseStoreDlg::OnStoreRename()
{
	INT idx = GetSelectedStore();
	if (idx!=-1)
	{
		if (GetFocus()!=&m_wndExplorerList)
			m_wndExplorerList.SetFocus();

		m_wndExplorerList.EditLabel(idx);
	}
}

void LFChooseStoreDlg::OnStoreProperties()
{
	INT idx = GetSelectedStore();
	if (idx!=-1)
	{
		LFStorePropertiesDlg dlg(m_pResult->m_Items[idx]->StoreID, this);
		dlg.DoModal();
	}
}

void LFChooseStoreDlg::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	INT idx = GetSelectedStore();
	if (idx!=-1)
	{
		LFItemDescriptor* item = m_pResult->m_Items[idx];
		b = ((item->Type & LFTypeMask)==LFTypeStore);

		switch (pCmdUI->m_nID)
		{
		case IDM_STORE_MAKEDEFAULT:
			b = !(item->Type & LFTypeDefault);
			break;
		case IDM_STORE_IMPORTFOLDER:
			b = FALSE;
			break;
		case IDM_STORE_SHORTCUT:
			b = (item->Type & LFTypeShortcutAllowed);
			break;
		case IDM_STORE_RENAME:
			b = (m_wndExplorerList.GetEditControl()==NULL);
			break;
		}
	}

	pCmdUI->Enable(b);
}
