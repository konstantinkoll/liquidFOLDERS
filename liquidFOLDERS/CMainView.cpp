
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


void CreateShortcut(LFTransactionListItem* pItem)
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


// CMainView
//

#define FileViewID     3

CMainView::CMainView()
{
	p_wndFileView = NULL;
	p_Filter = NULL;
	p_RawFiles = p_CookedFiles = NULL;
	p_FilterButton = p_OpenButton = p_InspectorButton = NULL;
	p_OrganizeButton = p_ViewButton = NULL;
	m_Context = m_ViewID = -1;
	m_Resizing = m_StoreIDValid = m_Alerted = FALSE;
}

BOOL CMainView::Create(CWnd* pParentWnd, UINT nID, BOOL IsClipboard)
{
	m_IsClipboard = IsClipboard;

	CString className = AfxRegisterWndClass(CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW));

	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, rect, pParentWnd, nID);
}

BOOL CMainView::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The file view gets the command first
	if (p_wndFileView)
		if (p_wndFileView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;

	// Check Inspector
	if (m_wndInspector.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// Check application commands
	if (theApp.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CMainView::CreateFileView(UINT ViewID, FVPersistentData* Data)
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
	case LFViewPreview:
		if ((m_ViewID<LFViewLargeIcons) || (m_ViewID>LFViewPreview))
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
		if (p_wndFileView)
		{
			p_wndFileView->GetWindowRect(rect);
			ScreenToClient(rect);
		}
		else
		{
			rect.SetRectEmpty();
		}

		pNewView->Create(this, FileViewID, rect, p_RawFiles, p_CookedFiles, Data);

		CFileView* pVictim = p_wndFileView;

		p_wndFileView = pNewView;
		p_wndFileView->SetOwner(GetOwner());
		if ((GetFocus()==pVictim) || (GetTopLevelParent()==GetActiveWindow()))
			p_wndFileView->SetFocus();

		RegisterDragDrop(p_wndFileView->GetSafeHwnd(), &m_DropTarget);

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

	p_OrganizeButton->SetValue(theApp.m_Attributes[theApp.m_Views[m_Context].SortBy].Name, TRUE, FALSE);

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

		LFStoreDescriptor s;
		WCHAR* pHint = (m_Context>LFContextAllFiles) ? p_CookedFiles->m_Hint : NULL;

		if ((m_Context<=LFLastQueryContext) && (m_StoreIDValid) && (!pHint))
			if (LFGetStoreSettings(m_StoreID, &s)==LFOk)
			{
				wcscpy_s(p_RawFiles->m_Name, 256, s.StoreName);
				wcscpy_s(p_CookedFiles->m_Name, 256, s.StoreName);

				pHint = s.StoreComment;
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
	if (p_wndFileView)
	{
		FVPersistentData Data;
		GetPersistentData(Data);
		if (!CreateFileView(theApp.m_Views[m_Context].Mode, &Data))
			p_wndFileView->UpdateViewOptions(m_Context);

		SetHeaderButtons();
	}
}

void CMainView::UpdateSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* Data, BOOL UpdateSelection)
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
		if (p_wndFileView)
			p_wndFileView->UpdateSearchResult(NULL, NULL, NULL);

		RevokeDragDrop(m_wndHeaderArea.GetSafeHwnd());
		RevokeDragDrop(p_wndFileView->GetSafeHwnd());
	}
	else
	{
		m_Context = pCookedFiles->m_Context;

		if (!CreateFileView(theApp.m_Views[pCookedFiles->m_Context].Mode, Data))
		{
			p_wndFileView->UpdateViewOptions(m_Context);
			p_wndFileView->UpdateSearchResult(pRawFiles, pCookedFiles, Data);
		}

		m_DropTarget.SetFilter(pFilter);
		RegisterDragDrop(m_wndHeaderArea.GetSafeHwnd(), &m_DropTarget);
		RegisterDragDrop(p_wndFileView->GetSafeHwnd(), &m_DropTarget);
	}

	if (m_IsClipboard)
		m_DropTarget.SetSearchResult(pRawFiles);

	SetHeader();
	if (UpdateSelection)
		OnUpdateSelection();
}

BOOL CMainView::StoreIDValid()
{
	return m_StoreIDValid;
}

CHAR* CMainView::GetStoreID()
{
	return m_StoreID;
}

INT CMainView::GetContext()
{
	return m_Context;
}

INT CMainView::GetViewID()
{
	return m_ViewID;
}

void CMainView::DismissNotification()
{
	m_wndExplorerNotification.DismissNotification();
}

void CMainView::ShowNotification(UINT Type, CString Message, UINT Command)
{
	m_wndExplorerNotification.SetNotification(Type, Message, Command);
}

void CMainView::ShowNotification(UINT Type, UINT ResID, UINT Command)
{
	WCHAR tmpStr[256];
	LFGetErrorText(tmpStr, 256, ResID);

	ShowNotification(Type, tmpStr, Command);
}

