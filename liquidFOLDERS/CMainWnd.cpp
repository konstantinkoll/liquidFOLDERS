
// MainWnd.cpp: Implementierung der Klasse CMainWnd
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


// CMainWnd
//

CIcons CMainWnd::m_LargeIcons;
CIcons CMainWnd::m_SmallIcons;

CMainWnd::CMainWnd()
	: CBackstageWnd()
{
	m_pActiveFilter = NULL;
	m_pBreadcrumbBack = m_pBreadcrumbForward = NULL;
	m_pRawFiles = m_pCookedFiles = NULL;
	m_pStatistics = NULL;
	m_StatisticsID[0] = '\0';
}

CMainWnd::~CMainWnd()
{
	DeleteBreadcrumbs(&m_pBreadcrumbBack);
	DeleteBreadcrumbs(&m_pBreadcrumbForward);

	if (m_pActiveFilter)
		LFFreeFilter(m_pActiveFilter);

	if (m_pCookedFiles!=m_pRawFiles)
		LFFreeSearchResult(m_pCookedFiles);

	LFFreeSearchResult(m_pRawFiles);

	delete m_pStatistics;
}

BOOL CMainWnd::Create(BOOL IsClipboard)
{
	m_IsClipboard = IsClipboard;

	CString className = AfxRegisterWndClass(CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW), NULL, theApp.LoadIcon(IsClipboard ? IDR_CLIPBOARD : IDR_APPLICATION));

	CString Caption((LPCSTR)(IsClipboard ? IDR_CLIPBOARD : IDR_APPLICATION));

	return CBackstageWnd::Create(WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, Caption, IsClipboard ? _T("Clipboard") : _T("Main"), IsClipboard ? CSize(-1, -1) : CSize(0, 0));
}

BOOL CMainWnd::CreateRoot()
{
	m_pActiveFilter = GetRootFilter();

	return Create(FALSE);
}

BOOL CMainWnd::CreateStore(const CHAR* StoreID)
{
	ASSERT(StoreID);

	LFFilter* pFilter = LFAllocFilter();
	pFilter->Mode = LFFilterModeDirectoryTree;
	strcpy_s(pFilter->StoreID, LFKeySize, StoreID);
	pFilter->QueryContext = LFContextAuto;

	LFStoreDescriptor Store;
	if (LFGetStoreSettings(StoreID, &Store)==LFOk)
		wcscpy_s(pFilter->OriginalName, 256, Store.StoreName);

	return CreateFilter(pFilter);
}

BOOL CMainWnd::CreateFilter(LFFilter* pFilter)
{
	if (!pFilter)
		return FALSE;

	m_pActiveFilter = pFilter;

	m_pBreadcrumbBack = new BreadcrumbItem();
	ZeroMemory(m_pBreadcrumbBack, sizeof(BreadcrumbItem));
	m_pBreadcrumbBack->pFilter = GetRootFilter();

	return Create(FALSE);
}

LFFilter* CMainWnd::GetRootFilter()
{
	LFFilter* pFilter = LFAllocFilter();
	pFilter->Mode = LFFilterModeStores;

	wcscpy_s(pFilter->ResultName, 256, theApp.m_Contexts[LFContextStores].Name);

	return pFilter;
}

LPCSTR CMainWnd::GetStatisticsID() const
{
	ASSERT(m_pActiveFilter);

	return (m_pActiveFilter->Mode<LFFilterModeSearch) && IsWindow(m_wndMainView) ? m_wndMainView.GetStoreID() : "";
}

