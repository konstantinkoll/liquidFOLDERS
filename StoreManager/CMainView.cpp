
// CMainView.cpp: Implementierung der Klasse CMainView
//

#include "stdafx.h"
#include "CMainView.h"
#include "CCalendarView.h"
#include "CGlobeView.h"
#include "CListView.h"
#include "CTagcloudView.h"
#include "CTimelineView.h"
#include "EditFilterDlg.h"
#include "SortOptionsDlg.h"
#include "ViewOptionsDlg.h"
#include "StoreManager.h"


void CreateShortcut(LFTL_Item* i)
{
	// Get a pointer to the IShellLink interface
	IShellLink* pShellLink = NULL;
	if (SUCCEEDED(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pShellLink)))
	{
		WCHAR Ext[LFExtSize+1] = L".*";
		WCHAR* LastBackslash = wcsrchr(i->Path, L'\\');
		if (!LastBackslash)
			LastBackslash = i->Path;

		WCHAR* LastExt = wcsrchr(LastBackslash, L'.');
		if (LastExt)
			wcscpy_s(Ext, LFExtSize+1, LastExt);

		pShellLink->SetIDList(i->pidlFQ);
		pShellLink->SetIconLocation(Ext, 0);
		pShellLink->SetShowCmd(SW_SHOWNORMAL);

		LFCreateDesktopShortcut(pShellLink, i->Item->CoreAttributes.FileName);

		pShellLink->Release();
	}
}


// Thread workers
//

struct WorkerParameters
{
	LFWorkerParameters Hdr;
	CHAR StoreID[LFKeySize];
	HWND hWndSource;
	UINT Result;
	union
	{
		LFFileIDList* FileIDList;
		LFTransactionList* TransactionList;
	};
};

DWORD WINAPI WorkerImport(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	LFTransactionImport(wp->StoreID, wp->FileIDList, false, &p);

	LF_WORKERTHREAD_FINISH();
}