void CMainView::AdjustLayout()
{
	if (!IsWindow(m_wndTaskbar))
		return;
	if (!IsWindow(m_wndExplorerNotification))
		return;
	if (!IsWindow(m_wndHeaderArea))
		return;
	if (!IsWindow(m_wndInspector))
		return;

	m_Resizing = TRUE;

	CRect rect;
	GetClientRect(rect);

	const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
	m_wndTaskbar.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	const UINT NotificationHeight = m_wndExplorerNotification.GetPreferredHeight();
	m_wndExplorerNotification.SetWindowPos(&wndTop, rect.left+32, rect.bottom-NotificationHeight, rect.Width()-64, NotificationHeight, SWP_NOACTIVATE);

	const INT MaxWidth = (rect.Width()-128)/2;
	INT InspectorWidth = 0;
	if (MaxWidth>0)
	{
		theApp.m_InspectorWidth = max(32, m_wndInspector.GetPreferredWidth());
		InspectorWidth = theApp.m_InspectorWidth = min(MaxWidth, (INT)theApp.m_InspectorWidth);

		if (m_ShowInspectorPane)
		{
			m_wndInspector.SetMaxWidth(MaxWidth);
			m_wndInspector.SetWindowPos(NULL, rect.right-InspectorWidth, rect.top+TaskHeight, InspectorWidth, rect.Height()-TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER | SWP_SHOWWINDOW);
		}
		else
		{
			m_wndInspector.ShowWindow(SW_HIDE);
			InspectorWidth = 0;
		}
	}

	const UINT ExplorerHeight = m_wndHeaderArea.GetPreferredHeight();
	m_wndHeaderArea.SetWindowPos(NULL, rect.left, rect.top+TaskHeight, rect.Width()-InspectorWidth, ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	if (p_wndFileView)
		p_wndFileView->SetWindowPos(NULL, rect.left, rect.top+TaskHeight+ExplorerHeight, rect.Width()-InspectorWidth, rect.Height()-ExplorerHeight-TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	m_Resizing = FALSE;
}

INT CMainView::GetSelectedItem()
{
	return p_wndFileView ? p_wndFileView->GetSelectedItem() : -1;
}

INT CMainView::GetNextSelectedItem(INT n)
{
	return p_wndFileView ? p_wndFileView->GetNextSelectedItem(n) : -1;
}

void CMainView::GetPersistentData(FVPersistentData& Data)
{
	if (p_wndFileView)
	{
		p_wndFileView->GetPersistentData(Data);
	}
	else
	{
		ZeroMemory(&Data, sizeof(Data));
	}
}

void CMainView::SelectNone()
{
	if (p_wndFileView)
		p_wndFileView->SendMessage(WM_SELECTNONE);
}

void CMainView::AddTransactionItem(LFTransactionList* pTransactionList, LFItemDescriptor* pItemDescriptor, UINT_PTR UserData)
{
	switch (pItemDescriptor->Type & LFTypeMask)
	{
	case LFTypeVolume:
	case LFTypeStore:
	case LFTypeFile:
		LFAddTransactionItem(pTransactionList, pItemDescriptor, UserData);
		break;

	case LFTypeFolder:
		if ((pItemDescriptor->FirstAggregate!=-1) && (pItemDescriptor->LastAggregate!=-1))
			for (INT a=pItemDescriptor->FirstAggregate; a<=pItemDescriptor->LastAggregate; a++)
				LFAddTransactionItem(pTransactionList, p_RawFiles->m_Items[a], UserData);

		break;
	}
}

LFTransactionList* CMainView::BuildTransactionList(BOOL All, BOOL ResolvePhysicalLocations, BOOL IncludePIDL)
{
	LFTransactionList* pTransactionList = LFAllocTransactionList();

	if ((p_RawFiles) && (p_CookedFiles))
	{
		if (All)
		{
			for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
				AddTransactionItem(pTransactionList, p_CookedFiles->m_Items[a], a);
		}
		else
		{
			INT Index = GetNextSelectedItem(-1);
			while (Index!=-1)
			{
				AddTransactionItem(pTransactionList, p_CookedFiles->m_Items[Index], Index);
				Index = GetNextSelectedItem(Index);
			}
		}

		if (ResolvePhysicalLocations)
		{
			LFTransactionResolvePhysicalLocations(pTransactionList, IncludePIDL);
			LFErrorBox(pTransactionList->m_LastError, GetSafeHwnd());
		}
	}

	return pTransactionList;
}

void CMainView::RemoveTransactedItems(LFTransactionList* pTransactionList)
{
	if (!p_RawFiles)
		return;

	for (UINT a=0; a<pTransactionList->m_ItemCount; a++)
		if ((pTransactionList->m_Items[a].LastError==LFOk) && (pTransactionList->m_Items[a].Processed))
			pTransactionList->m_Items[a].pItemDescriptor->RemoveFlag = TRUE;

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

		LFTransactionDelete(pTransactionList, TRUE);
	}
	else
	{
		WorkerParameters wp;
		ZeroMemory(&wp, sizeof(wp));
		wp.TransactionList = pTransactionList;

		LFDoWithProgress(WorkerDelete, &wp.Hdr, this);
	}

	RemoveTransactedItems(pTransactionList);

	if (pTransactionList->m_LastError>LFCancel)
		ShowNotification(ENT_ERROR, pTransactionList->m_LastError);

	BOOL Modified = pTransactionList->m_Modified;
	LFFreeTransactionList(pTransactionList);

	return Modified;
}

BOOL CMainView::RestoreFiles(UINT Flags, BOOL All)
{
	CWaitCursor csr;

	LFTransactionList* pTransactionList = BuildTransactionList(All);
	LFTransactionRestore(pTransactionList, Flags);
	RemoveTransactedItems(pTransactionList);

	if (pTransactionList->m_LastError>LFCancel)
		ShowNotification(ENT_ERROR, pTransactionList->m_LastError);

	BOOL Changes = pTransactionList->m_Modified;
	LFFreeTransactionList(pTransactionList);

	return Changes;
}

BOOL CMainView::UpdateItems(LFVariantData* Value1, LFVariantData* Value2, LFVariantData* Value3)
{
	CWaitCursor csr;

	LFTransactionList* pTransactionList = BuildTransactionList();
	LFTransactionUpdate(pTransactionList, Value1, Value2, Value3);

	if (p_wndFileView)
	{
		BOOL Deselected = FALSE;

		for (UINT a=0; a<pTransactionList->m_ItemCount; a++)
			if (pTransactionList->m_Items[a].LastError!=LFOk)
			{
				p_wndFileView->SelectItem((INT)pTransactionList->m_Items[a].UserData, FALSE, TRUE);
				Deselected = TRUE;
			}

		if (pTransactionList->m_Modified)
		{
			FVPersistentData Data;
			GetPersistentData(Data);
			UpdateSearchResult(p_Filter, p_RawFiles, p_CookedFiles, &Data, FALSE);
		}
		if (Deselected)
		{
			p_wndFileView->Invalidate();
			OnUpdateSelection();
		}
	}

	if (pTransactionList->m_LastError>LFCancel)
		ShowNotification(ENT_ERROR, pTransactionList->m_LastError);

	BOOL Changes = pTransactionList->m_Modified;
	LFFreeTransactionList(pTransactionList);

	return Changes;
}


BEGIN_MESSAGE_MAP(CMainView, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_THEMECHANGED()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE_VOID(WM_ADJUSTLAYOUT, OnAdjustLayout)
	ON_MESSAGE(WM_SETALERT, OnSetAlert)
	ON_MESSAGE_VOID(WM_UPDATESELECTION, OnUpdateSelection)
	ON_MESSAGE_VOID(WM_BEGINDRAGDROP, OnBeginDragDrop)
	ON_MESSAGE(WM_RENAMEITEM, OnRenameItem)
	ON_MESSAGE(WM_SENDTO, OnSendTo)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoreAttributesChanged)

	ON_COMMAND(ID_PANE_INSPECTOR, OnToggleInspector)
	ON_UPDATE_COMMAND_UI_RANGE(ID_PANE_FILTER, ID_PANE_INSPECTOR, OnUpdatePaneCommands)

	ON_COMMAND(IDM_ORGANIZE_OPTIONS, OnSortOptions)
	ON_COMMAND(IDM_ORGANIZE_TOGGLEAUTODIRS, OnToggleAutoDirs)
	ON_UPDATE_COMMAND_UI(IDM_ORGANIZE_OPTIONS, OnUpdateHeaderCommands)
	ON_UPDATE_COMMAND_UI(IDM_ORGANIZE_TOGGLEAUTODIRS, OnUpdateHeaderCommands)
	ON_MESSAGE(WM_GETMENU, OnGetMenu)
	ON_COMMAND_RANGE(IDM_ORGANIZE_FIRST, IDM_ORGANIZE_FIRST+LFAttributeCount-1, OnSort)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_ORGANIZE_FIRST, IDM_ORGANIZE_FIRST+LFAttributeCount-1, OnUpdateSortCommands)
	ON_COMMAND_RANGE(IDM_VIEW_FIRST, IDM_VIEW_FIRST+LFViewCount-1, OnView)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_VIEW_FIRST, IDM_VIEW_FIRST+LFViewCount-1, OnUpdateViewCommands)

	ON_COMMAND(IDM_STORES_ADD, OnStoresAdd)
	ON_COMMAND(IDM_STORES_REPAIRCORRUPTEDINDEX, OnStoresMaintainAll)
	ON_UPDATE_COMMAND_UI(IDM_STORES_ADD, OnUpdateStoresCommands)
	ON_UPDATE_COMMAND_UI(IDM_STORES_REPAIRCORRUPTEDINDEX, OnUpdateStoresCommands)

	ON_COMMAND(IDM_NEW_REMOVENEW, OnNewRemoveNew)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_NEW_REMOVENEW, IDM_NEW_REMOVENEW, OnUpdateNewCommands)

	ON_COMMAND(IDM_TRASH_EMPTY, OnTrashEmpty)
	ON_COMMAND(IDM_TRASH_RESTOREALL, OnTrashRestoreAll)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_TRASH_EMPTY, IDM_TRASH_RESTOREALL, OnUpdateTrashCommands)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILTERS_CREATENEW, IDM_FILTERS_EDIT, OnUpdateFiltersCommands)

	ON_UPDATE_COMMAND_UI(IDM_ITEM_OPEN, OnUpdateItemCommands)

	ON_COMMAND(IDM_VOLUME_FORMAT, OnVolumeFormat)
	ON_COMMAND(IDM_VOLUME_EJECT, OnVolumeEject)
	ON_COMMAND(IDM_VOLUME_PROPERTIES, OnVolumeProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_VOLUME_FORMAT, IDM_VOLUME_PROPERTIES, OnUpdateVolumeCommands)

	ON_COMMAND(IDM_STORE_MAKEDEFAULT, OnStoreMakeDefault)
	ON_COMMAND(IDM_STORE_IMPORTFOLDER, OnStoreImportFolder)
	ON_COMMAND(IDM_STORE_SHORTCUT, OnStoreShortcut)
	ON_COMMAND(IDM_STORE_DELETE, OnStoreDelete)
	ON_COMMAND(IDM_STORE_RENAME, OnStoreRename)
	ON_COMMAND(IDM_STORE_PROPERTIES, OnStoreProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORE_MAKEDEFAULT, IDM_STORE_PROPERTIES, OnUpdateStoreCommands)

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
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Taskbar
	if (!m_wndTaskbar.Create(this, IDB_TASKS_32, IDB_TASKS_16, 1))
		return -1;

	m_wndTaskbar.SetOwner(GetOwner());

	#define FilterIconOverlay     35
	p_FilterButton = m_wndTaskbar.AddButton(ID_PANE_FILTER, 0, TRUE, FALSE, TRUE);

	m_wndTaskbar.AddButton(IDM_STORES_ADD, 1);
	m_wndTaskbar.AddButton(IDM_NEW_REMOVENEW, 2, TRUE);
	m_wndTaskbar.AddButton(IDM_TRASH_EMPTY, 3, TRUE);
	m_wndTaskbar.AddButton(IDM_TRASH_RESTOREALL, 4, TRUE);
	m_wndTaskbar.AddButton(IDM_FILE_RESTORE, 5);
	m_wndTaskbar.AddButton(IDM_FILTERS_CREATENEW, 6, TRUE);
	m_wndTaskbar.AddButton(IDM_CALENDAR_PREVYEAR, 7, TRUE);
	m_wndTaskbar.AddButton(IDM_CALENDAR_NEXTYEAR, 8, TRUE);
	m_wndTaskbar.AddButton(IDM_CALENDAR_GOTOYEAR, 9);
	m_wndTaskbar.AddButton(IDM_GLOBE_JUMPTOLOCATION, 10, TRUE);
	m_wndTaskbar.AddButton(IDM_GLOBE_ZOOMIN, 11);
	m_wndTaskbar.AddButton(IDM_GLOBE_ZOOMOUT, 12);
	m_wndTaskbar.AddButton(IDM_GLOBE_AUTOSIZE, 13);
	m_wndTaskbar.AddButton(IDM_TAGCLOUD_SORTVALUE, 14);
	m_wndTaskbar.AddButton(IDM_TAGCLOUD_SORTCOUNT, 15);

	#define OpenIconFolder       16
	#define OpenIconExplorer     17
	p_OpenButton = m_wndTaskbar.AddButton(IDM_ITEM_OPEN, OpenIconFolder);

	m_wndTaskbar.AddButton(IDM_GLOBE_GOOGLEEARTH, 18, TRUE);
	m_wndTaskbar.AddButton(IDM_VOLUME_PROPERTIES, 19);
	m_wndTaskbar.AddButton(IDM_STORE_PROPERTIES, 20);
	m_wndTaskbar.AddButton(IDM_FILE_REMEMBER, 21);
	m_wndTaskbar.AddButton(IDM_FILE_REMOVE, 22);
	m_wndTaskbar.AddButton(IDM_FILE_ARCHIVE, 23);
	m_wndTaskbar.AddButton(IDM_FILE_DELETE, 24);
	m_wndTaskbar.AddButton(IDM_FILE_RENAME, 25);
	m_wndTaskbar.AddButton(IDM_STORE_MAKEDEFAULT, 26);

	#define InspectorIconVisible     27
	#define InspectorIconHidden      28
	p_InspectorButton = m_wndTaskbar.AddButton(ID_PANE_INSPECTOR, theApp.m_ShowInspectorPane ? InspectorIconVisible : InspectorIconHidden, TRUE, TRUE);

	m_wndTaskbar.AddButton(ID_APP_PURCHASE, 29, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ENTERLICENSEKEY, 30, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_SUPPORT, 31, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ABOUT, 32, TRUE, TRUE);

	// Drop target
	m_DropTarget.SetOwner(GetOwner());

	// Explorer header
	if (!m_wndHeaderArea.Create(this, 2, TRUE))
		return -1;

	m_wndHeaderArea.SetOwner(GetOwner());

	p_OrganizeButton = m_wndHeaderArea.AddButton(IDM_ORGANIZE);
	p_ViewButton = m_wndHeaderArea.AddButton(IDM_VIEW);

	// Inspector
	if (!m_wndInspector.Create(this, 4, FALSE, theApp.m_InspectorWidth))
		return -1;

	m_wndInspector.SetOwner(GetOwner());
	m_ShowInspectorPane = theApp.m_ShowInspectorPane;

	// Explorer notification
	if (!m_wndExplorerNotification.Create(this, 5))
		return -1;

	return 0;
}

