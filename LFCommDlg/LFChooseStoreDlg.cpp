
// LFChooseStoreDlg.cpp: Implementierung der Klasse LFChooseStoreDlg
//

#include "StdAfx.h"
#include "LFChooseStoreDlg.h"
#include "LFStoreNewDlg.h"
#include "LFStorePropertiesDlg.h"
#include "Resource.h"


// LFChooseStoreDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;
extern LFMessageIDs* MessageIDs;

#define GetSelectedStore() m_wndExplorerList.GetNextItem(-1, LVIS_SELECTED)

LFChooseStoreDlg::LFChooseStoreDlg(CWnd* pParentWnd, UINT Mode)
	: LFDialog(Mode==LFCSD_ChooseDefault ? IDD_CHOOSEDEFAULTSTORE : IDD_CHOOSESTORE, LFDS_White, pParentWnd)
{
	p_Result = NULL;
	m_Mode = Mode;
}

LFChooseStoreDlg::~LFChooseStoreDlg()
{
	if (p_Result)
		LFFreeSearchResult(p_Result);
}

void LFChooseStoreDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	if (pDX->m_bSaveAndValidate)
	{
		int idx = GetSelectedStore();
		strcpy_s(StoreID, LFKeySize, idx!=-1 ? p_Result->m_Items[idx]->StoreID : "");
	}
}

void LFChooseStoreDlg::AdjustLayout()
{
	if (!IsWindow(m_wndExplorerList))
		return;

	CRect rect;
	GetLayoutRect(rect);

	UINT ExplorerHeight = 0;
	if (IsWindow(m_wndExplorerHeader))
	{
		ExplorerHeight = m_wndExplorerHeader.GetPreferredHeight();
		m_wndExplorerHeader.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	}

	CRect borders(0, 0, 7, 7);
	MapDialogRect(&borders);

	int borderLeft = (p_App->OSVersion==OS_XP) ? 0 : (m_Mode>=LFCSD_Internal) ? borders.Width() : borders.Width()/2;
	int borderRight = (m_Mode>=LFCSD_Internal) ? 0 : 1;
	m_wndExplorerList.SetWindowPos(NULL, rect.left+borderLeft, rect.top+ExplorerHeight, rect.Width()-borderLeft-borderRight, rect.Height()-ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}

void LFChooseStoreDlg::UpdateOkButton()
{
	int idx = GetSelectedStore();
	BOOL bEnable = (idx!=-1);

	if (bEnable)
		switch (m_Mode)
		{
		case LFCSD_Mounted:
			bEnable &= !(p_Result->m_Items[idx]->Type & LFTypeNotMounted);
			break;
		}

	GetDlgItem(IDOK)->EnableWindow(bEnable);
}


BEGIN_MESSAGE_MAP(LFChooseStoreDlg, LFDialog)
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(NM_DBLCLK, IDC_STORELIST, OnDoubleClick)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_STORELIST, OnItemChanged)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_STORELIST, OnEndLabelEdit)
	ON_BN_CLICKED(IDC_NEWSTORE, OnNewStore)
	ON_REGISTERED_MESSAGE(MessageIDs->StoresChanged, OnUpdateStores)
	ON_REGISTERED_MESSAGE(MessageIDs->StoreAttributesChanged, OnUpdateStores)
	ON_REGISTERED_MESSAGE(MessageIDs->DefaultStoreChanged, OnUpdateStores)
	ON_COMMAND(IDM_STORE_MAKEDEFAULT, OnMakeDefault)
	ON_COMMAND(IDM_STORE_MAKEHYBRID, OnMakeHybrid)
	ON_COMMAND(IDM_STORE_RENAME, OnRename)
	ON_COMMAND(IDM_STORE_DELETE, OnDelete)
	ON_COMMAND(IDM_STORE_PROPERTIES, OnProperties)
END_MESSAGE_MAP()