DWORD WINAPI WorkerDelete(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	LFTransactionDelete(wp->TransactionList, false, &p);

	LF_WORKERTHREAD_FINISH();
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

BOOL CMainView::Create(BOOL IsClipboard, CWnd* pParentWnd, UINT nID)
{
	m_IsClipboard = IsClipboard;

	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), dwStyle, rect, pParentWnd, nID);
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

	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDM_VIEW_FIRST+m_ViewID));
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
		CString Mask;

		if (m_Context==LFContextStores)
		{
			ENSURE(Mask.LoadString(p_CookedFiles->m_StoreCount==1 ? IDS_STORES_SINGULAR : IDS_STORES_PLURAL));
			Hint.Format(Mask, p_CookedFiles->m_StoreCount);
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
	LFGetErrorText(tmpStr, ResID);

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

void CMainView::AddFileIDItem(LFFileIDList* il, LFItemDescriptor* item)
{
	switch (item->Type & LFTypeMask)
	{
	case LFTypeFile:
		LFAddFileID(il, item->StoreID, item->CoreAttributes.FileID, NULL);
		break;
	case LFTypeFolder:
		if ((item->FirstAggregate!=-1) && (item->LastAggregate!=-1))
			for (INT a=item->FirstAggregate; a<=item->LastAggregate; a++)
				LFAddFileID(il, p_RawFiles->m_Items[a]->StoreID, p_RawFiles->m_Items[a]->CoreAttributes.FileID, NULL);
		break;
	}
}

LFFileIDList* CMainView::BuildFileIDList(BOOL All)
{
	LFFileIDList* il = LFAllocFileIDList();

	if ((p_RawFiles) && (p_CookedFiles))
		if (All)
		{
			for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
				AddFileIDItem(il, p_CookedFiles->m_Items[a]);
		}
		else
		{
			INT idx = GetNextSelectedItem(-1);
			while (idx!=-1)
			{
				AddFileIDItem(il, p_CookedFiles->m_Items[idx]);
				idx = GetNextSelectedItem(idx);
			}
		}

	return il;
}

void CMainView::AddTransactionItem(LFTransactionList* tl, LFItemDescriptor* item, UINT UserData)
{
	switch (item->Type & LFTypeMask)
	{
	case LFTypeVolume:
	case LFTypeStore:
	case LFTypeFile:
		LFAddItemDescriptor(tl, item, UserData);
		break;
	case LFTypeFolder:
		if ((item->FirstAggregate!=-1) && (item->LastAggregate!=-1))
			for (INT a=item->FirstAggregate; a<=item->LastAggregate; a++)
				LFAddItemDescriptor(tl, p_RawFiles->m_Items[a], UserData);
		break;
	}
}

LFTransactionList* CMainView::BuildTransactionList(BOOL All, BOOL ResolvePhysicalLocations, BOOL IncludePIDL)
{
	LFTransactionList* tl = LFAllocTransactionList();

	if ((p_RawFiles) && (p_CookedFiles))
	{
		if (All)
		{
			for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
				AddTransactionItem(tl, p_CookedFiles->m_Items[a], a);
		}
		else
		{
			INT idx = GetNextSelectedItem(-1);
			while (idx!=-1)
			{
				AddTransactionItem(tl, p_CookedFiles->m_Items[idx], idx);
				idx = GetNextSelectedItem(idx);
			}
		}

		if (ResolvePhysicalLocations)
		{
			LFTransactionResolvePhysicalLocations(tl, IncludePIDL==TRUE);
			LFErrorBox(tl->m_LastError, GetSafeHwnd());
		}
	}

	return tl;
}

void CMainView::RemoveTransactedItems(LFFileIDList* il)
{
	if (!p_RawFiles)
		return;

	for (UINT a=0; a<il->m_ItemCount; a++)
		if ((il->m_Items[a].LastError==LFOk) && (il->m_Items[a].Processed))
			for (UINT b=0; b<p_RawFiles->m_ItemCount; b++)
			{
				LFItemDescriptor* i = p_RawFiles->m_Items[b];

				if ((i->Type & LFTypeMask)==LFTypeFile)
					if ((strcmp(il->m_Items[a].StoreID, i->StoreID)==0) && (strcmp(il->m_Items[a].FileID, i->CoreAttributes.FileID)==0))
					{
						p_RawFiles->m_Items[b]->DeleteFlag = true;
						break;
					}
			}

	LFRemoveFlaggedItemDescriptors(p_RawFiles);

	FVPersistentData Data;
	GetPersistentData(Data);
	GetOwner()->SendMessage(WM_COOKFILES, (WPARAM)&Data);
}

void CMainView::RemoveTransactedItems(LFTransactionList* tl)
{
	if (!p_RawFiles)
		return;

	for (UINT a=0; a<tl->m_ItemCount; a++)
		if ((tl->m_Items[a].LastError==LFOk) && (tl->m_Items[a].Processed))
			tl->m_Items[a].Item->DeleteFlag = true;

	LFRemoveFlaggedItemDescriptors(p_RawFiles);

	FVPersistentData Data;
	GetPersistentData(Data);
	GetOwner()->SendMessage(WM_COOKFILES, (WPARAM)&Data);
}

BOOL CMainView::DeleteFiles(BOOL Trash, BOOL All)
{
	LFTransactionList* tl = BuildTransactionList(All);

	if (Trash)
	{
		CWaitCursor csr;

		LFTransactionDelete(tl, true);
	}
	else
	{
		WorkerParameters wp;
		ZeroMemory(&wp, sizeof(wp));
		wp.TransactionList = tl;

		LFDoWithProgress(WorkerDelete, &wp.Hdr, this);
	}

	RemoveTransactedItems(tl);

	if (tl->m_LastError>LFCancel)
		ShowNotification(ENT_ERROR, tl->m_LastError);

	BOOL Changes = tl->m_Changes;
	LFFreeTransactionList(tl);

	return Changes;
}

BOOL CMainView::RestoreFiles(UINT Flags, BOOL All)
{
	CWaitCursor csr;

	LFTransactionList* tl = BuildTransactionList(All);
	LFTransactionRestore(tl, Flags);
	RemoveTransactedItems(tl);

	if (tl->m_LastError>LFCancel)
		ShowNotification(ENT_ERROR, tl->m_LastError);

	BOOL Changes = tl->m_Changes;
	LFFreeTransactionList(tl);

	return Changes;
}

BOOL CMainView::UpdateItems(LFVariantData* Value1, LFVariantData* Value2, LFVariantData* Value3)
{
	CWaitCursor csr;

	LFTransactionList* tl = BuildTransactionList();
	LFTransactionUpdate(tl, GetOwner()->GetSafeHwnd(), Value1, Value2, Value3);

	if (p_wndFileView)
	{
		BOOL Deselected = FALSE;

		for (UINT a=0; a<tl->m_ItemCount; a++)
			if (tl->m_Items[a].LastError!=LFOk)
			{
				p_wndFileView->SelectItem(tl->m_Items[a].UserData, FALSE, TRUE);
				Deselected = TRUE;
			}

		if (tl->m_Changes)
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

	if (tl->m_LastError>LFCancel)
		ShowNotification(ENT_ERROR, tl->m_LastError);

	BOOL Changes = tl->m_Changes;
	LFFreeTransactionList(tl);

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
	ON_COMMAND(IDM_VIEW_OPTIONS, OnViewOptions)
	ON_UPDATE_COMMAND_UI(IDM_ORGANIZE_OPTIONS, OnUpdateHeaderCommands)
	ON_UPDATE_COMMAND_UI(IDM_ORGANIZE_TOGGLEAUTODIRS, OnUpdateHeaderCommands)
	ON_UPDATE_COMMAND_UI(IDM_VIEW_OPTIONS, OnUpdateHeaderCommands)
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

	#define FilterIcon             0
	#define FilterIconOverlay     35
	p_FilterButton = m_wndTaskbar.AddButton(ID_PANE_FILTER, FilterIcon, TRUE, FALSE, TRUE);

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

	#define OpenIconFolder       17
	#define OpenIconExplorer     18
	p_OpenButton = m_wndTaskbar.AddButton(IDM_ITEM_OPEN, OpenIconFolder);

	m_wndTaskbar.AddButton(IDM_GLOBE_GOOGLEEARTH, 19, TRUE);
	m_wndTaskbar.AddButton(IDM_VOLUME_PROPERTIES, 20);
	m_wndTaskbar.AddButton(IDM_STORE_PROPERTIES, 22);
	m_wndTaskbar.AddButton(IDM_FILE_REMEMBER, 23);
	m_wndTaskbar.AddButton(IDM_FILE_REMOVE, 24);
	m_wndTaskbar.AddButton(IDM_FILE_ARCHIVE, 25);
	m_wndTaskbar.AddButton(IDM_FILE_DELETE, 26);
	m_wndTaskbar.AddButton(IDM_FILE_RENAME, 27);
	m_wndTaskbar.AddButton(IDM_STORE_MAKEDEFAULT, 28);

	#define InspectorIconVisible     29
	#define InspectorIconHidden      30
	p_InspectorButton = m_wndTaskbar.AddButton(ID_PANE_INSPECTOR, theApp.m_ShowInspectorPane ? InspectorIconVisible : InspectorIconHidden, TRUE, TRUE);

	m_wndTaskbar.AddButton(ID_APP_PURCHASE, 31, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ENTERLICENSEKEY, 32, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_SUPPORT, 33, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ABOUT, 34, TRUE, TRUE);

	// Drop target
	m_DropTarget.SetOwner(GetOwner());

	// Explorer header
	if (!m_wndHeaderArea.Create(this, 2, TRUE))
		return -1;

	m_wndHeaderArea.SetOwner(GetOwner());

	p_OrganizeButton = m_wndHeaderArea.AddButton(IDM_ORGANIZE);
	p_ViewButton = m_wndHeaderArea.AddButton(IDM_VIEW);

	// Inspector
	if (!m_wndInspector.Create(FALSE, theApp.m_InspectorWidth, this, 4))
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

	// Insert separator
	if (pPopup->GetMenuItemCount())
		pPopup->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);

	// Append "Open as FileDrop" and "Import folder" command when viewing a store
	if (m_StoreIDValid && (m_Context<=LFLastGroupContext))
	{
		pPopup->AppendMenu(MF_SEPARATOR | MF_BYPOSITION);

		ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_OPENFILEDROP));
		pPopup->AppendMenu(MF_STRING | MF_BYPOSITION, IDM_ITEM_OPENFILEDROP, tmpStr);

		CMenu* pSubMenu = new CMenu();
		pSubMenu->CreatePopupMenu();

		ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_IMPORTFOLDER));
		pSubMenu->AppendMenu(MF_STRING | MF_BYPOSITION, IDM_STORE_IMPORTFOLDER, tmpStr);

		ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_ADDFILES));
		pPopup->AppendMenu(MF_POPUP | MF_BYPOSITION, (UINT_PTR)pSubMenu->m_hMenu, tmpStr);
	}

	// Insert view options command
	CString mask;
	ENSURE(mask.LoadString(IDS_CONTEXTMENU_VIEWOPTIONS));
	tmpStr.Format(mask, theApp.m_Contexts[m_Context].Name);
	pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_VIEW_OPTIONS, tmpStr);

	// Separate sort and view options
	pPopup->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);

	// Special: submenu for tagcloud
	if (m_ViewID==LFViewTagcloud)
	{
		ENSURE(tmpStr.LoadString(IDM_TAGCLOUD_SORT));
		pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT_PTR)LoadMenu(NULL, MAKEINTRESOURCE(IDM_TAGCLOUD_SORT)), tmpStr);
	}

	// Insert auto dir command
	if (theApp.m_Contexts[m_Context].AllowGroups)
	{
		ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_AUTODIRS));
		pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_ORGANIZE_TOGGLEAUTODIRS, tmpStr);
	}

	// Insert sort options command
	ENSURE(mask.LoadString(IDS_CONTEXTMENU_SORTOPTIONS));
	tmpStr.Format(mask, theApp.m_Contexts[m_Context].Name);
	pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_ORGANIZE_OPTIONS, tmpStr);

	// Insert "Select all" command
	if (p_wndFileView->MultiSelectAllowed())
	{
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
		p_FilterButton->SetIconID(FilterIcon, m_Alerted ? FilterIconOverlay : -1);

	return NULL;
}