void CMainView::OnDestroy()
{
	if (p_wndFileView)
	{
		p_wndFileView->DestroyWindow();
		delete p_wndFileView;
	}

	CWnd::OnDestroy();
}

BOOL CMainView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

LRESULT CMainView::OnThemeChanged()
{
	if (theApp.OSVersion==OS_Vista)
		SendMessage(WM_SETALERT, (WPARAM)m_Alerted);

	return TRUE;
}
void CMainView::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CMainView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	if (p_wndFileView)
		if (p_wndFileView->IsWindowEnabled())
		{
			p_wndFileView->SetFocus();
			return;
		}

	m_wndTaskbar.SetFocus();
}

void CMainView::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	if (p_wndFileView)
		p_wndFileView->SendMessage(WM_SELECTNONE);

	SetFocus();
}

void CMainView::OnRButtonUp(UINT nFlags, CPoint point)
{
	if (p_wndFileView)
		p_wndFileView->SendMessage(WM_SELECTNONE);

	CWnd::OnRButtonUp(nFlags, point);
}

void CMainView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (!p_wndFileView)
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
		pMenu = p_wndFileView->GetViewContextMenu();
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
	if (m_StoreIDValid && (m_Context<=LFLastGroupContext))
	{
		if (pPopup->GetMenuItemCount())
			pPopup->AppendMenu(MF_SEPARATOR | MF_BYPOSITION);

		ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_OPENFILEDROP));
		pPopup->AppendMenu(MF_STRING | MF_BYPOSITION, IDM_ITEM_OPENFILEDROP, tmpStr);

		pPopup->AppendMenu(MF_SEPARATOR | MF_BYPOSITION);

		ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_IMPORTFOLDER));
		pPopup->AppendMenu(MF_POPUP | MF_BYPOSITION, IDM_STORE_IMPORTFOLDER, tmpStr);
	}

	// Insert "Select all" command
	if (p_wndFileView->MultiSelectAllowed())
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

