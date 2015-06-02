
// MigrationWnd.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "CMigrationList.h"
#include "DeleteFilesDlg.h"
#include "MigrationWnd.h"
#include "ReportDlg.h"
#include "StoreManager.h"


// Thread workers
//

struct WorkerParameters
{
	LFWorkerParameters Hdr;
	CHAR StoreID[LFKeySize];
	BOOL DeleteSource;
	CMigrationList MigrationList;
	CReportList Results[2];
};

DWORD WINAPI WorkerMigration(void* lParam)
{
	LF_WORKERTHREAD_START_EX(lParam, wp->MigartionList.mItemCount);

	for (UINT a=0; a<wp->MigrationList.m_ItemCount; a++)
	{
		LFTransactionImport(wp->StoreID, wp->MigrationList.m_Items[a].List, wp->MigrationList.m_Items[a].Template, wp->MigrationList.m_Items[a].Recursive==TRUE, wp->DeleteSource==TRUE, &p);
		if (wp->MigrationList.m_Items[a].List->m_LastError==LFCancel)
			break;

		wp->Results[wp->MigrationList.m_Items[a].List->m_LastError==LFOk ? 0 : 1].AddItem(&wp->MigrationList.m_Items[a]);
		p.MajorCurrent++;
	}

	LF_WORKERTHREAD_FINISH();
}


// CMigrationWnd
//

BOOL CMigrationWnd::Create(CHAR* Store)
{
	p_Store = Store;

	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW), NULL, theApp.LoadIcon(IDR_MIGRATIONWIZARD));

	CString caption;
	ENSURE(caption.LoadString(IDR_MIGRATIONWIZARD));

	return CGlassWindow::Create(WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, caption, _T("Migration"));
}