void CMainView::OnUpdateSelection()
{
	m_wndInspector.UpdateStart();

	INT idx = GetNextSelectedItem(-1);
	m_FilesSelected = FALSE;

	while (idx>=0)
	{
		LFItemDescriptor* item = p_CookedFiles->m_Items[idx];
		m_wndInspector.UpdateAdd(item, p_RawFiles);

		m_FilesSelected |= ((item->Type & LFTypeMask)==LFTypeFile) ||
			(((item->Type & LFTypeMask)==LFTypeFolder) && (item->FirstAggregate!=-1) && (item->LastAggregate!=-1));

		idx = GetNextSelectedItem(idx);
	}

	m_wndInspector.UpdateFinish();
	m_wndTaskbar.PostMessage(WM_IDLEUPDATECMDUI);
}

void CMainView::OnBeginDragDrop()
{
	// Stores haben keinen physischen Speicherort, der von einer LFTransactionList aufgelöst werden kann
	if (m_Context==LFContextStores)
	{
		INT idx = GetSelectedItem();
		if (idx!=-1)
		{
			LFItemDescriptor* item = p_CookedFiles->m_Items[idx];
			if ((item->Type & LFTypeStore) && (item->Type & LFTypeShortcutAllowed))
			{
				m_DropTarget.SetDragging(TRUE);

				LFStoreDataObject* pDataObject = new LFStoreDataObject(item);
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
	LFTransactionList* tl = BuildTransactionList(FALSE, TRUE);
	if (tl->m_ItemCount)
	{
		m_DropTarget.SetDragging(TRUE);

		LFTransactionDataObject* pDataObject = new LFTransactionDataObject(tl);
		LFDropSource* pDropSource = new LFDropSource();

		DWORD dwEffect;
		SHDoDragDrop(GetSafeHwnd(), pDataObject, pDropSource, DROPEFFECT_COPY , &dwEffect);

		pDropSource->Release();
		pDataObject->Release();

		m_DropTarget.SetDragging(FALSE);
	}

	LFFreeTransactionList(tl);
}

LRESULT CMainView::OnRenameItem(WPARAM wParam, LPARAM lParam)
{
	LFTransactionList* tl = LFAllocTransactionList();
	LFAddItemDescriptor(tl, p_CookedFiles->m_Items[(UINT)wParam]);

	LFVariantData value;
	value.Attr = LFAttrFileName;
	value.Type = LFTypeUnicodeString;
	value.IsNull = false;

	wcsncpy_s(value.UnicodeString, 256, (WCHAR*)lParam, 255);

	LFTransactionUpdate(tl, GetSafeHwnd(), &value);

	if (tl->m_Changes)
	{
		FVPersistentData Data;
		GetPersistentData(Data);
		UpdateSearchResult(p_Filter, p_RawFiles, p_CookedFiles, &Data);
	}

	if (tl->m_LastError>LFCancel)
		ShowNotification(ENT_ERROR, tl->m_LastError);

	BOOL changes = tl->m_Changes;
	LFFreeTransactionList(tl);
	return changes;
}

LRESULT CMainView::OnSendTo(WPARAM wParam, LPARAM /*lParam*/)
{
	SendToItemData* pItemData = (SendToItemData*)wParam;
	if (pItemData->IsStore)
	{
		WorkerParameters wp;
		ZeroMemory(&wp, sizeof(wp));
		wp.FileIDList = BuildFileIDList();
		strcpy_s(wp.StoreID, LFKeySize, pItemData->StoreID);

		if (strcmp(wp.StoreID, "CHOOSE")==0)
		{
			LFChooseStoreDlg dlg(this);
			if (dlg.DoModal()!=IDOK)
				return NULL;

			strcpy_s(wp.StoreID, LFKeySize, dlg.m_StoreID);
		}

		LFDoWithProgress(WorkerImport, &wp.Hdr, this);

		LFErrorBox(wp.FileIDList->m_LastError, GetSafeHwnd());
		LFFreeFileIDList(wp.FileIDList);
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
						LFTransactionList* tl = BuildTransactionList(FALSE, TRUE);
						if (tl->m_ItemCount)
						{
							CWaitCursor csr;
							LFTransactionDataObject* pDataObject = new LFTransactionDataObject(tl);

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

						LFFreeTransactionList(tl);
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
	SortOptionsDlg dlg(this, m_Context);
	if (dlg.DoModal()==IDOK)
		theApp.UpdateSortOptions(m_Context);
}

void CMainView::OnToggleAutoDirs()
{
	theApp.m_Views[m_Context].AutoDirs = !theApp.m_Views[m_Context].AutoDirs;
	theApp.UpdateSortOptions(m_Context);
}

void CMainView::OnViewOptions()
{
	ViewOptionsDlg dlg(this, m_Context);
	if (dlg.DoModal()==IDOK)
		theApp.UpdateViewOptions(m_Context);
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
#define AppendAttribute(hMenu, attr) if (theApp.m_Contexts[m_Context].AllowedAttributes.IsSet(attr)) { tmpStr = theApp.m_Attributes[attr].Name; AppendMenu(hMenu, MF_STRING, IDM_ORGANIZE_FIRST+attr, _T("&")+tmpStr); }
#define AppendSeparator(hMenu, attr) if (theApp.m_Contexts[m_Context].AllowedAttributes.IsSet(attr)) { AppendMenu(hMenu, MF_SEPARATOR, 0, NULL); }
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
		AppendAttribute(hMenu, LFAttrTags);
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
			if (theApp.m_AllowedViews[m_Context]->IsSet(a))
			{
				if ((a>LFViewPreview) && (!Separator))
				{
					AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
					Separator = TRUE;
				}

				UINT nID = IDM_VIEW_FIRST+a;

				CString tmpStr;
				ENSURE(tmpStr.LoadString(nID));

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

	pCmdUI->Enable(theApp.m_Contexts[m_Context].AllowedAttributes.IsSet(Attr));
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

	pCmdUI->Enable(theApp.m_AllowedViews[m_Context]->IsSet(View));
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
	LFVariantData Value;
	Value.Attr = LFAttrFlags;
	LFGetNullVariantData(&Value);

	Value.Flags.Flags = 0;
	Value.Flags.Mask = LFFlagNew;
	Value.IsNull = false;

	CWaitCursor csr;

	LFTransactionList* tl = BuildTransactionList(TRUE);
	LFTransactionUpdate(tl, GetOwner()->GetSafeHwnd(), &Value);
	RemoveTransactedItems(tl);

	if (tl->m_LastError>LFCancel)
		ShowNotification(ENT_ERROR, tl->m_LastError);

	LFFreeTransactionList(tl);
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

	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		switch (pCmdUI->m_nID)
		{
		case IDM_TRASH_RESTOREALL:
			b = FALSE;
			break;
		}
	}

	pCmdUI->Enable(b);
}


// Filters

void CMainView::OnUpdateFiltersCommands(CCmdUI* pCmdUI)
{
	BOOL b = (m_Context==LFContextFilters);

	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		LFItemDescriptor* item = p_CookedFiles->m_Items[idx];
		switch (pCmdUI->m_nID)
		{
		case IDM_FILTERS_EDIT:
			b &= ((item->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeFile);
			break;
		}
	}

	pCmdUI->Enable(b);
}


// Item

void CMainView::OnUpdateItemCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		LFItemDescriptor* item = p_CookedFiles->m_Items[idx];
		switch (pCmdUI->m_nID)
		{
		case IDM_ITEM_OPEN:
			b = (item->NextFilter!=NULL) ||
				((item->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeFile) ||
				((item->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeVolume);

			if (p_OpenButton)
				p_OpenButton->SetIconID((item->Type & LFTypeMask)==LFTypeVolume ? OpenIconExplorer : OpenIconFolder);

			break;
		}
	}

	pCmdUI->Enable(b);
}


// Volume

void CMainView::OnVolumeFormat()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		CHAR cVolume = p_CookedFiles->m_Items[idx]->CoreAttributes.FileID[0];
		if (LFStoresOnVolume(cVolume))
		{
			CString caption;
			CString mask;
			CString text;
			ENSURE(mask.LoadString(IDS_FORMAT_CAPTION));
			ENSURE(text.LoadString(IDS_FORMAT_TEXT));
			caption.Format(mask, p_CookedFiles->m_Items[idx]->CoreAttributes.FileName);

			MessageBox(text, caption, MB_ICONWARNING);
		}
		else
		{
			theApp.ExecuteExplorerContextMenu(cVolume, "format");
		}
	}
}

void CMainView::OnVolumeEject()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
		theApp.ExecuteExplorerContextMenu(p_CookedFiles->m_Items[idx]->CoreAttributes.FileID[0], "eject");
}

void CMainView::OnVolumeProperties()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
		theApp.ExecuteExplorerContextMenu(p_CookedFiles->m_Items[idx]->CoreAttributes.FileID[0], "properties");
}

void CMainView::OnUpdateVolumeCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		LFItemDescriptor* item = p_CookedFiles->m_Items[idx];
		switch (pCmdUI->m_nID)
		{
		case IDM_VOLUME_FORMAT:
		case IDM_VOLUME_EJECT:
			b = ((item->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeVolume);
			break;
		case IDM_VOLUME_PROPERTIES:
			b = ((item->Type & LFTypeMask)==LFTypeVolume);
			break;
		}
	}

	pCmdUI->Enable(b);
}