LRESULT CMainView::OnSetAlert(WPARAM wParam, LPARAM /*lParam*/)
{
	m_Alerted = (wParam!=0);

	if (p_FilterButton)
		p_FilterButton->SetIconID(0, m_Alerted ? FilterIconOverlay : -1);

	return NULL;
}

void CMainView::OnUpdateSelection()
{
	m_wndInspector.UpdateStart();

	INT Index = GetNextSelectedItem(-1);
	m_FilesSelected = FALSE;

	while (Index>=0)
	{
		LFItemDescriptor* pItemDescriptor = p_CookedFiles->m_Items[Index];
		m_wndInspector.UpdateAdd(pItemDescriptor, p_RawFiles);

		m_FilesSelected |= ((pItemDescriptor->Type & LFTypeMask)==LFTypeFile) ||
			(((pItemDescriptor->Type & LFTypeMask)==LFTypeFolder) && (pItemDescriptor->FirstAggregate!=-1) && (pItemDescriptor->LastAggregate!=-1));

		Index = GetNextSelectedItem(Index);
	}

	m_wndInspector.UpdateFinish();
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
			LFItemDescriptor* pItemDescriptor = p_CookedFiles->m_Items[Index];
			if ((pItemDescriptor->Type & LFTypeStore) && (pItemDescriptor->Type & LFTypeShortcutAllowed))
			{
				m_DropTarget.SetDragging(TRUE);

				LFStoreDataObject* pDataObject = new LFStoreDataObject(pItemDescriptor);
				LFDropSource* pDropSource = new LFDropSource();

				DWORD dwEffect;
				SHDoDragDrop(GetSafeHwnd(), pDataObject, pDropSource, DROPEFFECT_COPY , &dwEffect);

				pDropSource->Release();
				pDataObject->Release();

				m_DropTarget.SetDragging(FALSE);
			}
		}

		return;
	}

	// Alle anderen Kontexte
	LFTransactionList* pTransactionList = BuildTransactionList(FALSE, TRUE);
	if (pTransactionList->m_ItemCount)
	{
		m_DropTarget.SetDragging(TRUE);

		LFTransactionDataObject* pDataObject = new LFTransactionDataObject(pTransactionList);
		LFDropSource* pDropSource = new LFDropSource();

		DWORD dwEffect;
		SHDoDragDrop(GetSafeHwnd(), pDataObject, pDropSource, DROPEFFECT_COPY , &dwEffect);

		pDropSource->Release();
		pDataObject->Release();

		m_DropTarget.SetDragging(FALSE);
	}

	LFFreeTransactionList(pTransactionList);
}

