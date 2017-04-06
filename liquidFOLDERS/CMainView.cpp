
// CMainView.cpp: Implementierung der Klasse CMainView
//

#include "stdafx.h"
#include "CCalendarView.h"
#include "CGlobeView.h"
#include "CListView.h"
#include "CTagcloudView.h"
#include "CTimelineView.h"
#include "liquidFOLDERS.h"
#include "OrganizeDlg.h"


// CMainView
//

#define FileViewID     3

CIcons CMainView::m_LargeIcons;
CIcons CMainView::m_SmallIcons;

CMainView::CMainView()
	: CFrontstageWnd()
{
	m_pWndFileView = NULL;
	p_Filter = NULL;
	p_RawFiles = p_CookedFiles = NULL;
	p_InspectorButton = NULL;
	p_OrganizeButton = p_ViewButton = NULL;
	m_Context = m_ViewID = -1;
	m_Resizing = m_StoreIDValid = m_Alerted = FALSE;
}

BOOL CMainView::Create(CWnd* pParentWnd, UINT nID, BOOL IsClipboard)
{
	m_IsClipboard = IsClipboard;

	CString className = AfxRegisterWndClass(CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW));

	return CFrontstageWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, CRect(0, 0, 0, 0), pParentWnd, nID);
}

BOOL CMainView::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The file view gets the command first
	if (m_pWndFileView)
		if (m_pWndFileView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;

	// Check Inspector
	if (m_wndInspectorPane.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// Check application commands
	if (theApp.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CFrontstageWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CMainView::CreateFileView(UINT ViewID, FVPersistentData* pPersistentData)
{
	CFileView* pNewView = NULL;

	switch (ViewID)
	{
	case LFViewLargeIcons:
	case LFViewSmallIcons:
	case LFViewList:
	case LFViewDetails:
	case LFViewTiles:
	case LFViewStrips:
	case LFViewContent:
		if ((m_ViewID<LFViewLargeIcons) || (m_ViewID>LFViewContent))
			pNewView = new CListView();

		break;

	case LFViewCalendar:
		if (m_ViewID!=LFViewCalendar)
			pNewView = new CCalendarView();

		break;

	case LFViewTimeline:
		if (m_ViewID!=LFViewTimeline)
			pNewView = new CTimelineView();

		break;

	case LFViewGlobe:
		if (m_ViewID!=LFViewGlobe)
			pNewView = new CGlobeView();

		break;

	case LFViewTagcloud:
		if (m_ViewID!=LFViewTagcloud)
			pNewView = new CTagcloudView();

		break;
	}

	m_ViewID = ViewID;

	// Exchange view
	if (pNewView)
	{
		CRect rect;
		if (m_pWndFileView)
		{
			m_pWndFileView->GetWindowRect(rect);
			ScreenToClient(rect);
		}
		else
		{
			rect.SetRectEmpty();
		}

		pNewView->Create(this, FileViewID, rect, p_RawFiles, p_CookedFiles, pPersistentData);

		CFileView* pVictim = m_pWndFileView;

		m_pWndFileView = pNewView;
		m_pWndFileView->SetOwner(GetOwner());

		if ((GetFocus()==pVictim) || (GetTopLevelParent()==GetActiveWindow()))
			m_pWndFileView->SetFocus();

		RegisterDragDrop(m_pWndFileView->GetSafeHwnd(), &m_DropTarget);

		if (pVictim)
		{
			pVictim->DestroyWindow();
			delete pVictim;
		}
	}

	return (pNewView!=NULL);
}

void CMainView::SetHeaderButtons()
{
	ASSERT(p_OrganizeButton);
	ASSERT(p_ViewButton);

	p_OrganizeButton->SetValue(theApp.m_Attributes[theApp.m_Views[m_Context].SortBy].Name, FALSE);

	CString tmpStr((LPCSTR)IDM_VIEW_FIRST+m_ViewID);
	p_ViewButton->SetValue(tmpStr);
}

void CMainView::SetHeader()
{
	if (!p_CookedFiles)
	{
		m_wndHeaderArea.SetText(_T(""), _T(""));
	}
	else
	{
		CString Hint;

		if (m_Context==LFContextStores)
		{
			Hint.Format(p_CookedFiles->m_StoreCount==1 ? IDS_STORES_SINGULAR : IDS_STORES_PLURAL, p_CookedFiles->m_StoreCount);
		}
		else
		{
			WCHAR tmpStr[256];
			LFCombineFileCountSize(p_CookedFiles->m_FileCount, p_CookedFiles->m_FileSize, tmpStr, 256);

			Hint = tmpStr;
		}

		LFStoreDescriptor Store;
		WCHAR* pHint = (m_Context>LFContextAllFiles) ? p_CookedFiles->m_Hint : NULL;

		if ((m_Context<=LFLastQueryContext) && (m_StoreIDValid) && (!pHint))
			if (LFGetStoreSettings(m_StoreID, &Store)==LFOk)
			{
				wcscpy_s(p_RawFiles->m_Name, 256, Store.StoreName);
				wcscpy_s(p_CookedFiles->m_Name, 256, Store.StoreName);

				pHint = Store.Comments;
			}

		if (pHint)
			if (*pHint!=L'\0')
			{
				Hint.Insert(0, (GetThreadLocale() & 0x1FF)==LANG_ENGLISH ? _T("—") : _T(" – "));
				Hint.Insert(0, pHint);
			}

		m_wndHeaderArea.SetText(p_CookedFiles->m_Name, Hint, FALSE);
		SetHeaderButtons();

		GetOwner()->SetWindowText(p_CookedFiles->m_Name);
	}
}

void CMainView::UpdateViewOptions()
{
	if (m_pWndFileView)
	{
		FVPersistentData Data;
		GetPersistentData(Data);

		if (!CreateFileView(theApp.m_Views[m_Context].Mode, &Data))
			m_pWndFileView->UpdateViewOptions(m_Context);

		SetHeaderButtons();
	}
}

void CMainView::UpdateSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData, BOOL UpdateSelection)
{
	p_Filter = pFilter;
	p_RawFiles = pRawFiles;
	p_CookedFiles = pCookedFiles;

	if (!pFilter)
	{
		m_StoreIDValid = FALSE;
	}
	else
	{
		strcpy_s(m_StoreID, LFKeySize, pFilter->StoreID);
		m_StoreIDValid = (m_StoreID[0]!='\0');
	}

	if (!pCookedFiles)
	{
		if (m_pWndFileView)
			m_pWndFileView->UpdateSearchResult(NULL, NULL, NULL);

		RevokeDragDrop(m_wndHeaderArea.GetSafeHwnd());
		RevokeDragDrop(m_pWndFileView->GetSafeHwnd());
	}
	else
	{
		m_Context = pCookedFiles->m_Context;

		if (!CreateFileView(theApp.m_Views[m_Context].Mode, pPersistentData))
		{
			m_pWndFileView->UpdateViewOptions(m_Context);
			m_pWndFileView->UpdateSearchResult(pRawFiles, pCookedFiles, pPersistentData);
		}

		m_DropTarget.SetFilter(pFilter);
		RegisterDragDrop(m_wndHeaderArea.GetSafeHwnd(), &m_DropTarget);
		RegisterDragDrop(m_pWndFileView->GetSafeHwnd(), &m_DropTarget);
	}

	if (m_IsClipboard)
		m_DropTarget.SetSearchResult(pRawFiles);

	SetHeader();
	if (UpdateSelection)
		OnUpdateSelection();
}

void CMainView::ShowNotification(UINT Type, UINT Result, UINT Command)
{
	if (Result!=LFOk)
	{
		WCHAR tmpStr[256];
		LFGetErrorText(tmpStr, 256, Result);

		ShowNotification(Type, tmpStr, Command);
	}
}

void CMainView::ShowNotification(UINT Result)
{
	if (Result!=LFOk)
		ShowNotification((Result<LFFirstFatalError) ? ENT_WARNING : ENT_ERROR, Result);
}

void CMainView::AdjustLayout(UINT nFlags)
{
	m_Resizing = TRUE;

	CRect rect;
	GetClientRect(rect);

	const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
	m_wndTaskbar.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), TaskHeight, nFlags);

	const UINT NotificationHeight = m_wndExplorerNotification.GetPreferredHeight();
	m_wndExplorerNotification.SetWindowPos(&wndTop, rect.left+32, rect.bottom-NotificationHeight+1, rect.Width()-64, NotificationHeight, nFlags & ~(SWP_NOZORDER | SWP_NOOWNERZORDER));

	INT InspectorPaneWidth = 0;

	const INT MaxWidth = max(m_wndInspectorPane.GetMinWidth(), (rect.Width()-128)/2);
	if (MaxWidth>0)
	{
		InspectorPaneWidth = theApp.m_InspectorPaneWidth = min(MaxWidth, max(m_wndInspectorPane.GetMinWidth(), m_wndInspectorPane.GetPreferredWidth()));

		if (m_ShowInspectorPane)
		{
			m_wndInspectorPane.SetMaxWidth(MaxWidth);
			m_wndInspectorPane.SetWindowPos(NULL, rect.right-InspectorPaneWidth, rect.top+TaskHeight, InspectorPaneWidth, rect.Height()-TaskHeight, nFlags | (!m_wndInspectorPane.IsWindowVisible() ? SWP_SHOWWINDOW : 0));
		}
		else
		{
			if (m_wndInspectorPane.IsWindowVisible())
				m_wndInspectorPane.ShowWindow(SW_HIDE);

			InspectorPaneWidth = 0;
		}
	}

	const UINT HeaderHeight = m_wndHeaderArea.GetPreferredHeight();
	m_wndHeaderArea.SetWindowPos(NULL, rect.left, rect.top+TaskHeight, rect.Width()-InspectorPaneWidth, HeaderHeight, nFlags);

	if (m_pWndFileView)
		m_pWndFileView->SetWindowPos(NULL, rect.left, rect.top+TaskHeight+HeaderHeight, rect.Width()-InspectorPaneWidth, rect.Height()-HeaderHeight-TaskHeight, nFlags);

	m_Resizing = FALSE;
}