BOOL CMainWnd::PreTranslateMessage(MSG* pMsg)
{
	// Filter
	if (pMsg->message==WM_KEYDOWN)
		if (IsWindow(m_wndSearch))
		{
			// Set focus
			if ((pMsg->wParam=='F') && (GetKeyState(VK_CONTROL)<0))
			{
				m_wndSearch.SetFocus();

				return TRUE;
			}

			// Lose focus
			if ((pMsg->wParam==VK_ESCAPE) && (pMsg->hwnd==m_wndSearch))
			{
				m_wndMainView.SetFocus();

				return TRUE;
			}

			// Start search
			if ((pMsg->wParam==VK_RETURN) && (pMsg->hwnd==m_wndSearch))
			{
				LFFilter* pFilter = LFAllocFilter();
				pFilter->Mode = LFFilterModeSearch;
				m_wndSearch.GetWindowText(pFilter->Searchterm, 256);

				SendMessage(WM_NAVIGATETO, (WPARAM)pFilter);

				if (!m_IsClipboard)
					m_wndMainView.SetFocus();

				return TRUE;
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

BOOL CMainWnd::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The main view gets the command first
	if (m_wndMainView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CBackstageWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

INT CMainWnd::GetCaptionHeight(BOOL IncludeBottomMargin) const
{
	return m_IsClipboard ? CBackstageWnd::GetCaptionHeight(IncludeBottomMargin) : (m_ShowCaption || m_ShowExpireCaption) ? LFGetApp()->m_SmallBoldFont.GetFontHeight()+(IncludeBottomMargin ? 2 : 1)*BACKSTAGECAPTIONMARGIN : 0;
}

BOOL CMainWnd::GetLayoutRect(LPRECT lpRect) const
{
	CBackstageWnd::GetLayoutRect(lpRect);

	if (!m_IsClipboard)
	{
		INT BarHeight = 2*BACKSTAGEBORDER+CBackstageBar::GetPreferredHeight();

		if (BarHeight>lpRect->top)
			lpRect->top = BarHeight;

		lpRect->top += GetCaptionHeight(FALSE);
	}

	return TRUE;
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

BOOL CMainWnd::AddClipItem(LFItemDescriptor* pItemDescriptor)
{
	ASSERT(m_IsClipboard);

	for (UINT a=0; a<m_pRawFiles->m_ItemCount; a++)
		if ((strcmp(pItemDescriptor->StoreID, (*m_pRawFiles)[a]->StoreID)==0) &&
			(strcmp(pItemDescriptor->CoreAttributes.FileID, (*m_pRawFiles)[a]->CoreAttributes.FileID)==0))
			return FALSE;

	LFAddItem(m_pRawFiles, LFCloneItemDescriptor(pItemDescriptor));

	return TRUE;
}

void CMainWnd::NavigateTo(LFFilter* pFilter, UINT NavMode, FVPersistentData* Data, INT FirstAggregate, INT LastAggregate)
{
	ASSERT(pFilter);

	// Open new window if current window is not navigable
	if (m_IsClipboard)
	{
		CMainWnd* pFrameWnd = new CMainWnd();
		pFrameWnd->CreateFilter(pFilter);
		pFrameWnd->ShowWindow(SW_SHOW);

		return;
	}

	// Navigate
	if (NavMode<NAVMODE_RELOAD)
	{
		// Slide the filter pane away
		HideSidebar();

		theApp.PlayNavigateSound();
	}

	if (m_pActiveFilter)
		if (NavMode==NAVMODE_NORMAL)
		{
			DeleteBreadcrumbs(&m_pBreadcrumbForward);

			FVPersistentData Data;
			m_wndMainView.GetPersistentData(Data);

			// Goto root
			if ((pFilter->Options.IsPersistent || (pFilter->Mode==LFFilterModeSearch)) && (!pFilter->Options.IsSubfolder))
				while ((m_pBreadcrumbBack!=NULL) && (m_pActiveFilter!=NULL))
				{
					LFFreeFilter(m_pActiveFilter);
					ConsumeBreadcrumbItem(&m_pBreadcrumbBack, &m_pActiveFilter, &Data);
				}

			AddBreadcrumbItem(&m_pBreadcrumbBack, m_pActiveFilter, Data);
		}
		else
		{
			LFFreeFilter(m_pActiveFilter);
		}

	m_pActiveFilter = pFilter;

	if (NavMode<NAVMODE_RELOAD)
		m_wndMainView.UpdateSearchResult(NULL, NULL, NULL);

	INT OldContext = -1;
	LFSearchResult* pVictim = NULL;
	if (m_pRawFiles)
	{
		OldContext = m_pRawFiles->m_Context;
		if (m_pRawFiles!=m_pCookedFiles)
			pVictim = m_pRawFiles;
	}

	if ((m_pRawFiles) && (FirstAggregate!=-1) && (LastAggregate!=-1))
	{
		m_pRawFiles = LFQueryEx(pFilter, m_pRawFiles, FirstAggregate, LastAggregate);

		if ((pVictim) && (pVictim!=m_pRawFiles))
			LFFreeSearchResult(pVictim);
	}
	else
	{
		m_pRawFiles = LFQuery(m_pActiveFilter);

		if (pVictim)
			LFFreeSearchResult(pVictim);
	}

	OnCookFiles((WPARAM)Data);
	UpdateHistory();
}

void CMainWnd::UpdateHistory()
{
	if (IsWindow(m_wndMainView))
		if (m_pCookedFiles->m_LastError>LFCancel)
		{
			m_wndMainView.ShowNotification((m_pCookedFiles->m_LastError<=LFFirstFatalError) ? ENT_WARNING : ENT_ERROR, m_pCookedFiles->m_LastError, (m_pCookedFiles->m_LastError==LFIndexAccessError) || (m_pCookedFiles->m_LastError==LFIndexTableLoadError) ? IDM_STORES_RUNMAINTENANCE : 0);
		}
		else
		{
			m_wndMainView.DismissNotification();
		}

	if (!m_IsClipboard)
		m_wndHistory.SetHistory(m_pActiveFilter, m_pBreadcrumbBack);

	if (m_pActiveFilter)
		m_wndSearch.SetWindowText(m_pActiveFilter->Searchterm);
}

void CMainWnd::WriteTXTItem(CStdioFile& pFilter, LFItemDescriptor* pItemDescriptor)
{
	for (UINT Attr=0; Attr<LFAttributeCount; Attr++)
	{
		LFVariantData Value;
		LFGetAttributeVariantDataEx(pItemDescriptor, Attr, Value);

		if (!LFIsNullVariantData(Value))
		{
			WCHAR tmpBuf[256];
			LFVariantDataToString(Value, tmpBuf, 256);

			CString tmpStr(theApp.m_Attributes[Attr].Name);
			tmpStr.Append(_T(": "));
			tmpStr.Append(tmpBuf);
			tmpStr.Append(_T("\n"));

			pFilter.WriteString(tmpStr);
		}
	}
}

void CMainWnd::WriteXMLItem(CStdioFile& pFilter, LFItemDescriptor* pItemDescriptor)
{
	CString Type(_T("unknown"));
	switch (pItemDescriptor->Type & LFTypeMask)
	{
	case LFTypeStore:
		Type = _T("store");
		break;

	case LFTypeFolder:
		Type = _T("folder");
		break;

	case LFTypeFile:
		Type = _T("file");
		break;
	}

	pFilter.WriteString(_T("\t<Item type=\"")+Type+_T("\">\n"));

	for (UINT Attr=0; Attr<LFAttributeCount; Attr++)
	{
		LFVariantData v;
		LFGetAttributeVariantDataEx(pItemDescriptor, Attr, v);

		if (!LFIsNullVariantData(v))
		{
			WCHAR tmpBuf[256];
			LFVariantDataToString(v, tmpBuf, 256);

			CString tmpStr;
			tmpStr.Format(_T("\t\t<property name=\"%s\" id=\"%u\">%s</property>\n"), theApp.m_Attributes[Attr].XMLID, Attr, tmpBuf);

			pFilter.WriteString(tmpStr);
		}
	}

	pFilter.WriteString(_T("\t</Item>\n"));
}


BEGIN_MESSAGE_MAP(CMainWnd, CBackstageWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_GETMINMAXINFO()
	ON_WM_SETFOCUS()
	ON_EN_SETFOCUS(3, OnSearchSetFocus)
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()

	ON_COMMAND(ID_NAV_BACK, OnNavigateBack)
	ON_COMMAND(ID_NAV_FORWARD, OnNavigateForward)
	ON_COMMAND(ID_NAV_RELOAD, OnNavigateReload)
	ON_UPDATE_COMMAND_UI_RANGE(ID_NAV_BACK, ID_NAV_RELOAD, OnUpdateNavCommands)

	ON_COMMAND_RANGE(IDM_NAV_SWITCHCONTEXT, IDM_NAV_SWITCHCONTEXT+LFLastQueryContext, OnSwitchContext)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_NAV_SWITCHCONTEXT, IDM_NAV_SWITCHCONTEXT+LFLastQueryContext, OnUpdateSwitchContextCommands)

	ON_COMMAND(IDM_FILTERS_CREATENEW, OnFiltersCreateNew)

	ON_COMMAND(IDM_ITEM_OPEN, OnItemOpen)
	ON_COMMAND(IDM_ITEM_OPENNEWWINDOW, OnItemOpenNewWindow)
	ON_COMMAND(IDM_ITEM_OPENFILEDROP, OnItemOpenFileDrop)
	ON_COMMAND(IDM_INSPECTOR_EXPORTMETADATA, OnExportMetadata)

	ON_MESSAGE(WM_CONTEXTVIEWCOMMAND, OnContextViewCommand)
	ON_MESSAGE_VOID(WM_UPDATEVIEWOPTIONS, OnUpdateViewOptions)
	ON_MESSAGE_VOID(WM_UPDATESORTOPTIONS, OnUpdateSortOptions)
	ON_MESSAGE_VOID(WM_UPDATECOUNTS, OnUpdateCounts)
	ON_MESSAGE_VOID(WM_RELOAD, OnNavigateReload)
	ON_MESSAGE(WM_COOKFILES, OnCookFiles)
	ON_MESSAGE(WM_NAVIGATEBACK, OnNavigateBack)
	ON_MESSAGE(WM_NAVIGATETO, OnNavigateTo)

	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->ItemsDropped, OnItemsDropped)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoreAttributesChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StatisticsChanged, OnStatisticsChanged)

	ON_NOTIFY(REQUEST_TOOLTIP_DATA, 4, OnRequestTooltipData)
END_MESSAGE_MAP()

INT CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CBackstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	if (!m_IsClipboard)
	{
		// Journal-Button
		if (!m_wndJournalButton.Create(this, 1))
			return -1;

		// History
		if (!m_wndHistory.Create(this, 2))
			return -1;

		// Suchbegriff
		CString tmpStr((LPCSTR)IDS_SEARCHTERM);
		if (!m_wndSearch.Create(this, 3, tmpStr))
			return -1;

		// Sidebar
		if (m_wndSidebar.Create(this, m_LargeIcons, m_SmallIcons, IDB_CONTEXTS_16, 4, TRUE))
		{
			for (UINT a=0; a<=LFLastQueryContext; a++)
			{
				switch (a)
				{
				case 2:
					m_wndSidebar.AddCaption(IDS_FILETYPES);
					break;

				case LFContextDocuments:
					m_wndSidebar.AddCaption();
					break;

				case LFLastGroupContext+1:
					m_wndSidebar.AddCaption(IDS_HOUSEKEEPING);
					break;

				case LFContextFilters:
					m_wndSidebar.AddCaption(theApp.m_Contexts[LFContextFilters].Name);
					break;
				}

				m_wndSidebar.AddCommand(IDM_NAV_SWITCHCONTEXT+a, a, theApp.m_Contexts[a].Name, (a==LFContextNew) ? 0xFF6020 : (a==LFContextTrash) ? 0x0000FF : (COLORREF)-1);
			}

			SetSidebar(&m_wndSidebar);
		}
	}

	// Hauptansicht erstellen
	if (!m_wndMainView.Create(this, 5, m_IsClipboard))
		return -1;

	// Entweder leeres Suchergebnis oder Stores-Kontext öffnen
	m_pRawFiles = m_IsClipboard ? LFAllocSearchResult(LFContextClipboard) : LFQuery(m_pActiveFilter);
	OnCookFiles();
	UpdateHistory();
	SetFocus();

	// Clipboard
	if (m_IsClipboard)
		theApp.p_ClipboardWnd = this;

	return 0;
}

void CMainWnd::OnDestroy()
{
	CBackstageWnd::OnDestroy();

	if (theApp.p_ClipboardWnd==this)
		theApp.p_ClipboardWnd = NULL;
}

void CMainWnd::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CBackstageWnd::OnGetMinMaxInfo(lpMMI);

	if (IsWindow(*m_pSidebarWnd))
	{
		CRect rect;
		m_pSidebarWnd->GetWindowRect(rect);
		ScreenToClient(rect);

		lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, rect.top+m_pSidebarWnd->GetMinHeight()+BACKSTAGERADIUS+2+2);
	}
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
	OnNavigateBack(1);
}

