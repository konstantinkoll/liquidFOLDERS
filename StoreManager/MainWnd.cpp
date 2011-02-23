
// MainWnd.cpp: Implementierung der Klasse CMainWnd
//

#include "stdafx.h"
#include "StoreManager.h"
#include "SortOptionsDlg.h"
#include "ViewOptionsDlg.h"
#include "LFCommDlg.h"


LFFilter* GetRootFilter(char* RootStore=NULL)
{
	LFFilter* f = LFAllocFilter();
	f->Mode = RootStore ? LFFilterModeStoreHome : LFFilterModeStores;
	f->Options.AddDrives = true;
	f->ShowEmptyDrives = (theApp.m_ShowEmptyDrives==TRUE);

	if (RootStore)
		strcpy_s(f->StoreID, LFKeySize, RootStore);

	return f;
}


// CMainWnd
//

CMainWnd::CMainWnd()
{
	m_hIcon = NULL;
	ActiveViewID = -1;
	ActiveContextID = -1;
	ActiveViewParameters = &theApp.m_Views[LFContextDefault];
	RawFiles = NULL;
	CookedFiles = NULL;
	m_BreadcrumbBack = m_BreadcrumbForward = NULL;
}

CMainWnd::~CMainWnd()
{
	if (m_hIcon)
		DestroyIcon(m_hIcon);
	if (ActiveFilter)
		LFFreeFilter(ActiveFilter);
	if (CookedFiles!=RawFiles)
		LFFreeSearchResult(CookedFiles);
	LFFreeSearchResult(RawFiles);
	DeleteBreadcrumbs(&m_BreadcrumbBack);
	DeleteBreadcrumbs(&m_BreadcrumbForward);
}

BOOL CMainWnd::Create(BOOL IsClipboard, CHAR* RootStore)
{
	m_hIcon = theApp.LoadIcon(IsClipboard ? IDR_CLIPBOARD : IDR_APPLICATION);
	m_IsClipboard = IsClipboard;
	ActiveFilter = GetRootFilter(RootStore);

	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW), NULL, m_hIcon);

	CRect rect;
	SystemParametersInfo(SPI_GETWORKAREA, NULL, &rect, NULL);
	rect.DeflateRect(32, 32);

	CString caption;
	ENSURE(caption.LoadString(IsClipboard ? IDR_CLIPBOARD : IDR_APPLICATION));

	return CGlasWindow::Create(WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, caption, rect);
}

