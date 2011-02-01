
// MainFrm.cpp: Implementierung der Klasse CMainFrame
//

#include "stdafx.h"
#include "StoreManager.h"
#include "MainFrm.h"
#include "LFCore.h"
#include "SortOptionsDlg.h"
#include "ViewOptionsDlg.h"
#include "LFCommDlg.h"


// CMainFrame
//

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
	ON_WM_DESTROY()

	ON_UPDATE_COMMAND_UI_RANGE(ID_APP_NEWVIEW, ID_APP_VIEW_TAGCLOUD, OnUpdateAppCommands)
	ON_UPDATE_COMMAND_UI(ID_VIEW_AUTODIRS, OnUpdateAppCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SORT_FILENAME, ID_SORT_FILENAME+99, OnUpdateSortCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_NAV_BACK, ID_NAV_GOTOHISTORY, OnUpdateNavCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_PANE_FILTERWND, ID_PANE_HISTORYWND, OnUpdatePaneCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_DROP_NAME, ID_DROP_DIMENSION, OnUpdateDropCommands)

	ON_COMMAND(ID_APP_CLOSE, OnClose)
	ON_COMMAND(ID_APP_CLOSEOTHERS, OnCloseOthers)
	ON_COMMAND(ID_APP_SORTOPTIONS, OnSortOptions)
	ON_COMMAND(ID_APP_VIEWOPTIONS, OnViewOptions)
	ON_COMMAND(ID_VIEW_AUTODIRS, OnToggleAutoDirs)

	ON_COMMAND_RANGE(ID_APP_VIEW_LARGEICONS, ID_APP_VIEW_TAGCLOUD, OnChangeChildView)
	ON_COMMAND_RANGE(ID_SORT_FILENAME, ID_SORT_FILENAME+99, OnSort)

	ON_COMMAND(ID_PANE_FILTERWND, OnToggleFilterWnd)
	ON_COMMAND(ID_PANE_INSPECTORWND, OnToggleInspectorWnd)
	ON_COMMAND(ID_PANE_HISTORYWND, OnToggleHistoryWnd)

	ON_COMMAND(ID_NAV_BACK, OnNavigateBack)
	ON_COMMAND(ID_NAV_FORWARD, OnNavigateForward)
	ON_COMMAND(ID_NAV_RELOAD, OnNavigateReload)
	ON_COMMAND(ID_NAV_STORES, OnNavigateStores)
	ON_COMMAND(ID_NAV_HOME, OnNavigateHome)

	ON_COMMAND(IDM_ITEM_OPEN, OnItemOpen)

	ON_MESSAGE_VOID(WM_UPDATEVIEWOPTIONS, OnUpdateViewOptions)
	ON_MESSAGE_VOID(WM_UPDATESORTOPTIONS, OnUpdateSortOptions)
	ON_MESSAGE_VOID(WM_RELOAD, OnNavigateReload)
	ON_MESSAGE(WM_COOKFILES, OnCookFiles)
	ON_MESSAGE_VOID(WM_UPDATESEARCHRESULT, OnUpdateSearchResult)
	ON_MESSAGE_VOID(WM_INVALIDATE, OnInvalidate)

	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DrivesChanged, OnDrivesChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoreAttributesChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
END_MESSAGE_MAP()


// CMainFrame-Erstellung/Zerstörung

LFFilter* GetRootFilter(char* RootStore=NULL)
{
	LFFilter* f = LFAllocFilter();
	f->Mode = RootStore ? LFFilterModeStoreHome : LFFilterModeStores;
	f->Options.AddBacklink = true;
	f->Options.AddDrives = true;
	f->HideEmptyDrives = (theApp.m_ShowEmptyDrives==FALSE);

	if (RootStore)
		strcpy_s(f->StoreID, LFKeySize, RootStore);

	return f;
}

CMainFrame::CMainFrame(char* RootStore, BOOL _IsClipboard)
{
	IsClipboard = _IsClipboard;
	ActiveViewID = -1;
	ActiveContextID = -1;
	ActiveFilter = GetRootFilter(RootStore);
	ActiveViewParameters = &theApp.m_Views[LFContextDefault];
	RawFiles = NULL;
	CookedFiles = NULL;
	m_BreadcrumbBack = m_BreadcrumbForward = NULL;
	m_wndFilter = NULL;
	m_wndHistory = NULL;
}

CMainFrame::~CMainFrame()
{
	if (ActiveFilter)
		LFFreeFilter(ActiveFilter);
	if (CookedFiles!=RawFiles)
		LFFreeSearchResult(CookedFiles);
	LFFreeSearchResult(RawFiles);
	DeleteBreadcrumbs(&m_BreadcrumbBack);
	DeleteBreadcrumbs(&m_BreadcrumbForward);
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CFrameWndEx::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0, NULL, NULL, LoadIcon(AfxGetResourceHandle(), MAKEINTRESOURCE(IsClipboard ? IDR_CLIPBOARD : IDR_MAINFRAME)));

	return TRUE;
}