LRESULT CMainView::OnRenameItem(WPARAM wParam, LPARAM lParam)
{
	LFTransactionList* pTransactionList = LFAllocTransactionList();
	LFAddTransactionItem(pTransactionList, p_CookedFiles->m_Items[(UINT)wParam]);

	LFVariantData value;
	value.Attr = LFAttrFileName;
	value.Type = LFTypeUnicodeString;
	value.IsNull = FALSE;

	wcsncpy_s(value.UnicodeString, 256, (WCHAR*)lParam, 255);

	LFTransactionUpdate(pTransactionList, &value);

	if (pTransactionList->m_Modified)
	{
		FVPersistentData Data;
		GetPersistentData(Data);
		UpdateSearchResult(p_Filter, p_RawFiles, p_CookedFiles, &Data);
	}

	if (pTransactionList->m_LastError>LFCancel)
		ShowNotification(ENT_ERROR, pTransactionList->m_LastError);

	BOOL changes = pTransactionList->m_Modified;
	LFFreeTransactionList(pTransactionList);

	return changes;
}

LRESULT CMainView::OnSendTo(WPARAM wParam, LPARAM /*lParam*/)
{
	SendToItemData* pItemData = (SendToItemData*)wParam;
	if (pItemData->IsStore)
	{
		WorkerParameters wp;
		ZeroMemory(&wp, sizeof(wp));
		wp.TransactionList = BuildTransactionList();
		strcpy_s(wp.StoreID, LFKeySize, pItemData->StoreID);

		if (strcmp(wp.StoreID, "CHOOSE")==0)
		{
			LFChooseStoreDlg dlg(this);
			if (dlg.DoModal()!=IDOK)
				return NULL;

			strcpy_s(wp.StoreID, LFKeySize, dlg.m_StoreID);
		}

		LFDoWithProgress(WorkerImportFromStore, &wp.Hdr, this);

		LFErrorBox(wp.TransactionList->m_LastError, GetSafeHwnd());
		LFFreeTransactionList(wp.TransactionList);
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
	BOOL b = FALSE;

	switch (pCmdUI->m_nID)
	{
	case ID_PANE_FILTER:
		b = !m_IsClipboard;
		break;

	case ID_PANE_INSPECTOR:
		b = TRUE;
		break;
	}

	pCmdUI->Enable(b);
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
		pCmdUI->Enable((theApp.m_Contexts[m_Context].AllowGroups) && (theApp.m_Views[m_Context].Mode<=LFViewPreview));
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
				if ((a>LFViewPreview) && (!Separator))
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
		theApp.m_Views[m_Context].Descending = theApp.m_Attributes[nID].PreferDescendingSort;
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
	LFAddStoreDlg dlg(this);
	dlg.DoModal();
}

void CMainView::OnStoresMaintainAll()
{
	LFRunMaintenance(this);
}

void CMainView::OnUpdateStoresCommands(CCmdUI* pCmdUI)
{
	BOOL b = (p_CookedFiles) && (m_Context==LFContextStores);

	if (pCmdUI->m_nID==IDM_STORES_REPAIRCORRUPTEDINDEX)
		b &= (LFGetStoreCount()>0);

	pCmdUI->Enable(b);
}


// New

void CMainView::OnNewRemoveNew()
{
	LFVariantData v;
	LFInitVariantData(v, LFAttrFlags);
	v.Flags.Flags = 0;
	v.Flags.Mask = LFFlagNew;
	v.IsNull = FALSE;

	CWaitCursor csr;

	LFTransactionList* pTransactionList = BuildTransactionList(TRUE);
	LFTransactionUpdate(pTransactionList, &v);
	RemoveTransactedItems(pTransactionList);

	if (pTransactionList->m_LastError>LFCancel)
		ShowNotification(ENT_ERROR, pTransactionList->m_LastError);

	LFFreeTransactionList(pTransactionList);
}

void CMainView::OnUpdateNewCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(((m_Context==LFContextNew) && (p_CookedFiles)) ? p_CookedFiles->m_ItemCount : FALSE);
}


