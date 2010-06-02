
// MainFrm.cpp: Implementierung der Klasse CMainFrame
//

#include "stdafx.h"
#include "StoreManager.h"
#include "MainFrm.h"
#include "liquidFOLDERS.h"
#include "LFCore.h"
#include "..\\LFCore\\resource.h"
#include "Kitchen.h"
#include "CFileView.h"
#include "CListView.h"
#include "CCalendarYearView.h"
#include "CCalendarWeekView.h"
#include "CCalendarDayView.h"
#include "CGlobeView.h"
#include "CTagcloudView.h"
#include "CTimelineView.h"
#include "SortOptionsDlg.h"
#include "ViewOptionsDlg.h"
#include "LFCommDlg.h"
#include <io.h>


// CTextureComboBox
//

class CTextureComboBox : public CMFCRibbonComboBox
{
public:
	CTextureComboBox(UINT nID, int nWidth)
		: CMFCRibbonComboBox(nID, FALSE, nWidth)
	{
		AddItem(_T("Automatic"));
		AddItem(_T("1024×1024"));
		AddItem(_T("2048×2048"));
		AddItem(_T("4096×4096"));
		AddItem(_T("8192×4096"));
		SelectItem((int)theApp.m_nTextureSize);
	}

	virtual void OnSelectItem(int nItem)
	{
		theApp.m_nTextureSize = nItem;
		theApp.UpdateViewOptions();
	}
};


// CMainFrame
//

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWndEx)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
	ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DrivesChanged, OnDrivesChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->LookChanged, OnLookChanged)
	ON_COMMAND(ID_APP_UPDATESELECTION, OnUpdateSelection)

	ON_UPDATE_COMMAND_UI_RANGE(ID_APP_HELP, ID_APP_PROMPT, OnUpdateAppCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_APP_NEWVIEW, ID_CONTEXT_SAVEALL, OnUpdateAppCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SORT_FILENAME, ID_SORT_FILENAME+99, OnUpdateAppSortCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_NAV_FIRST, ID_NAV_CLEARHISTORY, OnUpdateNavCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_PANE_CAPTIONBAR, ID_PANE_HISTORYWND, OnUpdatePaneCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_CLIP_COPY, ID_CLIP_REMEMBERNEW, OnUpdateClipCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_FILES_SHOWINSPECTOR, ID_FILES_DELETE, OnUpdateFileCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_STORE_NEW, ID_STORE_BACKUP, OnUpdateStoreCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_DROP_CALENDAR, ID_DROP_RESOLUTION, OnUpdateDropCommands)

	ON_COMMAND(ID_APP_CLOSE, OnClose)
	ON_COMMAND(ID_APP_CLOSEOTHERS, OnAppCloseOthers)
	ON_COMMAND(ID_APP_SORTOPTIONS, OnAppSortOptions)
	ON_COMMAND(ID_APP_VIEWOPTIONS, OnAppViewOptions)

	ON_COMMAND_RANGE(ID_APP_VIEW_AUTOMATIC, ID_APP_VIEW_TIMELINE, OnChangeChildView)
	ON_COMMAND_RANGE(ID_SORT_FILENAME, ID_SORT_FILENAME+99, OnAppSort)
/*	ON_COMMAND(ID_VIEWMODE_AUTODIRS, OnToggleAutoDirs)*/
	ON_COMMAND(ID_CONTEXT_CHOOSE, OnChooseContext)
	ON_COMMAND(ID_CONTEXT_ALWAYSSAVE, OnAlwaysSaveContext)
	ON_COMMAND(ID_CONTEXT_RESTORE, OnRestoreContext)
	ON_COMMAND(ID_CONTEXT_SAVENOW, OnSaveContextNow)
	ON_COMMAND(ID_CONTEXT_SAVEALL, OnSaveContextAll)

	ON_COMMAND(ID_PANE_CAPTIONBAR, OnToggleCaptionBar)
	ON_COMMAND(ID_PANE_FILTERWND, OnToggleFilterWnd)
	ON_COMMAND(ID_PANE_INSPECTORWND, OnToggleInspectorWnd)
	ON_COMMAND(ID_PANE_HISTORYWND, OnToggleHistoryWnd)

	ON_COMMAND(ID_NAV_FIRST, OnNavigateFirst)
	ON_COMMAND(ID_NAV_BACKONE, OnNavigateBackOne)
	ON_MESSAGE(ID_NAV_BACK, OnNavigateBack)
	ON_MESSAGE(ID_NAV_FORWARD, OnNavigateForward)
	ON_COMMAND(ID_NAV_FORWARDONE, OnNavigateForwardOne)
	ON_COMMAND(ID_NAV_LAST, OnNavigateLast)
	ON_COMMAND(ID_NAV_RELOAD, OnNavigateReload)
	ON_COMMAND(ID_NAV_RELOAD_SHOWALL, OnNavigateReloadShowAll)
	ON_COMMAND(ID_NAV_SHOWHISTORY, OnShowHistoryWnd)
	ON_COMMAND(ID_NAV_STORES, OnNavigateStores)
	ON_COMMAND(ID_NAV_HOME, OnNavigateHome)
	ON_COMMAND(ID_NAV_STARTNAVIGATION, OnStartNavigation)
	ON_COMMAND(ID_NAV_CLEARHISTORY, OnClearHistory)

	ON_COMMAND(ID_CLIP_REMOVE, OnClipRemove)
	ON_COMMAND(ID_CLIP_REMEMBERLAST, OnClipRememberLast)
	ON_COMMAND(ID_CLIP_REMEMBERNEW, OnClipRememberNew)

	ON_COMMAND(ID_FILES_SHOWINSPECTOR, OnShowInspectorWnd)
	ON_COMMAND(ID_FILES_DELETE, OnFilesDelete)

	ON_COMMAND(ID_STORE_NEW, OnStoreNew)
	ON_COMMAND(ID_STORE_NEWINTERNAL, OnStoreNewInternal)
	ON_COMMAND(ID_STORE_NEWDRIVE, OnStoreNewDrive)
	ON_COMMAND(ID_STORE_DELETE, OnStoreDelete)
	ON_COMMAND(ID_STORE_RENAME, OnStoreRename)
	ON_COMMAND(ID_STORE_MAKEDEFAULT, OnStoreMakeDefault)
	ON_COMMAND(ID_STORE_MAKEHYBRID, OnStoreMakeHybrid)
	ON_COMMAND(ID_STORE_PROPERTIES, OnStoreProperties)
	ON_COMMAND(ID_STORE_MAINTENANCE, OnStoreMaintenance)
	ON_COMMAND(ID_STORE_BACKUP, OnStoreBackup)
END_MESSAGE_MAP()


// CMainFrame-Erstellung/Zerstörung

LFFilter* GetRootFilter()
{
	LFFilter* f = LFAllocFilter();
	f->Mode = LFFilterModeStores;
	f->Options.AddBacklink = true;
	f->Options.AddDrives = true;
	f->HideEmptyDrives = (theApp.m_HideEmptyDrives==TRUE);

	return f;
}

CMainFrame::CMainFrame(BOOL _IsClipboard)
{
	IsClipboard = _IsClipboard;
	CaptionBarUsed = 0;
	ActiveViewID = -1;
	ActiveContextID = -1;
	ActiveFilter = GetRootFilter();
	ActiveViewParameters = &theApp.m_Views[LFContextDefault];
	RawFiles = NULL;
	CookedFiles = NULL;
	m_wndView = NULL;
	m_BreadcrumbBack = NULL;
	m_BreadcrumbForward = NULL;
	m_wndFilter = NULL;
	m_wndHistory = NULL;
	m_sbItemCount = NULL;
	m_sbHint = NULL;
}

CMainFrame::~CMainFrame()
{
	if (ActiveFilter)
		LFFreeFilter(ActiveFilter);
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
	cs.lpszClass = AfxRegisterWndClass(0, 0, 0, LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IsClipboard ? IDR_CLIPBOARD : IDR_MAINFRAME)));

	return TRUE;
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWndEx::OnCreate(lpCreateStruct)==-1)
		return -1;
	theApp.AddFrame(this);
	EnableDocking(CBRS_ALIGN_LEFT | CBRS_ALIGN_RIGHT);

	CString tmpStr;

	// Ggf. Fenstertitel und Symbol gegen Clipboard-Icon austauschen
	if (IsClipboard)
	{
		tmpStr = "Clipboard";
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

	m_sbFileCount = new CMFCRibbonStatusBarPane(ID_PANE_STATUSBAR_FILECOUNT, _T(""), TRUE);
	m_sbItemCount = new CMFCRibbonStatusBarPane(ID_PANE_STATUSBAR_ITEMCOUNT, _T(""), TRUE);
	m_sbHint = new CMFCRibbonStatusBarPane(ID_PANE_STATUSBAR_HINT, _T(""), TRUE);

	m_wndStatusBar.AddElement(m_sbFileCount, _T("File count"));
	m_wndStatusBar.AddSeparator();
	m_wndStatusBar.AddElement(m_sbItemCount, _T("Item count"));
	m_wndStatusBar.AddSeparator();
	m_wndStatusBar.AddElement(m_sbHint, _T("Hint"));

	CMFCRibbonButtonsGroup* pGroupPanels = new CMFCRibbonButtonsGroup();
	if (!IsClipboard)
	{
		pGroupPanels->AddButton(new CMFCRibbonButton(ID_PANE_CAPTIONBAR, _T(""), m_PanelImages.ExtractIcon(8)));
		pGroupPanels->AddButton(new CMFCRibbonButton(ID_PANE_FILTERWND, _T(""), m_PanelImages.ExtractIcon(1)));
		pGroupPanels->AddButton(new CMFCRibbonButton(ID_PANE_HISTORYWND, _T(""), m_PanelImages.ExtractIcon(10)));
	}
	pGroupPanels->AddButton(new CMFCRibbonButton(ID_PANE_INSPECTORWND, _T(""), m_PanelImages.ExtractIcon(2)));

	tmpStr = "Panes";
	m_wndStatusBar.AddExtendedElement(pGroupPanels, tmpStr);

	if (!IsClipboard)
	{
		// Subfolders
		CMFCRibbonButtonsGroup* pGroupSortOptions = new CMFCRibbonButtonsGroup();
		pGroupSortOptions->AddButton(new CMFCRibbonButton(ID_VIEW_AUTODIRS, _T(""), m_PanelImages.ExtractIcon(3)));
		tmpStr = "Arrangement";
		m_wndStatusBar.AddExtendedElement(pGroupSortOptions, tmpStr);
	}

	CMFCRibbonButtonsGroup* pGroupViewOptions = new CMFCRibbonButtonsGroup();
	pGroupViewOptions->AddButton(new CMFCRibbonButton(ID_VIEW_GRANNY, _T(""), m_PanelImages.ExtractIcon(15)));
	if (!IsClipboard)
		pGroupViewOptions->AddButton(new CMFCRibbonButton(ID_VIEW_CATEGORIES, _T(""), m_PanelImages.ExtractIcon(7)));

	tmpStr = "Display";
	m_wndStatusBar.AddExtendedElement(pGroupViewOptions, tmpStr);

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

	// Titelleiste erstellen
	if (!m_wndCaptionBar.Create(WS_CHILD | WS_CLIPSIBLINGS, this, ID_PANE_CAPTIONBAR, -1, TRUE))
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
	CookFiles(RawFiles->m_Context);

	// Ggf. "Manage stores" im Ribbon aktivieren
	if (!IsClipboard)
		m_wndRibbonBar.SetActiveCategory(m_wndRibbonBar.GetCategory(RibbonCategory_Stores));

	return 0;
}

