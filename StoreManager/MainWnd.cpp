
// MainWnd.cpp: Implementierung der Klasse CMainWnd
//

#include "stdafx.h"
#include "StoreManager.h"
#include "LFCommDlg.h"


LFFilter* GetRootFilter(CHAR* RootStore=NULL)
{
	LFFilter* f = LFAllocFilter();
	f->Mode = RootStore ? LFFilterModeStoreHome : LFFilterModeStores;
	f->ShowEmptyVolumes = (theApp.m_ShowEmptyVolumes==TRUE);
	f->ShowEmptyDomains = (theApp.m_ShowEmptyDomains==TRUE);
	f->Options.AddVolumes = true;

	if (RootStore)
	{
		strcpy_s(f->StoreID, LFKeySize, RootStore);

		LFStoreDescriptor s;
		if (LFGetStoreSettings(RootStore, &s)==LFOk)
			wcscpy_s(f->Name, 256, s.StoreName);
	}
	else
	{
		wcscpy_s(f->Name, 256, theApp.m_Contexts[LFContextStores]->Name);
	}

	return f;
}

void WriteTXTItem(CStdioFile& f, LFItemDescriptor* i)
{
	for (UINT attr=0; attr<LFAttributeCount; attr++)
	{
		LFVariantData v;
		v.Attr = attr;
		LFGetAttributeVariantData(i, &v);

		if (!LFIsNullVariantData(&v))
		{
			WCHAR tmpBuf[256];
			LFVariantDataToString(&v, tmpBuf, 256);

			CString tmpStr = theApp.m_Attributes[attr]->Name;
			tmpStr.Append(_T(": "));
			tmpStr.Append(tmpBuf);
			tmpStr.Append(_T("\n"));

			f.WriteString(tmpStr);
		}
	}
}

void WriteXMLItem(CStdioFile& f, LFItemDescriptor* i)
{
	CString Type(_T("unknown"));
	switch (i->Type & LFTypeMask)
	{
	case LFTypeStore:
		Type = _T("store");
		break;
	case LFTypeVolume:
		Type = _T("volume");
		break;
	case LFTypeVirtual:
		Type = _T("virtual");
		break;
	case LFTypeFile:
		Type = _T("file");
		break;
	}

	f.WriteString(_T("\t<item type=\"")+Type+_T("\">\n"));

	for (UINT attr=0; attr<LFAttributeCount; attr++)
	{
		LFVariantData v;
		v.Attr = attr;
		LFGetAttributeVariantData(i, &v);

		if (!LFIsNullVariantData(&v))
		{
			WCHAR tmpBuf[256];
			LFVariantDataToString(&v, tmpBuf, 256);

			CString tmpStr;
			tmpStr.Format(_T("\t\t<property name=\"%s\" id=\"%d\">%s</property>\n"), theApp.m_Attributes[attr]->XMLID, attr, tmpBuf);

			f.WriteString(tmpStr);
		}
	}

	f.WriteString(_T("\t</item>\n"));
}


// CMainWnd
//

CMainWnd::CMainWnd()
{
	m_pActiveFilter = NULL;
	m_pRawFiles = m_pCookedFiles = NULL;
	m_BreadcrumbBack = m_BreadcrumbForward = NULL;
}

CMainWnd::~CMainWnd()
{
	if (m_pActiveFilter)
		LFFreeFilter(m_pActiveFilter);
	if (m_pCookedFiles!=m_pRawFiles)
		LFFreeSearchResult(m_pCookedFiles);
	LFFreeSearchResult(m_pRawFiles);
	DeleteBreadcrumbs(&m_BreadcrumbBack);
	DeleteBreadcrumbs(&m_BreadcrumbForward);
}

BOOL CMainWnd::Create(BOOL IsClipboard)
{
	m_IsClipboard = IsClipboard;

	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW), NULL, theApp.LoadIcon(IsClipboard ? IDR_CLIPBOARD : IDR_APPLICATION));

	CRect rect;
	SystemParametersInfo(SPI_GETWORKAREA, NULL, &rect, NULL);
	rect.DeflateRect(32, 32);

	CString caption;
	ENSURE(caption.LoadString(IsClipboard ? IDR_CLIPBOARD : IDR_APPLICATION));

	return CGlassWindow::Create(WS_MINIMIZEBOX | WS_MAXIMIZEBOX, className, caption, rect);
}