// Store

void CMainView::OnStoreMakeDefault()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
		LFErrorBox(LFMakeDefaultStore(p_CookedFiles->m_Items[idx]->StoreID, NULL), GetSafeHwnd());
}

void CMainView::OnStoreImportFolder()
{
	if (m_Context==LFContextStores)
	{
		INT idx = GetSelectedItem();
		if (idx!=-1)
			LFImportFolder(p_CookedFiles->m_Items[idx]->StoreID, this);
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
	INT idx = GetSelectedItem();
	if (idx!=-1)
		if (LFAskCreateShortcut(GetSafeHwnd()))
			LFCreateDesktopShortcutForStore(p_CookedFiles->m_Items[idx]);
}

void CMainView::OnStoreDelete()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
		LFErrorBox(theApp.DeleteStore(p_CookedFiles->m_Items[idx], this), GetSafeHwnd());
}

void CMainView::OnStoreRename()
{
	INT idx = GetSelectedItem();
	if ((idx!=-1) && (p_wndFileView))
		p_wndFileView->EditLabel(idx);
}

void CMainView::OnStoreProperties()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		LFStorePropertiesDlg dlg(p_CookedFiles->m_Items[idx]->StoreID, this);
		dlg.DoModal();
	}
}

void CMainView::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	if (m_Context==LFContextStores)
	{
		INT idx = GetSelectedItem();
		if (idx!=-1)
		{
			LFItemDescriptor* item = p_CookedFiles->m_Items[idx];
			if ((item->Type & LFTypeMask)==LFTypeStore)
				switch (pCmdUI->m_nID)
				{
				case IDM_STORE_MAKEDEFAULT:
					b = !(item->Type & LFTypeDefault);
					break;
				case IDM_STORE_IMPORTFOLDER:
					b = !(item->Type & LFTypeNotMounted);
					break;
				case IDM_STORE_SHORTCUT:
					b = (item->Type & LFTypeShortcutAllowed);
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
	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		WCHAR Path[MAX_PATH];
		UINT res = LFGetFileLocation(p_CookedFiles->m_Items[idx], Path, MAX_PATH, true, true);
		if (res==LFOk)
		{
			WCHAR Cmd[300];
			wcscpy_s(Cmd, 300, L"shell32.dll,OpenAs_RunDLL ");
			wcscat_s(Cmd, 300, Path);
			ShellExecute(GetSafeHwnd(), _T("open"), _T("rundll32.exe"), Cmd, Path, SW_SHOW);
		}
		else
		{
			LFErrorBox(res, GetSafeHwnd());
		}
	}
}

void CMainView::OnFileOpenBrowser()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
		ShellExecuteA(GetSafeHwnd(), "open", p_CookedFiles->m_Items[idx]->CoreAttributes.URL, NULL, NULL, SW_SHOW);
}