INT CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct)==-1)
		return -1;
	theApp.AddFrame(this);
	EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);

	CString tmpStr;

	// Ggf. Fenstertitel und Symbol gegen Clipboard-Icon austauschen
	if (IsClipboard)
	{
		tmpStr = _T("Clipboard");
		SetWindowText(tmpStr);
		SetTitle(tmpStr);
	}

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

	CMFCRibbonButtonsGroup* pGroupPanels = new CMFCRibbonButtonsGroup();
	if (!IsClipboard)
	{
		pGroupPanels->AddButton(new CMFCRibbonButton(ID_PANE_FILTERWND, _T(""), m_PanelImages.ExtractIcon(1)));
		pGroupPanels->AddButton(new CMFCRibbonButton(ID_PANE_HISTORYWND, _T(""), m_PanelImages.ExtractIcon(10)));
	}
	pGroupPanels->AddButton(new CMFCRibbonButton(ID_PANE_INSPECTORWND, _T(""), m_PanelImages.ExtractIcon(2)));

	tmpStr = "Panes";
	m_wndStatusBar.AddExtendedElement(pGroupPanels, tmpStr);

	UINT dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_FLOAT_MULTI;

	if (!IsClipboard)
	{
		// Filter-Pane erstellen
		tmpStr = "Filter";
		m_wndFilter = new CFilterWnd();
		if (!m_wndFilter->Create(tmpStr, this, CRect(0, 0, 250, 550), TRUE, ID_PANE_FILTERWND, dwStyle | CBRS_LEFT))
			return -1;
		m_wndFilter->SetIcon(m_PanelImages.ExtractIcon(1), FALSE);
		m_wndFilter->EnableDocking(CBRS_ALIGN_ANY | CBRS_FLOAT_MULTI);

		// History-Pane erstellen
		tmpStr = "History";
		m_wndHistory = new CHistoryWnd();
		if (!m_wndHistory->Create(tmpStr, this, CRect(0, 0, 250, 550), TRUE, ID_PANE_HISTORYWND, dwStyle | CBRS_LEFT))
			return -1;
		m_wndHistory->SetIcon(m_PanelImages.ExtractIcon(10), FALSE);
		m_wndHistory->EnableDocking(CBRS_ALIGN_ANY | CBRS_FLOAT_MULTI);

		// Tabs
		CTabbedPane* TabbedPane = new CTabbedPane(TRUE);
		if (TabbedPane->Create(_T(""), this, CRect(0, 0, 220, 550), TRUE, UINT(-1), dwStyle | CBRS_LEFT))
		{
			TabbedPane->AddTab(m_wndFilter, TRUE, TRUE, TRUE);
			TabbedPane->AddTab(m_wndHistory, TRUE, FALSE, TRUE);
			TabbedPane->EnableDocking(CBRS_ALIGN_ANY | CBRS_FLOAT_MULTI);
			DockPane(TabbedPane, AFX_IDW_DOCKBAR_LEFT);
		}

		// Diese Panes standardmäßig nicht zeigen
		m_wndFilter->ShowPane(FALSE, FALSE, FALSE);
		m_wndHistory->ShowPane(FALSE, FALSE, FALSE);
		TabbedPane->ShowPane(FALSE, FALSE, FALSE);
	}

	// Inspector-Pane erstellen
	if (!IsClipboard)
		dwStyle |= WS_VISIBLE;

	tmpStr = "Inspector";
	if (!m_wndInspector.Create(tmpStr, this, CRect(0, 0, 250, 550), TRUE, ID_PANE_INSPECTORWND, dwStyle | CBRS_RIGHT | WS_VISIBLE))
		return -1;
	m_wndInspector.SetIcon(m_PanelImages.ExtractIcon(2), FALSE);
	m_wndInspector.EnableDocking(CBRS_ALIGN_ANY | CBRS_FLOAT_MULTI);
	DockPane(&m_wndInspector, AFX_IDW_DOCKBAR_RIGHT);

	// Hauptansicht erstellen
	if (!m_wndMainView.Create(this, AFX_IDW_PANE_FIRST))
		return -1;

	// Entweder leeres Suchergebnis oder Stores-Kontext öffnen
	if (IsClipboard)
	{
		RawFiles = LFAllocSearchResult(LFContextClipboard);
	}
	else
	{
		RawFiles = LFQuery(ActiveFilter);
	}
	OnCookFiles();

	return 0;
}

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	theApp.m_pMainWnd = this;

	if (IsWindow(m_wndMainView))
		m_wndMainView.SetFocus();
}

void CMainFrame::OnClose()
{
	theApp.ReplaceMainFrame(this);
	CFrameWndEx::OnClose();
}

void CMainFrame::OnDestroy()
{
	if (m_wndFilter)
	{
		m_wndFilter->DestroyWindow();
		delete m_wndFilter;
	}
	if (m_wndHistory)
	{
		m_wndHistory->DestroyWindow();
		delete m_wndHistory;
	}

	CFrameWndEx::OnDestroy();
	theApp.KillFrame(this);
}