LRESULT CMainWnd::OnNavigateBack(WPARAM wParam, LPARAM /*lParam*/)
{
	ASSERT(!m_IsClipboard);

	if (m_pActiveFilter)
	{
		LFFilter* pFilter = m_pActiveFilter;
		FVPersistentData Data;
		m_wndMainView.GetPersistentData(Data);
		m_pActiveFilter = NULL;

		for (UINT a=0; a<(UINT)wParam; a++)
		{
			AddBreadcrumbItem(&m_pBreadcrumbForward, pFilter, Data);
			ConsumeBreadcrumbItem(&m_pBreadcrumbBack, &pFilter, &Data);
		}

		NavigateTo(pFilter, NAVMODE_HISTORY, &Data);
	}

	return NULL;
}

void CMainWnd::OnNavigateForward()
{
	ASSERT(!m_IsClipboard);

	if (m_pActiveFilter)
	{
		LFFilter* pFilter = m_pActiveFilter;
		FVPersistentData Data;
		m_wndMainView.GetPersistentData(Data);
		m_pActiveFilter = NULL;

		AddBreadcrumbItem(&m_pBreadcrumbBack, pFilter, Data);
		ConsumeBreadcrumbItem(&m_pBreadcrumbForward, &pFilter, &Data);

		NavigateTo(pFilter, NAVMODE_HISTORY, &Data);
	}
}