BOOL LFChooseStoreDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	CString caption;
	ENSURE(caption.LoadString(m_Mode==LFCSD_ChooseDefault ? IDS_CHOOSEDEFAULTSTORE_CAPTION : IDS_CHOOSESTORE_CAPTION));

	CString hint;
	switch (m_Mode)
	{
	case LFCSD_Mounted:
		ENSURE(hint.LoadString(IDS_CHOOSESTORE_HINT));
		break;
	case LFCSD_ChooseDefault:
		ENSURE(hint.LoadString(IDS_CHOOSEDEFAULTSTORE_HINT));
		break;
	}

	m_wndExplorerHeader.Create(this, IDC_EXPLORERHEADER);
	m_wndExplorerHeader.SetText(caption, hint, FALSE);
	m_wndExplorerHeader.SetColors(0x126E00, (COLORREF)-1, FALSE);

	const UINT dwStyle = WS_VISIBLE | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | LVS_SHOWSELALWAYS | LVS_AUTOARRANGE | LVS_SHAREIMAGELISTS | LVS_ALIGNTOP | LVS_EDITLABELS;
	CRect rect;
	rect.SetRectEmpty();
	m_wndExplorerList.Create(dwStyle, rect, this, IDC_STORELIST);

	LFApplication* pApp = (LFApplication*)AfxGetApp();
	m_wndExplorerList.SetImageList(&pApp->m_CoreImageListSmall, LVSIL_SMALL);
	m_wndExplorerList.SetImageList(&pApp->m_CoreImageListLarge, LVSIL_NORMAL);

	IMAGEINFO ii;
	pApp->m_CoreImageListLarge.GetImageInfo(0, &ii);
	CDC* dc = GetWindowDC();
	CFont* pOldFont = dc->SelectObject(&pApp->m_DefaultFont);
	m_wndExplorerList.SetIconSpacing(GetSystemMetrics(SM_CXICONSPACING), ii.rcImage.bottom-ii.rcImage.top+dc->GetTextExtent(_T("Wy"), 2).cy*2+4);
	dc->SelectObject(pOldFont);
	ReleaseDC(dc);

	m_wndExplorerList.AddStoreColumns();
	m_wndExplorerList.AddItemCategories();
	m_wndExplorerList.SetMenus(IDM_STORES, TRUE, IDM_CREATENEWSTORE);
	m_wndExplorerList.EnableGroupView(m_Mode<LFCSD_Internal);
	m_wndExplorerList.SetView(LV_VIEW_TILE);
	m_wndExplorerList.SetFocus();

	SendMessage(MessageIDs->StoresChanged, LFMSGF_IntStores | LFMSGF_ExtHybStores);

	AdjustLayout();
	AddBottomControl(IDC_NEWSTORE);

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

LRESULT LFChooseStoreDlg::OnUpdateStores(WPARAM wParam, LPARAM lParam)
{
	UINT Mask = LFMSGF_IntStores | (m_Mode<LFCSD_Internal ? LFMSGF_ExtHybStores : 0);
	if ((wParam & Mask) && (m_hWnd!=(HWND)lParam))
	{
		char StoreID[LFKeySize] = "";
		if (p_Result)
		{
			int idx = m_wndExplorerList.GetNextItem(-1, LVIS_SELECTED);
			if (idx!=-1)
				strcpy_s(StoreID, LFKeySize, p_Result->m_Items[idx]->StoreID);

			LFFreeSearchResult(p_Result);
		}

		LFFilter* filter = LFAllocFilter();
		filter->Options.OnlyInternalStores = (m_Mode>=LFCSD_Internal);
		p_Result = LFQuery(filter);
		LFFreeFilter(filter);

		m_wndExplorerList.SetSearchResult(p_Result);

		int idx = -1;
		for (UINT a=0; a<p_Result->m_ItemCount; a++)
			if (((idx==-1) && (p_Result->m_Items[a]->Type & LFTypeDefaultStore)) || (!strcmp(StoreID, p_Result->m_Items[a]->StoreID)))
				idx = a;

		m_wndExplorerList.SetItemState(idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}

	return NULL;
}

void LFChooseStoreDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	if (GetSelectedStore()!=-1)
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

	if ((p_Result) && (pDispInfo->item.pszText))
		if (pDispInfo->item.pszText[0]!=L'\0')
		{
			LFTransactionList* tl = LFAllocTransactionList();
			LFAddItemDescriptor(tl, p_Result->m_Items[pDispInfo->item.iItem]);

			LFVariantData value;
			value.Attr = LFAttrFileName;
			value.Type = LFTypeUnicodeString;
			value.IsNull = false;
			wcscpy_s(value.UnicodeString, 256, pDispInfo->item.pszText);

			LFTransactionUpdate(tl, NULL, &value);
			LFErrorBox(tl->m_LastError);

			LFFreeTransactionList(tl);
			*pResult = TRUE;
		}
}

void LFChooseStoreDlg::OnNewStore()
{
	LFStoreDescriptor* s = LFAllocStoreDescriptor();

	LFStoreNewDlg dlg(this, s);
	if (dlg.DoModal()==IDOK)
		LFErrorBox(LFCreateStore(s, dlg.MakeDefault));

	LFFreeStoreDescriptor(s);
}

void LFChooseStoreDlg::OnMakeDefault()
{
	int idx = GetSelectedStore();
	if (idx!=-1)
		LFErrorBox(LFMakeDefaultStore(p_Result->m_Items[idx]->StoreID, NULL), m_hWnd);
}

void LFChooseStoreDlg::OnMakeHybrid()
{
	int idx = GetSelectedStore();
	if (idx!=-1)
		LFErrorBox(LFMakeHybridStore(p_Result->m_Items[idx]->StoreID, NULL), m_hWnd);
}

void LFChooseStoreDlg::OnRename()
{
	int idx = GetSelectedStore();
	if (idx!=-1)
		m_wndExplorerList.EditLabel(idx);
}

void LFChooseStoreDlg::OnDelete()
{
	int idx = GetSelectedStore();
	if (idx!=-1)
		LFErrorBox(((LFApplication*)AfxGetApp())->DeleteStore(p_Result->m_Items[idx], this));
}

void LFChooseStoreDlg::OnProperties()
{
	int idx = GetSelectedStore();
	if (idx!=-1)
	{
		LFStorePropertiesDlg dlg(p_Result->m_Items[idx]->StoreID, this);
		dlg.DoModal();
	}
}