BOOL CMainFrame::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The main view gets the command first
	if (m_wndMainView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnCloseOthers()
{
	theApp.m_pMainWnd = this;
	theApp.CloseAllFrames(TRUE);
}

void CMainFrame::OnSortOptions()
{
	SortOptionsDlg dlg(this, ActiveViewParameters, ActiveContextID);
	if (dlg.DoModal()==IDOK)
		theApp.UpdateSortOptions(ActiveContextID);
}

void CMainFrame::OnViewOptions()
{
	ViewOptionsDlg dlg(this, ActiveViewParameters, ActiveContextID);
	if (dlg.DoModal()==IDOK)
		theApp.UpdateViewOptions(ActiveContextID);
}

void CMainFrame::OnToggleAutoDirs()
{
	ActiveViewParameters->AutoDirs = (!ActiveViewParameters->AutoDirs);
	theApp.UpdateSortOptions(ActiveContextID);
}

void CMainFrame::OnUpdateAppCommands(CCmdUI* pCmdUI)
{
	UINT view;
	switch (pCmdUI->m_nID)
	{
	case ID_APP_CLOSEOTHERS:
		pCmdUI->Enable(theApp.m_MainFrames.GetCount()>1);
		break;
	case ID_VIEW_AUTODIRS:
		pCmdUI->SetCheck((ActiveViewParameters->AutoDirs) || (ActiveContextID>=LFContextSubfolderDefault));
		pCmdUI->Enable((theApp.m_Contexts[ActiveContextID]->AllowGroups) && (ActiveViewParameters->Mode<=LFViewPreview));
		break;
	case ID_APP_VIEW_LARGEICONS:
	case ID_APP_VIEW_SMALLICONS:
	case ID_APP_VIEW_LIST:
	case ID_APP_VIEW_DETAILS:
	case ID_APP_VIEW_TILES:
	case ID_APP_VIEW_SEARCHRESULT:
	case ID_APP_VIEW_PREVIEW:
	case ID_APP_VIEW_CALENDAR:
	case ID_APP_VIEW_GLOBE:
	case ID_APP_VIEW_TAGCLOUD:
		view = pCmdUI->m_nID-ID_APP_VIEW_LARGEICONS+LFViewLargeIcons;
		pCmdUI->SetCheck(ActiveViewID==(INT)view);
		pCmdUI->Enable(theApp.m_AllowedViews[ActiveContextID]->IsSet(view));
	}
}

void CMainFrame::OnSort(UINT nID)
{
	nID -= ID_SORT_FILENAME;
	if (ActiveViewParameters->SortBy!=nID)
	{
		ActiveViewParameters->SortBy = nID;
		ActiveViewParameters->Descending = (theApp.m_Attributes[nID]->Type==LFTypeRating) || (theApp.m_Attributes[nID]->Type==LFTypeTime);

		theApp.UpdateSortOptions(ActiveContextID);
	}
}

BOOL CMainFrame::AttributeAllowedForSorting(INT attr)
{
	return theApp.m_Contexts[ActiveContextID]->AllowedAttributes->IsSet(attr);
}

void CMainFrame::OnUpdateSortCommands(CCmdUI* pCmdUI)
{
	UINT attr = pCmdUI->m_nID-ID_SORT_FILENAME;
	pCmdUI->SetCheck(ActiveViewParameters->SortBy==attr);
	pCmdUI->Enable(AttributeAllowedForSorting(attr));
}

void CMainFrame::OnUpdateDropCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_DROP_NAME:
		pCmdUI->SetCheck((ActiveViewParameters->SortBy==LFAttrFileName) ||
			(ActiveViewParameters->SortBy==LFAttrTitle));
		pCmdUI->Enable(AttributeAllowedForSorting(LFAttrFileName) ||
			AttributeAllowedForSorting(LFAttrTitle));
		break;
	case ID_DROP_TIME:
		pCmdUI->SetCheck((ActiveViewParameters->SortBy==LFAttrCreationTime) ||
			(ActiveViewParameters->SortBy==LFAttrAddTime) ||
			(ActiveViewParameters->SortBy==LFAttrFileTime) ||
			(ActiveViewParameters->SortBy==LFAttrDeleteTime) ||
			(ActiveViewParameters->SortBy==LFAttrRecordingTime) ||
			(ActiveViewParameters->SortBy==LFAttrDueTime) ||
			(ActiveViewParameters->SortBy==LFAttrDoneTime));
		pCmdUI->Enable(AttributeAllowedForSorting(LFAttrCreationTime) ||
			AttributeAllowedForSorting(LFAttrAddTime) ||
			AttributeAllowedForSorting(LFAttrFileTime) ||
			AttributeAllowedForSorting(LFAttrDeleteTime) ||
			AttributeAllowedForSorting(LFAttrRecordingTime) ||
			AttributeAllowedForSorting(LFAttrDueTime) ||
			AttributeAllowedForSorting(LFAttrDoneTime));
		break;
	case ID_DROP_LOCATION:
		pCmdUI->SetCheck((ActiveViewParameters->SortBy==LFAttrLocationName) ||
			(ActiveViewParameters->SortBy==LFAttrLocationIATA) ||
			(ActiveViewParameters->SortBy==LFAttrLocationGPS));
		pCmdUI->Enable(AttributeAllowedForSorting(LFAttrLocationName) ||
			AttributeAllowedForSorting(LFAttrLocationIATA) ||
			AttributeAllowedForSorting(LFAttrLocationGPS));
		break;
	case ID_DROP_DIMENSION:
		pCmdUI->SetCheck((ActiveViewParameters->SortBy==LFAttrDimension) ||
			(ActiveViewParameters->SortBy==LFAttrWidth) ||
			(ActiveViewParameters->SortBy==LFAttrHeight) ||
			(ActiveViewParameters->SortBy==LFAttrAspectRatio));
		pCmdUI->Enable(AttributeAllowedForSorting(LFAttrDimension) ||
			AttributeAllowedForSorting(LFAttrWidth) ||
			AttributeAllowedForSorting(LFAttrHeight) ||
			AttributeAllowedForSorting(LFAttrAspectRatio));
		break;
	default:
		pCmdUI->Enable(TRUE);
	}
}