void CMainFrame::OnSetFocus(CWnd* /*pOldWnd*/)
{
	theApp.m_pMainWnd = this;
	if (m_wndView)
		m_wndView->SetFocus();
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
	if (m_wndView)
	{
		m_wndView->DestroyWindow();
		delete m_wndView;
	}

	CFrameWndEx::OnDestroy();
	theApp.KillFrame(this);
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// Ansichtsfenster erhält ersten Eindruck vom Befehl
	if (m_wndView)
		if (m_wndView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;

	// Andernfalls die Standardbehandlung durchführen
	return CFrameWndEx::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainFrame::OnAppCloseOthers()
{
	theApp.m_pMainWnd = this;
	theApp.CloseAllFrames(TRUE);
}

void CMainFrame::OnAppSortOptions()
{
	SortOptionsDlg dlg(this, ActiveViewParameters, ActiveContextID, IsClipboard);
	if (dlg.DoModal()==IDOK)
	{
		theApp.SaveViewOptions(ActiveContextID);
		theApp.UpdateSortOptions(ActiveContextID);
	}
}

void CMainFrame::OnAppViewOptions()
{
	ViewOptionsDlg dlg(this, theApp.m_nAppLook, ActiveViewParameters, ActiveContextID, CookedFiles);
	if (dlg.DoModal()==IDOK)
	{
		if (dlg.RibbonColor!=theApp.m_nAppLook)
			::SendNotifyMessage(HWND_BROADCAST, theApp.p_MessageIDs->LookChanged, dlg.RibbonColor, 0);

		theApp.SaveViewOptions(ActiveContextID);
		theApp.OpenChildViews(ActiveContextID, TRUE);
	}
}

/*void CMainFrame::OnToggleAutoDirs()
{
	ActiveViewParameters->AutoDirs = !ActiveViewParameters->AutoDirs;
	theApp.SaveViewOptions(ActiveContextID);
	theApp.UpdateSortOptions(ActiveContextID);
}*/

void CMainFrame::OnChooseContext()
{
	m_cbxActiveContext->ClosePopupMenu();
	Invalidate();
	UpdateWindow();
	CookFiles(m_cbxActiveContext->GetCurSel());
}

void CMainFrame::OnAlwaysSaveContext()
{
	ActiveViewParameters->AlwaysSave = !ActiveViewParameters->AlwaysSave;
	theApp.SaveViewOptions(ActiveContextID, SaveMode_FlagOnly);
}

void CMainFrame::OnRestoreContext()
{
	theApp.LoadViewOptions(ActiveContextID);
	CookFiles(ActiveContextID);
}

void CMainFrame::OnSaveContextNow()
{
	theApp.SaveViewOptions(ActiveContextID, SaveMode_Force);
}

void CMainFrame::OnSaveContextAll()
{
	for (int a=0; a<LFContextCount; a++)
		if (theApp.m_Views[a].Changed)
			theApp.SaveViewOptions(a, SaveMode_Force);
}

void CMainFrame::OnUpdateAppCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;
	switch (pCmdUI->m_nID)
	{
	case ID_APP_CLOSEOTHERS:
		pCmdUI->Enable(theApp.m_listMainFrames.size()>1);
		break;
	case ID_CONTEXT_CHOOSE:
		if (RawFiles)
			b = (RawFiles->m_Context>LFContextClipboard);
		pCmdUI->Enable(b);
		break;
	case ID_CONTEXT_ALWAYSSAVE:
		pCmdUI->SetCheck(ActiveViewParameters->AlwaysSave);
		pCmdUI->Enable(TRUE);
		break;
	case ID_CONTEXT_RESTORE:
	case ID_CONTEXT_SAVENOW:
		pCmdUI->Enable((!ActiveViewParameters->AlwaysSave) && (ActiveViewParameters->Changed));
		break;
	case ID_CONTEXT_SAVEALL:
			for (int a=0; a<LFContextCount; a++)
				b |= (!theApp.m_Views[a].AlwaysSave) && (theApp.m_Views[a].Changed);
		pCmdUI->Enable(b);
		break;
/*	case ID_VIEWMODE_AUTODIRS:
		pCmdUI->SetCheck(ActiveViewParameters->AutoDirs);
		pCmdUI->Enable((theApp.m_Contexts[ActiveContextID]->AllowGroups) && (ActiveViewParameters->Mode<=LFViewPreview));
		break;*/
	case ID_APP_VIEW_LARGEICONS:
	case ID_APP_VIEW_SMALLICONS:
	case ID_APP_VIEW_LIST:
	case ID_APP_VIEW_DETAILS:
	case ID_APP_VIEW_TILES:
		pCmdUI->Enable(LFAttributeSortableInView(ActiveViewParameters->SortBy, pCmdUI->m_nID-ID_APP_VIEW_AUTOMATIC+LFViewAutomatic));
	case ID_APP_VIEW_AUTOMATIC:
		pCmdUI->SetCheck(ActiveViewParameters->Mode==pCmdUI->m_nID-ID_APP_VIEW_AUTOMATIC+LFViewAutomatic);
		break;
	case ID_APP_VIEW_PREVIEW:
		pCmdUI->SetCheck(ActiveViewParameters->Mode==LFViewPreview);
		pCmdUI->Enable((ActiveContextID>LFContextStoreHome) && LFAttributeSortableInView(ActiveViewParameters->SortBy, pCmdUI->m_nID-ID_APP_VIEW_AUTOMATIC+LFViewAutomatic));
		break;
	case ID_APP_VIEW_CALENDAR_YEAR:
	case ID_APP_VIEW_CALENDAR_WEEK:
	case ID_APP_VIEW_CALENDAR_DAY:
	case ID_APP_VIEW_GLOBE:
	case ID_APP_VIEW_TAGCLOUD:
	case ID_APP_VIEW_TIMELINE:
		pCmdUI->SetCheck(ActiveViewParameters->Mode==pCmdUI->m_nID-ID_APP_VIEW_AUTOMATIC+LFViewAutomatic);
		pCmdUI->Enable(theApp.m_Contexts[ActiveContextID]->AllowExtendedViews && LFAttributeSortableInView(ActiveViewParameters->SortBy, pCmdUI->m_nID-ID_APP_VIEW_AUTOMATIC+LFViewAutomatic));
		break;
	default:
		theApp.OnUpdateAppCommands(pCmdUI);
	}
}

void CMainFrame::OnAppSort(UINT nID)
{
	nID -= ID_SORT_FILENAME;
	if (ActiveViewParameters->SortBy!=nID)
	{
		ActiveViewParameters->SortBy = nID;
		ActiveViewParameters->Descending = (theApp.m_Attributes[nID]->Type==LFTypeRating);
		if (!LFAttributeSortableInView(ActiveViewParameters->SortBy, ActiveViewParameters->Mode))
			ActiveViewParameters->Mode = LFViewAutomatic;

		theApp.SaveViewOptions(ActiveContextID);
		theApp.UpdateSortOptions(ActiveContextID);
	}
}

BOOL CMainFrame::AttributeAllowedForSorting(int attr)
{
	return theApp.m_Contexts[ActiveContextID]->AllowedAttributes->IsSet(attr);
}

void CMainFrame::OnUpdateAppSortCommands(CCmdUI* pCmdUI)
{
	UINT attr = pCmdUI->m_nID-ID_SORT_FILENAME;
	pCmdUI->SetCheck(ActiveViewParameters->SortBy==attr);
	pCmdUI->Enable(AttributeAllowedForSorting(attr));
}

void CMainFrame::OnUpdateDropCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_DROP_CALENDAR:
		pCmdUI->SetCheck((ActiveViewParameters->Mode>=LFViewCalendarYear) &&
			(ActiveViewParameters->Mode<=LFViewCalendarDay));
		pCmdUI->Enable(theApp.m_Contexts[ActiveContextID]->AllowExtendedViews &&
			(LFAttributeSortableInView(ActiveViewParameters->SortBy, LFViewCalendarYear) ||
			LFAttributeSortableInView(ActiveViewParameters->SortBy, LFViewCalendarWeek) ||
			LFAttributeSortableInView(ActiveViewParameters->SortBy, LFViewCalendarDay)));
		break;
	case ID_DROP_NAME:
		pCmdUI->SetCheck((ActiveViewParameters->SortBy==LFAttrFileName) ||
			(ActiveViewParameters->SortBy==LFAttrTitle));
		pCmdUI->Enable(AttributeAllowedForSorting(LFAttrFileName) ||
			AttributeAllowedForSorting(LFAttrTitle));
		break;
	case ID_DROP_TIME:
		pCmdUI->SetCheck((ActiveViewParameters->SortBy==LFAttrCreationTime) ||
			(ActiveViewParameters->SortBy==LFAttrFileTime) ||
			(ActiveViewParameters->SortBy==LFAttrRecordingTime) ||
			(ActiveViewParameters->SortBy==LFAttrDueTime) ||
			(ActiveViewParameters->SortBy==LFAttrDoneTime));
		pCmdUI->Enable(AttributeAllowedForSorting(LFAttrCreationTime) ||
			AttributeAllowedForSorting(LFAttrFileTime) ||
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
	case ID_DROP_RESOLUTION:
		pCmdUI->SetCheck((ActiveViewParameters->SortBy==LFAttrResolution) ||
			(ActiveViewParameters->SortBy==LFAttrHeight) ||
			(ActiveViewParameters->SortBy==LFAttrWidth) ||
			(ActiveViewParameters->SortBy==LFAttrAspectRatio));
		pCmdUI->Enable(AttributeAllowedForSorting(LFAttrResolution) ||
			AttributeAllowedForSorting(LFAttrHeight) ||
			AttributeAllowedForSorting(LFAttrWidth) ||
			AttributeAllowedForSorting(LFAttrAspectRatio));
		break;
	default:
		pCmdUI->Enable(TRUE);
	}
}