void CMainWnd::OnNavigateReload()
{
	ASSERT(!m_IsClipboard);

	if (m_pActiveFilter)
	{
		FVPersistentData Data;
		m_wndMainView.GetPersistentData(Data);
		NavigateTo(LFAllocFilter(m_pActiveFilter), NAVMODE_RELOAD, &Data);
	}
}

void CMainWnd::OnUpdateNavCommands(CCmdUI* pCmdUI)
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

	DeleteBreadcrumbs(&m_pBreadcrumbForward);

	// Exit persistent filter or subfolder
	while ((m_pBreadcrumbBack!=NULL) && ((m_pActiveFilter!=NULL) ? (m_pActiveFilter->Options.IsPersistent || m_pActiveFilter->Options.IsSubfolder) : FALSE))
	{
		LFFreeFilter(m_pActiveFilter);

		FVPersistentData Data;
		ConsumeBreadcrumbItem(&m_pBreadcrumbBack, &m_pActiveFilter, &Data);
	}

	// Create filter
	LFFilter* pFilter = LFAllocFilter();
	pFilter->QueryContext = (BYTE)nID;

	if (m_pActiveFilter)
	{
		strcpy_s(pFilter->StoreID, LFKeySize, m_pActiveFilter->StoreID);

		if (pFilter->StoreID[0])
			wcscpy_s(pFilter->OriginalName, LFKeySize, m_pActiveFilter->OriginalName);

		pFilter->Mode = (m_pActiveFilter->Mode>LFFilterModeStores) ? m_pActiveFilter->Mode : LFFilterModeSearch;
	}
	else
	{
		pFilter->Mode = LFFilterModeSearch;
	}

	NavigateTo(pFilter, m_pBreadcrumbBack ? NAVMODE_RELOAD : NAVMODE_NORMAL);

	// Slide the filter pane away
	HideSidebar();

	SetFocus();
}