void CMainView::GetPersistentData(FVPersistentData& Data) const
{
	if (m_pWndFileView)
	{
		m_pWndFileView->GetPersistentData(Data);
	}
	else
	{
		ZeroMemory(&Data, sizeof(Data));
	}
}

void CMainView::SelectNone()
{
	if (m_pWndFileView)
		m_pWndFileView->SendMessage(WM_SELECTNONE);
}

void CMainView::AddTransactionItem(LFTransactionList* pTransactionList, LFItemDescriptor* pItemDescriptor, UINT_PTR UserData) const
{
	switch (pItemDescriptor->Type & LFTypeMask)
	{
	case LFTypeStore:
	case LFTypeFile:
		LFAddTransactionItem(pTransactionList, pItemDescriptor, UserData);
		break;

	case LFTypeFolder:
		if ((pItemDescriptor->FirstAggregate!=-1) && (pItemDescriptor->LastAggregate!=-1))
			for (INT a=pItemDescriptor->FirstAggregate; a<=pItemDescriptor->LastAggregate; a++)
				LFAddTransactionItem(pTransactionList, (*p_RawFiles)[a], UserData);

		break;
	}
}

LFTransactionList* CMainView::BuildTransactionList(BOOL All, BOOL ResolveLocations, BOOL IncludePIDL)
{
	LFTransactionList* pTransactionList = LFAllocTransactionList();

	if ((p_RawFiles) && (p_CookedFiles))
	{
		if (All)
		{
			for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
				AddTransactionItem(pTransactionList, (*p_CookedFiles)[a], a);
		}
		else
		{
			INT Index = GetNextSelectedItem(-1);
			while (Index!=-1)
			{
				AddTransactionItem(pTransactionList, (*p_CookedFiles)[Index], Index);
				Index = GetNextSelectedItem(Index);
			}
		}

		if (ResolveLocations)
		{
			LFDoTransaction(pTransactionList, LFTransactionTypeResolveLocations, NULL, IncludePIDL);
			ShowNotification(pTransactionList->m_LastError);
		}
	}

	return pTransactionList;
}

void CMainView::RemoveTransactedItems(LFTransactionList* pTransactionList)
{
	if (!p_RawFiles)
		return;

	for (UINT a=0; a<pTransactionList->m_ItemCount; a++)
		if (((*pTransactionList)[a].LastError==LFOk) && ((*pTransactionList)[a].Processed))
			(*pTransactionList)[a].pItemDescriptor->RemoveFlag = TRUE;

	LFRemoveFlaggedItems(p_RawFiles);

	FVPersistentData Data;
	GetPersistentData(Data);
	GetOwner()->SendMessage(WM_COOKFILES, (WPARAM)&Data);
}

BOOL CMainView::DeleteFiles(BOOL Trash, BOOL All)
{
	LFTransactionList* pTransactionList = BuildTransactionList(All);

	if (Trash)
	{
		CWaitCursor csr;

		LFDoTransaction(pTransactionList, LFTransactionTypePutInTrash);
	}
	else
	{
		WorkerParameters wp;
		ZeroMemory(&wp, sizeof(wp));
		wp.pTransactionList = pTransactionList;

		LFDoWithProgress(WorkerDelete, &wp.Hdr, this);
	}

	RemoveTransactedItems(pTransactionList);

	ShowNotification(pTransactionList->m_LastError);

	BOOL Modified = pTransactionList->m_Modified;
	LFFreeTransactionList(pTransactionList);

	return Modified;
}

void CMainView::RestoreFiles(BOOL All)
{
	CWaitCursor csr;

	LFTransactionList* pTransactionList = BuildTransactionList(All);
	LFDoTransaction(pTransactionList, LFTransactionTypeRestore);
	RemoveTransactedItems(pTransactionList);

	ShowNotification(pTransactionList->m_LastError);

	LFFreeTransactionList(pTransactionList);
}