void CMainFrame::OnToggleFilterWnd()
{
	if (m_wndFilter)
	{
		BOOL b = m_wndFilter->IsVisible() ? FALSE : TRUE;
		m_wndFilter->ShowPane(b, FALSE, b);
		RecalcLayout(FALSE);
	}
}

void CMainFrame::OnToggleInspectorWnd()
{
	BOOL b = m_wndInspector.IsVisible() ? FALSE : TRUE;
	m_wndInspector.ShowPane(b, FALSE, b);
	RecalcLayout(FALSE);
}

void CMainFrame::OnToggleHistoryWnd()
{
	if (m_wndHistory)
	{
		BOOL b = m_wndHistory->IsVisible() ? FALSE : TRUE;
		m_wndHistory->ShowPane(b, FALSE, b);
		RecalcLayout(FALSE);
	}
}

void CMainFrame::OnUpdatePaneCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_PANE_FILTERWND:
		if (m_wndFilter)
			pCmdUI->SetCheck(m_wndFilter->IsVisible());
		pCmdUI->Enable(m_wndFilter!=NULL);
		break;
	case ID_PANE_INSPECTORWND:
		pCmdUI->SetCheck(m_wndInspector.IsVisible());
		break;
	case ID_PANE_HISTORYWND:
		if (m_wndHistory)
			pCmdUI->SetCheck(m_wndHistory->IsVisible());
		pCmdUI->Enable(m_wndHistory!=NULL);
		break;
	}
}

BOOL CMainFrame::AddClipItem(LFItemDescriptor* i)
{
	ASSERT(IsClipboard);

	for (UINT a=0; a<RawFiles->m_ItemCount; a++)
		if ((strcmp(i->StoreID, RawFiles->m_Items[a]->StoreID)==0) &&
			(strcmp(i->CoreAttributes.FileID, RawFiles->m_Items[a]->CoreAttributes.FileID)==0))
			return FALSE;

	LFAddItemDescriptor(RawFiles, LFAllocItemDescriptor(i));
	return TRUE;
}



void CMainFrame::UpdateSearchResult(BOOL SetEmpty, FVPersistentData* Data)
{
	if ((!SetEmpty) && (CookedFiles))
	{
		ActiveContextID = CookedFiles->m_Context;
		ActiveViewParameters = &theApp.m_Views[ActiveContextID];
	}

	m_wndMainView.UpdateSearchResult(SetEmpty ? NULL : RawFiles, SetEmpty ? NULL : CookedFiles, Data);

	ActiveViewID = ActiveViewParameters->Mode;
}

void CMainFrame::OnChangeChildView(UINT nID)
{
	ActiveViewParameters->Mode = nID-ID_APP_VIEW_LARGEICONS+LFViewLargeIcons;
	theApp.UpdateViewOptions(ActiveContextID);
}

BOOL CMainFrame::UpdateSelectedItems(LFVariantData* value1, LFVariantData* value2, LFVariantData* value3)
{
	return m_wndMainView.UpdateItems(value1, value2, value3);
}

void CMainFrame::OnNavigateBack()
{
	if (ActiveFilter)
	{
		LFFilter* f = ActiveFilter;
		FVPersistentData Data;
		m_wndMainView.GetPersistentData(Data);
		ActiveFilter = NULL;

		AddBreadcrumbItem(&m_BreadcrumbForward, f, Data);
		ConsumeBreadcrumbItem(&m_BreadcrumbBack, &f, &Data);

		NavigateTo(f, NAVMODE_HISTORY, &Data);
	}
}

void CMainFrame::OnNavigateForward()
{
	if (ActiveFilter)
	{
		LFFilter* f = ActiveFilter;
		FVPersistentData Data;
		m_wndMainView.GetPersistentData(Data);
		ActiveFilter = NULL;

		AddBreadcrumbItem(&m_BreadcrumbBack, f, Data);
		ConsumeBreadcrumbItem(&m_BreadcrumbForward, &f, &Data);

		NavigateTo(f, NAVMODE_HISTORY, &Data);
	}
}

void CMainFrame::OnNavigateReload()
{
	if (ActiveFilter)
	{
		FVPersistentData Data;
		m_wndMainView.GetPersistentData(Data);
		NavigateTo(LFAllocFilter(ActiveFilter), NAVMODE_RELOAD, &Data);
	}
}