void CMainWnd::OnUpdateSwitchContextCommands(CCmdUI* pCmdUI)
{
	ASSERT(pCmdUI->m_nID>=IDM_NAV_SWITCHCONTEXT);
	ASSERT(pCmdUI->m_nID<=IDM_NAV_SWITCHCONTEXT+LFLastQueryContext);

	BOOL bEnable = !m_IsClipboard;

	const UINT Context = pCmdUI->m_nID-IDM_NAV_SWITCHCONTEXT;
	if ((Context!=LFContextAllFiles) && (Context!=LFContextFilters))
		bEnable &= m_pStatistics ? m_pStatistics->FileCount[Context]>0 : FALSE;

	pCmdUI->Enable(bEnable);
}


// Create new filter

void CMainWnd::OnFiltersCreateNew()
{
	HideSidebar();

	LFEditFilterDlg dlg(this, m_pActiveFilter ? m_pActiveFilter->StoreID : NULL);
	if (dlg.DoModal()==IDOK)
		OnNavigateReload();
}


// Global messages

void CMainWnd::OnItemOpen()
{
	INT Index = m_wndMainView.GetSelectedItem();
	if (Index!=-1)
	{
		LFItemDescriptor* pItemDescriptor = (*m_pCookedFiles)[Index];

		if (pItemDescriptor->NextFilter)
		{
			NavigateTo(LFAllocFilter(pItemDescriptor->NextFilter), NAVMODE_NORMAL, NULL, pItemDescriptor->FirstAggregate, pItemDescriptor->LastAggregate);
		}
		else
			if (pItemDescriptor->Type & LFTypeMounted)
			{
				WCHAR Path[MAX_PATH];
				UINT Result;

				switch (pItemDescriptor->Type & LFTypeMask)
				{
				case LFTypeFile:
					if (strcmp(pItemDescriptor->CoreAttributes.FileFormat, "filter")==0)
					{
						LFFilter* pFilter = LFLoadFilter(pItemDescriptor);

						if (pFilter)
							NavigateTo(pFilter);
					}
					else
					{
						Result = LFGetFileLocation(pItemDescriptor, Path, MAX_PATH, TRUE);
						if (Result==LFOk)
						{
							if (ShellExecute(NULL, _T("open"), Path, NULL, NULL, SW_SHOWNORMAL)==(HINSTANCE)SE_ERR_NOASSOC)
								SendMessage(WM_COMMAND, IDM_FILE_OPENWITH);
						}
						else
						{
							LFErrorBox(this, Result);
						}
					}

					break;
				}
			}
	}
}