BOOL CMainView::UpdateItems(LFVariantData* Value1, LFVariantData* Value2, LFVariantData* Value3)
{
	CWaitCursor csr;

	LFTransactionList* pTransactionList = BuildTransactionList();
	LFDoTransaction(pTransactionList, LFTransactionTypeUpdate, NULL, NULL, Value1, Value2, Value3);

	if (m_pWndFileView)
		if (pTransactionList->m_Modified)
		{
			FVPersistentData Data;
			GetPersistentData(Data);
			UpdateSearchResult(p_Filter, p_RawFiles, p_CookedFiles, &Data, FALSE);
		}

	ShowNotification(pTransactionList->m_LastError);

	BOOL Changes = pTransactionList->m_Modified;
	LFFreeTransactionList(pTransactionList);

	return Changes;
}

void CMainView::CreateShortcut(LFTransactionListItem* pItem)
{
	// Get a pointer to the IShellLink interface
	IShellLink* pShellLink = NULL;
	if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pShellLink)))
	{
		WCHAR Ext[LFExtSize+1] = L".*";
		WCHAR* Ptr = wcsrchr(pItem->Path, L'\\');
		if (!Ptr)
			Ptr = pItem->Path;

		WCHAR* LastExt = wcsrchr(Ptr, L'.');
		if (LastExt)
			wcscpy_s(Ext, LFExtSize+1, LastExt);

		pShellLink->SetIDList(pItem->pidlFQ);
		pShellLink->SetIconLocation(Ext, 0);
		pShellLink->SetShowCmd(SW_SHOWNORMAL);

		LFCreateDesktopShortcut(pShellLink, pItem->pItemDescriptor->CoreAttributes.FileName);

		pShellLink->Release();
	}
}


BEGIN_MESSAGE_MAP(CMainView, CFrontstageWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MEASUREITEM()
	ON_WM_DRAWITEM()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE_VOID(WM_ADJUSTLAYOUT, OnAdjustLayout)
	ON_MESSAGE_VOID(WM_UPDATESELECTION, OnUpdateSelection)
	ON_MESSAGE_VOID(WM_BEGINDRAGDROP, OnBeginDragDrop)
	ON_MESSAGE(WM_RENAMEITEM, OnRenameItem)
	ON_MESSAGE(WM_SENDTO, OnSendTo)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoreAttributesChanged)

	ON_COMMAND(ID_PANE_INSPECTOR, OnToggleInspector)
	ON_UPDATE_COMMAND_UI(ID_PANE_INSPECTOR, OnUpdatePaneCommands)

	ON_COMMAND(IDM_ORGANIZE_OPTIONS, OnSortOptions)
	ON_COMMAND(IDM_ORGANIZE_TOGGLEAUTODIRS, OnToggleAutoDirs)
	ON_UPDATE_COMMAND_UI(IDM_ORGANIZE_OPTIONS, OnUpdateHeaderCommands)
	ON_UPDATE_COMMAND_UI(IDM_ORGANIZE_TOGGLEAUTODIRS, OnUpdateHeaderCommands)
	ON_UPDATE_COMMAND_UI(IDM_VIEW, OnUpdateHeaderCommands)
	ON_UPDATE_COMMAND_UI(IDM_ORGANIZE, OnUpdateHeaderCommands)
	ON_MESSAGE(WM_GETMENU, OnGetMenu)
	ON_COMMAND_RANGE(IDM_ORGANIZE_FIRST, IDM_ORGANIZE_FIRST+LFAttributeCount-1, OnSort)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_ORGANIZE_FIRST, IDM_ORGANIZE_FIRST+LFAttributeCount-1, OnUpdateSortCommands)
	ON_COMMAND_RANGE(IDM_VIEW_FIRST, IDM_VIEW_FIRST+LFViewCount-1, OnView)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_VIEW_FIRST, IDM_VIEW_FIRST+LFViewCount-1, OnUpdateViewCommands)

	ON_COMMAND(IDM_STORES_ADD, OnStoresAdd)
	ON_COMMAND(IDM_STORES_SYNCHRONIZE, OnStoresSynchronize)
	ON_COMMAND(IDM_STORES_RUNMAINTENANCE, OnStoresRunMaintenance)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORES_ADD, IDM_STORES_RUNMAINTENANCE, OnUpdateStoresCommands)

	ON_COMMAND(IDM_NEW_CLEARNEW, OnNewClearNew)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_NEW_CLEARNEW, IDM_NEW_CLEARNEW, OnUpdateNewCommands)

	ON_COMMAND(IDM_TRASH_EMPTY, OnTrashEmpty)
	ON_COMMAND(IDM_TRASH_RESTOREALL, OnTrashRestoreAll)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_TRASH_EMPTY, IDM_TRASH_RESTOREALL, OnUpdateTrashCommands)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILTERS_CREATENEW, IDM_FILTERS_EDIT, OnUpdateFiltersCommands)

	ON_UPDATE_COMMAND_UI(IDM_ITEM_OPEN, OnUpdateItemCommands)

	ON_COMMAND(IDM_STORE_SYNCHRONIZE, OnStoreSynchronize)
	ON_COMMAND(IDM_STORE_MAKEDEFAULT, OnStoreMakeDefault)
	ON_COMMAND(IDM_STORE_IMPORTFOLDER, OnStoreImportFolder)
	ON_COMMAND(IDM_STORE_SHORTCUT, OnStoreShortcut)
	ON_COMMAND(IDM_STORE_DELETE, OnStoreDelete)
	ON_COMMAND(IDM_STORE_RENAME, OnStoreRename)
	ON_COMMAND(IDM_STORE_PROPERTIES, OnStoreProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORE_SYNCHRONIZE, IDM_STORE_PROPERTIES, OnUpdateStoreCommands)

	ON_COMMAND(IDM_FILE_OPENWITH, OnFileOpenWith)
	ON_COMMAND(IDM_FILE_OPENBROWSER, OnFileOpenBrowser)
	ON_COMMAND(IDM_FILE_EDIT, OnFileEdit)
	ON_COMMAND(IDM_FILE_REMEMBER, OnFileRemember)
	ON_COMMAND(IDM_FILE_REMOVE, OnFileRemove)
	ON_COMMAND(IDM_FILE_ARCHIVE, OnFileArchive)
	ON_COMMAND(IDM_FILE_COPY, OnFileCopy)
	ON_COMMAND(IDM_FILE_SHORTCUT, OnFileShortcut)
	ON_COMMAND(IDM_FILE_DELETE, OnFileDelete)
	ON_COMMAND(IDM_FILE_RENAME, OnFileRename)
	ON_COMMAND(IDM_FILE_PROPERTIES, OnFileProperties)
	ON_COMMAND(IDM_FILE_RESTORE, OnFileRestore)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILE_OPENWITH, IDM_FILE_RESTORE, OnUpdateFileCommands)
END_MESSAGE_MAP()