void CMainFrame::OnToggleCaptionBar()
{
	BOOL b = m_wndCaptionBar.IsVisible() ? SW_HIDE : SW_SHOW;
	m_wndCaptionBar.ShowWindow(b);
	RecalcLayout(FALSE);
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

void CMainFrame::OnShowInspectorWnd()
{
	if (!m_wndInspector.IsVisible())
	{
		m_wndInspector.ShowPane(TRUE, FALSE, TRUE);
		RecalcLayout(FALSE);
	}
	m_wndInspector.SetFocus();
}

void CMainFrame::OnToggleInspectorWnd()
{
	BOOL b = m_wndInspector.IsVisible() ? FALSE : TRUE;
	m_wndInspector.ShowPane(b, FALSE, b);
	RecalcLayout(FALSE);
}

void CMainFrame::OnShowHistoryWnd()
{
	if (m_wndHistory)
	{
		if (!m_wndHistory->IsVisible())
		{
			m_wndHistory->ShowPane(TRUE, FALSE, TRUE);
			RecalcLayout(FALSE);
		}
		m_wndHistory->SetFocus();
	}
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
	case ID_PANE_CAPTIONBAR:
		pCmdUI->SetCheck(m_wndCaptionBar.IsVisible());
		pCmdUI->Enable(CaptionBarUsed);
		break;
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

	#ifndef _DEBUG
	if ((i->Type & LFTypeMask)!=LFTypeFile)
		return FALSE;
	#endif

	for (UINT a=0; a<RawFiles->m_ItemCount; a++)
		if ((strcmp(i->CoreAttributes.StoreID, RawFiles->m_Items[a]->CoreAttributes.StoreID)==0) &&
			(strcmp(i->CoreAttributes.FileID, RawFiles->m_Items[a]->CoreAttributes.FileID)==0))
			return FALSE;

	LFAddItemDescriptor(RawFiles, LFAllocItemDescriptor(i));
	return TRUE;
}

void CMainFrame::OnClipRemove()
{
	ASSERT(IsClipboard);

	if (RawFiles)
	{
		int idx = GetNextSelectedItem(-1);
		while (idx!=-1)
		{
			int pos = CookedFiles->m_Items[idx]->Position;
			if (pos!=-1)
			{
				RawFiles->m_Items[pos]->DeleteFlag = true;
				m_wndView->SelectItem(idx, FALSE, TRUE);
			}

			idx = GetNextSelectedItem(idx);
		}

		LFRemoveFlaggedItemDescriptors(RawFiles);
		CookFiles(ActiveContextID, GetFocusItem());
	}
}

void CMainFrame::Remember(CMainFrame* clip)
{
	ASSERT(!IsClipboard);
	ASSERT(clip);

	if (CookedFiles)
	{
		int idx = GetNextSelectedItem(-1);
		BOOL changes = FALSE;
		while (idx!=-1)
		{
			if (clip->AddClipItem(CookedFiles->m_Items[idx]))
				changes = TRUE;

			idx = GetNextSelectedItem(idx);
		}

		if (changes)
			clip->CookFiles(clip->RawFiles->m_Context, clip->GetFocusItem());
	}
}

void CMainFrame::OnClipRememberLast()
{
	Remember(theApp.GetClipboard(FALSE));
}

void CMainFrame::OnClipRememberNew()
{
	Remember(theApp.GetClipboard(TRUE));
}

void CMainFrame::OnUpdateClipCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;
	switch (pCmdUI->m_nID)
	{
	case ID_CLIP_COPY:
		b = FilesSelected;
		break;
	case ID_CLIP_PASTE:
		b = (!IsClipboard) && FALSE;
		break;
	case ID_CLIP_REMOVE:
		b = (IsClipboard) && (GetNextSelectedItem(-1)!=-1);
		break;
	case ID_CLIP_REMEMBERLAST:
	case ID_CLIP_REMEMBERNEW:
		b = (!IsClipboard)
		#ifndef _DEBUG
		&& FilesSelected
		#endif
		;
		break;
	}

	pCmdUI->Enable(b);
}

void CMainFrame::OnFilesDelete()
{
	UpdateTrashFlag(TRUE);
}

void CMainFrame::OnRestoreSelectedFiles()
{
	UpdateTrashFlag(FALSE);
}

void CMainFrame::OnRestoreAllFiles()
{
	UpdateTrashFlag(FALSE, TRUE);
}

void CMainFrame::OnUpdateFileCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	switch (pCmdUI->m_nID)
	{
	case ID_FILES_SHOWINSPECTOR:
		b = TRUE;
		break;
	case ID_FILES_DELETE:
		//b = RawFiles->m_FileCount;
		// TODO
		b = TRUE;
		break;
	}

	pCmdUI->Enable(b);
}

void CMainFrame::ExecuteCreateStoreDlg(UINT nIDTemplate, char drv)
{
	LFStoreDescriptor* s = LFAllocStoreDescriptor();

	LFStoreNewDlg dlg(this, nIDTemplate, drv, s);
	if (dlg.DoModal()==IDOK)
		LFErrorBox(LFCreateStore(s, dlg.makeDefault));

	LFFreeStoreDescriptor(s);
}

void CMainFrame::OnStoreNew()
{
	ExecuteCreateStoreDlg(IDD_STORENEW, '\0');
}

void CMainFrame::OnStoreNewInternal()
{
	LFStoreDescriptor* s = LFAllocStoreDescriptor();
	s->AutoLocation = TRUE;
	s->StoreMode = LFStoreModeInternal;

	UINT res = LFCreateStore(s);
	LFErrorBox(res);

	if ((res==LFOk) && (CookedFiles) && (m_wndView))
		for (UINT a=0; a<CookedFiles->m_ItemCount; a++)
		{
			LFItemDescriptor* i = CookedFiles->m_Items[a];
			if ((strcmp(s->StoreID, i->CoreAttributes.StoreID)==0) && ((i->Type & LFTypeMask)==LFTypeStore))
			{
				m_wndView->SetFocus();
				m_wndView->EditLabel(a);
				break;
			}
		}

	LFFreeStoreDescriptor(s);
}

void CMainFrame::OnStoreNewDrive()
{
	int i = GetSelectedItem();

	if (i!=-1)
		ExecuteCreateStoreDlg(IDD_STORENEWDRIVE, CookedFiles->m_Items[i]->CoreAttributes.FileID[0]);
}

void CMainFrame::OnStoreDelete()
{
	int i = GetSelectedItem();

	if (i!=-1)
	{
		LFItemDescriptor* store = LFAllocItemDescriptor(CookedFiles->m_Items[i]);
		LFErrorBox(theApp.DeleteStore(store));
		LFFreeItemDescriptor(store);
	}
}

void CMainFrame::OnStoreRename()
{
	if (m_wndView)
	{
		m_wndView->SetFocus();
		m_wndView->EditLabel(GetSelectedItem());
	}
}

void CMainFrame::OnStoreMakeDefault()
{
	int i = GetSelectedItem();

	if (i!=-1)
		LFErrorBox(LFMakeDefaultStore(CookedFiles->m_Items[i]->CoreAttributes.StoreID));
}

void CMainFrame::OnStoreMakeHybrid()
{
	int i = GetSelectedItem();

	if (i!=-1)
		LFErrorBox(LFMakeHybridStore(CookedFiles->m_Items[i]->CoreAttributes.StoreID));
}

CString MakeHex(BYTE* x, UINT bCount)
{
	CString tmpStr;
	for (UINT a=0; a<bCount; a++)
	{
		CString digit;
		digit.Format(_T("%.2x"), x[a]);
		tmpStr += digit;
		if (a<bCount-1)
			tmpStr += _T(",");
	}
	return tmpStr;
}

void CEscape(CString &s)
{
	for (int a = s.GetLength()-1; a>=0; a--)
		if ((s[a]=='\\') || (s[a]=='\"'))
			s.Insert(a, '\\');
}

void CMainFrame::OnStoreProperties()
{
	int i = GetSelectedItem();

	if (i!=-1)
	{
		LFStorePropertiesDlg dlg(this, CookedFiles->m_Items[i]->CoreAttributes.StoreID);
		dlg.DoModal();
	}
}

void CMainFrame::OnStoreMaintenance()
{
	LFMaintenanceDlgParameters p;
	ZeroMemory(&p, sizeof(p));

	LFErrorBox(LFStoreMaintenance(&p.Repaired, &p.NoAccess, &p.NoFreeSpace, &p.RepairError), GetSafeHwnd());

	LFStoreMaintenanceDlg dlg(&p, this);
	dlg.DoModal();
}

void CMainFrame::OnStoreBackup()
{
	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_REGFILEFILTER));
	tmpStr += _T(" (*.reg)|*.reg||");

	CFileDialog dlg(FALSE, _T(".reg"), NULL, OFN_OVERWRITEPROMPT, tmpStr, this);

	if (dlg.DoModal()==IDOK)
	{
		CStdioFile f;
		if (!f.Open(dlg.GetFileName(), CFile::modeCreate | CFile::modeWrite))
		{
			LFErrorBox(LFDriveNotReady);
		}
		else
		{
			try
			{
				f.WriteString(_T("Windows Registry Editor Version 5.00\n"));

				for (UINT a=0; a<CookedFiles->m_ItemCount; a++)
				{
					LFItemDescriptor* i = CookedFiles->m_Items[a];
					if ((i->Type & LFTypeStore) && (i->CategoryID==LFCategoryInternalStores))
					{
						LFStoreDescriptor s;
						if (LFGetStoreSettings(i->CoreAttributes.StoreID, &s)==LFOk)
						{
							// Header
							tmpStr = _T("\n[HKEY_CURRENT_USER\\Software\\liquidFOLDERS\\Stores\\");
							tmpStr += s.StoreID;
							f.WriteString(tmpStr+_T("]\n"));

							// Name
							tmpStr = s.StoreName;
							CEscape(tmpStr);
							f.WriteString(_T("\"Name\"=\"")+tmpStr+_T("\"\n"));

							// Mode
							tmpStr.Format(_T("\"Mode\"=dword:%.8x\n"), s.StoreMode);
							f.WriteString(tmpStr);

							// AutoLocation
							tmpStr.Format(_T("\"AutoLocation\"=dword:%.8x\n"), s.AutoLocation);
							f.WriteString(tmpStr);

							if (!s.AutoLocation)
							{
								// Path
								tmpStr = s.DatPath;
								CEscape(tmpStr);
								f.WriteString(_T("\"Path\"=\"")+tmpStr+_T("\"\n"));
							}

							// GUID
							f.WriteString(_T("\"GUID\"=hex:")+MakeHex((BYTE*)&s.guid, sizeof(s.guid))+_T("\n"));

							// CreationTime
							f.WriteString(_T("\"CreationTime\"=hex:")+MakeHex((BYTE*)&s.CreationTime, sizeof(s.CreationTime))+_T("\n"));

							// FileTime
							f.WriteString(_T("\"FileTime\"=hex:")+MakeHex((BYTE*)&s.FileTime, sizeof(s.FileTime))+_T("\n"));
						}
					}
				}
			}
			catch(CFileException ex)
			{
				LFErrorBox(LFDriveNotReady);
			}

			f.Close();
		}
	}
}

void CMainFrame::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;
	if ((ActiveContextID==LFContextStores) && (CookedFiles))
	{
		int i = GetSelectedItem();
		LFItemDescriptor* f = (i==-1 ? NULL : CookedFiles->m_Items[i]);

		switch (pCmdUI->m_nID)
		{
		case ID_STORE_NEW:
		case ID_STORE_NEWINTERNAL:
			b = TRUE;
			break;
		case ID_STORE_NEWDRIVE:
			if (f)
				b = (f->Type & LFTypeDrive) &&
					(!(f->Type & LFTypeNotMounted));
			break;
		case ID_STORE_RENAME:
			if (f)
				b = (f->Type & LFTypeStore) && (!(f->Type & LFTypeNotMounted)) && (ActiveViewID>=LFViewLargeIcons) && (ActiveViewID<=LFViewPreview);
			if ((b) && (m_wndView))
				b ^= m_wndView->IsEditing();
			break;
		case ID_STORE_DELETE:
		case ID_STORE_PROPERTIES:
			if (f)
				b = (f->Type & LFTypeStore);
			break;
		case ID_STORE_MAKEDEFAULT:
			if (f)
				b = (f->Type & LFTypeStore) &&
					(f->CategoryID==LFCategoryInternalStores) &&
					((f->Type & LFTypeDefaultStore)==0);
			break;
		case ID_STORE_MAKEHYBRID:
			if (f)
				b = (f->Type & LFTypeStore) &&
					(f->CategoryID==LFCategoryExternalStores);
			break;
		case ID_STORE_MAINTENANCE:
			b = LFGetStoreCount();
			break;
		case ID_STORE_BACKUP:
			b = TRUE;
		}
	}

	pCmdUI->Enable(b);
}

