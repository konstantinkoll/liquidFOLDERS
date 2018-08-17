
// MainWnd.cpp: Implementierung der Klasse CMainWnd
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


// CMainWnd
//

CIcons CMainWnd::m_LargeIcons;
CIcons CMainWnd::m_SmallIcons;

const UINT CMainWnd::m_ContextOrder[LFLastQueryContext+1] = {
	LFContextAllFiles, LFContextFavorites,
	LFContextAudio, LFContextBooks, LFContextMovies, LFContextMusic, LFContextPictures, LFContextPodcasts, LFContextTVShows, LFContextVideos,
	LFContextDocuments, LFContextFonts, LFContextContacts, LFContextMessages, LFContextApps,
	LFContextNew, LFContextTasks, LFContextArchive, LFContextTrash,
	LFContextFilters
};

CMainWnd::CMainWnd()
	: CBackstageWnd()
{
	m_pActiveFilter = NULL;
	m_pBreadcrumbBack = m_pBreadcrumbForward = NULL;
	m_pRawFiles = m_pCookedFiles = NULL;

	ZeroMemory(&m_Statistics, sizeof(LFStatistics));
	m_StatisticsID[0] = '\0';
	m_StatisticsResult = LFCancel;
}

BOOL CMainWnd::Create(LFFilter* pFilter, BOOL IsClipboard)
{
	ASSERT(pFilter || IsClipboard);

	// Filter and clipboard
	if (((m_pActiveFilter=pFilter)!=NULL) && pFilter->Query.Mode)
	{
		m_pBreadcrumbBack = new BreadcrumbItem;
		ZeroMemory(m_pBreadcrumbBack, sizeof(BreadcrumbItem));

		wcscpy_s((m_pBreadcrumbBack->pFilter=GetRootFilter())->Result.Name, 256, theApp.m_Contexts[LFContextStores].Name);
	}

	m_IsClipboard = IsClipboard;

	// Create window object
	CString className = AfxRegisterWndClass(CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW), NULL, theApp.LoadIcon(IsClipboard ? IDR_CLIPBOARD : IDR_APPLICATION));

	return CBackstageWnd::Create(WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, CString((LPCSTR)(IsClipboard ? IDR_CLIPBOARD : IDR_APPLICATION)), IsClipboard ? _T("Clipboard") : _T("Main"), IsClipboard ? CSize(-1, -1) : CSize(0, 0));
}

BOOL CMainWnd::Create(const ABSOLUTESTOREID& StoreID)
{
	// Filter
	LFFilter* pFilter = LFAllocFilter(LFFilterModeDirectoryTree);
	pFilter->Query.StoreID = StoreID;

	// Store name as filter name
	LFStoreDescriptor StoreDescriptor;
	if (LFGetStoreSettings(StoreID, StoreDescriptor)==LFOk)
		wcscpy_s(pFilter->Name, 256, StoreDescriptor.StoreName);

	return Create(pFilter);
}

BOOL CMainWnd::Create(LPCSTR IATACode)
{
	ASSERT(strlen(IATACode)==3);

	// Variant Data
	LFVariantData VData;
	LFInitVariantData(VData, LFAttrLocationIATA);

	strcpy_s(VData.IATACode, 4, IATACode);
	VData.IsNull = FALSE;

	// Filter with condition
	LFFilter* pFilter = LFAllocFilter();
	pFilter->Query.pConditionList = LFAllocFilterCondition(LFFilterCompareIsEqual, VData);

	// Airport name as filter name
	LFAirport* pAirport;
	if (LFIATAGetAirportByCode(IATACode, pAirport))
		MultiByteToWideChar(CP_ACP, 0, pAirport->Name, -1, pFilter->Name, 256);

	return Create(pFilter);
}

BOOL CMainWnd::Create(BYTE ContextID)
{
	ASSERT(ContextID<LFContextCount);

	// Filter
	LFFilter* pFilter = LFAllocFilter();
	pFilter->Query.Context = ContextID;

	// Store name as filter name
	wcscpy_s(pFilter->Name, 256, theApp.m_Contexts[ContextID].Name);

	return Create(pFilter);
}