INT CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Taskbar
	if (!m_wndTaskbar.Create(this, m_LargeIcons, m_SmallIcons, IDB_TASKS_16, 1))
		return -1;

	m_wndTaskbar.SetOwner(GetOwner());

	m_wndTaskbar.AddButton(IDM_BACKSTAGE_TOGGLESIDEBAR, 0, TRUE, FALSE, TRUE);
	m_wndTaskbar.AddButton(IDM_STORES_ADD, 1);
	m_wndTaskbar.AddButton(IDM_STORES_SYNCHRONIZE, 2);
	m_wndTaskbar.AddButton(IDM_NEW_CLEARNEW, 3, TRUE);
	m_wndTaskbar.AddButton(IDM_TRASH_EMPTY, 4, TRUE);
	m_wndTaskbar.AddButton(IDM_TRASH_RESTOREALL, 5, TRUE);
	m_wndTaskbar.AddButton(IDM_FILE_RESTORE, 6);
	m_wndTaskbar.AddButton(IDM_FILTERS_CREATENEW, 7);
	m_wndTaskbar.AddButton(IDM_CALENDAR_PREVYEAR, 8, TRUE);
	m_wndTaskbar.AddButton(IDM_CALENDAR_NEXTYEAR, 9, TRUE);
	m_wndTaskbar.AddButton(IDM_CALENDAR_GOTOYEAR, 10);
	m_wndTaskbar.AddButton(IDM_GLOBE_JUMPTOLOCATION, 11, TRUE);
	m_wndTaskbar.AddButton(IDM_GLOBE_ZOOMIN, 12);
	m_wndTaskbar.AddButton(IDM_GLOBE_ZOOMOUT, 13);
	m_wndTaskbar.AddButton(IDM_GLOBE_AUTOSIZE, 14);
	m_wndTaskbar.AddButton(IDM_TAGCLOUD_SORTVALUE, 15);
	m_wndTaskbar.AddButton(IDM_TAGCLOUD_SORTCOUNT, 16);

	m_wndTaskbar.AddButton(IDM_ITEM_OPEN, 17, TRUE);
	m_wndTaskbar.AddButton(IDM_FILE_OPENBROWSER, 18, TRUE);

	m_wndTaskbar.AddButton(IDM_GLOBE_GOOGLEEARTH, 19, TRUE);
	m_wndTaskbar.AddButton(IDM_STORE_MAKEDEFAULT, 20);
	m_wndTaskbar.AddButton(IDM_STORE_PROPERTIES, 21);
	m_wndTaskbar.AddButton(IDM_FILE_REMEMBER, 22);
	m_wndTaskbar.AddButton(IDM_FILE_REMOVE, 23);
	m_wndTaskbar.AddButton(IDM_FILE_ARCHIVE, 24);
	m_wndTaskbar.AddButton(IDM_FILE_DELETE, 25);
	m_wndTaskbar.AddButton(IDM_FILE_RENAME, 26);

	#define InspectorIconVisible     27
	#define InspectorIconHidden      28
	p_InspectorButton = m_wndTaskbar.AddButton(ID_PANE_INSPECTOR, theApp.m_ShowInspectorPane ? InspectorIconVisible : InspectorIconHidden, TRUE, TRUE);

	m_wndTaskbar.AddButton(IDM_BACKSTAGE_PURCHASE, 29, TRUE, TRUE);
	m_wndTaskbar.AddButton(IDM_BACKSTAGE_ENTERLICENSEKEY, 30, TRUE, TRUE);
	m_wndTaskbar.AddButton(IDM_BACKSTAGE_SUPPORT, 31, TRUE, TRUE);
	m_wndTaskbar.AddButton(IDM_BACKSTAGE_ABOUT, 32, TRUE, TRUE);

	// Drop target
	m_DropTarget.SetOwner(GetOwner());

	// Explorer header
	if (!m_wndHeaderArea.Create(this, 2, TRUE))
		return -1;

	m_wndHeaderArea.SetOwner(GetOwner());

	p_OrganizeButton = m_wndHeaderArea.AddButton(IDM_ORGANIZE);
	p_ViewButton = m_wndHeaderArea.AddButton(IDM_VIEW);

	// Inspector
	if (!m_wndInspectorPane.Create(this, 4, FALSE, theApp.m_InspectorPaneWidth, TRUE))
		return -1;

	m_wndInspectorPane.SetOwner(GetOwner());
	m_ShowInspectorPane = theApp.m_ShowInspectorPane;

	// Explorer notification
	if (!m_wndExplorerNotification.Create(this, 5))
		return -1;

	return 0;
}

void CMainView::OnDestroy()
{
	if (m_pWndFileView)
	{
		m_pWndFileView->DestroyWindow();
		delete m_pWndFileView;
	}

	CFrontstageWnd::OnDestroy();
}

BOOL CMainView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMainView::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);

	AdjustLayout(SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS);
}

void CMainView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	if (m_pWndFileView)
		if (m_pWndFileView->IsWindowEnabled())
		{
			m_pWndFileView->SetFocus();
			return;
		}

	m_wndTaskbar.SetFocus();
}

void CMainView::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	if (m_pWndFileView)
		m_pWndFileView->SendMessage(WM_SELECTNONE);

	SetFocus();
}

void CMainView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (m_pWndFileView)
		m_pWndFileView->SendMessage(WM_SELECTNONE);

	CFrontstageWnd::OnRButtonUp(nFlags, point);
}

void CMainView::OnMeasureItem(INT nIDCtl, LPMEASUREITEMSTRUCT lpmis)
{
	if (m_pWndFileView)
		m_pWndFileView->SendMessage(WM_MEASUREITEM, (WPARAM)nIDCtl, (LPARAM)lpmis);
}

void CMainView::OnDrawItem(INT nIDCtl, LPDRAWITEMSTRUCT lpdis)
{
	if (m_pWndFileView)
		m_pWndFileView->SendMessage(WM_DRAWITEM, (WPARAM)nIDCtl, (LPARAM)lpdis);
}

void CMainView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (!m_pWndFileView)
		return;

	if ((point.x<0) || (point.y<0))
	{
		CRect rect;
		GetClientRect(rect);

		point.x = (rect.left+rect.right)/2;
		point.y = (rect.top+rect.bottom)/2;
		ClientToScreen(&point);
	}

	// Empty menu
	CMenu* pMenu = NULL;

	// Check contexts with their own menu, overriding view menus
	switch (m_Context)
	{
	case LFContextStores:
		pMenu = new CMenu();
		pMenu->LoadMenu(IDM_STORES);

		break;

	case LFContextNew:
		pMenu = new CMenu();
		pMenu->LoadMenu(IDM_NEW);

		break;

	case LFContextTrash:
		pMenu = new CMenu();
		pMenu->LoadMenu(IDM_TRASH);

		break;

	case LFContextFilters:
		pMenu = new CMenu();
		pMenu->LoadMenu(IDM_FILTERS);

		break;

	default:
		pMenu = m_pWndFileView->GetViewContextMenu();
	}

	// Create empty menu
	if (!pMenu)
	{
		pMenu = new CMenu();
		pMenu->CreatePopupMenu();
		pMenu->AppendMenu(MF_POPUP, (UINT_PTR)CreateMenu(), _T("POPUP"));
	}

	// Get the popup
	CMenu* pPopup = pMenu->GetSubMenu(0);
	ASSERT_VALID(pPopup);

	CString tmpStr;

	// Append "Open as FileDrop" and "Import folder" command when viewing a store
	if (m_StoreIDValid)
	{
		if (pPopup->GetMenuItemCount())
			pPopup->AppendMenu(MF_SEPARATOR | MF_BYPOSITION);

		ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_OPENFILEDROP));
		pPopup->AppendMenu(MF_STRING | MF_BYPOSITION, IDM_ITEM_OPENFILEDROP, tmpStr);

		if (m_Context<=LFLastGroupContext)
		{
			pPopup->AppendMenu(MF_SEPARATOR | MF_BYPOSITION);

			ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_IMPORTFOLDER));
			pPopup->AppendMenu(MF_POPUP | MF_BYPOSITION, IDM_STORE_IMPORTFOLDER, tmpStr);
		}
	}

	// Insert "Select all" command
	if (m_pWndFileView->MultiSelectAllowed())
	{
		if (pPopup->GetMenuItemCount())
			pPopup->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);

		ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_SELECTALL));
		pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_SELECTALL, tmpStr);
	}

	// Go!
	pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetOwner(), NULL);

	delete pMenu;
}

