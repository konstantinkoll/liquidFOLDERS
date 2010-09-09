
// LFChooseDefaultStoreDlg.cpp: Implementierung der Klasse LFChooseDefaultStoreDlg
//

#include "StdAfx.h"
#include "LFChooseDefaultStoreDlg.h"
#include "LFStoreNewDlg.h"
#include "LFStorePropertiesDlg.h"
#include "Resource.h"


// LFChooseDefaultStoreDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;
extern LFMessageIDs* MessageIDs;

#define GetSelectedStore() m_wndExplorerList.GetNextItem(-1, LVIS_SELECTED)

LFChooseDefaultStoreDlg::LFChooseDefaultStoreDlg(CWnd* pParentWnd)
	: LFDialog(IDD_CHOOSEDEFAULTSTORE, LFDS_White, pParentWnd)
{
	result = NULL;
}

LFChooseDefaultStoreDlg::~LFChooseDefaultStoreDlg()
{
	if (result)
		LFFreeSearchResult(result);
}

void LFChooseDefaultStoreDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	if ((pDX->m_bSaveAndValidate) && (result))
		MakeDefault(m_hWnd);
}

void LFChooseDefaultStoreDlg::MakeDefault(HWND hWnd)
{
	int idx = GetSelectedStore();
	if (idx!=-1)
		LFErrorBox(LFMakeDefaultStore(result->m_Items[idx]->StoreID, hWnd), m_hWnd);
}

void LFChooseDefaultStoreDlg::AdjustLayout()
{
	if (!IsWindow(m_wndExplorerList.GetSafeHwnd()))
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

	m_wndExplorerList.SetWindowPos(NULL, rect.left+borders.Width(), rect.top+ExplorerHeight, rect.Width()-borders.Width(), rect.Height()-ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(LFChooseDefaultStoreDlg, LFDialog)
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(NM_DBLCLK, IDC_STORELIST, OnDoubleClick)
	ON_BN_CLICKED(IDC_NEWSTORE, OnNewStore)
	ON_REGISTERED_MESSAGE(MessageIDs->StoresChanged, OnUpdateStores)
	ON_REGISTERED_MESSAGE(MessageIDs->StoreAttributesChanged, OnUpdateStores)
	ON_REGISTERED_MESSAGE(MessageIDs->DefaultStoreChanged, OnUpdateStores)
	ON_COMMAND(IDM_STORE_MAKEDEFAULT, OnMakeDefault)
	ON_COMMAND(IDM_STORE_RENAME, OnRename)
	ON_COMMAND(IDM_STORE_DELETE, OnDelete)
	ON_COMMAND(IDM_STORE_PROPERTIES, OnProperties)
END_MESSAGE_MAP()

BOOL LFChooseDefaultStoreDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	CString caption;
	ENSURE(caption.LoadString(IDS_CHOOSEDEFAULT_CAPTION));
	CString hint;
	ENSURE(hint.LoadString(IDS_CHOOSEDEFAULT_HINT));

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
	m_wndExplorerList.SetMenus(IDM_STORE, IDM_CREATENEWSTORE);
	m_wndExplorerList.SetView(LV_VIEW_TILE);
	m_wndExplorerList.SetFocus();

	SendMessage(MessageIDs->StoresChanged, LFMSGF_IntStores | LFMSGF_ExtHybStores);

	AdjustLayout();
	AddBottomControl(IDC_NEWSTORE);

	return FALSE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFChooseDefaultStoreDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	LFDialog::OnGetMinMaxInfo(lpMMI);

	CRect rect;
	GetWindowRect(rect);
	if (rect.Width())
		lpMMI->ptMinTrackSize.x = lpMMI->ptMaxTrackSize.x = rect.Width();

	lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, 300);
}

LRESULT LFChooseDefaultStoreDlg::OnUpdateStores(WPARAM wParam, LPARAM lParam)
{
	if ((wParam & LFMSGF_IntStores) && (m_hWnd!=(HWND)lParam))
	{
		char StoreID[LFKeySize] = "";
		if (result)
		{
			int idx = m_wndExplorerList.GetNextItem(-1, LVIS_SELECTED);
			if (idx!=-1)
				strcpy_s(StoreID, LFKeySize, result->m_Items[idx]->StoreID);

			LFFreeSearchResult(result);
		}

		LFFilter* filter = LFAllocFilter();
		filter->Options.OnlyInternalStores = true;
		result = LFQuery(filter);
		LFFreeFilter(filter);

		m_wndExplorerList.SetSearchResult(result);
		m_wndExplorerHeader.SetLineStyle(!result->m_ItemCount, FALSE);
		GetDlgItem(IDOK)->EnableWindow(result->m_ItemCount);

		int idx = -1;
		for (UINT a=0; a<result->m_ItemCount; a++)
			if (((idx==-1) && (result->m_Items[a]->Type & LFTypeDefaultStore)) || (!strcmp(StoreID, result->m_Items[a]->StoreID)))
				idx = a;

		m_wndExplorerList.SetItemState(idx, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
	}

	return NULL;
}

void LFChooseDefaultStoreDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	if (result)
		if (result->m_ItemCount)
			PostMessage(WM_COMMAND, (WPARAM)IDOK);
}

void LFChooseDefaultStoreDlg::OnNewStore()
{
	LFStoreDescriptor* s = LFAllocStoreDescriptor();

	LFStoreNewDlg dlg(this, IDD_STORENEW, '\0', s);
	if (dlg.DoModal()==IDOK)
		LFErrorBox(LFCreateStore(s, dlg.makeDefault));

	LFFreeStoreDescriptor(s);
}

void LFChooseDefaultStoreDlg::OnMakeDefault()
{
	MakeDefault(NULL);
}

void LFChooseDefaultStoreDlg::OnRename()
{
	int idx = GetSelectedStore();
	if (idx!=-1)
		m_wndExplorerList.EditLabel(idx);
}

void LFChooseDefaultStoreDlg::OnDelete()
{
	int idx = GetSelectedStore();
	if (idx!=-1)
		LFErrorBox(((LFApplication*)AfxGetApp())->DeleteStore(result->m_Items[idx], this));
}

void LFChooseDefaultStoreDlg::OnProperties()
{
	int idx = GetSelectedStore();
	if (idx!=-1)
	{
		LFStorePropertiesDlg dlg(result->m_Items[idx]->StoreID, this);
		dlg.DoModal();
	}
}