void CMainFrame::UpdateViewOptions()
{
	if (m_wndView)
	{
		if (ActiveViewID==LFViewGlobe)
		{
			CMFCRibbonBaseElement* cbx = m_wndRibbonBar.FindByID(ID_GLOBE_TEXTURESIZE, FALSE, TRUE);
			if (cbx)
				((CTextureComboBox*)cbx)->SelectItem((int)theApp.m_nTextureSize);
		}

		m_wndView->OnUpdateViewOptions();
	}
}

void CMainFrame::UpdateSortOptions()
{
	CookFiles(ActiveContextID);
}

void CMainFrame::UpdateSearchResult(BOOL SetEmpty, int FocusItem)
{
	if ((!SetEmpty) && (CookedFiles))
	{
		// Im Debug-Modus bleiben alle Kategorien sichtbar
		#ifndef _DEBUG
		m_wndRibbonBar.ShowCategory(RibbonCategory_Stores, CookedFiles->m_Context==LFContextStores);
		m_wndRibbonBar.ShowCategory(RibbonCategory_Trash, (CookedFiles->m_Context==LFContextHousekeeping) && (ActiveFilter->Result.FilterType==LFFilterTypeTrash));
		m_wndRibbonBar.ShowCategory(RibbonCategory_UnknownFileFormats, (CookedFiles->m_Context==LFContextHousekeeping) && (ActiveFilter->Result.FilterType==LFFilterTypeUnknownFileFormats));
		#endif

		// ChildView austauschen:
		// - Wenn ein anderer Kontext mit ggf. anderen Views gewünscht wird
		// - Wenn im Kontext die Ansicht auf "automatisch" steht
		// - Wenn sich für die Liste das Kategorien-Flag ändert (wg. virtual mode)
		BOOL change = (ActiveContextID!=CookedFiles->m_ContextView) || (ActiveViewID!=(int)SelectViewMode(ActiveViewParameters->Mode));
		BOOL force = FALSE;
		if ((!change) && (m_wndView) && (ActiveViewID>=LFViewLargeIcons) && (ActiveViewID<=LFViewPreview))
		{
			change |= (m_wndView->HasCategories()!=(CookedFiles->m_HasCategories==true));
			force = TRUE;
		}
		if (change)
		{
			ActiveContextID = CookedFiles->m_ContextView;
			ActiveViewParameters = &theApp.m_Views[ActiveContextID];
			if (m_cbxActiveContext)
				m_cbxActiveContext->SelectItem(ActiveContextID);
			if (OpenChildView(force))
				return;
		}
		#ifndef _DEBUG
		else
		{
			// Erst hinterher updaten, wenn OpenChildView() nicht aufgerufen wurde (dort wird Update() auch aufgerufen)
			m_wndRibbonBar.Update();
		}
		#endif
	}

	if (m_wndView)
		m_wndView->OnUpdateSearchResult(SetEmpty ? NULL : CookedFiles, FocusItem);
	OnUpdateSelection();
}

void CMainFrame::OnChangeChildView(UINT nID)
{
	ActiveViewParameters->Mode = nID-ID_APP_VIEW_AUTOMATIC+LFViewAutomatic;
	theApp.SaveViewOptions(ActiveContextID);
	theApp.OpenChildViews(ActiveContextID);
}

int CMainFrame::GetFocusItem()
{
	return (m_wndView ? m_wndView->GetFocusItem() : -1);
}

int CMainFrame::GetSelectedItem()
{
	return (m_wndView ? m_wndView->GetSelectedItem() : -1);
}

int CMainFrame::GetNextSelectedItem(int n)
{
	return (m_wndView ? m_wndView->GetNextSelectedItem(n) : -1);
}

void CMainFrame::OnUpdateSelection()
{
	// Focus
	int i = GetFocusItem();

	if (m_sbHint)
		if (i==-1)
		{
			m_sbHint->SetText(_T(""));
		}
		else
		{
			CString tmpStr;
			if ((CookedFiles->m_Items[i]->Type & (LFTypeStore | LFTypeRequiresMaintenance))==(LFTypeStore | LFTypeRequiresMaintenance))
			{
				ENSURE(tmpStr.LoadString(IDS_REQUIRESMAINTENANCE));
			}
			else
			{
				tmpStr = CookedFiles->m_Items[i]->Hint;
			}
			m_sbHint->SetText(tmpStr);
		}

	// Selection
	m_wndInspector.UpdateStart();
	i = GetNextSelectedItem(-1);
	FilesSelected = FALSE;
	UINT Count = 0;
	__int64 Size = 0;

	while (i>=0)
	{
		m_wndInspector.UpdateAdd(CookedFiles->m_Items[i]);
		FilesSelected |= (CookedFiles->m_Items[i]->Type & LFTypeMask)==LFTypeFile;
		Count++;
		Size += *(__int64*)CookedFiles->m_Items[i]->AttributeValues[LFAttrFileSize];
		i = GetNextSelectedItem(i);
	}

	m_wndInspector.UpdateFinish();

	if (m_sbItemCount)
	{
		CString maskStr;
		wchar_t sizeStr[256];
		CString tmpStr;
		if (Count)
		{
			ENSURE(maskStr.LoadString(Count==1 ? IDS_SELECTED_SINGULAR : IDS_SELECTED_PLURAL));
			StrFormatByteSizeW(Size, sizeStr, 256);
		}
		else
		{
			if (CookedFiles)
				Count = CookedFiles->m_ItemCount;
	
			ENSURE(maskStr.LoadString(Count==1 ? IDS_ITEMS_SINGULAR : IDS_ITEMS_PLURAL));
		}
		tmpStr.Format(maskStr, Count, sizeStr);
		m_sbItemCount->SetText(tmpStr);
	}

	if ((m_sbHint) || (m_sbItemCount))
	{
		// Update
		m_wndStatusBar.RecalcLayout();
		m_wndStatusBar.Invalidate();
	}
}

void CMainFrame::OnUpdateFileCount()
{
	if (m_sbFileCount)
	{
		CString tmpStr;
		if (CookedFiles->m_FileCount)
		{
			CString maskStr;
			ENSURE(maskStr.LoadString(CookedFiles->m_FileCount==1 ? IDS_FILES_SINGULAR : IDS_FILES_PLURAL));
			
			wchar_t sizeStr[256];
			StrFormatByteSizeW(CookedFiles->m_FileSize, sizeStr, 256);

			tmpStr.Format(maskStr, CookedFiles->m_FileCount, sizeStr);
		}
		else
		{
			ENSURE(tmpStr.LoadString(IDS_NOFILES));
		}
		m_sbFileCount->SetText(tmpStr);

		// Update
		m_wndStatusBar.RecalcLayout();
		m_wndStatusBar.Invalidate();
	}

}

BOOL CMainFrame::RenameSingleItem(UINT n, CString Name)
{
	ASSERT(theApp.m_Attributes[LFAttrFileName]->Type==LFTypeUnicodeString);

	BOOL result = FALSE;

	if (Name!="")
	{
		LFTransactionList* tl = LFAllocTransactionList();
		LFAddItemDescriptor(tl, CookedFiles->m_Items[n]);

		LFVariantData value;
		value.Attr = LFAttrFileName;
		value.Type = LFTypeUnicodeString;
		value.IsNull = false;
		wcscpy_s(value.UnicodeString, 256, Name.GetBuffer());

		LFTransactionUpdate(tl, GetSafeHwnd(), &value);

		if (tl->m_Changes)
			m_wndView->OnUpdateSearchResult(CookedFiles, GetFocusItem());

		if (tl->m_LastError>LFCancel)
			ShowCaptionBar(IDB_CANCEL, tl->m_LastError);

		LFFreeTransactionList(tl);
	}

	return result;
}

LFTransactionList* CMainFrame::BuildTransactionList(BOOL All)
{
	LFTransactionList* tl = NULL;

	if ((RawFiles) && (CookedFiles))
	{
		tl = LFAllocTransactionList();

		if (All)
		{
			for (unsigned int a=0; a<CookedFiles->m_ItemCount; a++)
				LFAddItemDescriptor(tl, CookedFiles->m_Items[a], a);
		}
		else
		{
			int idx = GetNextSelectedItem(-1);
			while (idx!=-1)
			{
				LFAddItemDescriptor(tl, CookedFiles->m_Items[idx], idx);
				idx = GetNextSelectedItem(idx);
			}
		}
	}

	return tl;
}

BOOL CMainFrame::UpdateSelectedItems(LFVariantData* value1, LFVariantData* value2, LFVariantData* value3)
{
	LFTransactionList* tl = BuildTransactionList();
	LFTransactionUpdate(tl, GetSafeHwnd(), value1, value2, value3);

	if (m_wndView)
	{
		BOOL deselected = FALSE;

		for (UINT a=0; a<tl->m_Count; a++)
			if (tl->m_Entries[a].LastError!=LFOk)
			{
				m_wndView->SelectItem(tl->m_Entries[a].UserData, FALSE, TRUE);
				deselected = TRUE;
			}

		if (tl->m_Changes)
			m_wndView->OnUpdateSearchResult(CookedFiles, GetFocusItem());
		if (deselected)
			OnUpdateSelection();
	}

	if (tl->m_LastError>LFCancel)
		ShowCaptionBar(IDB_CANCEL, tl->m_LastError);

	BOOL changes = tl->m_Changes;
	LFFreeTransactionList(tl);
	return changes;
}

BOOL CMainFrame::UpdateTrashFlag(BOOL Trash, BOOL All)
{
	LFVariantData value;
	value.Attr = LFAttrFlags;
	LFGetNullVariantData(&value);

	value.Flags.Flags = Trash ? LFFlagTrash : 0;
	value.Flags.Mask = LFFlagTrash;

	LFTransactionList* tl = BuildTransactionList(All);
	LFTransactionUpdate(tl, GetSafeHwnd(), &value);

	if (m_wndView)
	{
		for (UINT a=0; a<tl->m_Count; a++)
			if (tl->m_Entries[a].LastError!=LFOk)
			{
				m_wndView->SelectItem(tl->m_Entries[a].UserData, FALSE, TRUE);
			}
			else
			{
				tl->m_Entries[a].Item->DeleteFlag = true;
			}

		LFRemoveFlaggedItemDescriptors(RawFiles);
		CookFiles(ActiveContextID, GetFocusItem());
	}

	if (tl->m_LastError>LFCancel)
		ShowCaptionBar(IDB_CANCEL, tl->m_LastError);

	BOOL changes = tl->m_Changes;
	LFFreeTransactionList(tl);
	return changes;
}

