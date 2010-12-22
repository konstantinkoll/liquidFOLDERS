
// MigrateWnd.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "Migrate.h"
#include "MigrateWnd.h"
#include "Resource.h"
#include "LFCore.h"
#include "DeleteFilesDlg.h"
#include "CMigrationList.h"
#include "ReportDlg.h"


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

BOOL CMigrateWnd::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The main view gets the command first
	if (m_wndMainView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CGlasWindow::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
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
	ON_COMMAND(IDM_VIEW_SELECTROOT, OnSelectRoot)
	ON_COMMAND(IDC_MIGRATE, OnMigrate)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
	ON_NOTIFY(NM_SELCHANGED, 1, OnRootChanged)
	ON_NOTIFY(NM_SELUPDATE, 1, OnRootUpdate)
END_MESSAGE_MAP()

INT CMigrateWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
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
	// Folders checked?
	if (!m_wndMainView.FoldersChecked())
	{
		CString caption;
		CString message;
		ENSURE(caption.LoadString(IDS_NOFOLDERS_CAPTION));
		ENSURE(message.LoadString(IDS_NOFOLDERS_MESSAGE));
		MessageBox(message, caption, MB_OK | MB_ICONINFORMATION);

		return;
	}

	// Choose store if none is selected
	if (m_wndStore.IsEmpty())
	{
		LFChooseStoreDlg dlg(this, LFCSD_Mounted);
		if (dlg.DoModal()!=IDOK)
			return;

		m_wndStore.SetItem(dlg.StoreID);
	}

	// Paranoid: does the store really exist?
	CHAR StoreID[LFKeySize];
	if (!m_wndStore.GetStoreID(StoreID))
	{
		LFErrorBox(LFStoreNotFound, m_hWnd);
		return;
	}

	// Item template
	LFItemDescriptor* it = LFAllocItemDescriptor();
	LFItemTemplateDlg dlg(this, it, StoreID);
	if (dlg.DoModal()==IDCANCEL)
	{
		LFFreeItemDescriptor(it);
		return;
	}

	// Create snapshot
	CMigrationList ml;
	m_wndMainView.PopulateMigrationList(&ml, it);
	LFFreeItemDescriptor(it);

	// UAC warning if source files should be deleted
	CButton* btn = (CButton*)m_wndBottomArea.GetDlgItem(IDC_DELETESOURCE);
	BOOL DeleteSource = btn->GetCheck();
	if (DeleteSource)
	{
		DeleteFilesDlg dlg(this);
		if (dlg.DoModal()==IDCANCEL)
		{
			LFFreeItemDescriptor(it);
			return;
		}

		DeleteSource = dlg.m_Delete;
		btn->SetCheck(DeleteSource);
	}

	// Migration starten
	CReportList Results[2];
	for (UINT a=0; a<ml.m_ItemCount; a++)
	{
		UINT res = LFImportFiles(StoreID, ml.m_Items[a].List, ml.m_Items[a].Template, ml.m_Items[a].Recursive==TRUE, DeleteSource==TRUE);
		if (res==LFCancel)
			break;

		Results[res==LFOk ? 0 : 1].AddItem(&ml.m_Items[a]);
	}

	// Ergebnis zeigen
	ReportDlg repdlg(this, &Results[0], &Results[1]);
	if (repdlg.DoModal()==IDOK)
		if (repdlg.m_UncheckMigrated)
			m_wndMainView.UncheckMigrated(&Results[0]);
}

LRESULT CMigrateWnd::OnStoresChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_wndStore.Update();

	PostMessage(WM_IDLEUPDATECMDUI);

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
	PostMessage(WM_IDLEUPDATECMDUI);
}

void CMigrateWnd::OnRootUpdate(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	m_wndMainView.SetRoot(m_wndFolder.pidl, TRUE, FALSE);

	PostMessage(WM_IDLEUPDATECMDUI);
}