void CMainView::OnAdjustLayout()
{
	if (!m_Resizing)
		AdjustLayout();
}

void CMainView::OnUpdateSelection()
{
	m_wndInspectorPane.UpdateStart();

	INT Index = GetNextSelectedItem(-1);
	m_FilesSelected = FALSE;

	while (Index>=0)
	{
		LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];
		m_wndInspectorPane.UpdateAdd(pItemDescriptor, p_RawFiles);

		m_FilesSelected |= ((pItemDescriptor->Type & LFTypeMask)==LFTypeFile) ||
			(((pItemDescriptor->Type & LFTypeMask)==LFTypeFolder) && (pItemDescriptor->FirstAggregate!=-1) && (pItemDescriptor->LastAggregate!=-1));

		Index = GetNextSelectedItem(Index);
	}

	m_wndInspectorPane.UpdateFinish();
	m_wndTaskbar.PostMessage(WM_IDLEUPDATECMDUI);
}

void CMainView::OnBeginDragDrop()
{
	// Stores haben keinen physischen Speicherort, der von einer LFTransactionList aufgelöst werden kann
	if (m_Context==LFContextStores)
	{
		INT Index = GetSelectedItem();
		if (Index!=-1)
		{
			LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];
			if (((pItemDescriptor->Type & LFTypeMask)==LFTypeStore) && (pItemDescriptor->Type & LFTypeShortcutAllowed))
			{
				m_DropTarget.SetDragging(TRUE);

				LFStoreDataObject* pDataObject = new LFStoreDataObject(pItemDescriptor);
				LFDropSource* pDropSource = new LFDropSource();

				DWORD dwEffect;
				SHDoDragDrop(GetSafeHwnd(), pDataObject, pDropSource, DROPEFFECT_COPY, &dwEffect);

				pDropSource->Release();
				pDataObject->Release();

				m_DropTarget.SetDragging(FALSE);
			}
		}

		return;
	}

	// Alle anderen Kontexte
	LFTransactionList* pTransactionList = BuildTransactionList(FALSE, TRUE);
	if (pTransactionList->m_LastError==LFOk)
	{
		m_DropTarget.SetDragging(TRUE);

		LFTransactionDataObject* pDataObject = new LFTransactionDataObject(pTransactionList);
		LFDropSource* pDropSource = new LFDropSource();

		DWORD dwEffect;
		SHDoDragDrop(GetSafeHwnd(), pDataObject, pDropSource, DROPEFFECT_COPY, &dwEffect);

		pDropSource->Release();
		pDataObject->Release();

		m_DropTarget.SetDragging(FALSE);
	}

	LFFreeTransactionList(pTransactionList);
}

LRESULT CMainView::OnRenameItem(WPARAM wParam, LPARAM lParam)
{
	LFTransactionList* pTransactionList = LFAllocTransactionList();
	LFAddTransactionItem(pTransactionList, (*p_CookedFiles)[(UINT)wParam]);

	LFVariantData Value;
	Value.Attr = LFAttrFileName;
	Value.Type = LFTypeUnicodeString;
	Value.IsNull = FALSE;

	wcsncpy_s(Value.UnicodeString, 256, (WCHAR*)lParam, _TRUNCATE);

	LFDoTransaction(pTransactionList, LFTransactionTypeUpdate, NULL, NULL, &Value);

	if (pTransactionList->m_Modified)
	{
		FVPersistentData Data;
		GetPersistentData(Data);
		UpdateSearchResult(p_Filter, p_RawFiles, p_CookedFiles, &Data);
	}

	ShowNotification(pTransactionList->m_LastError);

	BOOL Changes = pTransactionList->m_Modified;
	LFFreeTransactionList(pTransactionList);

	return Changes;
}

LRESULT CMainView::OnSendTo(WPARAM wParam, LPARAM /*lParam*/)
{
	SendToItemData* pItemData = (SendToItemData*)wParam;
	if (pItemData->IsStore)
	{
		WorkerParameters wp;
		ZeroMemory(&wp, sizeof(wp));
		wp.pTransactionList = BuildTransactionList();
		strcpy_s(wp.StoreID, LFKeySize, pItemData->StoreID);

		if (strcmp(wp.StoreID, "CHOOSE")==0)
		{
			LFChooseStoreDlg dlg(this);
			if (dlg.DoModal()==IDOK)
			{
				strcpy_s(wp.StoreID, LFKeySize, dlg.m_StoreID);

				LFDoWithProgress(WorkerSendTo, &wp.Hdr, this);

				ShowNotification(wp.pTransactionList->m_LastError);
			}
		}

		LFFreeTransactionList(wp.pTransactionList);
	}
	else
	{
		IShellFolder* pDesktop = NULL;
		if (SUCCEEDED(SHGetDesktopFolder(&pDesktop)))
		{
			LPITEMIDLIST pidlFQ;
			if (SUCCEEDED(pDesktop->ParseDisplayName(NULL, NULL, pItemData->Path, NULL, &pidlFQ, NULL)))
			{
				IShellFolder* pParentWnd = NULL;
				LPCITEMIDLIST pidlRel;
				if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder, (void**)&pParentWnd, &pidlRel)))
				{
					IDropTarget* pDropTarget = NULL;
					if (SUCCEEDED(pParentWnd->GetUIObjectOf(GetSafeHwnd(), 1, &pidlRel, IID_IDropTarget, NULL, (void**)&pDropTarget)))
					{
						LFTransactionList* pTransactionList = BuildTransactionList(FALSE, TRUE);
						if (pTransactionList->m_ItemCount)
						{
							CWaitCursor csr;
							LFTransactionDataObject* pDataObject = new LFTransactionDataObject(pTransactionList);

							POINTL pt = { 0, 0 };
							DWORD dwEffect = DROPEFFECT_COPY;
							if (SUCCEEDED(pDropTarget->DragEnter(pDataObject, MK_LBUTTON, pt, &dwEffect)))
							{
								pDropTarget->Drop(pDataObject, MK_LBUTTON, pt, &dwEffect);
							}
							else
							{
								pDropTarget->DragLeave();
							}

							pDataObject->Release();
						}

						LFFreeTransactionList(pTransactionList);
						pDropTarget->Release();
					}

					pParentWnd->Release();
				}

				theApp.GetShellManager()->FreeItem(pidlFQ);
			}

			pDesktop->Release();
		}
	}

	return NULL;
}