void CMainFrame::OnStartNavigation()
{
	int idx = GetSelectedItem();

	if (idx!=-1)
	{
		LFItemDescriptor* i = CookedFiles->m_Items[idx];

		if (i->NextFilter)
		{
			// Es ist ein weiterer Filter angehängt, also dorthin navigieren
			NavigateTo(LFAllocFilter(i->NextFilter));
		}
		else
		{
			if (!(i->Type & LFTypeNotMounted))
			{
				char Path[MAX_PATH];
				unsigned int res;

				switch (i->Type & LFTypeMask)
				{
				case LFTypeDrive:
					ExecuteCreateStoreDlg(IDD_STORENEWDRIVE, i->CoreAttributes.FileID[0]);
					break;
				case LFTypeFile:
					res = LFGetFileLocation(i, Path, MAX_PATH);
					if (res==LFOk)
					{
						ShellExecuteA(NULL, "open", Path, NULL, NULL, SW_SHOW);
					}
					else
					{
						LFErrorBox(res);
					}

					break;
				default:
					ASSERT(FALSE);
					break;
				}
			}
		}
	}
}

void CMainFrame::OnNavigateFirst()
{
	UINT steps = 0;
	BreadcrumbItem* i = m_BreadcrumbBack;

	while (i)
	{
		i = i->next;
		steps++;
	}

	OnNavigateBack((WPARAM)steps, 0);
}

void CMainFrame::OnNavigateBackOne()
{
	OnNavigateBack(1, 0);
}

LRESULT CMainFrame::OnNavigateBack(WPARAM wParam, LPARAM /*lParam*/)
{
	if (ActiveFilter)
	{
		LFFilter* f = ActiveFilter;
		int focus = GetFocusItem();
		ActiveFilter = NULL;

		UINT steps = (UINT)wParam;
		while (steps)
		{
			AddBreadcrumbItem(&m_BreadcrumbForward, f, focus);
			ConsumeBreadcrumbItem(&m_BreadcrumbBack, &f, &focus);
			steps--;
		}
	
		NavigateTo(f, NAVMODE_HISTORY, focus);
	}

	return 0;
}

LRESULT CMainFrame::OnNavigateForward(WPARAM wParam, LPARAM /*lParam*/)
{
	if (ActiveFilter)
	{
		LFFilter* f = ActiveFilter;
		int focus = GetFocusItem();
		ActiveFilter = NULL;

		UINT steps = (UINT)wParam;
		while (steps)
		{
			AddBreadcrumbItem(&m_BreadcrumbBack, f, focus);
			ConsumeBreadcrumbItem(&m_BreadcrumbForward, &f, &focus);
			steps--;
		}

		NavigateTo(f, NAVMODE_HISTORY, focus);
	}

	return 0;
}

void CMainFrame::OnNavigateForwardOne()
{
	OnNavigateForward(1, 0);
}

void CMainFrame::OnNavigateLast()
{
	UINT steps = 0;
	BreadcrumbItem* i = m_BreadcrumbForward;

	while (i)
	{
		i = i->next;
		steps++;
	}

	OnNavigateForward((WPARAM)steps, 0);
}

void CMainFrame::OnNavigateReload()
{
	if (ActiveFilter)
		NavigateTo(LFAllocFilter(ActiveFilter), NAVMODE_RELOAD, GetFocusItem());
}

void CMainFrame::OnNavigateReloadShowAll()
{
	if (ActiveFilter)
	{
		LFFilter* f = LFAllocFilter(ActiveFilter);
		f->UnhideAll = true;

		NavigateTo(f, NAVMODE_RELOAD, GetFocusItem());
	}
}