void CMainView::OnFileEdit()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
		if (strcmp(p_CookedFiles->m_Items[idx]->CoreAttributes.FileFormat, "filter")==0)
		{
			LFFilter* f = LFLoadFilter(p_CookedFiles->m_Items[idx]);

			EditFilterDlg dlg(this, f ? f->StoreID[0]!='\0' ? f->StoreID : m_StoreID : m_StoreID, f);
			if (dlg.DoModal()==IDOK)
				GetOwner()->PostMessage(WM_RELOAD);

			LFFreeFilter(f);
		}
}

void CMainView::OnFileRemember()
{
	CMainWnd* pClipboard = theApp.GetClipboard();
	BOOL changes = FALSE;

	INT idx = GetNextSelectedItem(-1);
	while (idx!=-1)
	{
		LFItemDescriptor* item = p_CookedFiles->m_Items[idx];
		switch (item->Type & LFTypeMask)
		{
		case LFTypeFile:
			if (pClipboard->AddClipItem(item))
				changes = TRUE;
			break;
		case LFTypeFolder:
			if ((item->FirstAggregate!=-1) && (item->LastAggregate!=-1))
				for (INT a=item->FirstAggregate; a<=item->LastAggregate; a++)
					if (pClipboard->AddClipItem(p_RawFiles->m_Items[a]))
						changes = TRUE;
			break;
		}

		idx = GetNextSelectedItem(idx);
	}

	if (changes)
		pClipboard->SendMessage(WM_COOKFILES);
}