// Trash

void CMainView::OnTrashRestoreAll()
{
	RestoreFiles(LFFlagTrash, TRUE);
}

void CMainView::OnTrashEmpty()
{
	if (DeleteFiles(FALSE, TRUE))
		theApp.PlayTrashSound();
}

void CMainView::OnUpdateTrashCommands(CCmdUI* pCmdUI)
{
	BOOL b = (m_Context==LFContextTrash) && (p_CookedFiles) ? p_CookedFiles->m_ItemCount : FALSE;

	INT Index = GetSelectedItem();
	if (Index!=-1)
		if (pCmdUI->m_nID==IDM_TRASH_RESTOREALL)
			b = FALSE;

	pCmdUI->Enable(b);
}


// Filters

void CMainView::OnUpdateFiltersCommands(CCmdUI* pCmdUI)
{
	BOOL b = (m_Context==LFContextFilters);

	INT Index = GetSelectedItem();
	if (Index!=-1)
	{
		LFItemDescriptor* pItemDescriptor = p_CookedFiles->m_Items[Index];
		switch (pCmdUI->m_nID)
		{
		case IDM_FILTERS_EDIT:
			b &= ((pItemDescriptor->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeFile);
			break;
		}
	}

	pCmdUI->Enable(b);
}


// pItemDescriptor

void CMainView::OnUpdateItemCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	INT Index = GetSelectedItem();
	if (Index!=-1)
	{
		LFItemDescriptor* pItemDescriptor = p_CookedFiles->m_Items[Index];
		switch (pCmdUI->m_nID)
		{
		case IDM_ITEM_OPEN:
			b = (pItemDescriptor->NextFilter!=NULL) ||
				((pItemDescriptor->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeFile) ||
				((pItemDescriptor->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeVolume);

			if (p_OpenButton)
				p_OpenButton->SetIconID((pItemDescriptor->Type & LFTypeMask)==LFTypeVolume ? OpenIconExplorer : OpenIconFolder);

			break;
		}
	}

	pCmdUI->Enable(b);
}


// Volume

void CMainView::OnVolumeFormat()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
		theApp.ExecuteExplorerContextMenu(p_CookedFiles->m_Items[Index]->CoreAttributes.FileID[0], "format");
}

void CMainView::OnVolumeEject()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
		theApp.ExecuteExplorerContextMenu(p_CookedFiles->m_Items[Index]->CoreAttributes.FileID[0], "eject");
}