void CMainFrame::OnClearHistory()
{
	DeleteBreadcrumbs(&m_BreadcrumbBack);
	DeleteBreadcrumbs(&m_BreadcrumbForward);
	UpdateHistory();
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
	int i;
	switch (pCmdUI->m_nID)
	{
	case ID_NAV_FIRST:
	case ID_NAV_BACKONE:
	case ID_NAV_BACK:
		b &= (m_BreadcrumbBack!=NULL);
		break;
	case ID_NAV_FORWARD:
	case ID_NAV_FORWARDONE:
	case ID_NAV_LAST:
		b &= (m_BreadcrumbForward!=NULL);
		break;
	case ID_NAV_STORES:
		if (RawFiles)
			if (RawFiles->m_Context==LFContextStores)
				b = FALSE;
		break;
	case ID_NAV_HOME:
		if (RawFiles)
			if (RawFiles->m_Context==LFContextStoreHome)
				b = FALSE;
		if (!LFDefaultStoreAvailable())
			b = FALSE;
		break;
	case ID_NAV_STARTNAVIGATION:
		i = GetSelectedItem();
		if (i!=-1)
		{
			b &= (CookedFiles->m_Items[i]->NextFilter!=NULL) ||
				((CookedFiles->m_Items[i]->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeDrive) ||
				((CookedFiles->m_Items[i]->Type & LFTypeMask)==LFTypeFile);
		}
		else
		{
			b = FALSE;
		}
		break;
	case ID_NAV_CLEARHISTORY:
		b &= (m_BreadcrumbBack!=NULL) || (m_BreadcrumbForward!=NULL);
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
		pMainPanel->Add(theApp.CommandButton(ID_APP_NEWCLIPBOARD, 1, 1));
		pMainPanel->Add(new CMFCRibbonSeparator(TRUE));
		pMainPanel->Add(theApp.CommandButton(ID_APP_NEWFILEDROP, 2, 2));
		pMainPanel->Add(theApp.CommandButton(ID_APP_NEWMIGRATE, 3, 3));
		pMainPanel->Add(theApp.CommandButton(ID_APP_PROMPT, 4, 4));
		pMainPanel->Add(new CMFCRibbonSeparator(TRUE));
		pMainPanel->Add(theApp.CommandButton(ID_APP_CLOSEOTHERS, 5, 5));
		pMainPanel->Add(theApp.CommandButton(ID_APP_CLOSE, 6, 6));

		strTemp = "About";
		pMainPanel->AddToBottom(new CMFCRibbonMainPanelButton(ID_APP_ABOUT, strTemp, 7));
		strTemp = "Exit";
		pMainPanel->AddToBottom(new CMFCRibbonMainPanelButton(ID_APP_EXIT, strTemp, 8));

	strTemp = "Home";
	CMFCRibbonCategory* pCategoryHome = m_wndRibbonBar.AddCategory(strTemp, IDB_RIBBONHOME_16, IDB_RIBBONHOME_32);

		if (!IsClipboard)
		{
			strTemp = "Navigate";
			CMFCRibbonPanel* pPanelNavigate = pCategoryHome->AddPanel(strTemp, m_PanelImages.ExtractIcon(5));
			pPanelNavigate->EnableLaunchButton(ID_NAV_SHOWHISTORY, 6);

				pPanelNavigate->Add(theApp.CommandButton(ID_NAV_FIRST, 0, 0));
				pPanelNavigate->Add(theApp.CommandButton(ID_NAV_BACKONE, 1, 1));
				pPanelNavigate->Add(theApp.CommandButton(ID_NAV_FORWARDONE, 2, 2));
				pPanelNavigate->Add(theApp.CommandButton(ID_NAV_LAST, 3, 3));
				pPanelNavigate->Add(theApp.CommandButton(ID_NAV_RELOAD, 4, 4));
				pPanelNavigate->AddSeparator();
				pPanelNavigate->Add(theApp.CommandButton(ID_NAV_STARTNAVIGATION, 5, 5));

			strTemp = "Places";
			CMFCRibbonPanel* pPanelPlaces = pCategoryHome->AddPanel(strTemp, m_PanelImages.ExtractIcon(16));

				pPanelPlaces->Add(theApp.CommandButton(ID_NAV_STORES, 7, 7));
				pPanelPlaces->Add(theApp.CommandButton(ID_NAV_HOME, 8, 8));
		}

		strTemp = "liquidFOLDERS";
		CMFCRibbonPanel* pPanelliquidFOLDERS = pCategoryHome->AddPanel(strTemp, m_PanelImages.ExtractIcon(0));
		pPanelliquidFOLDERS->EnableLaunchButton(ID_APP_ABOUT, 13);

			pPanelliquidFOLDERS->Add(theApp.CommandButton(ID_APP_HELP, 9, 9));
			pPanelliquidFOLDERS->Add(theApp.CommandButton(ID_APP_SUPPORT, 10, 10));

			if (!LFIsLicensed())
			{
				pPanelliquidFOLDERS->AddSeparator();
				pPanelliquidFOLDERS->Add(theApp.CommandButton(ID_APP_PURCHASE, 11, 11));
				pPanelliquidFOLDERS->Add(theApp.CommandButton(ID_APP_ENTERLICENSEKEY, 12, 12));
			}

	strTemp = "View";
	CMFCRibbonCategory* pCategoryView = m_wndRibbonBar.AddCategory(strTemp, IDB_RIBBONVIEW_16, IDB_RIBBONVIEW_32);

		strTemp = "Context";
		CMFCRibbonPanel* pPanelContext = pCategoryView->AddPanel(strTemp, m_PanelImages.ExtractIcon(5));

			strTemp = "Active context:";
			pPanelContext->Add(new CMFCRibbonLabel(strTemp));
			m_cbxActiveContext = new CMFCRibbonComboBox(ID_CONTEXT_CHOOSE, FALSE, 115);

				for (int a=0; a<LFContextCount; a++)
					m_cbxActiveContext->AddItem(theApp.m_Contexts[a]->Name);

			pPanelContext->Add(m_cbxActiveContext);
			pPanelContext->Add(theApp.CommandCheckBox(ID_CONTEXT_ALWAYSSAVE));
			pPanelContext->Add(theApp.CommandButton(ID_CONTEXT_RESTORE, 14));
			pPanelContext->Add(theApp.CommandButton(ID_CONTEXT_SAVENOW, 15));
			pPanelContext->Add(theApp.CommandButton(ID_CONTEXT_SAVEALL, 16));

		strTemp = "Arrange items by";
		CMFCRibbonPanel* pPanelArrange = pCategoryView->AddPanel(strTemp, m_PanelImages.ExtractIcon(6));
		pPanelArrange->EnableLaunchButton(ID_APP_SORTOPTIONS, 18);

			strTemp = "Name";
			CMFCRibbonButton* pBtnSortName = new CMFCRibbonButton(ID_DROP_NAME, strTemp, 19, 19);
			pBtnSortName->SetDefaultCommand(FALSE);

				strTemp = "By name";
				pBtnSortName->AddSubItem(new CMFCRibbonLabel(strTemp));
				pBtnSortName->AddSubItem(new CMFCRibbonButton(ID_SORT_FILENAME, theApp.m_Attributes[LFAttrFileName]->Name, 20, 20));
				pBtnSortName->AddSubItem(new CMFCRibbonButton(ID_SORT_TITLE, theApp.m_Attributes[LFAttrTitle]->Name, 21, 21));

			pPanelArrange->Add(pBtnSortName);

			strTemp = "Time";
			CMFCRibbonButton* pBtnSortDate = new CMFCRibbonButton(ID_DROP_TIME, strTemp, 22, 22);
			pBtnSortDate->SetDefaultCommand(FALSE);

				strTemp = "By timestamp";
				pBtnSortDate->AddSubItem(new CMFCRibbonLabel(strTemp));
				pBtnSortDate->AddSubItem(new CMFCRibbonButton(ID_SORT_CREATIONTIME, theApp.m_Attributes[LFAttrCreationTime]->Name, 23, 23));
				pBtnSortDate->AddSubItem(new CMFCRibbonButton(ID_SORT_FILETIME, theApp.m_Attributes[LFAttrFileTime]->Name, 24, 24));
				pBtnSortDate->AddSubItem(new CMFCRibbonButton(ID_SORT_RECORDINGTIME, theApp.m_Attributes[LFAttrRecordingTime]->Name, 25, 25));
				strTemp = "By workflow";
				pBtnSortDate->AddSubItem(new CMFCRibbonLabel(strTemp));
				pBtnSortDate->AddSubItem(new CMFCRibbonButton(ID_SORT_DUETIME, theApp.m_Attributes[LFAttrDueTime]->Name, 26, 26));
				pBtnSortDate->AddSubItem(new CMFCRibbonButton(ID_SORT_DONETIME, theApp.m_Attributes[LFAttrDoneTime]->Name, 27, 27));

			pPanelArrange->Add(pBtnSortDate);

			strTemp = "Location";
			CMFCRibbonButton* pBtnSortLocation = new CMFCRibbonButton(ID_DROP_LOCATION, strTemp, 10, 10);
			pBtnSortLocation->SetDefaultCommand(FALSE);

				strTemp = "By location";
				pBtnSortLocation->AddSubItem(new CMFCRibbonLabel(strTemp));
				pBtnSortLocation->AddSubItem(new CMFCRibbonButton(ID_SORT_LOCATIONNAME, theApp.m_Attributes[LFAttrLocationName]->Name, 28, 28));
				pBtnSortLocation->AddSubItem(new CMFCRibbonButton(ID_SORT_LOCATIONIATA, theApp.m_Attributes[LFAttrLocationIATA]->Name, 29, 29));
				pBtnSortLocation->AddSubItem(new CMFCRibbonButton(ID_SORT_LOCATIONGPS, theApp.m_Attributes[LFAttrLocationGPS]->Name, 30, 30));

			pPanelArrange->Add(pBtnSortLocation);

			pPanelArrange->Add(new CMFCRibbonButton(ID_SORT_RATING, theApp.m_Attributes[LFAttrRating]->Name, 31, 31));
			pPanelArrange->Add(new CMFCRibbonButton(ID_SORT_ROLL, theApp.m_Attributes[LFAttrRoll]->Name, 32, 32));

			pPanelArrange->AddSeparator();

			pPanelArrange->Add(new CMFCRibbonButton(ID_SORT_ARTIST, theApp.m_Attributes[LFAttrArtist]->Name, 33, 33));
			pPanelArrange->Add(new CMFCRibbonButton(ID_SORT_COMMENT, theApp.m_Attributes[LFAttrComment]->Name, 34, 34));
			pPanelArrange->Add(new CMFCRibbonButton(ID_SORT_DURATION, theApp.m_Attributes[LFAttrDuration]->Name, 35, 35));
			pPanelArrange->Add(new CMFCRibbonButton(ID_SORT_LANGUAGE, theApp.m_Attributes[LFAttrLanguage]->Name, 36, 36));

			CMFCRibbonButton* pBtnSortResolution = new CMFCRibbonButton(ID_DROP_RESOLUTION, theApp.m_Attributes[LFAttrResolution]->Name, 37, 37);
			pBtnSortResolution->SetDefaultCommand(FALSE);

				strTemp = "By overall dimension";
				pBtnSortResolution->AddSubItem(new CMFCRibbonLabel(strTemp));
				pBtnSortResolution->AddSubItem(new CMFCRibbonButton(ID_SORT_ASPECTRATIO, theApp.m_Attributes[LFAttrAspectRatio]->Name, 40, 40));
				pBtnSortResolution->AddSubItem(new CMFCRibbonButton(ID_SORT_RESOLUTION, theApp.m_Attributes[LFAttrResolution]->Name, 37, 37));
				strTemp = "By edge";
				pBtnSortResolution->AddSubItem(new CMFCRibbonLabel(strTemp));
				pBtnSortResolution->AddSubItem(new CMFCRibbonButton(ID_SORT_HEIGHT, theApp.m_Attributes[LFAttrHeight]->Name, 38, 38));
				pBtnSortResolution->AddSubItem(new CMFCRibbonButton(ID_SORT_WIDTH, theApp.m_Attributes[LFAttrWidth]->Name, 39, 39));

			pPanelArrange->Add(pBtnSortResolution);

		strTemp = "Display search result as";
		CMFCRibbonPanel* pPanelDisplay = pCategoryView->AddPanel(strTemp, m_PanelImages.ExtractIcon(4));
		pPanelDisplay->EnableLaunchButton(ID_APP_VIEWOPTIONS, 17);

			pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_AUTOMATIC, 0, 0));
			pPanelDisplay->AddSeparator();
			pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_LARGEICONS, 1, 1));
			pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_SMALLICONS, 2, 2));
			pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_LIST, 3, 3));
			pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_DETAILS, 4, 4));
			pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_TILES, 5, 5));
			pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_PREVIEW, 6, 6));

			if (!IsClipboard)
			{
				pPanelDisplay->AddSeparator();

				strTemp = "Calendar";
				CMFCRibbonButton* pBtnViewCalendar = new CMFCRibbonButton(ID_DROP_CALENDAR, strTemp, 13, 13);
				pBtnViewCalendar->SetDefaultCommand(FALSE);

					strTemp = "Calendar views";
					pBtnViewCalendar->AddSubItem(new CMFCRibbonLabel(strTemp));
					pBtnViewCalendar->AddSubItem(theApp.CommandButton(ID_APP_VIEW_CALENDAR_YEAR, 7, 7));
					pBtnViewCalendar->AddSubItem(theApp.CommandButton(ID_APP_VIEW_CALENDAR_WEEK, 8, 8));
					pBtnViewCalendar->AddSubItem(theApp.CommandButton(ID_APP_VIEW_CALENDAR_DAY, 9, 9));

				pPanelDisplay->Add(pBtnViewCalendar);

				pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_GLOBE, 10, 10));
				pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_TAGCLOUD, 11, 11));
				pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_TIMELINE, 12, 12));
			}

	strTemp = "Files";
	CMFCRibbonCategory* pCategoryFiles = m_wndRibbonBar.AddCategory(strTemp, IDB_RIBBONFILES_16, IDB_RIBBONFILES_32);

		strTemp = "Clipboard";
		CMFCRibbonPanel* pPanelClipboard = pCategoryFiles->AddPanel(strTemp, m_PanelImages.ExtractIcon(9));

			pPanelClipboard->Add(theApp.CommandButton(ID_CLIP_COPY, 0, 0));
			pPanelClipboard->Add(theApp.CommandButton(ID_VIEW_SELECTALL, 1));
			pPanelClipboard->Add(theApp.CommandButton(ID_VIEW_SELECTNONE, 2));
			if (IsClipboard)
			{
				pPanelClipboard->Add(theApp.CommandButton(ID_CLIP_REMOVE, 3));
			}
			else
			{
				pPanelClipboard->Add(theApp.CommandButton(ID_CLIP_PASTE, 4));
				pPanelClipboard->AddSeparator();

				strTemp = "Remember";
				CMFCRibbonButton* pBtnRemember = new CMFCRibbonButton(ID_CLIP_REMEMBERLAST, strTemp, 5, 5);

					strTemp = "Add files to";
					pBtnRemember->AddSubItem(new CMFCRibbonLabel(strTemp));
					pBtnRemember->AddSubItem(theApp.CommandButton(ID_CLIP_REMEMBERLAST, 5, 5));
					pBtnRemember->AddSubItem(theApp.CommandButton(ID_CLIP_REMEMBERNEW, 6, 6));

				pPanelClipboard->Add(pBtnRemember);
			}

		strTemp = "Manage";
		CMFCRibbonPanel* pPanelFileManage = pCategoryFiles->AddPanel(strTemp, m_PanelImages.ExtractIcon(0));
		pPanelFileManage->EnableLaunchButton(ID_FILES_SHOWINSPECTOR, 10);

			strTemp = "Open";
			pPanelFileManage->Add(new CMFCRibbonButton(ID_NAV_STARTNAVIGATION, strTemp, 7, 7));
			pPanelFileManage->AddSeparator();
			strTemp = "Rename";
			pPanelFileManage->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 8, 8));
			strTemp = "Delete";
			pPanelFileManage->Add(new CMFCRibbonButton(ID_FILES_DELETE, strTemp, 9, 9));

		strTemp = "Share";
		CMFCRibbonPanel* pPanelFileShare = pCategoryFiles->AddPanel(strTemp, m_PanelImages.ExtractIcon(0));

			strTemp = "Send";
			pPanelFileShare->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 11, 11));

			strTemp = "Upload";
			CMFCRibbonButton* pBtnUpload = new CMFCRibbonButton(0, strTemp, 12, 12);
			pBtnUpload->SetDefaultCommand(FALSE);

				strTemp = "Upload to";
				pBtnUpload->AddSubItem(new CMFCRibbonLabel(strTemp));
				strTemp = "Flickr";
				pBtnUpload->AddSubItem(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 13, 13));
				strTemp = "Slideshare";
				pBtnUpload->AddSubItem(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 14, 14));
				strTemp = "YouTube";
				pBtnUpload->AddSubItem(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 15, 15));

			pPanelFileShare->Add(pBtnUpload);

			strTemp = "Syndicate";
			CMFCRibbonButton* pBtnSyndicate = new CMFCRibbonButton(0, strTemp, 16, 16);
			pBtnSyndicate->SetDefaultCommand(FALSE);

				strTemp = "Syndicate URL on";
				pBtnSyndicate ->AddSubItem(new CMFCRibbonLabel(strTemp));
				strTemp = "Delicious";
				pBtnSyndicate ->AddSubItem(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 17, 17));
				strTemp = "Facebook";
				pBtnSyndicate ->AddSubItem(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 18, 18));
				strTemp = "Google";
				pBtnSyndicate ->AddSubItem(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 19, 19));
				strTemp = "Mr. Wong";
				pBtnSyndicate ->AddSubItem(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 20, 20));
				strTemp = "Twitter";
				pBtnSyndicate ->AddSubItem(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 21, 21));

			pPanelFileShare->Add(pBtnSyndicate);

	strTemp = "Mail";
	strCtx = "EMail";
	CMFCRibbonCategory* pCategoryEmail = m_wndRibbonBar.AddContextCategory(strTemp, strCtx, 3, AFX_CategoryColor_Green, IDB_RIBBONEMAIL_16, IDB_RIBBONEMAIL_32);
	
		strTemp = "Mail";
		CMFCRibbonPanel* pPanelEmail = pCategoryEmail->AddPanel(strTemp, m_PanelImages.ExtractIcon(0));

			strTemp = "Reply";
			pPanelEmail->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 0, 0));
			strTemp = "Reply all";
			pPanelEmail->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 1, 1));
			strTemp = "Forward";
			pPanelEmail->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 2, 2));
			strTemp = "Forward All";
			pPanelEmail->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 3, 3));

	strTemp = "Contacts";
	CMFCRibbonCategory* pCategoryEmailContacts = m_wndRibbonBar.AddContextCategory(strTemp, strCtx, 3, AFX_CategoryColor_Green, IDB_RIBBONCONTACTS_16, IDB_RIBBONCONTACTS_32);
	
		strTemp = "Contacts";
		CMFCRibbonPanel* pPanelContacts = pCategoryEmailContacts->AddPanel(strTemp, m_PanelImages.ExtractIcon(0));

			strTemp = "Show Contacts";
			pPanelContacts->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 0, 0));
			strTemp = "Add Contact";
			pPanelContacts->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 1, 1));
			strTemp = "Delete Contact";
			pPanelContacts->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 2, 2));
			strTemp = "Edit Contact";
			pPanelContacts->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 3, 3));
			strTemp = "Search Contact";
			pPanelContacts->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 4, 4));
			strTemp = "Send contact";
			pPanelContacts->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 5, 5));

	if (!IsClipboard)
	{
		strTemp = "Register";
		strCtx = "Unknown file formats";
		CMFCRibbonCategory* pCategoryUnknownFileFormats = m_wndRibbonBar.AddContextCategory(strTemp, strCtx, 5, AFX_CategoryColor_Green, IDB_RIBBONUNKNOWNFILEFORMATS_16, IDB_RIBBONUNKNOWNFILEFORMATS_32);

			CMFCRibbonPanel* pPanelRegister = pCategoryUnknownFileFormats->AddPanel(strTemp, m_PanelImages.ExtractIcon(19));
			pPanelRegister->EnableLaunchButton(ID_APP_ABOUT, 2);

				strTemp = "Register formats";
				pPanelRegister->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 0, 0));
				pPanelRegister->AddSeparator();
				strTemp = "Send registred formats to customer support";
				pPanelRegister->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 1, 1));

		strCtx = "View";
		strTemp = "Calendar";
		CMFCRibbonCategory* pCategoryCalendar = m_wndRibbonBar.AddContextCategory(strTemp, strCtx, 2, AFX_CategoryColor_Indigo, IDB_RIBBONCALENDAR_16, IDB_RIBBONCALENDAR_32);

			strTemp = "Navigate";
			CMFCRibbonPanel* pPanelCalendarNavigate = pCategoryCalendar->AddPanel(strTemp, m_PanelImages.ExtractIcon(0));

				strTemp = "Back";
				pPanelCalendarNavigate->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 5, 5));
				strTemp = "Forward";
				pPanelCalendarNavigate->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 6, 6));

			strTemp = "Commands";
			CMFCRibbonPanel* pPanelCalendarCommands = pCategoryCalendar->AddPanel(strTemp, m_PanelImages.ExtractIcon(0));

				strTemp = "Add Event";
				pPanelCalendarCommands->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 0, 0));
				strTemp = "Delete Event";
				pPanelCalendarCommands->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 1, 1));
				strTemp = "Edit Event";
				pPanelCalendarCommands->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 2, 2));

			strTemp = "Search";
			CMFCRibbonPanel* pPanelCalendarSearch = pCategoryCalendar->AddPanel(strTemp, m_PanelImages.ExtractIcon(0));

				strTemp = "Go To Date";
				pPanelCalendarSearch->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 3, 3));
				strTemp = "Search";
				pPanelCalendarSearch->Add(new CMFCRibbonButton(ID_APP_ABOUT, strTemp, 4, 4));

		strTemp = "Globe";
		CMFCRibbonCategory* pCategoryGlobe = m_wndRibbonBar.AddContextCategory(strTemp, strCtx, 2, AFX_CategoryColor_Indigo, IDB_RIBBONGLOBE_16, IDB_RIBBONGLOBE_32);

			strTemp = "View";
			CMFCRibbonPanel* pPanelGlobeView = pCategoryGlobe->AddPanel(strTemp, m_PanelImages.ExtractIcon(13));
			pPanelGlobeView->EnableLaunchButton(ID_GLOBE_JUMPTOLOCATION, 4);

				pPanelGlobeView->Add(theApp.CommandButton(ID_GLOBE_ZOOMIN, 0, 0));
				pPanelGlobeView->Add(theApp.CommandButton(ID_GLOBE_ZOOMOUT, 1, 1));
				pPanelGlobeView->Add(theApp.CommandButton(ID_GLOBE_SCALETOFIT, 2, 2));
				pPanelGlobeView->AddSeparator();
				pPanelGlobeView->Add(theApp.CommandButton(ID_GLOBE_SAVECAMERA, 3, 3));

			strTemp = "Places";
			CMFCRibbonPanel* pPanelGlobePlaces = pCategoryGlobe->AddPanel(strTemp, m_PanelImages.ExtractIcon(17));

				pPanelGlobePlaces->Add(theApp.CommandButton(ID_GLOBE_GOOGLEEARTH, 5, 5));

			strTemp = "Display options";
			CMFCRibbonPanel* pPanelGlobeOptions = pCategoryGlobe->AddPanel(strTemp, m_PanelImages.ExtractIcon(4));

				strTemp = "Texture size:";
				pPanelGlobeOptions->Add(new CMFCRibbonLabel(strTemp));
				pPanelGlobeOptions->Add(new CTextureComboBox(ID_GLOBE_TEXTURESIZE, 80));
				pPanelGlobeOptions->Add(theApp.CommandCheckBox(ID_GLOBE_HQMODEL));
				pPanelGlobeOptions->AddSeparator();
				pPanelGlobeOptions->Add(theApp.CommandButton(ID_GLOBE_SHOWBUBBLES, 6, 6));
				pPanelGlobeOptions->Add(theApp.CommandCheckBox(ID_GLOBE_SHOWAIRPORTNAMES));
				pPanelGlobeOptions->Add(theApp.CommandCheckBox(ID_GLOBE_SHOWGPS));
				pPanelGlobeOptions->Add(theApp.CommandCheckBox(ID_GLOBE_SHOWHINTS));

		strTemp = "Tagcloud";
		CMFCRibbonCategory* pCategoryTagcloud = m_wndRibbonBar.AddContextCategory(strTemp, strCtx, 2, AFX_CategoryColor_Indigo, IDB_RIBBONTAGCLOUD_16, IDB_RIBBONTAGCLOUD_32);

			strTemp = "Tags";
			CMFCRibbonPanel* pPanelTagcloudTags = pCategoryTagcloud->AddPanel(strTemp, m_PanelImages.ExtractIcon(18));

				pPanelTagcloudTags->Add(theApp.CommandButton(ID_TAGCLOUD_SORTNAME, 0, 0));
				pPanelTagcloudTags->Add(theApp.CommandButton(ID_TAGCLOUD_SORTCOUNT, 1, 1));
				pPanelTagcloudTags->AddSeparator();
				pPanelTagcloudTags->Add(theApp.CommandCheckBox(ID_TAGCLOUD_OMITRARE));

			strTemp = "Display options";
			CMFCRibbonPanel* pPanelTagcloudOptions = pCategoryTagcloud->AddPanel(strTemp, m_PanelImages.ExtractIcon(4));

				pPanelTagcloudOptions->Add(theApp.CommandCheckBox(ID_TAGCLOUD_USESIZE));
				pPanelTagcloudOptions->Add(theApp.CommandCheckBox(ID_TAGCLOUD_USECOLORS));
				pPanelTagcloudOptions->Add(theApp.CommandCheckBox(ID_TAGCLOUD_USEOPACITY));

		strTemp = "Timeline";
		CMFCRibbonCategory* pCategoryTimeline = m_wndRibbonBar.AddContextCategory(strTemp, strCtx, 2, AFX_CategoryColor_Indigo, IDB_RIBBONVIEW_16, IDB_RIBBONVIEW_32);

		strCtx = "Stores";
		strTemp = "Manage";
		CMFCRibbonCategory* pCategoryStores = m_wndRibbonBar.AddContextCategory(strTemp, strCtx, 7, AFX_CategoryColor_Yellow, IDB_RIBBONSTORES_16, IDB_RIBBONSTORES_32);

			strTemp = "Stores";
			CMFCRibbonPanel* pPanelStoresStores = pCategoryStores->AddPanel(strTemp, m_PanelImages.ExtractIcon(11));

				strTemp = "Create";
				CMFCRibbonButton* pBtnStoreNew = theApp.CommandButton(ID_STORE_NEW, 0, 0, TRUE);

					pBtnStoreNew->AddSubItem(new CMFCRibbonLabel(theApp.GetCommandName(ID_STORE_NEW)));
					pBtnStoreNew->AddSubItem(theApp.CommandButton(ID_STORE_NEWINTERNAL, 1, 1, TRUE));
					pBtnStoreNew->AddSubItem(theApp.CommandButton(ID_STORE_NEWDRIVE, 2, 2, TRUE));

				pPanelStoresStores->Add(pBtnStoreNew);
				pPanelStoresStores->Add(theApp.CommandButton(ID_STORE_DELETE, 3, 3));
				pPanelStoresStores->Add(theApp.CommandButton(ID_STORE_RENAME, 4, 4));
				pPanelStoresStores->AddSeparator();
				pPanelStoresStores->Add(theApp.CommandButton(ID_STORE_MAKEDEFAULT, 5, 5));
				pPanelStoresStores->Add(theApp.CommandButton(ID_STORE_MAKEHYBRID, 6, 6));

			strTemp = "Housekeeping";
			CMFCRibbonPanel* pPanelStoresInformation = pCategoryStores->AddPanel(strTemp, m_PanelImages.ExtractIcon(12));

				pPanelStoresInformation->Add(theApp.CommandButton(ID_STORE_PROPERTIES, 7, 7));
				pPanelStoresInformation->AddSeparator();
				pPanelStoresInformation->Add(theApp.CommandButton(ID_STORE_MAINTENANCE, 8, 8));
				pPanelStoresInformation->Add(theApp.CommandButton(ID_STORE_BACKUP, 9, 9));

		strTemp = "Deleted files";
		strCtx = "Trash";
		CMFCRibbonCategory* pCategoryTrash = m_wndRibbonBar.AddContextCategory(strTemp, strCtx, 4, AFX_CategoryColor_Red, IDB_RIBBONTRASH_16, IDB_RIBBONTRASH_32);

			CMFCRibbonPanel* pPanelDeletedFiles = pCategoryTrash->AddPanel(strTemp, m_PanelImages.ExtractIcon(14));

				pPanelDeletedFiles->Add(theApp.CommandButton(ID_TRASH_EMPTY, 0, 0));
				pPanelDeletedFiles->AddSeparator();
				pPanelDeletedFiles->Add(theApp.CommandButton(ID_TRASH_RESTORE, 1, 1));
				pPanelDeletedFiles->Add(theApp.CommandButton(ID_TRASH_RESTOREALL, 2, 2));
	}

	m_wndRibbonBar.SetActiveCategory(m_wndRibbonBar.GetCategory(RibbonDefaultCategory));

	/*
	AFX_CategoryColor_Red
	AFX_CategoryColor_Orange
	AFX_CategoryColor_Yellow
	AFX_CategoryColor_Green
	AFX_CategoryColor_Blue
	AFX_CategoryColor_Indigo
	AFX_CategoryColor_Violet
	*/

	// Im Debug-Modus alle Kategorien anzeigen
	#ifdef _DEBUG
	m_wndRibbonBar.ShowCategory(RibbonCategory_EMail_Mail);
	m_wndRibbonBar.ShowCategory(RibbonCategory_EMail_Contacts);
	m_wndRibbonBar.ShowCategory(RibbonCategory_View_Calendar);
	m_wndRibbonBar.ShowCategory(RibbonCategory_View_Globe);
	m_wndRibbonBar.ShowCategory(RibbonCategory_View_Tagcloud);
	m_wndRibbonBar.ShowCategory(RibbonCategory_View_Timeline);
	m_wndRibbonBar.ShowCategory(RibbonCategory_Stores);
	m_wndRibbonBar.ShowCategory(RibbonCategory_Trash);
	m_wndRibbonBar.ShowCategory(RibbonCategory_UnknownFileFormats);
	#endif

	// Symbolleistenbefehle für Schnellzugriff hinzufügen
	CList<UINT, UINT> lstQATCmds;
	lstQATCmds.AddTail(ID_NAV_BACKONE);
	lstQATCmds.AddTail(ID_NAV_FORWARDONE);
	lstQATCmds.AddTail(ID_NAV_STORES);
	lstQATCmds.AddTail(ID_NAV_HOME);
	m_wndRibbonBar.SetQuickAccessCommands(lstQATCmds);

	// Hilfe hinzufügen
	m_wndRibbonBar.AddToTabs(new CMFCRibbonButton(ID_APP_HELP, NULL, m_PanelImages.ExtractIcon(0)));
}