LRESULT CMainView::OnStoreAttributesChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if ((p_RawFiles) && (p_CookedFiles) && (m_Context<=LFLastQueryContext))
		SetHeader();

	return NULL;
}


// Panes

void CMainView::OnToggleInspector()
{
	ASSERT(p_InspectorButton);

	theApp.m_ShowInspectorPane = m_ShowInspectorPane = !m_ShowInspectorPane;
	p_InspectorButton->SetIconID(m_ShowInspectorPane ? InspectorIconVisible : InspectorIconHidden);
	AdjustLayout();
}

void CMainView::OnUpdatePaneCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


// Header

void CMainView::OnSortOptions()
{
	OrganizeDlg dlg(m_Context, this);
	if (dlg.DoModal()==IDOK)
		theApp.UpdateSortOptions(m_Context);
}

void CMainView::OnToggleAutoDirs()
{
	theApp.m_Views[m_Context].AutoDirs = !theApp.m_Views[m_Context].AutoDirs;
	theApp.UpdateSortOptions(m_Context);
}

void CMainView::OnUpdateHeaderCommands(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case IDM_ORGANIZE_TOGGLEAUTODIRS:
		pCmdUI->SetCheck((theApp.m_Views[m_Context].AutoDirs) || (m_Context>=LFContextSubfolderDefault));
		pCmdUI->Enable((theApp.m_Contexts[m_Context].AllowGroups) && (theApp.m_Views[m_Context].Mode<=LFViewContent));
		break;

	default:
		pCmdUI->Enable(TRUE);
	}
}

LRESULT CMainView::OnGetMenu(WPARAM wParam, LPARAM /*lParam*/)
{
	HMENU hMenu = CreatePopupMenu();
	HMENU hPopupMenu = NULL;
	BOOL Separator = FALSE;
	CString tmpStr;

	switch (wParam)
	{
	case IDM_ORGANIZE:
#define AppendAttribute(hMenu, Attr) if (theApp.IsAttributeAllowed(m_Context, Attr)) { tmpStr = theApp.m_Attributes[Attr].Name; AppendMenu(hMenu, MF_STRING, IDM_ORGANIZE_FIRST+Attr, _T("&")+tmpStr); }
#define AppendSeparator(hMenu, Attr) if (theApp.IsAttributeAllowed(m_Context, Attr)) { AppendMenu(hMenu, MF_SEPARATOR, 0, NULL); }
#define AppendPopup(nID) if (GetMenuItemCount(hPopupMenu)) { ENSURE(tmpStr.LoadString(nID)); AppendMenu(hMenu, MF_POPUP | MF_STRING, (UINT_PTR)hPopupMenu, tmpStr); } else { DestroyMenu(hPopupMenu); }
		AppendAttribute(hMenu, LFAttrFileName);
		AppendAttribute(hMenu, LFAttrTitle);
		AppendAttribute(hMenu, LFAttrComments);

		hPopupMenu = CreatePopupMenu();
		AppendAttribute(hPopupMenu, LFAttrCreationTime);
		AppendAttribute(hPopupMenu, LFAttrRecordingTime);
		AppendAttribute(hPopupMenu, LFAttrAddTime);
		AppendAttribute(hPopupMenu, LFAttrFileTime);
		AppendAttribute(hPopupMenu, LFAttrArchiveTime);
		AppendAttribute(hPopupMenu, LFAttrDeleteTime);
		AppendSeparator(hPopupMenu, LFAttrDueTime);
		AppendAttribute(hPopupMenu, LFAttrDueTime);
		AppendAttribute(hPopupMenu, LFAttrDoneTime);
		AppendPopup(IDS_CONTEXTMENU_TIME);

		hPopupMenu = CreatePopupMenu();
		AppendAttribute(hPopupMenu, LFAttrLocationName);
		AppendAttribute(hPopupMenu, LFAttrLocationIATA);
		AppendAttribute(hPopupMenu, LFAttrLocationGPS);
		AppendPopup(IDS_CONTEXTMENU_LOCATION);

		AppendAttribute(hMenu, LFAttrRating);
		AppendAttribute(hMenu, LFAttrPriority);
		AppendAttribute(hMenu, LFAttrHashtags);
		AppendAttribute(hMenu, LFAttrRoll);
		AppendAttribute(hMenu, LFAttrArtist);
		AppendAttribute(hMenu, LFAttrGenre);
		AppendAttribute(hMenu, LFAttrDuration);
		AppendAttribute(hMenu, LFAttrLanguage);

		hPopupMenu = CreatePopupMenu();
		AppendAttribute(hPopupMenu, LFAttrAspectRatio);
		AppendAttribute(hPopupMenu, LFAttrDimension);
		AppendSeparator(hPopupMenu, LFAttrWidth);
		AppendAttribute(hPopupMenu, LFAttrWidth);
		AppendAttribute(hPopupMenu, LFAttrHeight);
		AppendPopup(IDS_CONTEXTMENU_DIMENSION);

		if (theApp.m_Contexts[m_Context].AllowGroups)
		{
			AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

			ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_AUTODIRS));
			AppendMenu(hMenu, MF_STRING, IDM_ORGANIZE_TOGGLEAUTODIRS, tmpStr);
		}

		AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);

		ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_MORE));
		AppendMenu(hMenu, MF_STRING, IDM_ORGANIZE_OPTIONS, tmpStr);

		break;

	case IDM_VIEW:
		for (UINT a=0; a<LFViewCount; a++)
			if (theApp.IsViewAllowed(m_Context, a))
			{
				if ((a>LFViewContent) && (!Separator))
				{
					AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
					Separator = TRUE;
				}

				UINT nID = IDM_VIEW_FIRST+a;

				CString tmpStr((LPCSTR)nID);

				AppendMenu(hMenu, MF_STRING, nID, _T("&")+tmpStr);
			}

		break;
	}

	return (LRESULT)hMenu;
}

void CMainView::OnSort(UINT nID)
{
	nID -= IDM_ORGANIZE_FIRST;

	if (theApp.m_Views[m_Context].SortBy!=nID)
	{
		theApp.m_Views[m_Context].SortBy = nID;
		theApp.m_Views[m_Context].Descending = theApp.m_Attributes[nID].TypeProperties.PreferDescendingSort;
		theApp.UpdateSortOptions(m_Context);
	}

	SetFocus();
}

void CMainView::OnUpdateSortCommands(CCmdUI* pCmdUI)
{
	UINT Attr = pCmdUI->m_nID-IDM_ORGANIZE_FIRST;

	pCmdUI->Enable(theApp.IsAttributeAllowed(m_Context, Attr));
	pCmdUI->SetRadio(theApp.m_Views[m_Context].SortBy==Attr);
}