void CMainFrame::OnNavigateStores()
{
	NavigateTo(GetRootFilter());
}

void CMainFrame::OnNavigateHome()
{
	LFFilter* f = LFAllocFilter();
	f->Mode = LFFilterModeStoreHome;
	f->Options.AddBacklink = true;
	f->Options.AddDrives = true;
	NavigateTo(f);
}

void CMainFrame::OnUpdateNavCommands(CCmdUI* pCmdUI)
{
	BOOL b = !IsClipboard;

	switch (pCmdUI->m_nID)
	{
	case ID_NAV_BACK:
		b &= (m_BreadcrumbBack!=NULL);
		break;
	case ID_NAV_FORWARD:
		b &= (m_BreadcrumbForward!=NULL);
		break;
	case ID_NAV_STORES:
		if (CookedFiles)
			if (CookedFiles->m_Context==LFContextStores)
				b = FALSE;
		break;
	case ID_NAV_HOME:
		if (CookedFiles)
			if (CookedFiles->m_Context==LFContextStoreHome)
				b = FALSE;
		if (!LFDefaultStoreAvailable())
			b = FALSE;
		break;
	}

	pCmdUI->Enable(b);
}

void CMainFrame::InitializeRibbon()
{
	CString strTemp;
	CString strCtx;

	// Hauptschaltfläche initialisieren
	m_MainButton.SetImage(IsClipboard ? IDB_CLIPBOARDBUTTON : IDB_MAINBUTTON);
	strTemp = "Application menu";
	m_MainButton.SetToolTipText(strTemp);

	m_wndRibbonBar.SetApplicationButton(&m_MainButton, CSize (45, 45));
	CMFCRibbonMainPanel* pMainPanel = m_wndRibbonBar.AddMainCategory(strTemp, IDB_APPMENU_16, IDB_APPMENU_32);

		pMainPanel->Add(theApp.CommandButton(ID_APP_NEWVIEW, 0, 0));
		pMainPanel->Add(new CMFCRibbonSeparator(TRUE));
		pMainPanel->Add(theApp.CommandButton(ID_APP_CLOSEOTHERS, 1, 1));
		pMainPanel->Add(theApp.CommandButton(ID_APP_CLOSE, 2, 2));

		strTemp = "Exit";
		pMainPanel->AddToBottom(new CMFCRibbonMainPanelButton(ID_APP_EXIT, strTemp, 3));

	strTemp = "Home";
	CMFCRibbonCategory* pCategoryHome = m_wndRibbonBar.AddCategory(strTemp, IDB_RIBBONHOME_16, IDB_RIBBONHOME_32);

		if (!IsClipboard)
		{
			strTemp = "Navigate";
			CMFCRibbonPanel* pPanelNavigate = pCategoryHome->AddPanel(strTemp, m_PanelImages.ExtractIcon(5));

				pPanelNavigate->Add(theApp.CommandButton(ID_NAV_BACK, 0, 0));
				pPanelNavigate->Add(theApp.CommandButton(ID_NAV_FORWARD, 1, 1));
				pPanelNavigate->AddSeparator();
				pPanelNavigate->Add(theApp.CommandButton(ID_NAV_RELOAD, 2, 2));

			strTemp = "Places";
			CMFCRibbonPanel* pPanelPlaces = pCategoryHome->AddPanel(strTemp, m_PanelImages.ExtractIcon(16));

				pPanelPlaces->Add(theApp.CommandButton(ID_NAV_STORES, 3, 3));
				pPanelPlaces->Add(theApp.CommandButton(ID_NAV_HOME, 4, 4));
		}

	strTemp = "View";
	CMFCRibbonCategory* pCategoryView = m_wndRibbonBar.AddCategory(strTemp, IDB_RIBBONVIEW_16, IDB_RIBBONVIEW_32);

		strTemp = "Arrange items by";
		CMFCRibbonPanel* pPanelArrange = pCategoryView->AddPanel(strTemp, m_PanelImages.ExtractIcon(6));
		pPanelArrange->EnableLaunchButton(ID_APP_SORTOPTIONS, 13);

			strTemp = "Name";
			CMFCRibbonButton* pBtnSortName = new CMFCRibbonButton(ID_DROP_NAME, strTemp, 14, 14);
			pBtnSortName->SetDefaultCommand(FALSE);

				strTemp = "By name";
				pBtnSortName->AddSubItem(new CMFCRibbonLabel(strTemp));
				pBtnSortName->AddSubItem(new CMFCRibbonButton(ID_SORT_FILENAME, theApp.m_Attributes[LFAttrFileName]->Name, 15, 15));
				pBtnSortName->AddSubItem(new CMFCRibbonButton(ID_SORT_TITLE, theApp.m_Attributes[LFAttrTitle]->Name, 16, 16));

			pPanelArrange->Add(pBtnSortName);

			strTemp = "Time";
			CMFCRibbonButton* pBtnSortDate = new CMFCRibbonButton(ID_DROP_TIME, strTemp, 17, 17);
			pBtnSortDate->SetDefaultCommand(FALSE);

				strTemp = "By timestamp";
				pBtnSortDate->AddSubItem(new CMFCRibbonLabel(strTemp));
				pBtnSortDate->AddSubItem(new CMFCRibbonButton(ID_SORT_CREATIONTIME, theApp.m_Attributes[LFAttrCreationTime]->Name, 18, 18));
				pBtnSortDate->AddSubItem(new CMFCRibbonButton(ID_SORT_ADDTIME, theApp.m_Attributes[LFAttrAddTime]->Name, 19, 19));
				pBtnSortDate->AddSubItem(new CMFCRibbonButton(ID_SORT_FILETIME, theApp.m_Attributes[LFAttrFileTime]->Name, 20, 20));
				pBtnSortDate->AddSubItem(new CMFCRibbonButton(ID_SORT_RECORDINGTIME, theApp.m_Attributes[LFAttrRecordingTime]->Name, 21, 21));
				pBtnSortDate->AddSubItem(new CMFCRibbonButton(ID_SORT_DELETETIME, theApp.m_Attributes[LFAttrDeleteTime]->Name, 22, 22));
				strTemp = "By workflow";
				pBtnSortDate->AddSubItem(new CMFCRibbonLabel(strTemp));
				pBtnSortDate->AddSubItem(new CMFCRibbonButton(ID_SORT_DUETIME, theApp.m_Attributes[LFAttrDueTime]->Name, 23, 23));
				pBtnSortDate->AddSubItem(new CMFCRibbonButton(ID_SORT_DONETIME, theApp.m_Attributes[LFAttrDoneTime]->Name, 24, 24));

			pPanelArrange->Add(pBtnSortDate);

			strTemp = "Location";
			CMFCRibbonButton* pBtnSortLocation = new CMFCRibbonButton(ID_DROP_LOCATION, strTemp, 8, 8);
			pBtnSortLocation->SetDefaultCommand(FALSE);

				strTemp = "By location";
				pBtnSortLocation->AddSubItem(new CMFCRibbonLabel(strTemp));
				pBtnSortLocation->AddSubItem(new CMFCRibbonButton(ID_SORT_LOCATIONNAME, theApp.m_Attributes[LFAttrLocationName]->Name, 25, 25));
				pBtnSortLocation->AddSubItem(new CMFCRibbonButton(ID_SORT_LOCATIONIATA, theApp.m_Attributes[LFAttrLocationIATA]->Name, 26, 26));
				pBtnSortLocation->AddSubItem(new CMFCRibbonButton(ID_SORT_LOCATIONGPS, theApp.m_Attributes[LFAttrLocationGPS]->Name, 27, 27));

			pPanelArrange->Add(pBtnSortLocation);

			pPanelArrange->Add(new CMFCRibbonButton(ID_SORT_RATING, theApp.m_Attributes[LFAttrRating]->Name, 28, 28));
			pPanelArrange->Add(new CMFCRibbonButton(ID_SORT_ROLL, theApp.m_Attributes[LFAttrRoll]->Name, 29, 29));

			pPanelArrange->AddSeparator();

			pPanelArrange->Add(new CMFCRibbonButton(ID_SORT_ARTIST, theApp.m_Attributes[LFAttrArtist]->Name, 30, 30));
			pPanelArrange->Add(new CMFCRibbonButton(ID_SORT_COMMENT, theApp.m_Attributes[LFAttrComment]->Name, 31, 31));
			pPanelArrange->Add(new CMFCRibbonButton(ID_SORT_DURATION, theApp.m_Attributes[LFAttrDuration]->Name, 32, 32));
			pPanelArrange->Add(new CMFCRibbonButton(ID_SORT_LANGUAGE, theApp.m_Attributes[LFAttrLanguage]->Name, 33, 33));

			CMFCRibbonButton* pBtnSortResolution = new CMFCRibbonButton(ID_DROP_DIMENSION, theApp.m_Attributes[LFAttrDimension]->Name, 34, 34);
			pBtnSortResolution->SetDefaultCommand(FALSE);

				strTemp = "By overall dimension";
				pBtnSortResolution->AddSubItem(new CMFCRibbonLabel(strTemp));
				pBtnSortResolution->AddSubItem(new CMFCRibbonButton(ID_SORT_ASPECTRATIO, theApp.m_Attributes[LFAttrAspectRatio]->Name, 37, 37));
				pBtnSortResolution->AddSubItem(new CMFCRibbonButton(ID_SORT_DIMENSION, theApp.m_Attributes[LFAttrDimension]->Name, 34, 34));
				strTemp = "By edge";
				pBtnSortResolution->AddSubItem(new CMFCRibbonLabel(strTemp));
				pBtnSortResolution->AddSubItem(new CMFCRibbonButton(ID_SORT_WIDTH, theApp.m_Attributes[LFAttrWidth]->Name, 35, 35));
				pBtnSortResolution->AddSubItem(new CMFCRibbonButton(ID_SORT_HEIGHT, theApp.m_Attributes[LFAttrHeight]->Name, 36, 36));

			pPanelArrange->Add(pBtnSortResolution);
			pPanelArrange->Add(new CMFCRibbonButton(ID_SORT_TAGS, theApp.m_Attributes[LFAttrTags]->Name, 38, 38));

		strTemp = "Aggregate";
		CMFCRibbonPanel* pPanelAggregate = pCategoryView->AddPanel(strTemp, m_PanelImages.ExtractIcon(7));

			pPanelAggregate->Add(theApp.CommandButton(ID_VIEW_AUTODIRS, 12, 12));

		strTemp = "Display search result as";
		CMFCRibbonPanel* pPanelDisplay = pCategoryView->AddPanel(strTemp, m_PanelImages.ExtractIcon(4));
		pPanelDisplay->EnableLaunchButton(ID_APP_VIEWOPTIONS, 11);

			pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_LARGEICONS, 0, 0));
			pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_SMALLICONS, 1, 1));
			pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_LIST, 2, 2));
			pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_DETAILS, 3, 3));
			pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_TILES, 4, 4));
			pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_SEARCHRESULT, 5, 5));
			pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_PREVIEW, 6, 6));

			if (!IsClipboard)
			{
				pPanelDisplay->AddSeparator();
				pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_CALENDAR, 7, 7));
				pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_GLOBE, 8, 8));
				pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_TAGCLOUD, 9, 9));
			}

	m_wndRibbonBar.SetActiveCategory(m_wndRibbonBar.GetCategory(1));

	// Symbolleistenbefehle für Schnellzugriff hinzufügen
	CList<UINT, UINT> lstQATCmds;
	lstQATCmds.AddTail(ID_NAV_BACK);
	lstQATCmds.AddTail(ID_NAV_FORWARD);
	m_wndRibbonBar.SetQuickAccessCommands(lstQATCmds);
}