BOOL CMainWnd::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The main view gets the command first
	if (m_wndMainView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CGlasWindow::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainWnd::AdjustLayout()
{
	if (!IsWindow(m_wndMainView))
		return;

	CRect rect;
	GetLayoutRect(rect);

	m_wndMainView.SetWindowPos(NULL, rect.left, rect.top+m_Margins.cyTopHeight, rect.Width(), rect.bottom-m_Margins.cyTopHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(CMainWnd, CGlasWindow)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()
	ON_WM_DESTROY()

	ON_UPDATE_COMMAND_UI_RANGE(ID_APP_VIEWOPTIONS, ID_VIEW_AUTODIRS, OnUpdateAppCommands)
	ON_UPDATE_COMMAND_UI(ID_VIEW_AUTODIRS, OnUpdateAppCommands)
	ON_UPDATE_COMMAND_UI_RANGE(ID_NAV_BACK, ID_NAV_RELOAD, OnUpdateNavCommands)

	ON_COMMAND(ID_APP_SORTOPTIONS, OnSortOptions)
	ON_COMMAND(ID_APP_VIEWOPTIONS, OnViewOptions)
	ON_COMMAND(ID_VIEW_AUTODIRS, OnToggleAutoDirs)

	ON_COMMAND(ID_NAV_BACK, OnNavigateBack)
	ON_COMMAND(ID_NAV_FORWARD, OnNavigateForward)
	ON_COMMAND(ID_NAV_RELOAD, OnNavigateReload)

	ON_COMMAND(IDM_ITEM_OPEN, OnItemOpen)

	ON_MESSAGE_VOID(WM_UPDATEVIEWOPTIONS, OnUpdateViewOptions)
	ON_MESSAGE_VOID(WM_UPDATESORTOPTIONS, OnUpdateSortOptions)
	ON_MESSAGE_VOID(WM_RELOAD, OnNavigateReload)
	ON_MESSAGE(WM_COOKFILES, OnCookFiles)
	ON_MESSAGE_VOID(WM_UPDATEFOOTER, OnUpdateFooter)

	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DrivesChanged, OnDrivesChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoreAttributesChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
END_MESSAGE_MAP()

INT CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlasWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	theApp.AddFrame(this);

	// Hauptansicht erstellen
	if (!m_wndMainView.Create(m_IsClipboard, this, 3))
		return -1;

	// Aero
	MARGINS Margins = { 0, 0, 50+11, 0 };
	UseGlasBackground(Margins);

	// Entweder leeres Suchergebnis oder Stores-Kontext öffnen
	RawFiles = m_IsClipboard ? LFAllocSearchResult(LFContextClipboard) : LFQuery(ActiveFilter);
	OnCookFiles();

	m_wndMainView.SetFocus();
	AdjustLayout();
	return 0;
}

void CMainWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	theApp.m_pMainWnd = this;

	if (IsWindow(m_wndMainView))
		m_wndMainView.SetFocus();
}

void CMainWnd::OnDestroy()
{
	CGlasWindow::OnDestroy();
	theApp.KillFrame(this);
}

void CMainWnd::OnSortOptions()
{
	SortOptionsDlg dlg(this, ActiveContextID);
	if (dlg.DoModal()==IDOK)
		theApp.UpdateSortOptions(ActiveContextID);
}

void CMainWnd::OnViewOptions()
{
	ViewOptionsDlg dlg(this, ActiveContextID);
	if (dlg.DoModal()==IDOK)
		theApp.UpdateViewOptions(ActiveContextID);
}

void CMainWnd::OnToggleAutoDirs()
{
	ActiveViewParameters->AutoDirs = (!ActiveViewParameters->AutoDirs);
	theApp.UpdateSortOptions(ActiveContextID);
}

void CMainWnd::OnUpdateAppCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_VIEW_AUTODIRS:
		pCmdUI->SetCheck((ActiveViewParameters->AutoDirs) || (ActiveContextID>=LFContextSubfolderDefault));
		pCmdUI->Enable((theApp.m_Contexts[ActiveContextID]->AllowGroups) && (ActiveViewParameters->Mode<=LFViewPreview));
		break;
	}
}


BOOL CMainWnd::AddClipItem(LFItemDescriptor* i)
{
	ASSERT(m_IsClipboard);

	for (UINT a=0; a<RawFiles->m_ItemCount; a++)
		if ((strcmp(i->StoreID, RawFiles->m_Items[a]->StoreID)==0) &&
			(strcmp(i->CoreAttributes.FileID, RawFiles->m_Items[a]->CoreAttributes.FileID)==0))
			return FALSE;

	LFAddItemDescriptor(RawFiles, LFAllocItemDescriptor(i));
	return TRUE;
}



void CMainWnd::UpdateSearchResult(BOOL SetEmpty, FVPersistentData* Data)
{
	if ((!SetEmpty) && (CookedFiles))
	{
		ActiveContextID = CookedFiles->m_Context;
		ActiveViewParameters = &theApp.m_Views[ActiveContextID];
	}

	m_wndMainView.UpdateSearchResult(SetEmpty ? NULL : RawFiles, SetEmpty ? NULL : CookedFiles, Data);

	ActiveViewID = ActiveViewParameters->Mode;
}

BOOL CMainWnd::UpdateSelectedItems(LFVariantData* value1, LFVariantData* value2, LFVariantData* value3)
{
	return m_wndMainView.UpdateItems(value1, value2, value3);
}

