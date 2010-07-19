
// MainFrm.cpp: Implementierung der Klasse CMainFrame
//

#include "stdafx.h"
#include "Migrate.h"
#include "MainFrm.h"
#include "liquidFOLDERS.h"
#include "LFCore.h"
#include "LFCommDlg.h"
#include <io.h>
#include <shlobj.h>
#include <shlwapi.h>


// CMainFrame
//

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->LookChanged, OnLookChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnDefaultStoreChanged)

	ON_UPDATE_COMMAND_UI_RANGE(ID_APP_CLOSE, ID_APP_FOCUSMAIN, OnUpdateAppCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_PANE_WORKFLOWWND, ID_PANE_STOREWND, OnUpdatePaneCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_MIGRATE_START, ID_MIGRATE_SIMULATE, OnUpdateMigrateCommands)
	ON_UPDATE_COMMAND_UI(ID_STORE_NEW, OnUpdateStoreCommands)

	ON_COMMAND(ID_APP_CLOSE, OnClose)
	ON_COMMAND(ID_APP_GRANNYMODE, OnToggleGrannyMode)
	ON_COMMAND(ID_APP_SHOWPLACES, OnShowPlacesWnd)
	ON_COMMAND(ID_APP_SHOWSTORES, OnShowStoresWnd)
	ON_COMMAND(ID_APP_FOCUSMAIN, OnFocusMainWnd)

	ON_COMMAND(ID_PANE_WORKFLOWWND, OnToggleWorkflowWnd)
	ON_COMMAND(ID_PANE_PLACESWND, OnTogglePlacesWnd)
	ON_COMMAND(ID_PANE_STOREWND, OnToggleStoreWnd)

	ON_COMMAND(ID_MIGRATE_DELETE, OnToggleDeleteImported)
	ON_COMMAND(ID_MIGRATE_SIMULATE, OnToggleSimulate)

	ON_COMMAND(ID_STORE_NEW, OnStoreNew)
END_MESSAGE_MAP()


// CMainFrame-Erstellung/Zerstörung

CMainFrame::CMainFrame()
{
	m_sbItemCount = NULL;
	m_sbHint = NULL;
}

CMainFrame::~CMainFrame()
{
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWndEx::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0, 0, 0, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME)));

	return TRUE;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct)==-1)
		return -1;
	EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);

	CString tmpStr;

	// Bereichsbilder laden
	m_PanelImages.SetImageSize(CSize(16, 16));
	m_PanelImages.Load(IDB_PANEL_16);

	// Ribbon erstellen
	m_wndRibbonBar.Create(this);
	m_wndRibbonBar.EnablePrintPreview(FALSE);

	InitializeRibbon();

	// Statusleiste erstellen
	if (!m_wndStatusBar.Create(this))
		return -1;

	m_sbItemCount = new CMFCRibbonStatusBarPane(ID_PANE_STATUSBAR_ITEMCOUNT, _T(""), TRUE);
	m_sbHint = new CMFCRibbonStatusBarPane(ID_PANE_STATUSBAR_HINT, _T(""), TRUE);

	m_wndStatusBar.AddElement(m_sbItemCount, _T("Item count"));
	m_wndStatusBar.AddSeparator();
	m_wndStatusBar.AddElement(m_sbHint, _T("Hint"));

	CMFCRibbonButtonsGroup* pGroupPanels = new CMFCRibbonButtonsGroup();
	pGroupPanels->AddButton(new CMFCRibbonButton(ID_PANE_WORKFLOWWND, _T(""), m_PanelImages.ExtractIcon(1)));
	pGroupPanels->AddButton(new CMFCRibbonButton(ID_PANE_PLACESWND, _T(""), m_PanelImages.ExtractIcon(2)));
	pGroupPanels->AddButton(new CMFCRibbonButton(ID_PANE_STOREWND, _T(""), m_PanelImages.ExtractIcon(3)));

	tmpStr = "Panes";
	m_wndStatusBar.AddExtendedElement(pGroupPanels, tmpStr);

	CMFCRibbonButtonsGroup* pGroupViewOptions = new CMFCRibbonButtonsGroup();
	pGroupViewOptions->AddButton(new CMFCRibbonButton(ID_APP_GRANNYMODE, _T(""), m_PanelImages.ExtractIcon(4)));

	tmpStr = "Display";
	m_wndStatusBar.AddExtendedElement(pGroupViewOptions, tmpStr);

	const UINT dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;

	// Workflow-Pane erstellen
	if (!m_wndWorkflow.Create(WS_CHILD | WS_CLIPSIBLINGS, this, ID_PANE_WORKFLOWWND, 110))
		return -1;

	// Explorer-Pane erstellen
	tmpStr = "Places";
	if (!m_wndPlaces.Create(tmpStr, this, CRect(0, 0, 200, 550), TRUE, ID_PANE_PLACESWND, dwStyle | CBRS_FLOAT_MULTI | CBRS_LEFT))
		return -1;
	m_wndPlaces.SetIcon(m_PanelImages.ExtractIcon(2), FALSE);
	m_wndPlaces.EnableDocking(CBRS_ALIGN_ANY | CBRS_FLOAT_MULTI);
	DockPane(&m_wndPlaces, AFX_IDW_DOCKBAR_LEFT);

	// Store-Pane erstellen
	tmpStr = "Stores";
	if (!m_wndStores.Create(tmpStr, this, CRect(0, 0, 200, 550), TRUE, ID_PANE_STOREWND, dwStyle | CBRS_FLOAT_MULTI | CBRS_RIGHT))
		return -1;
	m_wndStores.SetIcon(m_PanelImages.ExtractIcon(3), FALSE);
	m_wndStores.EnableDocking(CBRS_ALIGN_ANY | CBRS_FLOAT_MULTI);
	DockPane(&m_wndStores, AFX_IDW_DOCKBAR_RIGHT);

	// View erstellen
	m_wndView.Create(this);
	//m_wndView.SetRoot(L"C:\\");

	return 0;
}

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	if (m_wndView)
		m_wndView.SetFocus();
}