void CMainFrame::ShowCaptionBar(int Icon, LPCWSTR Message, int Command, LPCWSTR Button)
{
	// Meldung nur anzeigen wenn Titelleiste unsichtbar ist oder die Meldung eine höhere Priorität hat
	if ((m_wndCaptionBar.IsVisible()) && (Icon<CaptionBarUsed))
		return;

	// Button
	if (Command)
	{
		m_wndCaptionBar.SetButton(Button, Command, CMFCCaptionBar::ALIGN_LEFT, FALSE);
		CString strTemp;
		ENSURE(strTemp.LoadString(Command));
		int y = strTemp.Find('\n');
		if (!y)
			y = strTemp.GetLength();
		m_wndCaptionBar.SetButtonToolTip(strTemp.Left(y));
	}
	else
	{
		m_wndCaptionBar.RemoveButton();
	}

	// Text und Icon
	m_wndCaptionBar.SetText(Message, CMFCCaptionBar::ALIGN_LEFT);
	m_wndCaptionBar.SetBitmap(Icon, RGB(255,255,255), FALSE, CMFCCaptionBar::ALIGN_LEFT);

	// Balken sichtbar machen wenn die Meldung keine Debug-Information ist oder selbige gewünscht wird
	if ((Icon!=IDB_INFO) || (theApp.m_ShowQueryDuration))
	{
		m_wndCaptionBar.ShowWindow(SW_SHOW);
		RecalcLayout(FALSE);
	}
	CaptionBarUsed = Icon;
}