void CMainFrame::NavigateTo(LFFilter* f, UINT NavMode, FVPersistentData* Data, INT FirstAggregate, INT LastAggregate)
{
	if (NavMode<NAVMODE_RELOAD)
		theApp.PlayNavigateSound();

	if (ActiveFilter)
		if (NavMode==NAVMODE_NORMAL)
		{
			FVPersistentData Data;
			m_wndMainView.GetPersistentData(Data);
			AddBreadcrumbItem(&m_BreadcrumbBack, ActiveFilter, Data);
			DeleteBreadcrumbs(&m_BreadcrumbForward);
		}
		else
		{
			LFFreeFilter(ActiveFilter);
		}
	ActiveFilter = f;

	// Flush the search result so no future paint will access the old search result
	if (NavMode<NAVMODE_RELOAD)
		UpdateSearchResult(TRUE, 0);

	ActiveFilter->HideEmptyDrives = (theApp.m_ShowEmptyDrives==FALSE);
	ActiveFilter->HideEmptyDomains = (theApp.m_ShowEmptyDomains==FALSE);

	INT OldContext = -1;
	LFSearchResult* victim = NULL;

	if (RawFiles)
	{
		OldContext = RawFiles->m_Context;
		if (RawFiles!=CookedFiles)
			victim = RawFiles;
	}

	if ((RawFiles) && (FirstAggregate!=-1) && (LastAggregate!=-1))
	{
		RawFiles = LFQuery(f, RawFiles, FirstAggregate, LastAggregate);
		if ((victim) && (victim!=RawFiles))
			LFFreeSearchResult(victim);
	}
	else
	{
		if (victim)
			LFFreeSearchResult(victim);
		RawFiles = LFQuery(ActiveFilter);
	}

	OnCookFiles((WPARAM)Data);

	if (CookedFiles->m_LastError>LFCancel)
	{
		m_wndMainView.ShowNotification(ActiveFilter->Result.FilterType==LFFilterTypeError ? ENT_ERROR : ENT_WARNING, CookedFiles->m_LastError, CookedFiles->m_LastError==LFIndexAccessError ? IDM_STORES_REPAIRCORRUPTEDINDEX : 0);
	}
	else
	{
		m_wndMainView.DismissNotification();
	}
}