void CMainWnd::OnItemOpenNewWindow()
{
	INT Index = m_wndMainView.GetSelectedItem();
	if (Index!=-1)
	{
		LFItemDescriptor* pItemDescriptor = (*m_pCookedFiles)[Index];

		ASSERT((pItemDescriptor->Type & LFTypeMask)==LFTypeStore);

		CMainWnd* pFrameWnd = new CMainWnd();
		pFrameWnd->CreateStore(pItemDescriptor->StoreID);
		pFrameWnd->ShowWindow(SW_SHOW);
	}
}

void CMainWnd::OnItemOpenFileDrop()
{
	if (m_wndMainView.GetContext()==LFContextStores)
	{
		INT Index = m_wndMainView.GetSelectedItem();
		if (Index!=-1)
		{
			LFItemDescriptor* pItemDescriptor = (*m_pCookedFiles)[Index];

			ASSERT((pItemDescriptor->Type & LFTypeMask)==LFTypeStore);
			theApp.GetFileDrop(pItemDescriptor->StoreID);
		}
	}
	else
		if (m_wndMainView.StoreIDValid())
		{
			theApp.GetFileDrop(m_wndMainView.GetStoreID());
		}
}

LRESULT CMainWnd::OnNavigateTo(WPARAM wParam, LPARAM /*lParam*/)
{
	NavigateTo((LFFilter*)wParam);

	return NULL;
}