BOOL CMainWnd::CreateClipboard()
{
	return Create(TRUE);
}

BOOL CMainWnd::CreateRoot()
{
	m_pActiveFilter = GetRootFilter();

	return Create(FALSE);
}

BOOL CMainWnd::CreateStore(CHAR* RootStore)
{
	ASSERT(RootStore);

	m_BreadcrumbBack = new BreadcrumbItem();
	ZeroMemory(m_BreadcrumbBack, sizeof(BreadcrumbItem));
	m_BreadcrumbBack->filter = GetRootFilter();

	m_pActiveFilter = GetRootFilter(RootStore);

	return Create(FALSE);
}

BOOL CMainWnd::CreateFilter(LFFilter* f)
{
	m_pActiveFilter = f;
	if (!m_pActiveFilter)
		return FALSE;

	return Create(FALSE);
}

BOOL CMainWnd::CreateFilter(WCHAR* FileName)
{
	m_pActiveFilter = LFLoadFilter(FileName);
	if (!m_pActiveFilter)
		return FALSE;

	return Create(FALSE);
}

BOOL CMainWnd::PreTranslateMessage(MSG* pMsg)
{
	if ((pMsg->message==WM_KEYDOWN) && (pMsg->wParam==VK_RETURN) && (pMsg->hwnd==m_wndSearch))
	{
		theApp.ShowNagScreen(NAG_EXPIRED | NAG_FORCE, this);

		LFFilter* f = LFAllocFilter();
		f->Mode = LFFilterModeSearch;
		f->Options.IsSearch = true;
		m_wndSearch.GetWindowText(f->Searchterm, 256);
		m_wndSearch.SetWindowText(_T(""));

		SendMessage(WM_NAVIGATETO, (WPARAM)f);
		m_wndMainView.SetFocus();

		return TRUE;
	}

	return CGlassWindow::PreTranslateMessage(pMsg);
}

