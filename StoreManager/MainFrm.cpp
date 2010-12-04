
// MainFrm.cpp: Implementierung der Klasse CMainFrame
//

#include "stdafx.h"
#include "StoreManager.h"
#include "MainFrm.h"
#include "LFCore.h"
#include "SortOptionsDlg.h"
#include "ViewOptionsDlg.h"
#include "ChooseDetailsDlg.h"
#include "LFCommDlg.h"
#include <io.h>


// CTextureComboBox
//

class CTextureComboBox : public CMFCRibbonComboBox
{
public:
	CTextureComboBox(UINT nID, INT nWidth)
		: CMFCRibbonComboBox(nID, FALSE, nWidth)
	{
		AddItem(_T("Automatic"));
		AddItem(_T("1024×1024"));
		AddItem(_T("2048×2048"));
		AddItem(_T("4096×4096"));
		AddItem(_T("8192×4096"));
		SelectItem((INT)theApp.m_nTextureSize);
	}

	virtual void OnSelectItem(INT nItem)
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
	ON_MESSAGE_VOID(WM_UPDATESELECTION, OnUpdateSelection)

	ON_UPDATE_COMMAND_UI_RANGE(ID_APP_NEWVIEW, ID_APP_VIEW_TIMELINE, OnUpdateAppCommands)
	ON_UPDATE_COMMAND_UI(ID_VIEW_AUTODIRS, OnUpdateAppCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_SORT_FILENAME, ID_SORT_FILENAME+99, OnUpdateSortCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_NAV_FIRST, ID_NAV_CLEARHISTORY, OnUpdateNavCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_PANE_FILTERWND, ID_PANE_HISTORYWND, OnUpdatePaneCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_CLIP_COPY, ID_CLIP_REMEMBERNEW, OnUpdateClipCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_ITEMS_SHOWINSPECTOR, ID_ITEMS_RENAME, OnUpdateItemCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_TRASH_EMPTY, ID_TRASH_RESTOREALL, OnUpdateTrashCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_STORE_NEW, ID_STORE_BACKUP, OnUpdateStoreCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_DROP_NAME, ID_DROP_DIMENSION, OnUpdateDropCommands)

	ON_COMMAND(ID_APP_CLOSE, OnClose)
	ON_COMMAND(ID_APP_CLOSEOTHERS, OnCloseOthers)
	ON_COMMAND(ID_APP_SORTOPTIONS, OnSortOptions)
	ON_COMMAND(ID_APP_VIEWOPTIONS, OnViewOptions)
	ON_COMMAND(ID_VIEW_CHOOSEDETAILS, OnChooseDetails)
	ON_COMMAND(ID_VIEW_AUTODIRS, OnToggleAutoDirs)

	ON_COMMAND_RANGE(ID_APP_VIEW_LARGEICONS, ID_APP_VIEW_TIMELINE, OnChangeChildView)
	ON_COMMAND_RANGE(ID_SORT_FILENAME, ID_SORT_FILENAME+99, OnSort)

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
	ON_COMMAND(ID_NAV_CLEARHISTORY, OnClearHistory)

	ON_COMMAND(ID_CLIP_REMOVE, OnClipRemove)
	ON_COMMAND(ID_CLIP_REMEMBERLAST, OnClipRememberLast)
	ON_COMMAND(ID_CLIP_REMEMBERNEW, OnClipRememberNew)

	ON_COMMAND(ID_ITEMS_SHOWINSPECTOR, OnShowInspectorWnd)
	ON_COMMAND(ID_ITEMS_OPEN, OnItemsOpen)
	ON_COMMAND(ID_ITEMS_OPENWITH, OnItemsOpenWith)
	ON_COMMAND(ID_ITEMS_DELETE, OnItemsDelete)
	ON_COMMAND(ID_ITEMS_RENAME, OnItemsRename)

	ON_COMMAND(ID_TRASH_EMPTY, OnEmptyTrash)
	ON_COMMAND(ID_TRASH_RESTORESELECTED, OnRestoreSelectedFiles)
	ON_COMMAND(ID_TRASH_RESTOREALL, OnRestoreAllFiles)