void CMainWnd::WriteMetadataTXT(CStdioFile& pFilter) const
{
#define Spacer { if (First) { First = FALSE; } else { pFilter.WriteString(_T("\n")); } }

	BOOL First = TRUE;
	INT Index = m_wndMainView.GetNextSelectedItem(-1);
	while (Index!=-1)
	{
		LFItemDescriptor* pItemDescriptor = (*m_pCookedFiles)[Index];

		if (((pItemDescriptor->Type & LFTypeMask)==LFTypeFolder) && (pItemDescriptor->FirstAggregate!=-1) && (pItemDescriptor->LastAggregate!=-1))
		{
			for (INT a=pItemDescriptor->FirstAggregate; a<=pItemDescriptor->LastAggregate; a++)
			{
				Spacer;
				WriteTXTItem(pFilter, (*m_pRawFiles)[a]);
			}
		}
		else
		{
			Spacer;
			WriteTXTItem(pFilter, pItemDescriptor);
		}

		Index = m_wndMainView.GetNextSelectedItem(Index);
	}
}

void CMainWnd::WriteMetadataXML(CStdioFile& pFilter) const
{
	pFilter.WriteString(_T("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\">\n<items>\n"));

	INT Index = m_wndMainView.GetNextSelectedItem(-1);
	while (Index!=-1)
	{
		LFItemDescriptor* pItemDescriptor = (*m_pCookedFiles)[Index];
		if (((pItemDescriptor->Type & LFTypeMask)==LFTypeFolder) && (pItemDescriptor->FirstAggregate!=-1) && (pItemDescriptor->LastAggregate!=-1))
		{
			for (INT a=pItemDescriptor->FirstAggregate; a<=pItemDescriptor->LastAggregate; a++)
				WriteXMLItem(pFilter, (*m_pRawFiles)[a]);
		}
		else
		{
			WriteXMLItem(pFilter, pItemDescriptor);
		}

		Index = m_wndMainView.GetNextSelectedItem(Index);
	}

	pFilter.WriteString(_T("</items>\n"));
}