void CMainView::OnFileRemove()
{
	LFTransactionList* tl = BuildTransactionList();

	for (UINT a=0; a<tl->m_ItemCount; a++)
		tl->m_Items[a].Processed = true;
	RemoveTransactedItems(tl);

	LFFreeTransactionList(tl);
}

void CMainView::OnFileArchive()
{
	LFTransactionList* tl = BuildTransactionList();

	CWaitCursor csr;
	LFTransactionArchive(tl);
	RemoveTransactedItems(tl);

	LFFreeTransactionList(tl);
}

void CMainView::OnFileCopy()
{
	LFTransactionList* tl = BuildTransactionList(FALSE, TRUE);
	if (tl->m_ItemCount)
	{
		CWaitCursor csr;
		LFTransactionDataObject* pDataObject = new LFTransactionDataObject(tl);

		OleSetClipboard(pDataObject);
		OleFlushClipboard();

		pDataObject->Release();
	}

	LFFreeTransactionList(tl);
}

void CMainView::OnFileShortcut()
{
	if (!LFAskCreateShortcut(GetSafeHwnd()))
		return;

	LFTransactionList* tl = BuildTransactionList(FALSE, TRUE, TRUE);
	if (tl->m_ItemCount)
	{
		CWaitCursor csr;
		for (UINT a=0; a<tl->m_ItemCount; a++)
			if ((tl->m_Items[a].LastError==LFOk) && (tl->m_Items[a].Processed))
				CreateShortcut(&tl->m_Items[a]);
	}

	LFFreeTransactionList(tl);
}