BOOL CMainWnd::PreTranslateMessage(MSG* pMsg)
{
	// Filter
	if ((pMsg->message==WM_KEYDOWN) && IsWindow(m_wndSearch))
	{
		// Set focus
		if ((pMsg->wParam=='F') && (GetKeyState(VK_CONTROL)<0))
		{
			m_wndSearch.SetFocus();

			return TRUE;
		}

		if (pMsg->hwnd==m_wndSearch)
		{
			// Lose focus
			if (pMsg->wParam==VK_ESCAPE)
			{
				m_wndMainView.SetFocus();

				return TRUE;
			}

			// Start query
			if (pMsg->wParam==VK_RETURN)
			{
				LFFilter* pFilter = LFAllocFilter();
				m_wndSearch.GetWindowText(pFilter->Query.SearchTerm, 256);

				NavigateTo(pFilter);

				if (!m_IsClipboard)
					m_wndMainView.SetFocus();

				return TRUE;
			}
		}
	}

	// X-Buttons
	if (pMsg->message==WM_XBUTTONDOWN)
		switch (pMsg->wParam & (MK_XBUTTON1 | MK_XBUTTON2))
		{
		case MK_XBUTTON1:
			if (m_pBreadcrumbBack)
				SendMessage(WM_COMMAND, ID_NAV_BACK);

			return TRUE;

		case MK_XBUTTON2:
			if (m_pBreadcrumbForward)
				SendMessage(WM_COMMAND, ID_NAV_FORWARD);

			return TRUE;
		}

	return CBackstageWnd::PreTranslateMessage(pMsg);
}

INT CMainWnd::GetCaptionHeight(BOOL IncludeBottomMargin) const
{
	return m_IsClipboard ? CBackstageWnd::GetCaptionHeight(IncludeBottomMargin) : (m_ShowCaption || m_ShowExpireCaption) ? theApp.m_SmallBoldFont.GetFontHeight()+(IncludeBottomMargin ? 2 : 1)*BACKSTAGECAPTIONMARGIN : 0;
}

void CMainWnd::GetLayoutRect(LPRECT lpRect)
{
	CBackstageWnd::GetLayoutRect(lpRect);

	if (!m_IsClipboard)
	{
		const INT BarHeight = 2*BACKSTAGEBORDER+CBackstageBar::GetPreferredHeight();

		if (BarHeight>lpRect->top)
			lpRect->top = BarHeight;

		lpRect->top += GetCaptionHeight(FALSE);
	}
}

void CMainWnd::AdjustLayout(const CRect& rectLayout, UINT nFlags)
{
	CSize CaptionButtonMargins;
	GetCaptionButtonMargins(&CaptionButtonMargins);

	if (!m_IsClipboard)
	{
		ASSERT(IsWindow(m_wndJournalButton));
		ASSERT(IsWindow(m_wndHistory));
		ASSERT(IsWindow(m_wndSearch));

		CRect rectClient;
		GetClientRect(rectClient);

		const UINT CaptionHeight = GetCaptionHeight(FALSE);
		const UINT BarHeight = CBackstageBar::GetPreferredHeight();

		const UINT JournalWidth = m_wndJournalButton.GetPreferredWidth();
		m_wndJournalButton.SetWindowPos(NULL, BACKSTAGEBORDER, CaptionHeight+(rectLayout.top-CaptionHeight-BarHeight)/2, JournalWidth, BarHeight, nFlags);

		const UINT SearchWidth = max(250, (rectClient.right-CaptionButtonMargins.cx-JournalWidth)/3);
		m_wndHistory.SetWindowPos(NULL, BACKSTAGEBORDER*2+JournalWidth, CaptionHeight+(rectLayout.top-CaptionHeight-BarHeight)/2, rectClient.right-CaptionButtonMargins.cx-JournalWidth-SearchWidth-4*BACKSTAGEBORDER, BarHeight, nFlags);
		m_wndSearch.SetWindowPos(NULL, rectClient.right-CaptionButtonMargins.cx-SearchWidth-BACKSTAGEBORDER, CaptionHeight+(rectLayout.top-CaptionHeight-BarHeight)/2, SearchWidth, BarHeight, nFlags);
	}

	m_wndMainView.SetWindowPos(NULL, rectLayout.left, rectLayout.top, rectLayout.Width(), rectLayout.Height(), nFlags);
}