void CMainWnd::OnExportMetadata()
{
	CString Extensions((LPCSTR)IDS_TXTFILEFILTER);
	Extensions += _T(" (*.txt)|*.txt|");

	CString tmpStr((LPCSTR)IDS_XMLFILEFILTER);
	Extensions += tmpStr+_T(" (*.xml)|*.xml||");

	CFileDialog dlg(FALSE, _T(".txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
	{
		FILE *fStream;
		if (_tfopen_s(&fStream, dlg.GetPathName(), _T("wt,ccs=UTF-8")))
		{
			LFErrorBox(this, LFDriveNotReady);
		}
		else
		{
			CStdioFile pFilter(fStream);
			try
			{
				if (dlg.GetFileExt()==_T("txt"))
				{
					WriteMetadataTXT(pFilter);
				}
				else
					if (dlg.GetFileExt()==_T("xml"))
					{
						WriteMetadataXML(pFilter);
					}
			}
			catch(CFileException ex)
			{
				LFErrorBox(this, LFDriveNotReady);
			}
			pFilter.Close();
		}
	}
}


LRESULT CMainWnd::OnContextViewCommand(WPARAM wParam, LPARAM lParam)
{
	if ((LOWORD(lParam)==m_wndMainView.GetContext()) || (LOWORD(lParam)==0xFFFF))
		if ((HIWORD(lParam)==m_wndMainView.GetViewID()) || (HIWORD(lParam)==0xFFFF))
			return SendMessage((UINT)wParam);

	return NULL;
}

void CMainWnd::OnUpdateViewOptions()
{
	if ((m_wndMainView.GetViewID()>LFViewPreview) || (theApp.m_Views[m_wndMainView.GetContext()].Mode>LFViewPreview))
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

void CMainWnd::OnUpdateCounts()
{
	delete m_pStatistics;

	m_pStatistics = LFQueryStatistics(m_StatisticsID);

	if (m_pSidebarWnd)
		for (UINT a=0; a<=LFLastQueryContext; a++)
			m_pSidebarWnd->SetCount(IDM_NAV_SWITCHCONTEXT+a, m_pStatistics->FileCount[a]);
}

LRESULT CMainWnd::OnCookFiles(WPARAM wParam, LPARAM /*lParam*/)
{
	LFSearchResult* pVictim = m_pCookedFiles;

	LFViewParameters* vp = &theApp.m_Views[m_pRawFiles->m_Context];
	LFAttributeDescriptor* Attr = &theApp.m_Attributes[vp->SortBy];

	if (((!m_IsClipboard) && (vp->AutoDirs) && (!m_pActiveFilter->Options.IsSubfolder)) || (vp->Mode>LFViewPreview))
	{
		m_pCookedFiles = LFGroupSearchResult(m_pRawFiles, vp->SortBy, ((vp->Mode<=LFViewPreview) && vp->Descending) || (vp->Mode==LFViewTimeline),
			((vp->Mode>LFViewPreview) && (vp->Mode!=LFViewTimeline)) || ((Attr->Type!=LFTypeTime) && (vp->SortBy!=LFAttrFileName) && (vp->SortBy!=LFAttrStoreID) && (vp->SortBy!=LFAttrFileID)),
			m_pActiveFilter);
	}
	else
	{
		LFSortSearchResult(m_pRawFiles, vp->SortBy, vp->Descending);
		m_pCookedFiles = m_pRawFiles;
	}

	m_wndMainView.UpdateSearchResult(m_pActiveFilter, m_pRawFiles, m_pCookedFiles, (FVPersistentData*)wParam);

	if (!m_IsClipboard)
	{
		INT Context = m_wndMainView.GetContext();
		if (Context>LFLastQueryContext)
			if (m_pActiveFilter)
				if (m_pActiveFilter->Options.IsSubfolder && (m_pBreadcrumbBack!=NULL))
					Context = m_pBreadcrumbBack->pFilter->ResultContext;

		if (m_pSidebarWnd)
		{
			if ((strcmp(m_StatisticsID, GetStatisticsID())!=0) || !m_pStatistics)
			{
				strcpy_s(m_StatisticsID, LFKeySize, GetStatisticsID());

				OnUpdateCounts();
			}

			m_pSidebarWnd->SetSelection(IDM_NAV_SWITCHCONTEXT+Context);
		}
	}

	if ((pVictim) && (pVictim!=m_pRawFiles))
		LFFreeSearchResult(pVictim);

	return m_pCookedFiles->m_LastError;
}


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
			PostMessage(WM_RELOAD);
			break;
		}

	return NULL;
}

LRESULT CMainWnd::OnStoresChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_pCookedFiles)
		if (m_pCookedFiles->m_Context==LFContextStores)
			PostMessage(WM_RELOAD);

	if (m_wndMainView.GetStoreID()[0]=='\0')
		PostMessage(WM_UPDATECOUNTS);

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
			if (m_pCookedFiles->m_Context==LFContextStores)
			{
				PostMessage(WM_RELOAD);
			}

	return NULL;
}

LRESULT CMainWnd::OnStatisticsChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_pCookedFiles)
		if (m_pCookedFiles->m_Context==LFContextStores)
			PostMessage(WM_RELOAD);

	PostMessage(WM_UPDATECOUNTS);

	return NULL;
}


void CMainWnd::OnRequestTooltipData(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_TOOLTIPDATA* pTooltipData = (NM_TOOLTIPDATA*)pNMHDR;

	const UINT Context = pTooltipData->Item-IDM_NAV_SWITCHCONTEXT;

	wcscpy_s(pTooltipData->Hint, 4096, theApp.m_Contexts[Context].Comment);

	if (m_pStatistics && (Context<=LFLastQueryContext))
		if (m_pStatistics->FileCount[Context])
		{
			if (pTooltipData->Hint[0])
				wcscat_s(pTooltipData->Hint, 4096, L"\n");

			SIZE_T Length = wcslen(pTooltipData->Hint);
			LFCombineFileCountSize(m_pStatistics->FileCount[Context], m_pStatistics->FileSize[Context], &pTooltipData->Hint[Length], 4096-Length);
		}

	*pResult = TRUE;
}