void CMainView::OnFileDelete()
{
	if (p_CookedFiles)
		DeleteFiles(p_CookedFiles->m_Context!=LFContextTrash);
}

void CMainView::OnFileRename()
{
	INT idx = GetSelectedItem();
	if ((idx!=-1) && (p_wndFileView))
		p_wndFileView->EditLabel(idx);
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

	INT idx = GetSelectedItem();
	LFItemDescriptor* item = (idx==-1) ? NULL : p_CookedFiles->m_Items[idx];

	switch (pCmdUI->m_nID)
	{
	case IDM_FILE_OPENWITH:
		if (item)
			b = ((item->Type & (LFTypeNotMounted | LFTypeMask))==LFTypeFile) && (item->CoreAttributes.ContextID!=LFContextFilters);
		break;
	case IDM_FILE_OPENBROWSER:
		if (item)
			b = ((item->Type & LFTypeMask)==LFTypeFile) && (item->CoreAttributes.URL[0]!='\0');
		break;
	case IDM_FILE_EDIT:
		if (item)
			b = ((item->Type & LFTypeMask)==LFTypeFile) && (item->CoreAttributes.ContextID==LFContextFilters);
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
		if ((item) && (m_Context!=LFContextArchive) && (m_Context!=LFContextTrash))
			b = ((item->Type & (LFTypeNotMounted | LFTypeMask))==LFTypeFile);
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