void CMainView::OnVolumeProperties()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
		theApp.ExecuteExplorerContextMenu(p_CookedFiles->m_Items[Index]->CoreAttributes.FileID[0], "properties");
}

void CMainView::OnUpdateVolumeCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	INT Index = GetSelectedItem();
	if (Index!=-1)
	{
		LFItemDescriptor* pItemDescriptor = p_CookedFiles->m_Items[Index];
		switch (pCmdUI->m_nID)
		{
		case IDM_VOLUME_FORMAT:
		case IDM_VOLUME_EJECT:
			b = ((pItemDescriptor->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeVolume);
			break;

		case IDM_VOLUME_PROPERTIES:
			b = ((pItemDescriptor->Type & LFTypeMask)==LFTypeVolume);
			break;
		}
	}

	pCmdUI->Enable(b);
}


// Store

void CMainView::OnStoreMakeDefault()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
		LFErrorBox(LFMakeDefaultStore(p_CookedFiles->m_Items[Index]->StoreID), GetSafeHwnd());
}

void CMainView::OnStoreImportFolder()
{
	if (m_Context==LFContextStores)
	{
		INT Index = GetSelectedItem();
		if (Index!=-1)
			LFImportFolder(p_CookedFiles->m_Items[Index]->StoreID, this);
	}
	else
		if (m_StoreIDValid)
		{
			LFImportFolder(m_StoreID, this);
			GetOwner()->PostMessage(WM_RELOAD);
		}
}

void CMainView::OnStoreShortcut()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
		LFCreateDesktopShortcutForStore(p_CookedFiles->m_Items[Index]);
}

void CMainView::OnStoreDelete()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
		LFDeleteStore(p_CookedFiles->m_Items[Index]->StoreID, this);
}

void CMainView::OnStoreRename()
{
	INT Index = GetSelectedItem();
	if ((Index!=-1) && (p_wndFileView))
		p_wndFileView->EditLabel(Index);
}

void CMainView::OnStoreProperties()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
	{
		LFStorePropertiesDlg dlg(p_CookedFiles->m_Items[Index]->StoreID, this);
		dlg.DoModal();
	}
}

void CMainView::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	if (m_Context==LFContextStores)
	{
		INT Index = GetSelectedItem();
		if (Index!=-1)
		{
			LFItemDescriptor* pItemDescriptor = p_CookedFiles->m_Items[Index];
			if ((pItemDescriptor->Type & LFTypeMask)==LFTypeStore)
				switch (pCmdUI->m_nID)
				{
				case IDM_STORE_MAKEDEFAULT:
					b = !(pItemDescriptor->Type & LFTypeDefault);
					break;

				case IDM_STORE_IMPORTFOLDER:
					b = !(pItemDescriptor->Type & LFTypeNotMounted);
					break;

				case IDM_STORE_SHORTCUT:
					b = (pItemDescriptor->Type & LFTypeShortcutAllowed);
					break;

				case IDM_STORE_RENAME:
					b = !p_wndFileView->IsEditing();
					break;

				default:
					b = TRUE;
				}
		}
	}
	else
	{
		if (pCmdUI->m_nID==IDM_STORE_IMPORTFOLDER)
			b = m_StoreIDValid && (m_Context<=LFLastGroupContext);
	}

	pCmdUI->Enable(b);
}


// File