BOOL CMainWnd::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The main view gets the command first
	if (m_wndMainView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CGlassWindow::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
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
	const UINT SearchWidth = max(150, (rect.Width()-JournalWidth)/4);
	m_wndHistory.SetWindowPos(NULL, rect.left+JournalWidth+7, rect.top+(m_Margins.cyTopHeight-HistoryHeight-3)/2, rect.Width()-JournalWidth-SearchWidth-14, HistoryHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndSearch.SetWindowPos(NULL, rect.right-SearchWidth, rect.top+(m_Margins.cyTopHeight-HistoryHeight-3)/2, SearchWidth, HistoryHeight, SWP_NOACTIVATE | SWP_NOZORDER);

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

void CMainWnd::NavigateTo(LFFilter* f, UINT NavMode, FVPersistentData* Data, INT FirstAggregate, INT LastAggregate)
{
	ASSERT(f);

	// Open new window if current window is not navigable
	if (m_IsClipboard)
	{
		CMainWnd* pFrame = new CMainWnd();
		pFrame->CreateFilter(f);
		pFrame->ShowWindow(SW_SHOW);

		return;
	}

	// Navigate
	if (NavMode<NAVMODE_RELOAD)
		theApp.PlayNavigateSound();

	if (m_pActiveFilter)
		if (NavMode==NAVMODE_NORMAL)
		{
			DeleteBreadcrumbs(&m_BreadcrumbForward);

			FVPersistentData Data;
			m_wndMainView.GetPersistentData(Data);

			if ((f->Options.IsSearch) && (!f->Options.IsSubfolder))
				while (m_pActiveFilter ? m_pActiveFilter->Options.IsSearch : false)
				{
					LFFreeFilter(m_pActiveFilter);
					ConsumeBreadcrumbItem(&m_BreadcrumbBack, &m_pActiveFilter, &Data);
				}

			AddBreadcrumbItem(&m_BreadcrumbBack, m_pActiveFilter, Data);
		}
		else
		{
			LFFreeFilter(m_pActiveFilter);
		}

	m_pActiveFilter = f;
	m_pActiveFilter->ShowEmptyVolumes = (theApp.m_ShowEmptyVolumes==TRUE);
	m_pActiveFilter->ShowEmptyDomains = (theApp.m_ShowEmptyDomains==TRUE);

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
		m_pRawFiles = LFQuery(f, m_pRawFiles, FirstAggregate, LastAggregate);
		if ((pVictim) && (pVictim!=m_pRawFiles))
			LFFreeSearchResult(pVictim);
	}
	else
	{
		if (pVictim)
			LFFreeSearchResult(pVictim);
		m_pRawFiles = LFQuery(m_pActiveFilter);
	}

	OnCookFiles((WPARAM)Data);
	UpdateHistory();

	if (m_pCookedFiles->m_LastError>LFCancel)
	{
		m_wndMainView.ShowNotification(m_pCookedFiles->m_LastError==LFDriveWriteProtected ? ENT_WARNING : ENT_ERROR, m_pCookedFiles->m_LastError, m_pCookedFiles->m_LastError==LFIndexAccessError ? IDM_STORES_REPAIRCORRUPTEDINDEX : 0);
	}
	else
	{
		m_wndMainView.DismissNotification();
	}
}

void CMainWnd::UpdateHistory()
{
	if (!m_IsClipboard)
		m_wndHistory.SetHistory(m_pActiveFilter, m_BreadcrumbBack);
}


BEGIN_MESSAGE_MAP(CMainWnd, CGlassWindow)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_SETFOCUS()
	ON_WM_CLOSE()

	ON_COMMAND(ID_NAV_BACK, OnNavigateBack)
	ON_COMMAND(ID_NAV_FORWARD, OnNavigateForward)
	ON_COMMAND(ID_NAV_RELOAD, OnNavigateReload)
	ON_UPDATE_COMMAND_UI_RANGE(ID_NAV_BACK, ID_NAV_RELOAD, OnUpdateNavCommands)

	ON_COMMAND(IDM_ITEM_OPEN, OnItemOpen)
	ON_COMMAND(IDM_ITEM_OPENNEWWINDOW, OnItemOpenNewWindow)
	ON_COMMAND(IDM_INSPECTOR_EXPORTMETADATA, OnExportMetadata)

	ON_MESSAGE_VOID(WM_UPDATEVIEWOPTIONS, OnUpdateViewOptions)
	ON_MESSAGE_VOID(WM_UPDATESORTOPTIONS, OnUpdateSortOptions)
	ON_MESSAGE_VOID(WM_RELOAD, OnNavigateReload)
	ON_MESSAGE(WM_COOKFILES, OnCookFiles)
	ON_MESSAGE_VOID(WM_UPDATEFOOTER, OnUpdateFooter)
	ON_MESSAGE(WM_NAVIGATEBACK, OnNavigateBack)
	ON_MESSAGE(WM_NAVIGATETO, OnNavigateTo)

	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->VolumesChanged, OnVolumesChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoresChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoreAttributesChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->DefaultStoreChanged, OnStoresChanged)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->ItemsDropped, OnItemsDropped)

	ON_REGISTERED_MESSAGE(theApp.m_WakeupMsg, OnWakeup)
END_MESSAGE_MAP()

INT CMainWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CGlassWindow::OnCreate(lpCreateStruct)==-1)
		return -1;

	theApp.AddFrame(this);

	// History
	if (!m_wndHistory.Create(this, 2))
		return -1;

	// Journal-Button
	if (!m_wndJournalButton.Create(m_wndHistory.GetPreferredHeight(), this, 1))
		return -1;

	// Suchbegriff
	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_FILTER_SEARCHTERM));
	if (!m_wndSearch.Create(tmpStr, this, 3, TRUE))
		return -1;

	// Hauptansicht erstellen
	if (!m_wndMainView.Create(m_IsClipboard, this, 4))
		return -1;

	// Aero
	MARGINS Margins = { 0, 0, max(m_wndJournalButton.GetPreferredHeight()+8, m_wndHistory.GetPreferredHeight()+11), 0 };
	UseGlasBackground(Margins);

	m_GlasChildren.AddTail(&m_wndJournalButton);
	m_GlasChildren.AddTail(&m_wndHistory);
	m_GlasChildren.AddTail(&m_wndSearch);

	// Entweder leeres Suchergebnis oder Stores-Kontext öffnen
	m_pRawFiles = m_IsClipboard ? LFAllocSearchResult(LFContextClipboard) : LFQuery(m_pActiveFilter);
	OnCookFiles();
	UpdateHistory();

	AdjustLayout();
	SetFocus();

	return 0;
}