void CMainWnd::OnNavigateBack()
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

void CMainWnd::OnNavigateForward()
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

void CMainWnd::OnNavigateReload()
{
	if (ActiveFilter)
	{
		FVPersistentData Data;
		m_wndMainView.GetPersistentData(Data);
		NavigateTo(LFAllocFilter(ActiveFilter), NAVMODE_RELOAD, &Data);
	}
}

void CMainWnd::OnUpdateNavCommands(CCmdUI* pCmdUI)
{
	BOOL b = !m_IsClipboard;

	switch (pCmdUI->m_nID)
	{
	case ID_NAV_BACK:
		b &= (m_BreadcrumbBack!=NULL);
		break;
	case ID_NAV_FORWARD:
		b &= (m_BreadcrumbForward!=NULL);
		break;
	}

	pCmdUI->Enable(b);
}


void CMainWnd::NavigateTo(LFFilter* f, UINT NavMode, FVPersistentData* Data, INT FirstAggregate, INT LastAggregate)
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

	ActiveFilter->ShowEmptyDrives = (theApp.m_ShowEmptyDrives==TRUE);
	ActiveFilter->ShowEmptyDomains = (theApp.m_ShowEmptyDomains==TRUE);

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
		if (theApp.m_NagCounter!=0)
		{
			m_wndMainView.DismissNotification();
		}
}

void CMainWnd::UpdateHistory()
{
	if (RawFiles)
	{
		ActiveFilter->Result.FileCount = RawFiles->m_FileCount;
		ActiveFilter->Result.FileSize = RawFiles->m_FileSize;
	}
	if (CookedFiles)
		ActiveFilter->Result.ItemCount = CookedFiles->m_ItemCount;

//	if (m_wndHistory)
//		m_wndHistory->UpdateList(m_BreadcrumbBack, ActiveFilter, m_BreadcrumbForward);
}






void CMainWnd::OnItemOpen()
{
	INT idx = m_wndMainView.GetSelectedItem();
	if (idx!=-1)
	{
		LFItemDescriptor* i = CookedFiles->m_Items[idx];

		if (i->NextFilter)
		{
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






void CMainWnd::OnUpdateViewOptions()
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

void CMainWnd::OnUpdateSortOptions()
{
	m_wndMainView.SelectNone();

	FVPersistentData Data;
	m_wndMainView.GetPersistentData(Data);
	OnCookFiles((WPARAM)&Data);

	ActiveViewID = ActiveViewParameters->Mode;
}

LRESULT CMainWnd::OnCookFiles(WPARAM wParam, LPARAM /*lParam*/)
{
	LFSearchResult* Victim = CookedFiles;

	LFViewParameters* vp = &theApp.m_Views[RawFiles->m_Context];
	LFAttributeDescriptor* attr = theApp.m_Attributes[vp->SortBy];

	if (((!m_IsClipboard) && (vp->AutoDirs) && (!ActiveFilter->Options.IsSubfolder)) || (vp->Mode>LFViewPreview))
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

	if ((CookedFiles->m_LastError<=LFCancel) && (!LFIsLicensed()))
		if ((++theApp.m_NagCounter)>25)
		{
			CString tmpStr;
			ENSURE(tmpStr.LoadString(IDS_NOLICENSE));
			m_wndMainView.ShowNotification(ENT_INFO, tmpStr, ID_APP_PURCHASE);

			theApp.m_NagCounter = 0;
		}

	return CookedFiles->m_LastError;
}

void CMainWnd::OnUpdateFooter()
{
	m_wndMainView.UpdateFooter();
}


LRESULT CMainWnd::OnDrivesChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
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

LRESULT CMainWnd::OnStoresChanged(WPARAM /*wParam*/, LPARAM lParam)
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

LRESULT CMainWnd::OnStoreAttributesChanged(WPARAM wParam, LPARAM lParam)
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