BOOL CMainWnd::AddClipItem(const LFItemDescriptor* pItemDescriptor, BOOL& First)
{
	ASSERT(m_IsClipboard);

	// Deselect all previous items
	if (First)
	{
		m_wndMainView.SelectNone();

		First = FALSE;
	}

	// Add item
	LFAddItem(m_pRawFiles, LFCloneItemDescriptor(pItemDescriptor));

	return TRUE;
}

void CMainWnd::UpdateHistory(UINT NavMode)
{
	// Error message as notification
	if (IsWindow(m_wndMainView))
		if (m_pCookedFiles->m_LastError>LFCancel)
		{
			m_wndMainView.ShowNotification((m_pCookedFiles->m_LastError<LFFirstFatalError) ? ENT_WARNING : ENT_ERROR, m_pCookedFiles->m_LastError, (m_pCookedFiles->m_LastError==LFIndexAccessError) || (m_pCookedFiles->m_LastError==LFIndexTableLoadError) ? IDM_STORES_RUNMAINTENANCE : 0);
		}
		else
		{
			if (NavMode<NAVMODE_RELOAD)
				m_wndMainView.DismissNotification();
		}

	// History bar
	if (!m_IsClipboard)
		m_wndHistory.SetHistory(m_pActiveFilter, m_pBreadcrumbBack);

	// Window title
	if (m_pActiveFilter)
		m_wndSearch.SetWindowText(m_pActiveFilter->Query.SearchTerm);
}

void CMainWnd::NavigateTo(LFFilter* pFilter, UINT NavMode, FVPersistentData* pPersistentData, INT AggregateFirst, INT AggregateLast)
{
	ASSERT(pFilter);

	// Open new window if current window is not navigable
	if (m_IsClipboard)
	{
		CMainWnd* pFrameWnd = new CMainWnd();
		pFrameWnd->Create(pFilter);
		pFrameWnd->ShowWindow(SW_SHOW);

		return;
	}

	// Navigate
	if (NavMode<NAVMODE_RELOAD)
		theApp.PlayNavigateSound();

	// Filter
	if (NavMode==NAVMODE_NORMAL)
	{
		DeleteBreadcrumbItems(m_pBreadcrumbForward);

		FVPersistentData Data;
		m_wndMainView.GetPersistentData(Data);

		// Go to root
		if ((pFilter->IsPersistent || (pFilter->Query.Mode==LFFilterModeQuery)) && !pFilter->IsSubfolder)
			while (m_pBreadcrumbBack && m_pActiveFilter)
			{
				ASSERT(m_pActiveFilter!=pFilter);
				LFFreeFilter(m_pActiveFilter);

				ConsumeBreadcrumbItem(m_pBreadcrumbBack, m_pActiveFilter, Data);
			}

		AddBreadcrumbItem(m_pBreadcrumbBack, m_pActiveFilter, Data);
	}
	else
	{
		if (m_pActiveFilter!=pFilter)			// NAVMODE_RELOAD can pass the same filter!
			LFFreeFilter(m_pActiveFilter);
	}

	m_pActiveFilter = pFilter;

	// Detach view from search result
	if (NavMode<NAVMODE_CONTEXT)
		m_wndMainView.UpdateSearchResult(NULL, NULL, NULL);

	// Query
	if (m_pRawFiles && (AggregateFirst!=-1) && (AggregateLast!=-1))
	{
		m_pRawFiles = LFQueryEx(pFilter, m_pRawFiles, AggregateFirst, AggregateLast);
	}
	else
	{
		if (m_pRawFiles!=m_pCookedFiles)
			LFFreeSearchResult(m_pRawFiles);

		m_pRawFiles = LFQuery(m_pActiveFilter);
	}

	// Cook search result
	OnCookFiles((WPARAM)pPersistentData);

	// Update histroy
	UpdateHistory(NavMode);
}