void CMainView::OnView(UINT nID)
{
	nID -= IDM_VIEW_FIRST;

	if (m_ViewID!=(INT)nID)
	{
		theApp.m_Views[m_Context].Mode = nID;
		theApp.UpdateViewOptions(m_Context);
	}

	SetFocus();
}

void CMainView::OnUpdateViewCommands(CCmdUI* pCmdUI)
{
	INT View = pCmdUI->m_nID-IDM_VIEW_FIRST;

	pCmdUI->Enable(theApp.IsViewAllowed(m_Context, View));
	pCmdUI->SetRadio(m_ViewID==View);
}


// Stores

void CMainView::OnStoresAdd()
{
	// Allowed?
	if (LFNagScreen(this))
	{
		CWaitCursor csr;

		LFAddStoreDlg dlg(this);
		dlg.DoModal();
	}
}

void CMainView::OnStoresSynchronize()
{
	LFRunSynchronizeAll(this);
}

void CMainView::OnStoresRunMaintenance()
{
	DismissNotification();

	LFRunMaintenance(this);
}

void CMainView::OnUpdateStoresCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = (p_CookedFiles!=NULL);

	switch (pCmdUI->m_nID)
	{
	case IDM_STORES_ADD:
		bEnable &= (m_Context==LFContextStores);
		break;

	case IDM_STORES_SYNCHRONIZE:
		bEnable &= (m_Context==LFContextStores) && (LFGetStoreCount()>0);
		break;

	case IDM_STORES_RUNMAINTENANCE:
		bEnable &= (LFGetStoreCount()>0);
		break;
	}

	pCmdUI->Enable(bEnable);
}


// New

void CMainView::OnNewClearNew()
{
	RestoreFiles(TRUE);
}

void CMainView::OnUpdateNewCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(((m_Context==LFContextNew) && (p_CookedFiles)) ? p_CookedFiles->m_ItemCount : FALSE);
}


// Trash

void CMainView::OnTrashRestoreAll()
{
	RestoreFiles(TRUE);
}

void CMainView::OnTrashEmpty()
{
	if (DeleteFiles(FALSE, TRUE))
		theApp.PlayTrashSound();
}

void CMainView::OnUpdateTrashCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = (m_Context==LFContextTrash) && (p_CookedFiles) ? p_CookedFiles->m_ItemCount : FALSE;

	INT Index = GetSelectedItem();
	if (Index!=-1)
		if (pCmdUI->m_nID==IDM_TRASH_RESTOREALL)
			bEnable = FALSE;

	pCmdUI->Enable(bEnable);
}


// Filters

void CMainView::OnUpdateFiltersCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = (m_Context==LFContextFilters);

	INT Index = GetSelectedItem();
	if (Index!=-1)
	{
		LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];
		switch (pCmdUI->m_nID)
		{
		case IDM_FILTERS_EDIT:
			bEnable &= ((pItemDescriptor->Type & (LFTypeMask | LFTypeMounted))==(LFTypeFile | LFTypeMounted));
			break;
		}
	}

	pCmdUI->Enable(bEnable);
}


// pItemDescriptor

void CMainView::OnUpdateItemCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;

	INT Index = GetSelectedItem();
	if (Index!=-1)
	{
		LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

		switch (pCmdUI->m_nID)
		{
		case IDM_ITEM_OPEN:
			bEnable = (pItemDescriptor->pNextFilter!=NULL) ||
				((pItemDescriptor->Type & (LFTypeMask | LFTypeMounted))==(LFTypeFile | LFTypeMounted));

			break;
		}
	}

	pCmdUI->Enable(bEnable);
}


// Store

void CMainView::OnStoreSynchronize()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
		LFRunSynchronize((*p_CookedFiles)[Index]->StoreID, this);
}

void CMainView::OnStoreMakeDefault()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
		LFErrorBox(this, LFSetDefaultStore((*p_CookedFiles)[Index]->StoreID));
}

void CMainView::OnStoreImportFolder()
{
	if (m_Context==LFContextStores)
	{
		INT Index = GetSelectedItem();
		if (Index!=-1)
			LFImportFolder((*p_CookedFiles)[Index]->StoreID, this);
	}
	else
		if (m_StoreIDValid)
		{
			if (LFImportFolder(m_StoreID, this))
				GetOwner()->PostMessage(WM_RELOAD);
		}
}

void CMainView::OnStoreShortcut()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
		LFCreateDesktopShortcutForStore((*p_CookedFiles)[Index]);
}

void CMainView::OnStoreDelete()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
		LFDeleteStore((*p_CookedFiles)[Index]->StoreID, this);
}

void CMainView::OnStoreRename()
{
	INT Index = GetSelectedItem();
	if ((Index!=-1) && (m_pWndFileView))
		m_pWndFileView->EditLabel(Index);
}

void CMainView::OnStoreProperties()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
	{
		LFStorePropertiesDlg dlg((*p_CookedFiles)[Index]->StoreID, this);
		dlg.DoModal();
	}
}

void CMainView::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;

	if (m_Context==LFContextStores)
	{
		INT Index = GetSelectedItem();
		if (Index!=-1)
		{
			LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];
			if ((pItemDescriptor->Type & LFTypeMask)==LFTypeStore)
				switch (pCmdUI->m_nID)
				{
				case IDM_STORE_SYNCHRONIZE:
					bEnable = ((pItemDescriptor->Type & (LFTypeSynchronizeAllowed | LFTypeMounted | LFTypeWriteable))==(LFTypeSynchronizeAllowed | LFTypeMounted | LFTypeWriteable));
					break;

				case IDM_STORE_MAKEDEFAULT:
					bEnable = !(pItemDescriptor->Type & LFTypeDefault);
					break;

				case IDM_STORE_IMPORTFOLDER:
					bEnable = ((pItemDescriptor->Type & (LFTypeMounted | LFTypeWriteable))==(LFTypeMounted | LFTypeWriteable));
					break;

				case IDM_STORE_SHORTCUT:
					bEnable = (pItemDescriptor->Type & LFTypeShortcutAllowed);
					break;

				case IDM_STORE_DELETE:
					bEnable = (pItemDescriptor->Type & LFTypeWriteable);
					break;

				case IDM_STORE_RENAME:
					bEnable = (pItemDescriptor->Type & LFTypeWriteable) && !m_pWndFileView->IsEditing();
					break;

				default:
					bEnable = TRUE;
				}
		}
	}
	else
	{
		if (pCmdUI->m_nID==IDM_STORE_IMPORTFOLDER)
			bEnable = m_StoreIDValid && (m_Context<=LFLastGroupContext);
	}

	pCmdUI->Enable(bEnable);
}


// File

void CMainView::OnFileOpenWith()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
	{
		WCHAR Path[MAX_PATH];
		UINT Result = LFGetFileLocation((*p_CookedFiles)[Index], Path, MAX_PATH, TRUE);
		if (Result==LFOk)
		{
			WCHAR Cmd[300];
			wcscpy_s(Cmd, 300, L"shell32.dll,OpenAs_RunDLL ");
			wcscat_s(Cmd, 300, Path);

			ShellExecute(GetSafeHwnd(), _T("open"), _T("rundll32.exe"), Cmd, Path, SW_SHOWNORMAL);
		}
		else
		{
			LFErrorBox(this, Result);
		}
	}
}

