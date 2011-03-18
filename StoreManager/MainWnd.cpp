
// MainWnd.cpp: Implementierung der Klasse CMainWnd
//

#include "stdafx.h"
#include "StoreManager.h"
#include "LFCommDlg.h"


LFFilter* GetRootFilter(CHAR* RootStore=NULL)
{
	LFFilter* f = LFAllocFilter();
	f->Mode = RootStore ? LFFilterModeStoreHome : LFFilterModeStores;
	f->ShowEmptyDrives = (theApp.m_ShowEmptyDrives==TRUE);
	f->ShowEmptyDomains = (theApp.m_ShowEmptyDomains==TRUE);
	f->Options.AddDrives = true;

	if (RootStore)
		strcpy_s(f->StoreID, LFKeySize, RootStore);

	return f;
}


// CMainWnd
//

CMainWnd::CMainWnd()
{
	m_hIcon = NULL;
	ActiveFilter = NULL;
	m_pRawFiles = m_pCookedFiles = NULL;
	m_BreadcrumbBack = m_BreadcrumbForward = NULL;
}

CMainWnd::~CMainWnd()
{
	if (m_hIcon)
		DestroyIcon(m_hIcon);
	if (ActiveFilter)
		LFFreeFilter(ActiveFilter);
	if (m_pCookedFiles!=m_pRawFiles)
		LFFreeSearchResult(m_pCookedFiles);
	LFFreeSearchResult(m_pRawFiles);
	DeleteBreadcrumbs(&m_BreadcrumbBack);
	DeleteBreadcrumbs(&m_BreadcrumbForward);
}