void CMainWnd::LeafBreadcrumbs(BreadcrumbItem*& pAddItem, BreadcrumbItem*& pConsumeItem, UINT Pages)
{
	ASSERT(!m_IsClipboard);

	// Active filter
	LFFilter* pFilter = m_pActiveFilter;
	m_pActiveFilter = NULL;

	// Current state
	FVPersistentData Data;
	m_wndMainView.GetPersistentData(Data);

	// Leaf through breadcrumbs
	for (UINT a=0; a<Pages; a++)
		LeafBreadcrumbItem(pAddItem, pConsumeItem, pFilter, Data);

	// Navigate to new filter
	NavigateTo(pFilter, NAVMODE_HISTORY, &Data);
}


BEGIN_MESSAGE_MAP(CMainWnd, CBackstageWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_EN_SETFOCUS(3, OnSearchSetFocus)
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()

	ON_COMMAND(ID_NAV_BACK, OnNavigateBack)
	ON_COMMAND(ID_NAV_FORWARD, OnNavigateForward)
	ON_COMMAND(ID_NAV_RELOAD, OnNavigateReload)
	ON_UPDATE_COMMAND_UI_RANGE(ID_NAV_BACK, ID_NAV_RELOAD, OnUpdateNavigateCommands)

	ON_COMMAND_RANGE(IDM_NAV_SWITCHCONTEXT, IDM_NAV_SWITCHCONTEXT+LFLastQueryContext, OnSwitchContext)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_NAV_SWITCHCONTEXT, IDM_NAV_SWITCHCONTEXT+LFLastQueryContext, OnUpdateSwitchContextCommands)

	ON_MESSAGE(WM_NAVIGATEBACK, OnNavigateBack)
	ON_MESSAGE(WM_NAVIGATETO, OnNavigateTo)
	ON_MESSAGE(WM_COOKFILES, OnCookFiles)
	ON_MESSAGE_VOID(WM_UPDATEVIEWSETTINGS, OnUpdateViewSettings)
	ON_MESSAGE(WM_CONTEXTVIEWCOMMAND, OnContextViewCommand)
	ON_MESSAGE_VOID(WM_UPDATESIDEBAR, OnUpdateSidebar)
	ON_NOTIFY(REQUEST_TOOLTIP_DATA, 4, OnRequestTooltipData)

	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->ItemsDropped, OnItemsDropped)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoreAttributesChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StatisticsChanged, OnStatisticsChanged)
END_MESSAGE_MAP()

INT CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBackstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	if (!m_IsClipboard)
	{
		// Journal button
		if (!m_wndJournalButton.Create(this, 1))
			return -1;

		// History
		if (!m_wndHistory.Create(this, 2))
			return -1;

		// Search term
		if (!m_wndSearch.Create(this, 3, CString((LPCSTR)IDS_SEARCHTERM)))
			return -1;

		// Sidebar
		if (m_wndSidebar.Create(this, m_LargeIcons, m_SmallIcons, IDB_CONTEXTS_16, 4, TRUE))
		{
			ASSERT((sizeof(m_ContextOrder)/sizeof(UINT))==LFLastQueryContext+1);

			for (UINT a=0; a<=LFLastQueryContext; a++)
			{
				const UINT Context = m_ContextOrder[a];

				switch (Context)
				{
				case LFContextAudio:
					m_wndSidebar.AddCaption(IDS_FILETYPES);
					break;

				case LFContextDocuments:
					m_wndSidebar.AddCaption();
					break;

				case LFContextNew:
					m_wndSidebar.AddCaption(IDS_HOUSEKEEPING);
					break;

				case LFContextFilters:
					m_wndSidebar.AddCaption(theApp.m_Contexts[LFContextFilters].Name);
					break;
				}

				m_wndSidebar.AddCommand(IDM_NAV_SWITCHCONTEXT+Context, Context, theApp.m_Contexts[Context].Name, (Context==LFContextNew) ? 0xFF6020 : (Context==LFContextTrash) ? 0x383030 : (COLORREF)-1);
			}

			SetSidebar(&m_wndSidebar);
		}
	}

	// Create main view
	if (!m_wndMainView.Create(this, 5, m_IsClipboard))
		return -1;

	// Start query or create blank search result for clipboard
	m_pRawFiles = m_IsClipboard ? LFAllocSearchResult(LFContextClipboard) : LFQuery(m_pActiveFilter);
	OnCookFiles();

	UpdateHistory(NAVMODE_NORMAL);
	SetFocus();

	// Clipboard
	if (m_IsClipboard)
	{
		theApp.p_ClipboardWnd = this;

		// Taskbar
		DisableTaskbarPinning(L"app.liquidFOLDERS.liquidFOLDERS.Clipboard");
	}

	return 0;
}