void CMainFrame::ShowCaptionBar(int Icon, UINT res, int Command, LPCWSTR Button)
{
	wchar_t* message = LFGetErrorText(res);
	ShowCaptionBar(Icon, message, Command, Button);
	free(message);
}

UINT CMainFrame::SelectViewMode(UINT ViewID)
{
	if ((ViewID<LFViewAutomatic) || (ViewID>=LFViewCount))
		ViewID = LFViewAutomatic;
	if (ViewID==LFViewAutomatic)
		ViewID = CookedFiles->m_RecommendedView;

	if (!LFAttributeSortableInView(ActiveViewParameters->SortBy, ViewID))
		for (UINT a=1; a<=LFViewTimeline; a++)
			if (LFAttributeSortableInView(ActiveViewParameters->SortBy, a))
				return a;

	return ViewID;
}

BOOL CMainFrame::OpenChildView(BOOL Force)
{
	UINT ViewID = SelectViewMode(ActiveViewParameters->Mode);
	if (ActiveViewParameters->Mode!=LFViewAutomatic)
		ActiveViewParameters->Mode = ViewID;

	ASSERT(LFAttributeSortableInView(ActiveViewParameters->SortBy, ViewID));
	CFileView* pNewView = NULL;

	switch (ViewID)
	{
	case LFViewLargeIcons:
	case LFViewSmallIcons:
	case LFViewList:
	case LFViewDetails:
	case LFViewTiles:
	case LFViewPreview:
		Force |= (ActiveViewID<LFViewLargeIcons) || (ActiveViewID>LFViewPreview);
		if ((m_wndView) && (CookedFiles))
			Force |= (m_wndView->HasCategories()!=(CookedFiles->m_HasCategories==true));
		if (Force)
		{
			pNewView = new CListView();
			((CListView*)pNewView)->Create(this, CookedFiles, ViewID);
		}
		break;
	case LFViewCalendarYear:
		if ((Force) || (ActiveViewID!=LFViewCalendarYear))
		{
			pNewView = new CCalendarYearView();
			((CCalendarYearView*)pNewView)->Create(this, CookedFiles);
		}
		break;
	case LFViewCalendarWeek:
		if ((Force) || (ActiveViewID!=LFViewCalendarWeek))
		{
			pNewView = new CCalendarWeekView();
			((CCalendarWeekView*)pNewView)->Create(this, CookedFiles);
		}
		break;
	case LFViewCalendarDay:
		if ((Force) || (ActiveViewID!=LFViewCalendarDay))
		{
			pNewView = new CCalendarDayView();
			((CCalendarDayView*)pNewView)->Create(this, CookedFiles);
		}
		break;
	case LFViewGlobe:
		if ((Force) || (ActiveViewID!=LFViewGlobe))
		{
			pNewView = new CGlobeView();
			((CGlobeView*)pNewView)->Create(this, CookedFiles);
		}
		break;
	case LFViewTagcloud:
		if ((Force) || (ActiveViewID!=LFViewTagcloud))
		{
			pNewView = new CTagcloudView();
			((CTagcloudView*)pNewView)->Create(this, CookedFiles);
		}
		break;
	case LFViewTimeline:
		if ((Force) || (ActiveViewID!=LFViewTimeline))
		{
			pNewView = new CTimelineView();
			((CTimelineView*)pNewView)->Create(this, CookedFiles);
		}
		break;
	}

	// Altes View entfernen, neues View setzen
	if (pNewView)
	{
		CWnd* pVictim = m_wndView;
		CRect oldRect;
		if (pVictim)
		{
			WINDOWPLACEMENT wp;
			pVictim->GetWindowPlacement(&wp);
			oldRect = wp.rcNormalPosition;

			pVictim->ShowWindow(SW_HIDE);
			SetWindowLong(pVictim->m_hWnd, GWL_ID, AFX_IDW_PANE_LAST);
		}

		m_wndView = pNewView;
		if (GetFocus()!=&m_wndHistory->m_wndList)
			m_wndView->SetFocus();

		if (pVictim)
		{
			m_wndView->SetWindowPos(NULL, oldRect.left, oldRect.top, oldRect.Width(), oldRect.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

			pVictim->DestroyWindow();
			delete pVictim;
		}

		m_wndView->ShowWindow(SW_SHOW);
		OnUpdateSelection();
	}
	else
	{
		m_wndView->OnUpdateViewOptions(ActiveContextID, ViewID);
	}

	ActiveViewID = ViewID;

	// Im Debug-Modus bleiben alle Kategorien sichtbar
	#ifndef _DEBUG
	m_wndRibbonBar.ShowCategory(RibbonCategory_View_Calendar, (ViewID>=LFViewCalendarYear) && (ViewID<=LFViewCalendarDay));
	m_wndRibbonBar.ShowCategory(RibbonCategory_View_Globe, ViewID==LFViewGlobe);
	m_wndRibbonBar.ShowCategory(RibbonCategory_View_Tagcloud, ViewID==LFViewTagcloud);
	m_wndRibbonBar.ShowCategory(RibbonCategory_View_Timeline, ViewID==LFViewTimeline);
	m_wndRibbonBar.Update();
	#endif

	return (pNewView!=NULL);
}

void CMainFrame::NavigateTo(LFFilter* f, UINT NavMode, int FocusItem)
{
	if (NavMode<NAVMODE_RELOAD)
		theApp.PlayNavigateSound();

	if (ActiveFilter)
		if (NavMode==NAVMODE_NORMAL)
		{
			AddBreadcrumbItem(&m_BreadcrumbBack, ActiveFilter, GetFocusItem());
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

	int OldContext = -1;
	if (RawFiles)
	{
		OldContext = RawFiles->m_Context;
		LFFreeSearchResult(RawFiles);
	}

	ActiveFilter->HideEmptyDrives = (theApp.m_HideEmptyDrives==TRUE);
	ActiveFilter->HideEmptyDomains = (theApp.m_HideEmptyDomains==TRUE);
	RawFiles = LFQuery(ActiveFilter);
	CookFiles(((OldContext!=RawFiles->m_Context) || (ActiveContextID==-1)) ? RawFiles->m_Context : ActiveContextID, FocusItem);

	if (CookedFiles->m_LastError>LFCancel)
	{
		theApp.PlayWarningSound();
		ShowCaptionBar(ActiveFilter->Result.FilterType==LFFilterTypeError ? IDB_CANCEL : IDB_WARNING, CookedFiles->m_LastError);
	}
}

void CMainFrame::CookFiles(int recipe, int FocusItem)
{
	// Das alte Suchergebnis wird in OldResult gespeichert, damit das View niemals eine ungültige
	// Referenz hat. Erst nach UpdateSearchResult() kann das ggf. vorhandene alte Suchergebnis
	// gelöscht werden.
	LFSearchResult* Victim = CookedFiles;

	DWORD start = GetTickCount();
	CookedFiles = LFAllocSearchResult(recipe, RawFiles);
	SortSearchResult(CookedFiles, &theApp.m_Views[recipe]);
	if (!IsClipboard)
		GroupSearchResult(CookedFiles, &theApp.m_Views[recipe]);

	DWORD stop = GetTickCount();

	UpdateSearchResult(FALSE, FocusItem);
	UpdateHistory();
	OnUpdateFileCount();

	if (Victim)
		LFFreeSearchResult(Victim);

	if ((CookedFiles->m_LastError==LFOk) && (!IsClipboard))
	{
		wchar_t* error = LFGetErrorText(CookedFiles->m_LastError);
		CString message;
		message.Format(error, RawFiles->m_QueryTime, stop-start, RawFiles->m_ItemCount, CookedFiles->m_ItemCount);
		ShowCaptionBar(IDB_INFO, message);
		free(error);
	}
}

void CMainFrame::UpdateHistory()
{
	if (m_wndHistory)
		m_wndHistory->UpdateList(m_BreadcrumbBack, ActiveFilter, m_BreadcrumbForward);
	if (m_wndFilter)
		m_wndFilter->UpdateList();
}

LRESULT CMainFrame::OnDrivesChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (RawFiles)
		if (RawFiles->m_Context==LFContextStores)
			OnNavigateReload();

	return NULL;
}

LRESULT CMainFrame::OnStoresChanged(WPARAM /*wParam*/, LPARAM lParam)
{
	if (RawFiles)
		switch (RawFiles->m_Context)
		{
		case LFContextStores:
			if (GetSafeHwnd()!=(HWND)lParam)
				OnNavigateReload();
			break;
		}

	return NULL;
}

LRESULT CMainFrame::OnLookChanged(WPARAM wParam, LPARAM /*lParam*/)
{
	if (this==theApp.m_pMainWnd)
		theApp.SetApplicationLook((UINT)wParam);

	return NULL;
}