BOOL CMainWnd::Create(BOOL IsClipboard, CHAR* RootStore)
{
	m_hIcon = theApp.LoadIcon(IsClipboard ? IDR_CLIPBOARD : IDR_APPLICATION);
	m_IsClipboard = IsClipboard;

	if (!IsClipboard)
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
	if (!IsWindow(m_wndJournalButton))
		return;
	if (!IsWindow(m_wndMainView))
		return;

	CRect rect;
	GetLayoutRect(rect);

	const UINT JournalHeight = m_wndJournalButton.GetPreferredHeight();
	const UINT JournalWidth = m_wndJournalButton.GetPreferredWidth();
	m_wndJournalButton.SetWindowPos(NULL, rect.left+1, rect.top+(m_Margins.cyTopHeight-JournalHeight-2)/2, JournalWidth, JournalHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	const UINT HistoryHeight = m_wndHistory.GetPreferredHeight();
	m_wndHistory.SetWindowPos(NULL, rect.left+JournalWidth+7, rect.top+(m_Margins.cyTopHeight-HistoryHeight-3)/2, rect.Width()-JournalWidth-7, HistoryHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	m_wndMainView.SetWindowPos(NULL, rect.left, rect.top+m_Margins.cyTopHeight, rect.Width(), rect.bottom-m_Margins.cyTopHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}

INT CMainWnd::GetContext()
{
	return m_wndMainView.GetContext();
}

INT CMainWnd::GetViewID()
{
	return m_wndMainView.GetViewID();
}

BOOL CMainWnd::AddClipItem(LFItemDescriptor* i)
{
	ASSERT(m_IsClipboard);

	for (UINT a=0; a<m_pRawFiles->m_ItemCount; a++)
		if ((strcmp(i->StoreID, m_pRawFiles->m_Items[a]->StoreID)==0) &&
			(strcmp(i->CoreAttributes.FileID, m_pRawFiles->m_Items[a]->CoreAttributes.FileID)==0))
			return FALSE;

	LFAddItemDescriptor(m_pRawFiles, LFAllocItemDescriptor(i));
	return TRUE;
}

BOOL CMainWnd::UpdateSelectedItems(LFVariantData* value1, LFVariantData* value2, LFVariantData* value3)
{
	return m_wndMainView.UpdateItems(value1, value2, value3);
}

void CMainWnd::NavigateTo(LFFilter* f, UINT NavMode, FVPersistentData* Data, INT FirstAggregate, INT LastAggregate)
{
	ASSERT(!m_IsClipboard);

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
	ActiveFilter->ShowEmptyDrives = (theApp.m_ShowEmptyDrives==TRUE);
	ActiveFilter->ShowEmptyDomains = (theApp.m_ShowEmptyDomains==TRUE);

	if (NavMode<NAVMODE_RELOAD)
		m_wndMainView.UpdateSearchResult(NULL, NULL, NULL);


	INT OldContext = -1;
	LFSearchResult* victim = NULL;

	if (m_pRawFiles)
	{
		OldContext = m_pRawFiles->m_Context;
		if (m_pRawFiles!=m_pCookedFiles)
			victim = m_pRawFiles;
	}

	if ((m_pRawFiles) && (FirstAggregate!=-1) && (LastAggregate!=-1))
	{
		m_pRawFiles = LFQuery(f, m_pRawFiles, FirstAggregate, LastAggregate);
		if ((victim) && (victim!=m_pRawFiles))
			LFFreeSearchResult(victim);
	}
	else
	{
		if (victim)
			LFFreeSearchResult(victim);
		m_pRawFiles = LFQuery(ActiveFilter);
	}

	OnCookFiles((WPARAM)Data);
	UpdateHistory();

	if (m_pCookedFiles->m_LastError>LFCancel)
	{
		m_wndMainView.ShowNotification(m_pCookedFiles->m_LastError==LFDriveWriteProtected ? ENT_WARNING : ENT_ERROR, m_pCookedFiles->m_LastError, m_pCookedFiles->m_LastError==LFIndexAccessError ? IDM_STORES_REPAIRCORRUPTEDINDEX : 0);
	}
	else
		if (theApp.m_NagCounter!=0)
		{
			m_wndMainView.DismissNotification();
		}
}

void CMainWnd::UpdateHistory()
{
	if (!m_IsClipboard)
		m_wndHistory.SetHistory(ActiveFilter, m_BreadcrumbBack);
}


BEGIN_MESSAGE_MAP(CMainWnd, CGlasWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()

	ON_COMMAND(ID_NAV_BACK, OnNavigateBack)
	ON_COMMAND(ID_NAV_FORWARD, OnNavigateForward)
	ON_COMMAND(ID_NAV_RELOAD, OnNavigateReload)
	ON_UPDATE_COMMAND_UI_RANGE(ID_NAV_BACK, ID_NAV_RELOAD, OnUpdateNavCommands)

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

	// History
	if (!m_wndHistory.Create(this, 2))
		return -1;

	// Journal-Button
	if (!m_wndJournalButton.Create(m_wndHistory.GetPreferredHeight(), this, 1))
		return -1;

	// Hauptansicht erstellen
	if (!m_wndMainView.Create(m_IsClipboard, this, 3))
		return -1;

	// Aero
	MARGINS Margins = { 0, 0, max(m_wndJournalButton.GetPreferredHeight()+8, m_wndHistory.GetPreferredHeight()+11), 0 };
	UseGlasBackground(Margins);

	// Entweder leeres Suchergebnis oder Stores-Kontext öffnen
	m_pRawFiles = m_IsClipboard ? LFAllocSearchResult(LFContextClipboard) : LFQuery(ActiveFilter);
	OnCookFiles();
	UpdateHistory();

	AdjustLayout();
	SetFocus();

	return 0;
}

void CMainWnd::OnDestroy()
{
	CGlasWindow::OnDestroy();
	theApp.KillFrame(this);
}

void CMainWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	theApp.m_pMainWnd = this;
	theApp.m_pActiveWnd = NULL;

	if (IsWindow(m_wndMainView))
		m_wndMainView.SetFocus();
}


// Navigation

void CMainWnd::OnNavigateBack()
{
	ASSERT(!m_IsClipboard);

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
	ASSERT(!m_IsClipboard);

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
	ASSERT(!m_IsClipboard);

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


// Global messages

void CMainWnd::OnItemOpen()
{
	INT idx = m_wndMainView.GetSelectedItem();
	if (idx!=-1)
	{
		LFItemDescriptor* i = m_pCookedFiles->m_Items[idx];

		if (i->NextFilter)
		{
			NavigateTo(LFAllocFilter(i->NextFilter), NAVMODE_NORMAL, NULL, i->FirstAggregate, i->LastAggregate);
		}
		else
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


void CMainWnd::OnUpdateViewOptions()
{
	if ((m_wndMainView.GetViewID()>LFViewPreview)!=(theApp.m_Views[m_wndMainView.GetContext()].Mode>LFViewPreview))
	{
		m_wndMainView.SelectNone();
		OnCookFiles();
	}
	else
	{
		m_wndMainView.UpdateViewOptions();
	}
}

void CMainWnd::OnUpdateSortOptions()
{
	m_wndMainView.SelectNone();

	FVPersistentData Data;
	m_wndMainView.GetPersistentData(Data);
	OnCookFiles((WPARAM)&Data);
}

LRESULT CMainWnd::OnCookFiles(WPARAM wParam, LPARAM /*lParam*/)
{
	LFSearchResult* Victim = m_pCookedFiles;

	LFViewParameters* vp = &theApp.m_Views[m_pRawFiles->m_Context];
	LFAttributeDescriptor* attr = theApp.m_Attributes[vp->SortBy];

	if (((!m_IsClipboard) && (vp->AutoDirs) && (!ActiveFilter->Options.IsSubfolder)) || (vp->Mode>LFViewPreview))
	{
		m_pCookedFiles = LFGroupSearchResult(m_pRawFiles, vp->SortBy, (vp->Mode<=LFViewPreview) && (vp->Descending==TRUE), attr->IconID,
			(vp->Mode>LFViewPreview) || ((attr->Type!=LFTypeTime) && (vp->SortBy!=LFAttrFileName) && (vp->SortBy!=LFAttrStoreID) && (vp->SortBy!=LFAttrFileID)),
			ActiveFilter);
	}
	else
	{
		LFSortSearchResult(m_pRawFiles, vp->SortBy, (vp->Mode<=LFViewPreview) && (vp->Descending==TRUE));
		m_pCookedFiles = m_pRawFiles;
	}

	m_wndMainView.UpdateSearchResult(m_pRawFiles, m_pCookedFiles, (FVPersistentData*)wParam);

	if ((Victim) && (Victim!=m_pRawFiles))
		LFFreeSearchResult(Victim);

	if ((m_pCookedFiles->m_LastError<=LFCancel) && (!LFIsLicensed()))
		if ((++theApp.m_NagCounter)>20)
		{
			CString tmpStr;
			ENSURE(tmpStr.LoadString(IDS_NOLICENSE));
			m_wndMainView.ShowNotification(ENT_INFO, tmpStr, ID_APP_PURCHASE);

			theApp.m_NagCounter = 0;
		}

	return m_pCookedFiles->m_LastError;
}

void CMainWnd::OnUpdateFooter()
{
	m_wndMainView.UpdateFooter();
}


LRESULT CMainWnd::OnDrivesChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_pCookedFiles)
		switch (m_pCookedFiles->m_Context)
		{
		case LFContextStores:
			PostMessage(WM_RELOAD);
			break;
		}

	return NULL;
}

LRESULT CMainWnd::OnStoresChanged(WPARAM /*wParam*/, LPARAM lParam)
{
	if ((m_pCookedFiles) && (GetSafeHwnd()!=(HWND)lParam))
		switch (m_pCookedFiles->m_Context)
		{
		case LFContextStores:
			PostMessage(WM_RELOAD);
			break;
		}

	return NULL;
}

LRESULT CMainWnd::OnStoreAttributesChanged(WPARAM wParam, LPARAM lParam)
{
	if ((m_pCookedFiles) && (GetSafeHwnd()!=(HWND)lParam))
		switch (m_pCookedFiles->m_Context)
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