void CMainWnd::OnDestroy()
{
	CBackstageWnd::OnDestroy();

	if (theApp.p_ClipboardWnd==this)
		theApp.p_ClipboardWnd = NULL;

	DeleteBreadcrumbItems(m_pBreadcrumbBack);
	DeleteBreadcrumbItems(m_pBreadcrumbForward);

	if (m_pCookedFiles!=m_pRawFiles)
		LFFreeSearchResult(m_pCookedFiles);

	LFFreeSearchResult(m_pRawFiles);
	LFFreeFilter(m_pActiveFilter);
}

void CMainWnd::OnSetFocus(CWnd* /*pOldWnd*/)
{
	theApp.m_pMainWnd = this;
	theApp.m_pActiveWnd = NULL;

	if (IsWindow(m_wndMainView))
		m_wndMainView.SetFocus();
}

void CMainWnd::OnSearchSetFocus()
{
	m_wndSearch.PostMessage(EM_SETSEL, (WPARAM)0, (LPARAM)-1);
}

void CMainWnd::OnMeasureItem(INT nIDCtl, LPMEASUREITEMSTRUCT lpmis)
{
	m_wndMainView.SendMessage(WM_MEASUREITEM, (WPARAM)nIDCtl, (LPARAM)lpmis);
}

void CMainWnd::OnDrawItem(INT nIDCtl, LPDRAWITEMSTRUCT lpdis)
{
	m_wndMainView.SendMessage(WM_DRAWITEM, (WPARAM)nIDCtl, (LPARAM)lpdis);
}


// Navigation

void CMainWnd::OnNavigateBack()
{
	LeafBreadcrumbs(m_pBreadcrumbForward, m_pBreadcrumbBack);
}

void CMainWnd::OnNavigateForward()
{
	LeafBreadcrumbs(m_pBreadcrumbBack, m_pBreadcrumbForward);
}

void CMainWnd::OnNavigateReload()
{
	ASSERT(!m_IsClipboard);

	// Current state
	FVPersistentData Data;
	m_wndMainView.GetPersistentData(Data);

	// Navigate to new (old) filter
	NavigateTo(m_pActiveFilter, NAVMODE_RELOAD, &Data);
}

void CMainWnd::OnUpdateNavigateCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = !m_IsClipboard;

	switch (pCmdUI->m_nID)
	{
	case ID_NAV_BACK:
		bEnable &= (m_pBreadcrumbBack!=NULL);
		break;

	case ID_NAV_FORWARD:
		bEnable &= (m_pBreadcrumbForward!=NULL);
		break;
	}

	pCmdUI->Enable(bEnable);
}


// Switch context