void CMainView::OnFileOpenWith()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
	{
		WCHAR Path[MAX_PATH];
		UINT Result = LFGetFileLocation(p_CookedFiles->m_Items[Index], Path, MAX_PATH, TRUE, TRUE);
		if (Result==LFOk)
		{
			WCHAR Cmd[300];
			wcscpy_s(Cmd, 300, L"shell32.dll,OpenAs_RunDLL ");
			wcscat_s(Cmd, 300, Path);
			ShellExecute(GetSafeHwnd(), _T("open"), _T("rundll32.exe"), Cmd, Path, SW_SHOW);
		}
		else
		{
			LFErrorBox(Result, GetSafeHwnd());
		}
	}
}

void CMainView::OnFileOpenBrowser()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
		ShellExecuteA(GetSafeHwnd(), "open", p_CookedFiles->m_Items[Index]->CoreAttributes.URL, NULL, NULL, SW_SHOW);
}

void CMainView::OnFileEdit()
{
	INT Index = GetSelectedItem();
	if (Index!=-1)
		if (strcmp(p_CookedFiles->m_Items[Index]->CoreAttributes.FileFormat, "filter")==0)
		{
			LFFilter* pFilter = LFLoadFilter(p_CookedFiles->m_Items[Index]);

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
		LFItemDescriptor* pItemDescriptor = p_CookedFiles->m_Items[Index];
		switch (pItemDescriptor->Type & LFTypeMask)
		{
		case LFTypeFile:
			if (pClipboard->AddClipItem(pItemDescriptor))
				changes = TRUE;

			break;

		case LFTypeFolder:
			if ((pItemDescriptor->FirstAggregate!=-1) && (pItemDescriptor->LastAggregate!=-1))
				for (INT a=pItemDescriptor->FirstAggregate; a<=pItemDescriptor->LastAggregate; a++)
					if (pClipboard->AddClipItem(p_RawFiles->m_Items[a]))
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
		pTransactionList->m_Items[a].Processed = TRUE;
	RemoveTransactedItems(pTransactionList);

	LFFreeTransactionList(pTransactionList);
}

void CMainView::OnFileArchive()
{
	LFTransactionList* pTransactionList = BuildTransactionList();

	CWaitCursor csr;
	LFTransactionArchive(pTransactionList);
	RemoveTransactedItems(pTransactionList);

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
			if ((pTransactionList->m_Items[a].LastError==LFOk) && (pTransactionList->m_Items[a].Processed))
				CreateShortcut(&pTransactionList->m_Items[a]);
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
	if ((Index!=-1) && (p_wndFileView))
		p_wndFileView->EditLabel(Index);
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

	m_wndInspector.SetFocus();
}

void CMainView::OnFileRestore()
{
	RestoreFiles((m_Context==LFContextArchive) ? LFFlagArchive : (m_Context==LFContextTrash) ? LFFlagTrash : 0);
}

void CMainView::OnUpdateFileCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	INT Index = GetSelectedItem();
	LFItemDescriptor* pItemDescriptor = (Index==-1) ? NULL : p_CookedFiles->m_Items[Index];

	switch (pCmdUI->m_nID)
	{
	case IDM_FILE_OPENWITH:
		if (pItemDescriptor)
			b = ((pItemDescriptor->Type & (LFTypeNotMounted | LFTypeMask))==LFTypeFile) && (pItemDescriptor->CoreAttributes.ContextID!=LFContextFilters);

		break;

	case IDM_FILE_OPENBROWSER:
		if (pItemDescriptor)
			b = ((pItemDescriptor->Type & LFTypeMask)==LFTypeFile) && (pItemDescriptor->CoreAttributes.URL[0]!='\0');

		break;

	case IDM_FILE_EDIT:
		if (pItemDescriptor)
			b = ((pItemDescriptor->Type & LFTypeMask)==LFTypeFile) && (pItemDescriptor->CoreAttributes.ContextID==LFContextFilters);

		break;

	case IDM_FILE_REMEMBER:
		b = m_FilesSelected && (m_Context!=LFContextClipboard) && (m_Context!=LFContextTrash);
		break;

	case IDM_FILE_REMOVE:
		b = m_FilesSelected && (m_Context==LFContextClipboard);
		break;

	case IDM_FILE_ARCHIVE:
		b = m_FilesSelected && (m_Context!=LFContextArchive) && (m_Context!=LFContextTrash);
		break;

	case IDM_FILE_COPY:
	case IDM_FILE_SHORTCUT:
	case IDM_FILE_DELETE:
		b = m_FilesSelected;
		break;

	case IDM_FILE_RENAME:
		if ((pItemDescriptor) && (m_Context!=LFContextArchive) && (m_Context!=LFContextTrash))
			b = ((pItemDescriptor->Type & (LFTypeNotMounted | LFTypeMask))==LFTypeFile);

		if (p_wndFileView)
			b &= !p_wndFileView->IsEditing();

		break;

	case IDM_FILE_PROPERTIES:
		b = m_FilesSelected && !m_ShowInspectorPane;
		break;

	case IDM_FILE_RESTORE:
		b = m_FilesSelected && ((m_Context==LFContextArchive) || (m_Context==LFContextTrash));
		break;
	}

	pCmdUI->Enable(b);
}