void CMainFrame::UpdateHistory()
{
	if (RawFiles)
	{
		ActiveFilter->Result.FileCount = RawFiles->m_FileCount;
		ActiveFilter->Result.FileSize = RawFiles->m_FileSize;
	}
	if (CookedFiles)
		ActiveFilter->Result.ItemCount = CookedFiles->m_ItemCount;

	if (m_wndHistory)
		m_wndHistory->UpdateList(m_BreadcrumbBack, ActiveFilter, m_BreadcrumbForward);
	if (m_wndFilter)
		m_wndFilter->UpdateList();
}






void CMainFrame::OnItemOpen()
{
	INT idx = m_wndMainView.GetSelectedItem();
	if (idx!=-1)
	{
		LFItemDescriptor* i = CookedFiles->m_Items[idx];

		if (i->NextFilter)
		{
			if (((i->Type & LFTypeMask)==LFTypeVirtual) && (strcmp(i->CoreAttributes.FileID, "BACK")==0) && (m_BreadcrumbBack))
				if (m_BreadcrumbBack->filter->Mode==i->NextFilter->Mode)
				{
					OnNavigateBack();
					return;
				}

			NavigateTo(LFAllocFilter(i->NextFilter), NAVMODE_NORMAL, 0, i->FirstAggregate, i->LastAggregate);
		}
		else
		{
			if (!(i->Type & LFTypeNotMounted))
			{
				WCHAR Path[MAX_PATH];
				UINT res;

				switch (i->Type & LFTypeMask)
				{
				case LFTypeDrive:
					SendMessage(WM_COMMAND, IDM_DRIVE_CREATENEWSTORE);
					break;
				case LFTypeFile:
					res = LFGetFileLocation(i, Path, MAX_PATH, true);
					if (res==LFOk)
					{
						if (ShellExecute(NULL, _T("open"), Path, NULL, NULL, SW_SHOW)==(HINSTANCE)SE_ERR_NOASSOC)
							SendMessage(WM_COMMAND, IDM_FILE_OPENWITH);
					}
					else
					{
						LFErrorBox(res, GetSafeHwnd());
					}
					break;
				default:
					ASSERT(FALSE);
				}
			}
		}
	}
}