void CMainWnd::OnSwitchContext(UINT nID)
{
	nID -= IDM_NAV_SWITCHCONTEXT;

	DeleteBreadcrumbItems(m_pBreadcrumbForward);

	// Exit persistent filter or subfolder
	while (m_pBreadcrumbBack && (!m_pActiveFilter || m_pActiveFilter->IsPersistent || m_pActiveFilter->IsSubfolder))
	{
		LFFreeFilter(m_pActiveFilter);

		FVPersistentData Data;
		ConsumeBreadcrumbItem(m_pBreadcrumbBack, m_pActiveFilter, Data);
	}

	// Create filter
	LFFilter* pFilter = LFAllocFilter();
	pFilter->Query.Context = (BYTE)nID;

	if (m_pActiveFilter->Query.StoreID[0])
	{
		wcscpy_s(pFilter->Name, 256, m_pActiveFilter->Name);
		pFilter->Query.StoreID = m_pActiveFilter->Query.StoreID;
	}

	ASSERT(LFFilterModeStores==0);
	if (m_pActiveFilter->Query.Mode)
		pFilter->Query.Mode = m_pActiveFilter->Query.Mode;

	// Navigate to new filter
	NavigateTo(pFilter, m_pBreadcrumbBack ? NAVMODE_CONTEXT : NAVMODE_NORMAL);

	SetFocus();
}

void CMainWnd::OnUpdateSwitchContextCommands(CCmdUI* pCmdUI)
{
	ASSERT(pCmdUI->m_nID>=IDM_NAV_SWITCHCONTEXT);
	ASSERT(pCmdUI->m_nID<=IDM_NAV_SWITCHCONTEXT+LFLastQueryContext);

	BOOL bEnable = !m_IsClipboard;

	const UINT Context = pCmdUI->m_nID-IDM_NAV_SWITCHCONTEXT;
	if ((Context!=LFContextAllFiles) && (Context!=LFContextFilters))
		bEnable &= (m_Statistics.FileCount[Context]>0);

	pCmdUI->Enable(bEnable);
}


// App messages

LRESULT CMainWnd::OnNavigateBack(WPARAM wParam, LPARAM /*lParam*/)
{
	LeafBreadcrumbs(m_pBreadcrumbForward, m_pBreadcrumbBack, (UINT)wParam);

	return NULL;
}

LRESULT CMainWnd::OnNavigateTo(WPARAM wParam, LPARAM lParam)
{
	LFItemDescriptor* pItemDescriptor = (LFItemDescriptor*)lParam;

	NavigateTo((LFFilter*)wParam, NAVMODE_NORMAL, NULL, pItemDescriptor ? pItemDescriptor->AggregateFirst : -1, pItemDescriptor ? pItemDescriptor->AggregateLast : -1);

	return NULL;
}

LRESULT CMainWnd::OnCookFiles(WPARAM wParam, LPARAM /*lParam*/)
{
	LFSearchResult* pVictim = m_pCookedFiles;
	m_pCookedFiles = m_pRawFiles;

	// Sort or group?
	LFContextViewSettings* pContextViewSettings = &theApp.m_ContextViewSettings[m_pRawFiles->m_Context];

	if (pContextViewSettings->View!=LFViewList)
		if ((!m_IsClipboard && theApp.m_Contexts[m_pRawFiles->m_Context].CtxProperties.AllowGroups && !m_pActiveFilter->IsSubfolder) || (pContextViewSettings->View>LFViewDetails))
		{
			m_pCookedFiles = LFGroupSearchResult(m_pRawFiles,
				pContextViewSettings->SortBy, CookSortDescending(pContextViewSettings),
				CookGroupSingle(pContextViewSettings),
				m_pActiveFilter);
		}
		else
		{
			LFSortSearchResult(m_pRawFiles, pContextViewSettings->SortBy, pContextViewSettings->SortDescending);
		}

	m_wndMainView.UpdateSearchResult(m_pActiveFilter, m_pRawFiles, m_pCookedFiles, (FVPersistentData*)wParam);

	if (!m_IsClipboard)
	{
		INT Context = m_wndMainView.GetContext();
		if ((Context>LFLastQueryContext) && m_pActiveFilter && m_pActiveFilter->IsSubfolder && m_pBreadcrumbBack)
			Context = m_pBreadcrumbBack->pFilter->Result.Context;

		if (m_pSidebarWnd)
		{
			if ((m_StatisticsID!=GetStatisticsID()) || (m_StatisticsResult!=LFOk))
			{
				m_StatisticsID = GetStatisticsID();

				OnUpdateSidebar();
			}

			m_pSidebarWnd->SetSelection(IDM_NAV_SWITCHCONTEXT+Context);
		}
	}

	if (pVictim && (pVictim!=m_pRawFiles))
		LFFreeSearchResult(pVictim);

	return m_pCookedFiles->m_LastError;
}