void CMainView::OnFileOpenBrowser()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
		ShellExecuteA(GetSafeHwnd(), "open", (*p_CookedFiles)[Index]->CoreAttributes.URL, NULL, NULL, SW_SHOWNORMAL);
}

void CMainView::OnFileEdit()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
		if (strcmp((*p_CookedFiles)[Index]->CoreAttributes.FileFormat, "filter")==0)
		{
			LFFilter* pFilter = LFLoadFilter((*p_CookedFiles)[Index]);

			LFEditFilterDlg dlg(this, pFilter ? pFilter->StoreID[0]!='\0' ? pFilter->StoreID : m_StoreID : m_StoreID, pFilter);
			if (dlg.DoModal()==IDOK)
				GetOwner()->PostMessage(WM_RELOAD);

			LFFreeFilter(pFilter);
		}
}

void CMainView::OnFileRemember()
{
	CMainWnd* pClipboard = theApp.GetClipboard();
	BOOL changes = FALSE;

	INT Index = GetNextSelectedItem(-1);
	while (Index!=-1)
	{
		LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];
		switch (pItemDescriptor->Type & LFTypeMask)
		{
		case LFTypeFile:
			if (pClipboard->AddClipItem(pItemDescriptor))
				changes = TRUE;

			break;

		case LFTypeFolder:
			if ((pItemDescriptor->FirstAggregate!=-1) && (pItemDescriptor->LastAggregate!=-1))
				for (INT a=pItemDescriptor->FirstAggregate; a<=pItemDescriptor->LastAggregate; a++)
					if (pClipboard->AddClipItem((*p_RawFiles)[a]))
						changes = TRUE;

			break;
		}

		Index = GetNextSelectedItem(Index);
	}

	if (changes)
		pClipboard->SendMessage(WM_COOKFILES);
}

void CMainView::OnFileRemove()
{
	LFTransactionList* pTransactionList = BuildTransactionList();

	for (UINT a=0; a<pTransactionList->m_ItemCount; a++)
		(*pTransactionList)[a].Processed = TRUE;

	RemoveTransactedItems(pTransactionList);

	LFFreeTransactionList(pTransactionList);
}

void CMainView::OnFileArchive()
{
	LFTransactionList* pTransactionList = BuildTransactionList();

	CWaitCursor csr;
	LFDoTransaction(pTransactionList, LFTransactionTypeArchive);
	RemoveTransactedItems(pTransactionList);

	ShowNotification(pTransactionList->m_LastError);

	LFFreeTransactionList(pTransactionList);
}

void CMainView::OnFileCopy()
{
	LFTransactionList* pTransactionList = BuildTransactionList(FALSE, TRUE);
	if (pTransactionList->m_ItemCount)
	{
		CWaitCursor csr;
		LFTransactionDataObject* pDataObject = new LFTransactionDataObject(pTransactionList);

		OleSetClipboard(pDataObject);
		OleFlushClipboard();

		pDataObject->Release();
	}

	LFFreeTransactionList(pTransactionList);
}

void CMainView::OnFileShortcut()
{
	LFTransactionList* pTransactionList = BuildTransactionList(FALSE, TRUE, TRUE);
	if (pTransactionList->m_ItemCount)
	{
		CWaitCursor csr;
		for (UINT a=0; a<pTransactionList->m_ItemCount; a++)
			if (((*pTransactionList)[a].LastError==LFOk) && ((*pTransactionList)[a].Processed))
				CreateShortcut(&(*pTransactionList)[a]);
	}

	LFFreeTransactionList(pTransactionList);
}

void CMainView::OnFileDelete()
{
	if (p_CookedFiles)
		DeleteFiles(p_CookedFiles->m_Context!=LFContextTrash);
}

void CMainView::OnFileRename()
{
	INT Index = GetSelectedItem();
	if ((Index!=-1) && (m_pWndFileView))
		m_pWndFileView->EditLabel(Index);
}

void CMainView::OnFileProperties()
{
	if (!m_ShowInspectorPane)
	{
		ASSERT(p_InspectorButton);

		m_ShowInspectorPane = TRUE;
		p_InspectorButton->SetIconID(InspectorIconVisible);
		AdjustLayout();
	}

	m_wndInspectorPane.SetFocus();
}

void CMainView::OnFileRestore()
{
	RestoreFiles();
}

void CMainView::OnUpdateFileCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;

	const INT Index = GetSelectedItem();
	LFItemDescriptor* pItemDescriptor = (Index==-1) ? NULL : (*p_CookedFiles)[Index];

	switch (pCmdUI->m_nID)
	{
	case IDM_FILE_OPENWITH:
		if (pItemDescriptor)
			bEnable = ((pItemDescriptor->Type & (LFTypeMask | LFTypeMounted))==(LFTypeFile | LFTypeMounted)) && (pItemDescriptor->CoreAttributes.ContextID!=LFContextFilters);

		break;

	case IDM_FILE_OPENBROWSER:
		if (pItemDescriptor)
			bEnable = ((pItemDescriptor->Type & LFTypeMask)==LFTypeFile) && (pItemDescriptor->CoreAttributes.URL[0]!='\0');

		break;

	case IDM_FILE_EDIT:
		if (pItemDescriptor)
			bEnable = ((pItemDescriptor->Type & LFTypeMask)==LFTypeFile) && (pItemDescriptor->CoreAttributes.ContextID==LFContextFilters);

		break;

	case IDM_FILE_REMEMBER:
		bEnable = m_FilesSelected && (m_Context!=LFContextClipboard) && (m_Context!=LFContextTrash);
		break;

	case IDM_FILE_REMOVE:
		bEnable = m_FilesSelected && (m_Context==LFContextClipboard);
		break;

	case IDM_FILE_ARCHIVE:
		bEnable = m_FilesSelected && (m_Context!=LFContextArchive) && (m_Context!=LFContextTrash);
		break;

	case IDM_FILE_COPY:
	case IDM_FILE_SHORTCUT:
	case IDM_FILE_DELETE:
		bEnable = m_FilesSelected;
		break;

	case IDM_FILE_RENAME:
		if ((pItemDescriptor) && (m_Context!=LFContextArchive) && (m_Context!=LFContextTrash))
			bEnable = ((pItemDescriptor->Type & (LFTypeMask | LFTypeMounted))==(LFTypeFile | LFTypeMounted));

		if (m_pWndFileView)
			bEnable &= !m_pWndFileView->IsEditing();

		break;

	case IDM_FILE_PROPERTIES:
		bEnable = m_FilesSelected && !m_ShowInspectorPane;
		break;

	case IDM_FILE_RESTORE:
		bEnable = m_FilesSelected && ((m_Context==LFContextArchive) || (m_Context==LFContextTrash));
		break;
	}

	pCmdUI->Enable(bEnable);
}