void CMainWnd::OnDestroy()
{
	CGlassWindow::OnDestroy();
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
	OnNavigateBack(1);
}

LRESULT CMainWnd::OnNavigateBack(WPARAM wParam, LPARAM /*lParam*/)
{
	ASSERT(!m_IsClipboard);

	if (m_pActiveFilter)
	{
		LFFilter* f = m_pActiveFilter;
		FVPersistentData Data;
		m_wndMainView.GetPersistentData(Data);
		m_pActiveFilter = NULL;

		for (UINT a=0; a<(UINT)wParam; a++)
		{
			AddBreadcrumbItem(&m_BreadcrumbForward, f, Data);
			ConsumeBreadcrumbItem(&m_BreadcrumbBack, &f, &Data);
		}

		NavigateTo(f, NAVMODE_HISTORY, &Data);
	}

	return NULL;
}

void CMainWnd::OnNavigateForward()
{
	ASSERT(!m_IsClipboard);

	if (m_pActiveFilter)
	{
		LFFilter* f = m_pActiveFilter;
		FVPersistentData Data;
		m_wndMainView.GetPersistentData(Data);
		m_pActiveFilter = NULL;

		AddBreadcrumbItem(&m_BreadcrumbBack, f, Data);
		ConsumeBreadcrumbItem(&m_BreadcrumbForward, &f, &Data);

		NavigateTo(f, NAVMODE_HISTORY, &Data);
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
				case LFTypeVolume:
					SendMessage(WM_COMMAND, IDM_VOLUME_CREATENEWSTORE);
					break;
				case LFTypeFile:
					if (strcmp(i->CoreAttributes.FileFormat, "filter")==0)
					{
						LFFilter* f = LFLoadFilter(i);
						if (f)
						{
							theApp.ShowNagScreen(NAG_EXPIRED | NAG_FORCE, this);

							m_wndMainView.SetFilter(f);
							NavigateTo(f);
						}
					}
					else
					{
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
					}
					break;
				default:
					ASSERT(FALSE);
				}
			}
	}
}

void CMainWnd::OnItemOpenNewWindow()
{
	INT idx = m_wndMainView.GetSelectedItem();
	if (idx!=-1)
	{
		LFItemDescriptor* i = m_pCookedFiles->m_Items[idx];

		ASSERT((i->Type & LFTypeMask)==LFTypeStore);

		CMainWnd* pFrame = new CMainWnd();
		pFrame->CreateStore(i->StoreID);
		pFrame->ShowWindow(SW_SHOW);
	}
}

LRESULT CMainWnd::OnNavigateTo(WPARAM wParam, LPARAM /*lParam*/)
{
	NavigateTo((LFFilter*)wParam);

	return NULL;
}


void CMainWnd::WriteMetadataTXT(CStdioFile& f)
{
#define Spacer { if (First) { First = FALSE; } else { f.WriteString(_T("\n")); } }

	BOOL First = TRUE;
	INT idx = m_wndMainView.GetNextSelectedItem(-1);
	while (idx!=-1)
	{
		LFItemDescriptor* i = m_pCookedFiles->m_Items[idx];

		if (((i->Type & LFTypeMask)==LFTypeVirtual) && (i->FirstAggregate!=-1) && (i->LastAggregate!=-1))
		{
			for (INT a=i->FirstAggregate; a<=i->LastAggregate; a++)
			{
				Spacer;
				WriteTXTItem(f, m_pRawFiles->m_Items[a]);
			}
		}
		else
		{
			Spacer;
			WriteTXTItem(f, i);
		}

		idx = m_wndMainView.GetNextSelectedItem(idx);
	}
}