BOOL CMigrationWnd::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The main view gets the command first
	if (m_wndMigrationView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CGlassWindow::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMigrationWnd::AdjustLayout()
{
	if (!IsWindow(m_wndFolder))
		return;
	if (!IsWindow(m_wndStore))
		return;
	if (!IsWindow(m_wndBottomArea))
		return;
	if (!IsWindow(m_wndMigrationView))
		return;

	CRect rect;
	GetLayoutRect(rect);

	const UINT Border = 4;
	const UINT SelectorHeight = m_wndFolder.GetPreferredHeight();
	m_wndFolder.SetWindowPos(NULL, rect.left, rect.top+Border, (rect.Width()-Border)/2, SelectorHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndStore.SetWindowPos(NULL, rect.right-(rect.Width()-Border)/2, rect.top+Border, (rect.Width()-Border)/2, SelectorHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	const UINT BottomHeight = MulDiv(45, LOWORD(GetDialogBaseUnits()), 8);
	m_wndBottomArea.SetWindowPos(NULL, rect.left, rect.bottom-BottomHeight, rect.Width(), BottomHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	m_wndMigrationView.SetWindowPos(NULL, rect.left, rect.top+m_Margins.cyTopHeight, rect.Width(), rect.bottom-BottomHeight-m_Margins.cyTopHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(CMigrationWnd, CGlassWindow)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
	ON_COMMAND(IDM_TREE_SELECTROOT, OnSelectRoot)
	ON_COMMAND(IDC_MIGRATE, OnMigrate)
	ON_NOTIFY(NM_SELCHANGED, 1, OnRootChanged)
	ON_NOTIFY(NM_SELUPDATE, 1, OnRootUpdate)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
END_MESSAGE_MAP()

INT CMigrationWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlassWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Folder selector
	CString hint;
	CString caption;
	ENSURE(hint.LoadString(IDS_FOLDER_HINT));
	ENSURE(caption.LoadString(IDS_FOLDER_CAPTION));
	if (!m_wndFolder.Create(hint, caption, this, 1))
		return -1;

	// Store selector
	ENSURE(hint.LoadString(IDS_STORE_HINT));
	ENSURE(caption.LoadString(IDS_STORE_CAPTION));
	if (!m_wndStore.Create(hint, caption, this, 2))
		return -1;

	if (p_Store)
		if (*p_Store!='\0')
			m_wndStore.SetItem(p_Store);

	// Main view
	if (!m_wndMigrationView.Create(this, 3))
		return -1;

	// Bottom area
	if (!m_wndBottomArea.Create(this, MAKEINTRESOURCE(IDD_BOTTOMAREA), 0, 4))
		return -1;

	// Aero
	MARGINS Margins = { 0, 0, m_wndFolder.GetPreferredHeight()+11, 0 };
	UseGlasBackground(Margins);

	m_GlasChildren.AddTail(&m_wndFolder);
	m_GlasChildren.AddTail(&m_wndStore);

	m_wndMigrationView.SetFocus();
	AdjustLayout();
	return 0;
}

void CMigrationWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	theApp.m_pMainWnd = this;
	theApp.m_pActiveWnd = NULL;

	if (IsWindow(m_wndMigrationView))
		m_wndMigrationView.SetFocus();
}

void CMigrationWnd::OnIdleUpdateCmdUI()
{
	CWnd* btn = m_wndBottomArea.GetDlgItem(IDC_MIGRATE);
	if (btn)
		btn->EnableWindow(!m_wndFolder.IsEmpty());
}

void CMigrationWnd::OnSelectRoot()
{
	CString Caption;
	CString Hint;
	ENSURE(Caption.LoadString(IDS_BROWSEFORFOLDER_CAPTION));
	ENSURE(Hint.LoadString(IDS_BROWSEFORFOLDER_HINT));

	LFBrowseForFolderDlg dlg(this, Caption, Hint, FALSE);
	if (dlg.DoModal()==IDOK)
		m_wndFolder.SetItem(dlg.m_FolderPIDL);
}

void CMigrationWnd::OnMigrate()
{
	theApp.ShowNagScreen(NAG_EXPIRED | NAG_FORCE, this);

	// Folders checked ?
	if (!m_wndMigrationView.FoldersChecked())
	{
		CString caption;
		CString message;
		ENSURE(caption.LoadString(IDS_NOFOLDERS_CAPTION));
		ENSURE(message.LoadString(IDS_NOFOLDERS_MESSAGE));
		MessageBox(message, caption, MB_OK | MB_ICONINFORMATION);

		return;
	}

	// Store
	WorkerParameters wp;
	m_wndStore.GetStoreID(wp.StoreID);

	if (wp.StoreID[0]=='\0')
	{
		LFChooseStoreDlg dlg(this);
		if (dlg.DoModal()!=IDOK)
			return;

		strcpy_s(wp.StoreID, LFKeySize, dlg.m_StoreID);
		m_wndStore.SetItem(dlg.m_StoreID);
		m_wndStore.UpdateWindow();
	}

	// Create snapshot
	m_wndMigrationView.PopulateMigrationList(&wp.MigrationList, NULL);

	// UAC warning if source files should be deleted
	CButton* btn = (CButton*)m_wndBottomArea.GetDlgItem(IDC_DELETESOURCE);
	wp.DeleteSource = btn->GetCheck();
	if (wp.DeleteSource)
	{
		DeleteFilesDlg dlg(this);
		if (dlg.DoModal()==IDCANCEL)
			return;

		wp.DeleteSource = dlg.m_Delete;
		btn->SetCheck(wp.DeleteSource);
	}

	// Migration
	LFDoWithProgress(WorkerMigration, &wp.Hdr, this);

	// Show report
	ReportDlg repdlg(this, &wp.Results[0], &wp.Results[1]);
	if (repdlg.DoModal()==IDOK)
		if (repdlg.m_UncheckMigrated)
			m_wndMigrationView.UncheckMigrated(&wp.Results[0]);
}

void CMigrationWnd::OnRootChanged(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	CString caption;
	ENSURE(caption.LoadString(IDR_MIGRATIONWIZARD));

	if (m_wndFolder.IsEmpty())
	{
		m_wndMigrationView.ClearRoot();
	}
	else
	{
		m_wndMigrationView.SetRoot(m_wndFolder.pidl, FALSE, theApp.m_MigrationExpandAll);

		SHFILEINFO sfi;
		if (SUCCEEDED(SHGetFileInfo((WCHAR*)m_wndFolder.pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_DISPLAYNAME)))
			caption = sfi.szDisplayName;
	}

	SetWindowText(caption);

	m_wndMigrationView.SetFocus();
	PostMessage(WM_IDLEUPDATECMDUI);
}

void CMigrationWnd::OnRootUpdate(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	m_wndMigrationView.SetRoot(m_wndFolder.pidl, TRUE, FALSE);

	PostMessage(WM_IDLEUPDATECMDUI);
}

LRESULT CMigrationWnd::OnStoresChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_wndStore.Update();

	PostMessage(WM_IDLEUPDATECMDUI);

	return NULL;
}