void CMainWnd::OnUpdateViewSettings()
{
	// Cook raw files when view has changed
	if (m_wndMainView.GetViewID()!=(INT)theApp.m_ContextViewSettings[m_wndMainView.GetContext()].View)
	{
		m_wndMainView.SelectNone();

		OnCookFiles();
	}
	else
	{
		m_wndMainView.UpdateViewSettings();
	}
}

LRESULT CMainWnd::OnContextViewCommand(WPARAM wParam, LPARAM lParam)
{
	if ((LOWORD(lParam)==m_wndMainView.GetContext()) || (LOWORD(lParam)==0xFFFF))
		if ((HIWORD(lParam)==m_wndMainView.GetViewID()) || (HIWORD(lParam)==0xFFFF))
			return SendMessage((UINT)wParam);

	return NULL;
}

void CMainWnd::OnUpdateSidebar()
{
	UINT64 GlobalContextSet;
	m_StatisticsResult = LFQueryStatistics(m_Statistics, m_StatisticsID, &GlobalContextSet);

	if (m_pSidebarWnd)
	{
		for (UINT a=0; a<=LFLastQueryContext; a++)
			m_pSidebarWnd->UpdateItem(IDM_NAV_SWITCHCONTEXT+a, m_Statistics.FileCount[a], 
				(GlobalContextSet & (1ull<<a)) || (a<=LFContextFilters) || (a>LFLastPersistentContext),
				(a==LFContextTasks) ? PriorityColor() : (COLORREF)-1);

		// Immediately update sidebar
		m_pSidebarWnd->SendMessage(WM_IDLEUPDATECMDUI);
	}
}

void CMainWnd::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	const UINT Context = pTooltipData->Item-IDM_NAV_SWITCHCONTEXT;

	wcscpy_s(pTooltipData->Hint, 4096, theApp.m_Contexts[Context].Comment);

	if (Context<=LFLastQueryContext)
		if (m_Statistics.FileCount[Context])
		{
			if (pTooltipData->Hint[0])
				wcscat_s(pTooltipData->Hint, 4096, L"\n");

			const SIZE_T Length = wcslen(pTooltipData->Hint);
			LFGetFileSummary(&pTooltipData->Hint[Length], 4096-Length, m_Statistics.FileCount[Context], m_Statistics.FileSize[Context]);
		}

	*pResult = TRUE;
}


// Global messages

LRESULT CMainWnd::OnItemsDropped(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_pCookedFiles)
		switch (m_pCookedFiles->m_Context)
		{
		case LFContextClipboard:
			PostMessage(WM_COOKFILES);

		case LFContextStores:
			break;

		default:
			Reload();
			break;
		}

	return NULL;
}

LRESULT CMainWnd::OnStoresChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_pCookedFiles && (m_pCookedFiles->m_Context==LFContextStores))
		Reload();

	if (m_wndMainView.GetStoreID()[0]=='\0')
		PostMessage(WM_UPDATESIDEBAR);

	return NULL;
}

LRESULT CMainWnd::OnStoreAttributesChanged(WPARAM wParam, LPARAM lParam)
{
	if (m_pCookedFiles)
		if (m_pCookedFiles->m_Context<=LFLastQueryContext)
		{
			m_wndMainView.PostMessage(theApp.p_MessageIDs->StoreAttributesChanged, wParam, lParam);
		}
		else
		{
			if (m_pCookedFiles->m_Context==LFContextStores)
				Reload();
		}

	return NULL;
}

LRESULT CMainWnd::OnStatisticsChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_pCookedFiles && (m_pCookedFiles->m_Context==LFContextStores))
		Reload();

	PostMessage(WM_UPDATESIDEBAR);

	return NULL;
}