void CMainWnd::WriteMetadataXML(CStdioFile& f)
{
	f.WriteString(_T("<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\">\n<items>\n"));

	INT idx = m_wndMainView.GetNextSelectedItem(-1);
	while (idx!=-1)
	{
		LFItemDescriptor* i = m_pCookedFiles->m_Items[idx];
		if (((i->Type & LFTypeMask)==LFTypeVirtual) && (i->FirstAggregate!=-1) && (i->LastAggregate!=-1))
		{
			for (INT a=i->FirstAggregate; a<=i->LastAggregate; a++)
				WriteXMLItem(f, m_pRawFiles->m_Items[a]);
		}
		else
		{
			WriteXMLItem(f, i);
		}

		idx = m_wndMainView.GetNextSelectedItem(idx);
	}

	f.WriteString(_T("</items>\n"));
}

void CMainWnd::OnExportMetadata()
{
	CString Extensions;
	ENSURE(Extensions.LoadString(IDS_TXTFILEFILTER));
	Extensions += _T(" (*.txt)|*.txt|");

	CString tmpStr;
	ENSURE(tmpStr.LoadStringW(IDS_XMLFILEFILTER));
	Extensions += tmpStr+_T(" (*.xml)|*.xml||");

	CFileDialog dlg(FALSE, _T(".txt"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Extensions, this);
	if (dlg.DoModal()==IDOK)
	{
		CStdioFile f;
		if (!f.Open(dlg.GetPathName(), CFile::modeCreate | CFile::modeWrite))
		{
			LFErrorBox(LFDriveNotReady, GetSafeHwnd());
		}
		else
		{
			try
			{
				if (dlg.GetFileExt()==_T("txt"))
				{
					WriteMetadataTXT(f);
				}
				else
					if (dlg.GetFileExt()==_T("xml"))
					{
						WriteMetadataXML(f);
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
	LFSearchResult* pVictim = m_pCookedFiles;

	LFViewParameters* vp = &theApp.m_Views[m_pRawFiles->m_Context];
	LFAttributeDescriptor* attr = theApp.m_Attributes[vp->SortBy];

	if (((!m_IsClipboard) && (vp->AutoDirs) && (!m_pActiveFilter->Options.IsSubfolder)) || (vp->Mode>LFViewPreview))
	{
		m_pCookedFiles = LFGroupSearchResult(m_pRawFiles, vp->SortBy, ((vp->Mode<=LFViewPreview) && (vp->Descending==TRUE)) || (vp->Mode==LFViewTimeline), attr->IconID,
			((vp->Mode>LFViewPreview) && (vp->Mode!=LFViewTimeline)) || ((attr->Type!=LFTypeTime) && (vp->SortBy!=LFAttrFileName) && (vp->SortBy!=LFAttrStoreID) && (vp->SortBy!=LFAttrFileID)),
			m_pActiveFilter);
	}
	else
	{
		LFSortSearchResult(m_pRawFiles, vp->SortBy, (vp->Mode<=LFViewPreview) && (vp->Descending==TRUE));
		m_pCookedFiles = m_pRawFiles;
	}

	m_wndMainView.UpdateSearchResult(m_pActiveFilter, m_pRawFiles, m_pCookedFiles, (FVPersistentData*)wParam);

	if ((pVictim) && (pVictim!=m_pRawFiles))
		LFFreeSearchResult(pVictim);

	if (m_pCookedFiles->m_LastError<=LFCancel)
		theApp.ShowNagScreen(NAG_EXPIRED, this);

	return m_pCookedFiles->m_LastError;
}

void CMainWnd::OnUpdateFooter()
{
	m_wndMainView.UpdateFooter();
}


LRESULT CMainWnd::OnVolumesChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
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

LRESULT CMainWnd::OnItemsDropped(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_pCookedFiles)
		switch (m_pCookedFiles->m_Context)
		{
		case LFContextStores:
			break;
		case LFContextClipboard:
			PostMessage(WM_COOKFILES);
			break;
		default:
			PostMessage(WM_RELOAD);
			break;
		}

	return NULL;
}


LRESULT CMainWnd::OnWakeup(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	CMainWnd* pFrame = new CMainWnd();
	pFrame->CreateRoot();
	pFrame->ShowWindow(SW_SHOW);

	return 24878;
}