	ON_COMMAND(ID_STORE_NEW, OnStoreNew)
	ON_COMMAND(ID_STORE_NEWINTERNAL, OnStoreNewInternal)
	ON_COMMAND(ID_STORE_NEWDRIVE, OnStoreNewDrive)
	ON_COMMAND(ID_STORE_DELETE, OnItemsDelete)
	ON_COMMAND(ID_STORE_RENAME, OnItemsRename)
	ON_COMMAND(ID_STORE_MAKEDEFAULT, OnStoreMakeDefault)
	ON_COMMAND(ID_STORE_MAKEHYBRID, OnStoreMakeHybrid)
	ON_COMMAND(ID_STORE_IMPORTFOLDER, OnStoreImportFolder)
	ON_COMMAND(ID_STORE_PROPERTIES, OnStoreProperties)
	ON_COMMAND(ID_STORE_MAINTENANCE, OnStoreMaintenance)
	ON_COMMAND(ID_STORE_BACKUP, OnStoreBackup)
END_MESSAGE_MAP()


// CMainFrame-Erstellung/Zerstörung

LFFilter* GetRootFilter(char* RootStore=NULL)
{
	LFFilter* f = LFAllocFilter();
	f->Mode = RootStore ? LFFilterModeStoreHome : LFFilterModeStores;
	f->Options.AddBacklink = true;
	f->Options.AddDrives = true;
	f->HideEmptyDrives = (theApp.m_HideEmptyDrives==TRUE);

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
	m_BreadcrumbBack = NULL;
	m_BreadcrumbForward = NULL;
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

	// Titelleiste erstellen
	if (!m_wndCaptionBar.Create(WS_CHILD | WS_CLIPSIBLINGS, this, ID_PANE_CAPTIONBAR, -1, TRUE))
		return -1;

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
	CookFiles(RawFiles->m_Context);

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
	// Ansichtsfenster erhält ersten Eindruck vom Befehl
	if (m_wndMainView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// Andernfalls die Standardbehandlung durchführen
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
		theApp.OpenChildViews(ActiveContextID, TRUE);
}

void CMainFrame::OnChooseDetails()
{
	ChooseDetailsDlg dlg(this, ActiveViewParameters, ActiveContextID);
	if (dlg.DoModal()==IDOK)
		theApp.OpenChildViews(ActiveContextID, TRUE);
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
		pCmdUI->Enable(theApp.m_listMainFrames.size()>1);
		break;
	case ID_VIEW_AUTODIRS:
		pCmdUI->SetCheck((ActiveViewParameters->AutoDirs) || (ActiveContextID>=LFContextSubfolderDefault));
		pCmdUI->Enable((theApp.m_Contexts[ActiveContextID]->AllowGroups) && (SelectViewMode(ActiveViewParameters->Mode)<=LFViewPreview));
		break;
	case ID_APP_VIEW_CALENDAR_YEAR:
		view = pCmdUI->m_nID-ID_APP_VIEW_LARGEICONS+LFViewLargeIcons;
		pCmdUI->SetCheck((ActiveViewID==(INT)view) || (ActiveViewID==(INT)view+1));
		pCmdUI->Enable(theApp.m_AllowedViews[ActiveContextID]->IsSet(view) || theApp.m_AllowedViews[ActiveContextID]->IsSet(view+1));
		break;
	case ID_APP_VIEW_LARGEICONS:
	case ID_APP_VIEW_SMALLICONS:
	case ID_APP_VIEW_LIST:
	case ID_APP_VIEW_DETAILS:
	case ID_APP_VIEW_TILES:
	case ID_APP_VIEW_SEARCHRESULT:
	case ID_APP_VIEW_PREVIEW:
	case ID_APP_VIEW_CALENDAR_DAY:
	case ID_APP_VIEW_GLOBE:
	case ID_APP_VIEW_TAGCLOUD:
	case ID_APP_VIEW_TIMELINE:
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
		ActiveViewParameters->Mode = SelectViewMode(ActiveViewParameters->Mode);

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
		if ((strcmp(i->StoreID, RawFiles->m_Items[a]->StoreID)==0) &&
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
		INT idx = m_wndMainView.GetNextSelectedItem(-1);
		while (idx!=-1)
		{
			INT pos = CookedFiles->m_Items[idx]->Position;
			if (pos!=-1)
				RawFiles->m_Items[pos]->DeleteFlag = true;

			idx = m_wndMainView.GetNextSelectedItem(idx);
		}

		LFRemoveFlaggedItemDescriptors(RawFiles);
		UpdateHistory();
		SendMessage(WM_COMMAND, ID_VIEW_SELECTNONE);
		CookFiles(m_wndMainView.GetFocusItem());
	}
}

void CMainFrame::Remember(CMainFrame* clip)
{
	ASSERT(!IsClipboard);
	ASSERT(clip);

	if (CookedFiles)
	{
		INT idx = m_wndMainView.GetNextSelectedItem(-1);
		BOOL changes = FALSE;
		while (idx!=-1)
		{
			LFItemDescriptor* i = CookedFiles->m_Items[idx];
			if (((i->Type & LFTypeMask)==LFTypeVirtual) && (i->FirstAggregate!=-1) && (i->LastAggregate!=-1))
			{
				for (INT a=i->FirstAggregate; a<=i->LastAggregate; a++)
					if (clip->AddClipItem(RawFiles->m_Items[a]))
						changes = TRUE;
			}
			else
				if (clip->AddClipItem(CookedFiles->m_Items[idx]))
					changes = TRUE;

			idx = m_wndMainView.GetNextSelectedItem(idx);
		}

		if (changes)
			clip->CookFiles(clip->m_wndMainView.GetFocusItem());
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
		b = (IsClipboard) && (m_wndMainView.GetNextSelectedItem(-1)!=-1);
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

void CMainFrame::OnItemsOpen()
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
					OnNavigateBackOne();
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
					OnStoreNewDrive(i->CoreAttributes.FileID[0]);
					break;
				case LFTypeFile:
					res = LFGetFileLocation(i, Path, MAX_PATH, true);
					if (res==LFOk)
					{
						if (ShellExecute(NULL, _T("open"), Path, NULL, NULL, SW_SHOW)==(HINSTANCE)SE_ERR_NOASSOC)
							OnItemsOpenWith();
					}
					else
					{
						m_wndMainView.Invalidate();
						LFErrorBox(res, GetSafeHwnd());
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

void CMainFrame::OnItemsOpenWith()
{
	INT idx = m_wndMainView.GetSelectedItem();

	if (idx!=-1)
	{
		LFItemDescriptor* i = CookedFiles->m_Items[idx];

		if ((!i->NextFilter) && ((i->Type & (LFTypeNotMounted | LFTypeMask))==LFTypeFile))
		{
			WCHAR Path[MAX_PATH];
			UINT res = LFGetFileLocation(i, Path, MAX_PATH, true);
			if (res==LFOk)
			{
				WCHAR Cmd[300];
				wcscpy_s(Cmd, 300, L"shell32.dll,OpenAs_RunDLL ");
				wcscat_s(Cmd, 300, Path);
				ShellExecute(GetSafeHwnd(), _T("open"), _T("rundll32.exe"), Cmd, Path, SW_SHOW);
			}
			else
			{
				m_wndMainView.Invalidate();
				LFErrorBox(res, GetSafeHwnd());
			}
		}
	}
}

void CMainFrame::OnItemsDelete()
{
	INT i;

	if (CookedFiles)
		switch (CookedFiles->m_Context)
		{
		case LFContextStores:
			i = m_wndMainView.GetSelectedItem();
			if (i!=-1)
			{
				LFItemDescriptor* store = LFAllocItemDescriptor(CookedFiles->m_Items[i]);
				LFErrorBox(theApp.DeleteStore(store, this));
				LFFreeItemDescriptor(store);
			}
			break;
		case LFContextTrash:
			DeleteFiles();
			break;
		default:
			UpdateTrashFlag(TRUE);
		}
}

void CMainFrame::OnItemsRename()
{
/*	if (m_wndMainView.p_wndFileView)
	{
		m_wndMainView.p_wndFileView->SetFocus();
		m_wndMainView.p_wndFileView->EditLabel(GetSelectedItem());
	}*/
}

void CMainFrame::OnUpdateItemCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;
	INT i = m_wndMainView.GetSelectedItem();
	LFItemDescriptor* f = (i==-1 ? NULL : CookedFiles->m_Items[i]);

	switch (pCmdUI->m_nID)
	{
	case ID_ITEMS_SHOWINSPECTOR:
		b = TRUE;
		break;
	case ID_ITEMS_OPEN:
		if (f)
			b = (f->NextFilter!=NULL) ||
				((f->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeDrive) ||
				((f->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeFile);
		break;
	case ID_ITEMS_OPENWITH:
		if (f)
			b = (!f->NextFilter) && ((f->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeFile);
		break;
	case ID_ITEMS_DELETE:
		if (CookedFiles)
			b = (CookedFiles->m_Context==LFContextStores) ? f ? ((f->Type & LFTypeMask)==LFTypeStore) : FALSE : FilesSelected;
		break;
	case ID_ITEMS_RENAME:
		if (CookedFiles)
			b = f ? (CookedFiles->m_Context==LFContextStores) ? ((f->Type & LFTypeMask)==LFTypeStore) && (ActiveViewID>=LFViewLargeIcons) && (ActiveViewID<=LFViewPreview) : (f->Type & LFTypeMask)==LFTypeFile : FALSE;
//		if ((b) && (m_wndMainView.p_wndFileView))
//			b ^= m_wndMainView.p_wndFileView->IsEditing();
		break;
	}

	pCmdUI->Enable(b);
}

void CMainFrame::OnEmptyTrash()
{
	if (DeleteFiles(TRUE))
		theApp.PlayTrashSound();
}

void CMainFrame::OnRestoreSelectedFiles()
{
	UpdateTrashFlag(FALSE);
}

void CMainFrame::OnRestoreAllFiles()
{
	UpdateTrashFlag(FALSE, TRUE);
}

void CMainFrame::OnUpdateTrashCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	if (CookedFiles)
		if (CookedFiles->m_Context==LFContextTrash)
			switch (pCmdUI->m_nID)
			{
			case ID_TRASH_EMPTY:
			case ID_TRASH_RESTOREALL:
				b = (CookedFiles->m_FileCount);
				break;
			case ID_TRASH_RESTORESELECTED:
				b = FilesSelected;
				break;
			}

	pCmdUI->Enable(b);
}

void CMainFrame::OnStoreNew()
{
	LFStoreDescriptor* s = LFAllocStoreDescriptor();

	LFStoreNewDlg dlg(this, s);
	if (dlg.DoModal()==IDOK)
		LFErrorBox(LFCreateStore(s, dlg.MakeDefault), m_hWnd);

	LFFreeStoreDescriptor(s);
}

void CMainFrame::OnStoreNewInternal()
{
	LFStoreDescriptor* s = LFAllocStoreDescriptor();
	s->AutoLocation = TRUE;
	s->StoreMode = LFStoreModeInternal;

	UINT res = LFCreateStore(s);
	LFErrorBox(res, GetSafeHwnd());

	if ((res==LFOk) && (CookedFiles))
		for (UINT a=0; a<CookedFiles->m_ItemCount; a++)
		{
			LFItemDescriptor* i = CookedFiles->m_Items[a];
			if ((strcmp(s->StoreID, i->StoreID)==0) && ((i->Type & LFTypeMask)==LFTypeStore))
			{
				m_wndMainView.SetFocus();
				//m_wndMainView.p_wndFileView->EditLabel(a);
				break;
			}
		}

	LFFreeStoreDescriptor(s);
}

void CMainFrame::OnStoreNewDrive(CHAR drv)
{
	INT i = m_wndMainView.GetSelectedItem();

	if (i!=-1)
	{
		LFStoreDescriptor* s = LFAllocStoreDescriptor();

		LFStoreNewDriveDlg dlg(this, drv, s);
		if (dlg.DoModal()==IDOK)
			LFErrorBox(LFCreateStore(s, FALSE), m_hWnd);

		LFFreeStoreDescriptor(s);
	}
}

void CMainFrame::OnStoreNewDrive()
{
	INT i = m_wndMainView.GetSelectedItem();

	if (i!=-1)
		OnStoreNewDrive(CookedFiles->m_Items[i]->CoreAttributes.FileID[0]);
}

void CMainFrame::OnStoreMakeDefault()
{
	INT i = m_wndMainView.GetSelectedItem();

	if (i!=-1)
		LFErrorBox(LFMakeDefaultStore(CookedFiles->m_Items[i]->StoreID), GetSafeHwnd());
}

void CMainFrame::OnStoreMakeHybrid()
{
	INT i = m_wndMainView.GetSelectedItem();

	if (i!=-1)
		LFErrorBox(LFMakeHybridStore(CookedFiles->m_Items[i]->StoreID), GetSafeHwnd());
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
	for (INT a = s.GetLength()-1; a>=0; a--)
		if ((s[a]=='\\') || (s[a]=='\"'))
			s.Insert(a, '\\');
}

void CMainFrame::OnStoreProperties()
{
	INT i = m_wndMainView.GetSelectedItem();

	if (i!=-1)
	{
		LFStorePropertiesDlg dlg(CookedFiles->m_Items[i]->StoreID, this);
		dlg.DoModal();
	}
}

void CMainFrame::OnStoreImportFolder()
{
	INT i = m_wndMainView.GetSelectedItem();

	if (i!=-1)
		LFImportFolder(CookedFiles->m_Items[i]->StoreID, this);
}

void CMainFrame::OnStoreMaintenance()
{
	LFMaintenanceList* ml = LFStoreMaintenance();
	LFErrorBox(ml->m_LastError);

	LFStoreMaintenanceDlg dlg(ml, this);
	dlg.DoModal();

	LFFreeMaintenanceList(ml);

	OnNavigateReload();
}

void CMainFrame::OnStoreBackup()
{
	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_REGFILEFILTER));
	tmpStr += _T(" (*.reg)|*.reg||");

	CFileDialog dlg(FALSE, _T(".reg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, tmpStr, this);

	if (dlg.DoModal()==IDOK)
	{
		CStdioFile f;
		if (!f.Open(dlg.GetFileName(), CFile::modeCreate | CFile::modeWrite))
		{
			LFErrorBox(LFDriveNotReady, GetSafeHwnd());
		}
		else
		{
			try
			{
				f.WriteString(_T("Windows Registry Editor Version 5.00\n"));

				for (UINT a=0; a<CookedFiles->m_ItemCount; a++)
				{
					LFItemDescriptor* i = CookedFiles->m_Items[a];
					if ((i->Type & LFTypeStore) && (i->CategoryID<=LFItemCategoryHybridStores))
					{
						LFStoreDescriptor s;
						if (LFGetStoreSettings(i->StoreID, &s)==LFOk)
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
				LFErrorBox(LFDriveNotReady, GetSafeHwnd());
			}

			f.Close();
		}
	}
}

void CMainFrame::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	BOOL b = (pCmdUI->m_nID==ID_STORE_MAINTENANCE) ? LFGetStoreCount() : FALSE;

	if (CookedFiles)
		if (CookedFiles->m_Context==LFContextStores)
		{
			INT i = m_wndMainView.GetSelectedItem();
			LFItemDescriptor* f = (i==-1 ? NULL : CookedFiles->m_Items[i]);

			switch (pCmdUI->m_nID)
			{
			case ID_STORE_NEW:
			case ID_STORE_NEWINTERNAL:
				b = TRUE;
				break;
			case ID_STORE_NEWDRIVE:
				if (f)
					b = (f->Type & LFTypeDrive) && (!(f->Type & LFTypeNotMounted));
				break;
			case ID_STORE_RENAME:
				if (f)
					b = (f->Type & LFTypeStore) && (ActiveViewID>=LFViewLargeIcons) && (ActiveViewID<=LFViewPreview);
//				if ((b) && (m_wndMainView.p_wndFileView))
//					b ^= m_wndMainView.p_wndFileView->IsEditing();
				break;
			case ID_STORE_IMPORTFOLDER:
				if (f)
					b = (f->Type & LFTypeStore) && (!(f->Type & LFTypeNotMounted));
				break;
			case ID_STORE_DELETE:
			case ID_STORE_PROPERTIES:
				if (f)
					b = (f->Type & LFTypeStore);
				break;
			case ID_STORE_MAKEDEFAULT:
				if (f)
					b = (f->Type & LFTypeStore) &&
						(f->CategoryID==LFItemCategoryInternalStores) &&
						((f->Type & LFTypeDefaultStore)==0);
				break;
			case ID_STORE_MAKEHYBRID:
				if (f)
					b = (f->Type & LFTypeStore) &&
						(f->CategoryID==LFItemCategoryExternalStores);
				break;
			case ID_STORE_BACKUP:
				b = TRUE;
			}
		}

	pCmdUI->Enable(b);
}

void CMainFrame::UpdateViewOptions()
{
	if (ActiveViewID==LFViewGlobe)
	{
		CMFCRibbonBaseElement* cbx = m_wndRibbonBar.FindByID(ID_GLOBE_TEXTURESIZE, FALSE, TRUE);
		if (cbx)
			((CTextureComboBox*)cbx)->SelectItem((INT)theApp.m_nTextureSize);
	}

	m_wndMainView.UpdateViewOptions(ActiveContextID);
}

void CMainFrame::UpdateSortOptions()
{
	CookFiles();
}

void CMainFrame::UpdateSearchResult(BOOL SetEmpty, INT FocusItem)
{
	if ((!SetEmpty) && (CookedFiles))
	{
		// Im Debug-Modus bleiben alle Kategorien sichtbar
		#ifndef _DEBUG
		m_wndRibbonBar.ShowCategory(RibbonCategory_Stores, (CookedFiles->m_Context==LFContextStores) && (ActiveFilter->Result.FilterType==LFFilterTypeStores), TRUE);
		m_wndRibbonBar.ShowCategory(RibbonCategory_Trash, (CookedFiles->m_Context==LFContextTrash) && (ActiveFilter->Result.FilterType==LFFilterTypeTrash), TRUE);
		m_wndRibbonBar.ShowCategory(RibbonCategory_UnknownFileFormats, (CookedFiles->m_Context==LFContextHousekeeping) && (ActiveFilter->Result.FilterType==LFFilterTypeUnknownFileFormats), TRUE);
		#endif

		// ChildView austauschen:
		// - Wenn ein anderer Kontext mit ggf. anderen Views gewünscht wird
		// - Wenn im Kontext die Ansicht auf "automatisch" steht
		// - Wenn sich für die Liste das Kategorien-Flag ändert (wg. virtual mode)
		BOOL change = (ActiveContextID!=CookedFiles->m_Context) || (ActiveViewID!=(INT)SelectViewMode(ActiveViewParameters->Mode));
		if (change)
		{
			ActiveContextID = CookedFiles->m_Context;
			ActiveViewParameters = &theApp.m_Views[ActiveContextID];
			if (OpenChildView(FocusItem))
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

	m_wndMainView.UpdateSearchResult(SetEmpty ? NULL : CookedFiles, FocusItem);
	OnUpdateSelection();
}

void CMainFrame::OnChangeChildView(UINT nID)
{
	ActiveViewParameters->Mode = nID-ID_APP_VIEW_LARGEICONS+LFViewLargeIcons;
	theApp.OpenChildViews(ActiveContextID);
}

void CMainFrame::OnUpdateSelection()
{
	// Selection
	m_wndInspector.UpdateStart(ActiveFilter);
	INT i = m_wndMainView.GetNextSelectedItem(-1);
	FilesSelected = FALSE;

	while (i>=0)
	{
		LFItemDescriptor* item = CookedFiles->m_Items[i];

		m_wndInspector.UpdateAdd(item, RawFiles);
		FilesSelected |= ((item->Type & LFTypeMask)==LFTypeFile) ||
						(((item->Type & LFTypeMask)==LFTypeVirtual) && (item->FirstAggregate!=-1) && (item->LastAggregate!=-1));

		i = m_wndMainView.GetNextSelectedItem(i);
	}

	m_wndInspector.UpdateFinish();
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
			m_wndMainView.UpdateSearchResult(CookedFiles, m_wndMainView.GetFocusItem());

		if (tl->m_LastError>LFCancel)
			ShowCaptionBar(IDI_ERROR, tl->m_LastError);

		LFFreeTransactionList(tl);
	}

	return result;
}

void CMainFrame::AddTransactionItem(LFTransactionList* tl, LFItemDescriptor* i, UINT UserData)
{
	switch (i->Type & LFTypeMask)
	{
	case LFTypeFile:
	case LFTypeStore:
		LFAddItemDescriptor(tl, i, UserData);
		break;
	case LFTypeVirtual:
		if ((i->FirstAggregate!=-1) && (i->LastAggregate!=-1))
			for (INT a=i->FirstAggregate; a<=i->LastAggregate; a++)
				LFAddItemDescriptor(tl, RawFiles->m_Items[a], UserData);
		break;
	}
}

LFTransactionList* CMainFrame::BuildTransactionList(BOOL All)
{
	LFTransactionList* tl = NULL;

	if ((RawFiles) && (CookedFiles))
	{
		tl = LFAllocTransactionList();

		if (All)
		{
			for (UINT a=0; a<CookedFiles->m_ItemCount; a++)
				AddTransactionItem(tl, CookedFiles->m_Items[a], a);
		}
		else
		{
			INT idx = m_wndMainView.GetNextSelectedItem(-1);
			while (idx!=-1)
			{
				AddTransactionItem(tl, CookedFiles->m_Items[idx], idx);
				idx = m_wndMainView.GetNextSelectedItem(idx);
			}
		}
	}

	return tl;
}

BOOL CMainFrame::UpdateSelectedItems(LFVariantData* value1, LFVariantData* value2, LFVariantData* value3)
{
	LFTransactionList* tl = BuildTransactionList();
	LFTransactionUpdate(tl, GetSafeHwnd(), value1, value2, value3);

	if (m_wndMainView.p_wndFileView)
	{
		BOOL deselected = FALSE;

		for (UINT a=0; a<tl->m_ItemCount; a++)
			if (tl->m_Items[a].LastError!=LFOk)
			{
//				m_wndMainView.p_wndFileView->SelectItem(tl->m_Items[a].UserData, FALSE, TRUE);
				deselected = TRUE;
			}

		if (tl->m_Changes)
			m_wndMainView.UpdateSearchResult(CookedFiles, m_wndMainView.GetFocusItem());
		if (deselected)
			OnUpdateSelection();
	}

	if (tl->m_LastError>LFCancel)
		ShowCaptionBar(IDI_ERROR, tl->m_LastError);

	BOOL changes = tl->m_Changes;
	LFFreeTransactionList(tl);
	return changes;
}

BOOL CMainFrame::UpdateTrashFlag(BOOL Trash, BOOL All)
{
	LFVariantData value1;
	value1.Attr = LFAttrFlags;
	LFGetNullVariantData(&value1);

	value1.IsNull = false;
	value1.Flags.Flags = Trash ? LFFlagTrash : 0;
	value1.Flags.Mask = LFFlagTrash;

	LFVariantData value2;
	value2.Attr = LFAttrDeleteTime;
	LFGetNullVariantData(&value2);
	value2.IsNull = false;

	if (Trash)
		GetSystemTimeAsFileTime(&value2.Time);

	LFTransactionList* tl = BuildTransactionList(All);
	LFTransactionUpdate(tl, GetSafeHwnd(), &value1, &value2);

	for (UINT a=0; a<tl->m_ItemCount; a++)
		if (tl->m_Items[a].LastError==LFOk)
			tl->m_Items[a].Item->DeleteFlag = true;

	LFRemoveFlaggedItemDescriptors(RawFiles);
	UpdateHistory();
	SendMessage(WM_COMMAND, ID_VIEW_SELECTNONE);
	CookFiles(m_wndMainView.GetFocusItem());

	if (tl->m_LastError>LFCancel)
		ShowCaptionBar(IDI_ERROR, tl->m_LastError);

	BOOL changes = tl->m_Changes;
	LFFreeTransactionList(tl);
	return changes;
}

BOOL CMainFrame::DeleteFiles(BOOL All)
{
	LFTransactionList* tl = BuildTransactionList(All);
	LFTransactionDelete(tl);

	if (m_wndMainView.p_wndFileView)
	{
		for (UINT a=0; a<tl->m_ItemCount; a++)
			if (tl->m_Items[a].LastError==LFOk)
				tl->m_Items[a].Item->DeleteFlag = true;

		LFRemoveFlaggedItemDescriptors(RawFiles);
		UpdateHistory();
		SendMessage(WM_COMMAND, ID_VIEW_SELECTNONE);
		CookFiles(m_wndMainView.GetFocusItem());
	}

	if (tl->m_LastError>LFCancel)
		ShowCaptionBar(IDI_ERROR, tl->m_LastError);

	BOOL changes = tl->m_Changes;
	LFFreeTransactionList(tl);
	return changes;
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
		INT focus = m_wndMainView.GetFocusItem();
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
		INT focus = m_wndMainView.GetFocusItem();
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
		NavigateTo(LFAllocFilter(ActiveFilter), NAVMODE_RELOAD, m_wndMainView.GetFocusItem());
}

void CMainFrame::OnNavigateReloadShowAll()
{
	if (ActiveFilter)
	{
		LFFilter* f = LFAllocFilter(ActiveFilter);
		f->UnhideAll = true;

		NavigateTo(f, NAVMODE_RELOAD, m_wndMainView.GetFocusItem());
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
		pMainPanel->Add(theApp.CommandButton(ID_APP_NEWMIGRATE, 2, 2));
		pMainPanel->Add(theApp.CommandButton(ID_APP_NEWFILEDROP, 3, 3));
		pMainPanel->Add(new CMFCRibbonSeparator(TRUE));
		pMainPanel->Add(theApp.CommandButton(ID_APP_CLOSEOTHERS, 4, 4));
		pMainPanel->Add(theApp.CommandButton(ID_APP_CLOSE, 5, 5));

		strTemp = "About";
		pMainPanel->AddToBottom(new CMFCRibbonMainPanelButton(ID_APP_ABOUT, strTemp, 6));
		strTemp = "Exit";
		pMainPanel->AddToBottom(new CMFCRibbonMainPanelButton(ID_APP_EXIT, strTemp, 7));

	strTemp = "Home";
	CMFCRibbonCategory* pCategoryHome = m_wndRibbonBar.AddCategory(strTemp, IDB_RIBBONHOME_16, IDB_RIBBONHOME_32);

		if (!IsClipboard)
		{
			strTemp = "Navigate";
			CMFCRibbonPanel* pPanelNavigate = pCategoryHome->AddPanel(strTemp, m_PanelImages.ExtractIcon(5));
			pPanelNavigate->EnableLaunchButton(ID_NAV_SHOWHISTORY, 5);

				pPanelNavigate->Add(theApp.CommandButton(ID_NAV_FIRST, 0, 0));
				pPanelNavigate->Add(theApp.CommandButton(ID_NAV_BACKONE, 1, 1));
				pPanelNavigate->Add(theApp.CommandButton(ID_NAV_FORWARDONE, 2, 2));
				pPanelNavigate->Add(theApp.CommandButton(ID_NAV_LAST, 3, 3));
				pPanelNavigate->AddSeparator();
				pPanelNavigate->Add(theApp.CommandButton(ID_NAV_RELOAD, 4, 4));

			strTemp = "Places";
			CMFCRibbonPanel* pPanelPlaces = pCategoryHome->AddPanel(strTemp, m_PanelImages.ExtractIcon(16));

				pPanelPlaces->Add(theApp.CommandButton(ID_NAV_STORES, 6, 6));
				pPanelPlaces->Add(theApp.CommandButton(ID_NAV_HOME, 7, 7));
		}

		strTemp = "liquidFOLDERS";
		CMFCRibbonPanel* pPanelliquidFOLDERS = pCategoryHome->AddPanel(strTemp, m_PanelImages.ExtractIcon(0));
		pPanelliquidFOLDERS->EnableLaunchButton(ID_APP_ABOUT, 12);

			pPanelliquidFOLDERS->Add(theApp.CommandButton(ID_APP_HELP, 8, 8));
			pPanelliquidFOLDERS->Add(theApp.CommandButton(ID_APP_SUPPORT, 9, 9));

			if (!LFIsLicensed())
			{
				pPanelliquidFOLDERS->AddSeparator();
				pPanelliquidFOLDERS->Add(theApp.CommandButton(ID_APP_PURCHASE, 10, 10));
				pPanelliquidFOLDERS->Add(theApp.CommandButton(ID_APP_ENTERLICENSEKEY, 11, 11));
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
				pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_CALENDAR_YEAR, 7, 7));
				pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_GLOBE, 8, 8));
				pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_TAGCLOUD, 9, 9));
				pPanelDisplay->Add(theApp.CommandButton(ID_APP_VIEW_TIMELINE, 10, 10));
			}

	strTemp = "Items";
	CMFCRibbonCategory* pCategoryItems = m_wndRibbonBar.AddCategory(strTemp, IDB_RIBBONITEMS_16, IDB_RIBBONITEMS_32);

		strTemp = "Clipboard";
		CMFCRibbonPanel* pPanelClipboard = pCategoryItems->AddPanel(strTemp, m_PanelImages.ExtractIcon(9));

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

		strTemp = "Basic";
		CMFCRibbonPanel* pPanelItemsBasic = pCategoryItems->AddPanel(strTemp, m_PanelImages.ExtractIcon(3));
		pPanelItemsBasic->EnableLaunchButton(ID_ITEMS_SHOWINSPECTOR, 10);

			pPanelItemsBasic->Add(theApp.CommandButton(ID_ITEMS_OPEN, 7, 7));
			pPanelItemsBasic->AddSeparator();
			pPanelItemsBasic->Add(theApp.CommandButton(ID_ITEMS_DELETE, 8, 8));
			pPanelItemsBasic->Add(theApp.CommandButton(ID_ITEMS_RENAME, 9, 9));

		/*strTemp = "Share";
		CMFCRibbonPanel* pPanelFileShare = pCategoryItems->AddPanel(strTemp, m_PanelImages.ExtractIcon(0));

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

			pPanelFileShare->Add(pBtnSyndicate);*/

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

			CMFCRibbonPanel* pPanelRegister = pCategoryUnknownFileFormats->AddPanel(strTemp, m_PanelImages.ExtractIcon(15));
			pPanelRegister->EnableLaunchButton(ID_UNKNOWN_EDITDB, 2);

			pPanelRegister->Add(theApp.CommandButton(ID_UNKNOWN_REGISTER, 0, 0));
				pPanelRegister->AddSeparator();
				pPanelRegister->Add(theApp.CommandButton(ID_UNKNOWN_SENDDB, 1, 1));

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

					CMFCRibbonButtonsGroup* p3DOptions = new CMFCRibbonButtonsGroup();
					p3DOptions->AddButton(theApp.CommandButton(ID_GLOBE_HQMODEL));
					p3DOptions->AddButton(theApp.CommandButton(ID_GLOBE_LIGHTING));

				pPanelGlobeOptions->Add(p3DOptions);

				strTemp = "Texture size:";
				pPanelGlobeOptions->Add(new CMFCRibbonLabel(strTemp));
				pPanelGlobeOptions->Add(new CTextureComboBox(ID_GLOBE_TEXTURESIZE, 100));
				pPanelGlobeOptions->AddSeparator();
				pPanelGlobeOptions->Add(theApp.CommandButton(ID_GLOBE_SHOWBUBBLES, 6, 6));
				pPanelGlobeOptions->Add(theApp.CommandCheckBox(ID_GLOBE_SHOWAIRPORTNAMES));
				pPanelGlobeOptions->Add(theApp.CommandCheckBox(ID_GLOBE_SHOWGPS));
				pPanelGlobeOptions->Add(theApp.CommandCheckBox(ID_GLOBE_SHOWHINTS));
				pPanelGlobeOptions->AddSeparator();
				pPanelGlobeOptions->Add(theApp.CommandCheckBox(ID_GLOBE_SHOWSPOTS));
				pPanelGlobeOptions->Add(theApp.CommandCheckBox(ID_GLOBE_SHOWVIEWPOINT));

		strTemp = "Tagcloud";
		CMFCRibbonCategory* pCategoryTagcloud = m_wndRibbonBar.AddContextCategory(strTemp, strCtx, 2, AFX_CategoryColor_Indigo, IDB_RIBBONTAGCLOUD_16, IDB_RIBBONTAGCLOUD_32);

			strTemp = "Tags";
			CMFCRibbonPanel* pPanelTagcloudTags = pCategoryTagcloud->AddPanel(strTemp, m_PanelImages.ExtractIcon(18));

				pPanelTagcloudTags->Add(theApp.CommandButton(ID_TAGCLOUD_SORTVALUE, 0, 0));
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
				pPanelStoresStores->Add(theApp.CommandButton(ID_STORE_DELETE, 4, 4));
				pPanelStoresStores->Add(theApp.CommandButton(ID_STORE_RENAME, 5, 5));
				pPanelStoresStores->AddSeparator();
				pPanelStoresStores->Add(theApp.CommandButton(ID_STORE_MAKEDEFAULT, 6, 6));
				pPanelStoresStores->Add(theApp.CommandButton(ID_STORE_MAKEHYBRID, 7, 7));

			strTemp = "Import files";
			CMFCRibbonPanel* pPanelStoresImport = pCategoryStores->AddPanel(strTemp, m_PanelImages.ExtractIcon(8));

				pPanelStoresImport->Add(theApp.CommandButton(ID_STORE_IMPORTFOLDER, 8, 8));
				pPanelStoresImport->AddSeparator();
				pPanelStoresImport->Add(theApp.CommandButton(ID_APP_NEWMIGRATE, 9, 9, FALSE, TRUE));
				pPanelStoresImport->Add(theApp.CommandButton(ID_APP_NEWFILEDROP, 10, 10));

			strTemp = "Housekeeping";
			CMFCRibbonPanel* pPanelStoresHousekeeping = pCategoryStores->AddPanel(strTemp, m_PanelImages.ExtractIcon(12));

				pPanelStoresHousekeeping->Add(theApp.CommandButton(ID_STORE_PROPERTIES, 11, 11));
				pPanelStoresHousekeeping->AddSeparator();
				pPanelStoresHousekeeping->Add(theApp.CommandButton(ID_STORE_MAINTENANCE, 12, 12));
				pPanelStoresHousekeeping->Add(theApp.CommandButton(ID_STORE_BACKUP, 13, 13));

		strTemp = "Deleted files";
		strCtx = "Trash";
		CMFCRibbonCategory* pCategoryTrash = m_wndRibbonBar.AddContextCategory(strTemp, strCtx, 4, AFX_CategoryColor_Red, IDB_RIBBONTRASH_16, IDB_RIBBONTRASH_32);

			CMFCRibbonPanel* pPanelDeletedFiles = pCategoryTrash->AddPanel(strTemp, m_PanelImages.ExtractIcon(14));

				pPanelDeletedFiles->Add(theApp.CommandButton(ID_TRASH_EMPTY, 0, 0));
				pPanelDeletedFiles->AddSeparator();
				pPanelDeletedFiles->Add(theApp.CommandButton(ID_TRASH_RESTORESELECTED, 1, 1));
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

void CMainFrame::ShowCaptionBar(LPCWSTR Icon, LPCWSTR Message, INT Command)
{
	// Text und Icon
	m_wndCaptionBar.SetText(Message, CMFCCaptionBar::ALIGN_LEFT);
	m_wndCaptionBar.SetIcon(Icon, CMFCCaptionBar::ALIGN_LEFT);
	m_wndCaptionBar.Invalidate();

	// Button
	if (Command)
	{
		m_wndCaptionBar.SetButton(theApp.GetCommandName(Command)+_T("..."), Command, CMFCCaptionBar::ALIGN_LEFT, FALSE);

		CString strTemp;
		ENSURE(strTemp.LoadString(Command));
		INT y = strTemp.Find('\n');
		if (!y)
			y = strTemp.GetLength();
		m_wndCaptionBar.SetButtonToolTip(strTemp.Left(y));
		m_wndCaptionBar.EnableButton(TRUE);
	}
	else
	{
		m_wndCaptionBar.SetButton(_T(" "), 0, CMFCCaptionBar::ALIGN_LEFT, FALSE);
		m_wndCaptionBar.EnableButton(FALSE);
	}

	// Balken sichtbar machen
	if (!m_wndCaptionBar.IsVisible())
	{
		m_wndCaptionBar.ShowWindow(SW_SHOW);
		RecalcLayout(FALSE);
	}
}

void CMainFrame::ShowCaptionBar(LPCWSTR Icon, UINT res, INT Command)
{
	WCHAR* message = LFGetErrorText(res);
	ShowCaptionBar(Icon, message, Command);
	free(message);
}

UINT CMainFrame::SelectViewMode(UINT ViewID)
{
	if (ViewID>=LFViewCount)
		ViewID = LFViewTiles;
	if (CookedFiles)
		if (!theApp.m_AllowedViews[CookedFiles->m_Context]->IsSet(ViewID))
			ViewID = LFViewTiles;

	if (!AttributeSortableInView(ActiveViewParameters->SortBy, ViewID))
		for (UINT a=0; a<=LFViewTimeline; a++)
			if (AttributeSortableInView(ActiveViewParameters->SortBy, a))
			{
				if (!CookedFiles)
					return a;

				if (theApp.m_AllowedViews[CookedFiles->m_Context]->IsSet(a))
					return a;
			}

	return ViewID;
}

BOOL CMainFrame::OpenChildView(INT FocusItem, BOOL Force, BOOL AllowChangeSort)
{
	UINT ViewID = SelectViewMode(ActiveViewParameters->Mode);

	if (AllowChangeSort)
	{
		if (!AttributeSortableInView(ActiveViewParameters->SortBy, ActiveViewParameters->Mode))
		{
			for (UINT a=0; a<LFAttributeCount; a++)
			if (AttributeSortableInView(a, ActiveViewParameters->Mode))
			{
				ActiveViewParameters->SortBy = a;
				break;
			}

			theApp.UpdateSortOptions(ActiveContextID);
			return FALSE;
		}

		if ((ViewID>LFViewPreview)!=(ActiveViewID>LFViewPreview))
			theApp.UpdateSortOptions(ActiveContextID);
	}

	ActiveViewParameters->Mode = ViewID;
	ASSERT(AttributeSortableInView(ActiveViewParameters->SortBy, ViewID));

	m_wndMainView.UpdateViewOptions(ActiveContextID);
	m_wndMainView.UpdateSearchResult(CookedFiles, FocusItem);

	OnUpdateSelection();

	ActiveViewID = ViewID;

	// Im Debug-Modus bleiben alle Kategorien sichtbar
	#ifndef _DEBUG
	m_wndRibbonBar.ShowCategory(RibbonCategory_View_Calendar, (ViewID>=LFViewCalendarYear) && (ViewID<=LFViewCalendarDay));
	m_wndRibbonBar.ShowCategory(RibbonCategory_View_Globe, ViewID==LFViewGlobe);
	m_wndRibbonBar.ShowCategory(RibbonCategory_View_Tagcloud, ViewID==LFViewTagcloud);
	m_wndRibbonBar.ShowCategory(RibbonCategory_View_Timeline, ViewID==LFViewTimeline);
	m_wndRibbonBar.Update();
	#endif

	return TRUE;
}

void CMainFrame::NavigateTo(LFFilter* f, UINT NavMode, INT FocusItem, INT FirstAggregate, INT LastAggregate)
{
	if (NavMode<NAVMODE_RELOAD)
		theApp.PlayNavigateSound();

	if (ActiveFilter)
		if (NavMode==NAVMODE_NORMAL)
		{
			AddBreadcrumbItem(&m_BreadcrumbBack, ActiveFilter, m_wndMainView.GetFocusItem());
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

	ActiveFilter->HideEmptyDrives = (theApp.m_HideEmptyDrives==TRUE);
	ActiveFilter->HideEmptyDomains = (theApp.m_HideEmptyDomains==TRUE);

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

	CookFiles(FocusItem);

	if (CookedFiles->m_LastError>LFCancel)
	{
		theApp.PlayWarningSound();
		ShowCaptionBar(ActiveFilter->Result.FilterType==LFFilterTypeError ? IDI_ERROR : IDI_EXCLAMATION, CookedFiles->m_LastError, CookedFiles->m_LastError==LFIndexAccessError ? ID_STORE_MAINTENANCE : 0);
	}
	else
		if (m_wndCaptionBar.IsVisible())
		{
			m_wndCaptionBar.ShowWindow(SW_HIDE);
			RecalcLayout(FALSE);
		}
}

void CMainFrame::CookFiles(INT FocusItem)
{
	// Das alte Suchergebnis wird in Victim gespeichert, damit das View niemals eine ungültige
	// Referenz hat. Erst nach UpdateSearchResult() kann das ggf. vorhandene alte Suchergebnis
	// gelöscht werden.
	LFSearchResult* Victim = CookedFiles;

	LFViewParameters* vp = &theApp.m_Views[RawFiles->m_Context];
	LFAttributeDescriptor* attr = theApp.m_Attributes[vp->SortBy];

	if (((!IsClipboard) && (vp->AutoDirs) && (!ActiveFilter->Options.IsSubfolder)) || (vp->Mode>LFViewPreview))
	{
		CookedFiles = LFGroupSearchResult(RawFiles, vp->SortBy, vp->Descending==TRUE, true, attr->IconID,
			(vp->Mode>LFViewPreview) || ((attr->Type!=LFTypeTime) && (vp->SortBy!=LFAttrFileName) && (vp->SortBy!=LFAttrStoreID) && (vp->SortBy!=LFAttrFileID)),
			ActiveFilter);
	}
	else
	{
		LFSortSearchResult(RawFiles, vp->SortBy, vp->Descending==TRUE, true);
		CookedFiles = RawFiles;
	}

	UpdateSearchResult(FALSE, FocusItem);
	UpdateHistory();

	if ((Victim) && (Victim!=RawFiles))
		LFFreeSearchResult(Victim);

	if (!LFIsLicensed())
		if ((++theApp.m_NagCounter)>25)
		{
			theApp.m_NagCounter = 0;
			MessageBox(_T("You are using an unregistered copy of liquidFOLDERS. liquidFOLDERS is shareware -\nif you decide to use it regulary, you are required to purchase a license from our website!"), _T("Unregistered copy"));
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

LRESULT CMainFrame::OnDrivesChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (CookedFiles)
		if (CookedFiles->m_Context==LFContextStores)
			OnNavigateReload();

	return NULL;
}

LRESULT CMainFrame::OnStoresChanged(WPARAM /*wParam*/, LPARAM lParam)
{
	if (CookedFiles)
		switch (CookedFiles->m_Context)
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
	{
		theApp.SetApplicationLook((UINT)wParam);
	#if (_MFC_VER>=0x1000)
		m_wndRibbonBar.SetWindows7Look((UINT)wParam==ID_VIEW_APPLOOK_WINDOWS_7);
	#endif
	}

	return NULL;
}
