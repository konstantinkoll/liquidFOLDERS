
// MigrateWnd.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "Migrate.h"
#include "MigrateWnd.h"
#include "Resource.h"
#include "LFCore.h"
#include "DeleteFilesDlg.h"
//#include <io.h>


CMigrateWnd::CMigrateWnd()
	: CGlasWindow()
{
	m_hIcon = NULL;
}

CMigrateWnd::~CMigrateWnd()
{
	if (m_hIcon)
		DestroyIcon(m_hIcon);
}

BOOL CMigrateWnd::Create()
{
	m_hIcon = theApp.LoadIcon(IDR_APPLICATION);

	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW), NULL, m_hIcon);

	CRect rect;
	SystemParametersInfo(SPI_GETWORKAREA, NULL, &rect, NULL);
	rect.DeflateRect(32, 32);

	CString caption;
	ENSURE(caption.LoadString(IDR_APPLICATION));

	return CGlasWindow::Create(WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, caption, rect);
}

void CMigrateWnd::AdjustLayout()
{
	if (!IsWindow(m_wndFolder))
		return;
	if (!IsWindow(m_wndStore))
		return;
	if (!IsWindow(m_wndBottomArea))
		return;
	if (!IsWindow(m_wndMainView))
		return;

	CRect rect;
	GetLayoutRect(rect);

	const UINT Border = 4;
	const UINT SelectorHeight = m_wndFolder.GetPreferredHeight();
	m_wndFolder.SetWindowPos(NULL, rect.left, rect.top+4, (rect.Width()-Border)/2, SelectorHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndStore.SetWindowPos(NULL, rect.right-(rect.Width()-Border)/2, rect.top+4, (rect.Width()-Border)/2, SelectorHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	const UINT BottomHeight = MulDiv(45, LOWORD(GetDialogBaseUnits()), 8);
	m_wndBottomArea.SetWindowPos(NULL, rect.left, rect.bottom-BottomHeight, rect.Width(), BottomHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	m_wndMainView.SetWindowPos(NULL, rect.left, rect.top+m_Margins.cyTopHeight, rect.Width(), rect.bottom-BottomHeight-m_Margins.cyTopHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(CMigrateWnd, CGlasWindow)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_COMMAND(ID_VIEW_SELECTROOT, OnSelectRoot)
	ON_COMMAND(IDC_MIGRATE, OnMigrate)
	ON_COMMAND(IDC_SIMULATE, OnSimulate)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
	ON_NOTIFY(NM_SELCHANGED, 1, OnRootChanged)
	ON_NOTIFY(NM_SELUPDATE, 1, OnRootUpdate)
END_MESSAGE_MAP()

int CMigrateWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlasWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Folder selector
	CString hint;
	ENSURE(hint.LoadString(IDS_FOLDER_HINT));
	CString caption;
	ENSURE(caption.LoadString(IDS_FOLDER_CAPTION));
	if (!m_wndFolder.Create(hint, caption, this, 1))
		return -1;

	// Store selector
	ENSURE(hint.LoadString(IDS_STORE_HINT));
	ENSURE(caption.LoadString(IDS_STORE_CAPTION));
	if (!m_wndStore.Create(hint, caption, this, 2))
		return -1;

	// Main view
	if (!m_wndMainView.Create(this, 3))
		return -1;

	// Bottom area
	if (!m_wndBottomArea.Create(this, MAKEINTRESOURCE(IDD_BOTTOMAREA), CBRS_BOTTOM, 4))
		return -1;

	// Aero
	MARGINS Margins = { 0, 0, m_wndFolder.GetPreferredHeight()+11, 0 };
	UseGlasBackground(Margins);

	m_wndMainView.SetFocus();
	AdjustLayout();
	return 0;
}

void CMigrateWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	if (IsWindow(m_wndMainView))
		m_wndMainView.SetFocus();
}

void CMigrateWnd::OnIdleUpdateCmdUI()
{
	CWnd* btn = m_wndBottomArea.GetDlgItem(IDC_MIGRATE);
	if (btn)
		btn->EnableWindow(!m_wndFolder.IsEmpty());
}

void CMigrateWnd::OnSelectRoot()
{
	CString caption;
	ENSURE(caption.LoadString(IDS_BROWSEFORFOLDER_CAPTION));
	CString hint;
	ENSURE(hint.LoadString(IDS_BROWSEFORFOLDER_HINT));

	LFBrowseForFolderDlg dlg(FALSE, _T(""), this, caption, hint);
	if (dlg.DoModal()==IDOK)
		m_wndFolder.SetItem(dlg.m_FolderPIDL);
}

void CMigrateWnd::OnMigrate()
{
	// Simulate
	if (((CButton*)m_wndBottomArea.GetDlgItem(IDC_SIMULATE))->GetCheck())
	{
		MessageBox(_T("Not implemented!"));
		return;
	}

	// Do the real McCoy
	if (m_wndStore.IsEmpty())
	{
		LFChooseStoreDlg dlg(this, LFCSD_Mounted);
		if (dlg.DoModal()!=IDOK)
			return;

		m_wndStore.SetItem(dlg.StoreID);
	}

	char StoreID[LFKeySize];
	if (!m_wndStore.GetStoreID(StoreID))
	{
		LFErrorBox(LFStoreNotFound, m_hWnd);
		return;
	}

	LFItemDescriptor* it = LFAllocItemDescriptor();
	LFItemTemplateDlg dlg(this, it, StoreID);
	if (dlg.DoModal()==IDCANCEL)
		return;

	CButton* btn = (CButton*)m_wndBottomArea.GetDlgItem(IDC_DELETESOURCE);
	BOOL DeleteSource = btn->GetCheck();
	if (DeleteSource)
	{
		DeleteFilesDlg dlg(this);
		if (dlg.DoModal()==IDCANCEL)
			return;

		DeleteSource = dlg.m_Delete;
		btn->SetCheck(DeleteSource);
	}

	MessageBox(_T("Not implemented!"));
	LFFreeItemDescriptor(it);
}

void CMigrateWnd::OnSimulate()
{
	m_wndBottomArea.GetDlgItem(IDC_DELETESOURCE)->EnableWindow(!((CButton*)m_wndBottomArea.GetDlgItem(IDC_SIMULATE))->GetCheck());
}

LRESULT CMigrateWnd::OnStoresChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_wndStore.Update();

	PostMessage(WM_KICKIDLE);

	return NULL;
}

void CMigrateWnd::OnRootChanged(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	if (m_wndFolder.IsEmpty())
	{
		m_wndMainView.ClearRoot();
	}
	else
	{
		m_wndMainView.SetRoot(m_wndFolder.pidl, FALSE, theApp.m_ExpandAll);
	}

	m_wndMainView.SetFocus();
	PostMessage(WM_KICKIDLE);
}

void CMigrateWnd::OnRootUpdate(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	m_wndMainView.SetRoot(m_wndFolder.pidl, TRUE, FALSE);

	PostMessage(WM_KICKIDLE);
}