// TODO
void CMainFrame::OnUpdateSelection()
{
	m_wndInspector.UpdateStart(ActiveFilter);
	INT i = m_wndMainView.GetNextSelectedItem(-1);

	while (i>=0)
	{
		m_wndInspector.UpdateAdd(CookedFiles->m_Items[i], RawFiles);
		i = m_wndMainView.GetNextSelectedItem(i);
	}

	m_wndInspector.UpdateFinish();
}







void CMainFrame::OnUpdateViewOptions()
{
	if ((ActiveViewID>LFViewPreview)!=(ActiveViewParameters->Mode>LFViewPreview))
	{
		m_wndMainView.SelectNone();
		OnCookFiles();
	}
	else
	{
		m_wndMainView.UpdateViewOptions(ActiveContextID);
	}

	ActiveViewID = ActiveViewParameters->Mode;
}

void CMainFrame::OnUpdateSortOptions()
{
	m_wndMainView.SelectNone();

	FVPersistentData Data;
	m_wndMainView.GetPersistentData(Data);
	OnCookFiles((WPARAM)&Data);

	ActiveViewID = ActiveViewParameters->Mode;
}

LRESULT CMainFrame::OnCookFiles(WPARAM wParam, LPARAM /*lParam*/)
{
	LFSearchResult* Victim = CookedFiles;

	LFViewParameters* vp = &theApp.m_Views[RawFiles->m_Context];
	LFAttributeDescriptor* attr = theApp.m_Attributes[vp->SortBy];

	if (((!IsClipboard) && (vp->AutoDirs) && (!ActiveFilter->Options.IsSubfolder)) || (vp->Mode>LFViewPreview))
	{
		CookedFiles = LFGroupSearchResult(RawFiles, vp->SortBy, (vp->Mode<=LFViewPreview) && (vp->Descending==TRUE), attr->IconID,
			(vp->Mode>LFViewPreview) || ((attr->Type!=LFTypeTime) && (vp->SortBy!=LFAttrFileName) && (vp->SortBy!=LFAttrStoreID) && (vp->SortBy!=LFAttrFileID)),
			ActiveFilter);
	}
	else
	{
		LFSortSearchResult(RawFiles, vp->SortBy, (vp->Mode<=LFViewPreview) && (vp->Descending==TRUE));
		CookedFiles = RawFiles;
	}

	UpdateSearchResult(FALSE, (FVPersistentData*)wParam);
	UpdateHistory();

	if ((Victim) && (Victim!=RawFiles))
		LFFreeSearchResult(Victim);

	if (!LFIsLicensed())
		if ((++theApp.m_NagCounter)>25)
		{
			theApp.m_NagCounter = 0;
			MessageBox(_T("You are using an unregistered copy of liquidFOLDERS. liquidFOLDERS is shareware -\nif you decide to use it regulary, you are required to purchase a license from our website!"), _T("Unregistered copy"));
		}

	return CookedFiles->m_LastError;
}

void CMainFrame::OnUpdateSearchResult()
{
	UpdateSearchResult(FALSE, NULL);
}

void CMainFrame::OnInvalidate()
{
	m_wndMainView.Invalidate();
}


LRESULT CMainFrame::OnDrivesChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (CookedFiles)
		switch (CookedFiles->m_Context)
		{
		case LFContextStores:
			PostMessage(WM_RELOAD);
			break;
		}

	return NULL;
}

LRESULT CMainFrame::OnStoresChanged(WPARAM /*wParam*/, LPARAM lParam)
{
	if ((CookedFiles) && (GetSafeHwnd()!=(HWND)lParam))
		switch (CookedFiles->m_Context)
		{
		case LFContextStores:
			PostMessage(WM_RELOAD);
			break;
		}

	return NULL;
}

LRESULT CMainFrame::OnStoreAttributesChanged(WPARAM wParam, LPARAM lParam)
{
	if ((CookedFiles) && (GetSafeHwnd()!=(HWND)lParam))
		switch (CookedFiles->m_Context)
		{
		case LFContextStores:
			PostMessage(WM_RELOAD);
			break;
		case LFContextStoreHome:
			m_wndMainView.PostMessage(theApp.p_MessageIDs->StoreAttributesChanged, wParam, lParam);
			break;
		}

	return NULL;
}