void CMainFrame::OnDestroy()
{
	m_wndPlaces.DestroyWindow();
	m_wndStores.DestroyWindow();
	m_wndView.DestroyWindow();

	CFrameWndEx::OnDestroy();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Ansichtsfenster erhält ersten Eindruck vom Befehl
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// Andernfalls die Standardbehandlung durchführen
	return CFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnShowPlacesWnd()
{
	if (!m_wndPlaces.IsVisible())
	{
		m_wndPlaces.ShowPane(TRUE, FALSE, TRUE);
		RecalcLayout(FALSE);
	}
	m_wndPlaces.SetFocus();
}

void CMainFrame::OnShowStoresWnd()
{
	if (!m_wndStores.IsVisible())
	{
		m_wndStores.ShowPane(TRUE, FALSE, TRUE);
		RecalcLayout(FALSE);
	}
	m_wndStores.SetFocus();
}

void CMainFrame::OnFocusMainWnd()
{
	m_wndView.SetFocus();
}

void CMainFrame::OnToggleGrannyMode()
{
	theApp.m_GrannyMode = !theApp.m_GrannyMode;
}

void CMainFrame::OnUpdateAppCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_APP_GRANNYMODE:
		pCmdUI->SetCheck(theApp.m_GrannyMode);
	case ID_APP_CLOSE:
	case ID_APP_SHOWPLACES:
	case ID_APP_SHOWSTORES:
	case ID_APP_FOCUSMAIN:
		pCmdUI->Enable(TRUE);
		break;
	}
}

void CMainFrame::OnToggleWorkflowWnd()
{
	BOOL b = m_wndWorkflow.IsVisible() ? FALSE : TRUE;
	m_wndWorkflow.ShowPane(b, FALSE, b);
	RecalcLayout(FALSE);
}

void CMainFrame::OnTogglePlacesWnd()
{
	BOOL b = m_wndPlaces.IsVisible() ? FALSE : TRUE;
	m_wndPlaces.ShowPane(b, FALSE, b);
	RecalcLayout(FALSE);
}

void CMainFrame::OnToggleStoreWnd()
{
	BOOL b = m_wndStores.IsVisible() ? FALSE : TRUE;
	m_wndStores.ShowPane(b, FALSE, b);
	RecalcLayout(FALSE);
}

void CMainFrame::OnUpdatePaneCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_PANE_WORKFLOWWND:
		pCmdUI->SetCheck(m_wndWorkflow.IsVisible());
		pCmdUI->Enable(TRUE);
		break;
	case ID_PANE_PLACESWND:
		pCmdUI->SetCheck(m_wndPlaces.IsVisible());
		pCmdUI->Enable(TRUE);
		break;
	case ID_PANE_STOREWND:
		pCmdUI->SetCheck(m_wndStores.IsVisible());
		pCmdUI->Enable(TRUE);
		break;
	}
}

void CMainFrame::OnToggleDeleteImported()
{
	theApp.m_DeleteImported = !theApp.m_DeleteImported;
}

void CMainFrame::OnToggleSimulate()
{
	theApp.m_Simulate = !theApp.m_Simulate;
}

void CMainFrame::OnUpdateMigrateCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_MIGRATE_START:
	case ID_MIGRATE_SELECTEXT:
		pCmdUI->Enable(FALSE);
		break;
	case ID_MIGRATE_DELETE:
		pCmdUI->SetCheck(theApp.m_DeleteImported);
		pCmdUI->Enable(!theApp.m_Simulate);
		break;
	case ID_MIGRATE_SIMULATE:
		pCmdUI->SetCheck(theApp.m_Simulate);
		pCmdUI->Enable(TRUE);
		break;
	}
}

void CMainFrame::OnStoreNew()
{
	m_wndStores.PostMessage(WM_COMMAND, ID_STORE_NEW);
}

void CMainFrame::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CMainFrame::InitializeRibbon()
{
	CString strTemp;
	CString strCtx;

	// Hauptschaltfläche initialisieren
	m_MainButton.SetImage(IDB_MAINBUTTON);
	strTemp = "Application menu";
	m_MainButton.SetToolTipText(strTemp);

	m_wndRibbonBar.SetApplicationButton(&m_MainButton, CSize (45, 45));
	CMFCRibbonMainPanel* pMainPanel = m_wndRibbonBar.AddMainCategory(strTemp, IDB_APPMENU_16, IDB_APPMENU_32);

		pMainPanel->Add(theApp.CommandButton(ID_APP_NEWFILEDROP, 0, 0));
		pMainPanel->Add(theApp.CommandButton(ID_APP_NEWSTOREMANAGER, 1, 1));
		pMainPanel->Add(theApp.CommandButton(ID_APP_PROMPT, 2, 2));
		pMainPanel->Add(new CMFCRibbonSeparator(TRUE));
		pMainPanel->Add(theApp.CommandButton(ID_APP_CLOSE, 3, 3));

		strTemp = "About";
		pMainPanel->AddToBottom(new CMFCRibbonMainPanelButton(ID_APP_ABOUT, strTemp, 4));
		strTemp = "Exit";
		pMainPanel->AddToBottom(new CMFCRibbonMainPanelButton(ID_APP_EXIT, strTemp, 5));

	strTemp = "Migrate to liquidFOLDERS";
	CMFCRibbonCategory* pCategoryMigrate = m_wndRibbonBar.AddCategory(strTemp, IDB_RIBBONMIGRATE_16, IDB_RIBBONMIGRATE_32);

		strTemp = "Directory subtree";
		CMFCRibbonPanel* pPanelDirectory = pCategoryMigrate->AddPanel(strTemp, m_PanelImages.ExtractIcon(5));
		pPanelDirectory->EnableLaunchButton(ID_APP_SHOWPLACES, 2);

			pPanelDirectory->Add(theApp.CommandButton(ID_FOLDER_ADD, 0, 0));
			pPanelDirectory->Add(theApp.CommandButton(ID_FOLDER_REMOVE, 1, 1));

		strTemp = "Migrate";
		CMFCRibbonPanel* pPanelMigrate = pCategoryMigrate->AddPanel(strTemp, m_PanelImages.ExtractIcon(6));

			pPanelMigrate->Add(theApp.CommandButton(ID_MIGRATE_START, 3, 3));
			pPanelMigrate->Add(theApp.CommandButton(ID_MIGRATE_SELECTEXT, 4));
			pPanelMigrate->Add(theApp.CommandCheckBox(ID_MIGRATE_DELETE));
			pPanelMigrate->Add(theApp.CommandCheckBox(ID_MIGRATE_SIMULATE));

		strTemp = "Stores";
		CMFCRibbonPanel* pPanelStores = pCategoryMigrate->AddPanel(strTemp, m_PanelImages.ExtractIcon(3));
		pPanelStores->EnableLaunchButton(ID_APP_SHOWSTORES, 6);

			pPanelStores->Add(theApp.CommandButton(ID_STORE_NEW, 5, 5));

		strTemp = "liquidFOLDERS";
		CMFCRibbonPanel* pPanelliquidFOLDERS = pCategoryMigrate->AddPanel(strTemp, m_PanelImages.ExtractIcon(0));
		pPanelliquidFOLDERS->EnableLaunchButton(ID_APP_ABOUT, 11);

			pPanelliquidFOLDERS->Add(theApp.CommandButton(ID_APP_HELP, 7, 7));
			pPanelliquidFOLDERS->Add(theApp.CommandButton(ID_APP_SUPPORT, 8, 8));

			if (!LFIsLicensed())
			{
				pPanelliquidFOLDERS->AddSeparator();
				pPanelliquidFOLDERS->Add(theApp.CommandButton(ID_APP_PURCHASE, 9, 9));
				pPanelliquidFOLDERS->Add(theApp.CommandButton(ID_APP_ENTERLICENSEKEY, 10, 10));
			}

	// Symbolleistenbefehle für Schnellzugriff hinzufügen
	CList<UINT, UINT> lstQATCmds;
	lstQATCmds.AddTail(ID_STORE_NEW);
	lstQATCmds.AddTail(ID_MIGRATE_START);
	m_wndRibbonBar.SetQuickAccessCommands(lstQATCmds);

	// Hilfe hinzufügen
	m_wndRibbonBar.AddToTabs(new CMFCRibbonButton(ID_APP_HELP, NULL, m_PanelImages.ExtractIcon(0)));
}

LRESULT CMainFrame::OnStoresChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_wndStores.UpdateStores(FALSE);
	return NULL;
}

LRESULT CMainFrame::OnDefaultStoreChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_wndStores.UpdateStores(TRUE);
	return NULL;
}

LRESULT CMainFrame::OnLookChanged(WPARAM wParam, LPARAM /*lParam*/)
{
	theApp.SetApplicationLook((UINT)wParam);
	return NULL;
}
