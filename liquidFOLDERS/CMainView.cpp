
// CMainView.cpp: Implementierung der Klasse CMainView
//

#include "stdafx.h"
#include "CCalendarView.h"
#include "CDetailsView.h"
#include "CGlobeView.h"
#include "CIconsView.h"
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
	m_Context = LFContextAllFiles;
	m_ViewID = -1;
	m_Resizing = m_StoreIDValid = m_Alerted = FALSE;
}

BOOL CMainView::Create(CWnd* pParentWnd, UINT nID, BOOL IsClipboard)
{
	m_IsClipboard = IsClipboard;

	const CString className = AfxRegisterWndClass(CS_DBLCLKS, theApp.LoadStandardCursor(IDC_ARROW));

	return CFrontstageWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP, CRect(0, 0, 0, 0), pParentWnd, nID);
}

BOOL CMainView::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The file view gets the command first
	if (m_pWndFileView && m_pWndFileView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// Check application commands
	if (theApp.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CFrontstageWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CMainView::CreateFileView(UINT ViewID, FVPersistentData* pPersistentData)
{
	// Same view, re-use control?
	if ((INT)ViewID==m_ViewID)
		return FALSE;

	// Create new view
	CFileView* pVictim = m_pWndFileView;

	switch (ViewID)
	{
	case LFViewIcons:
		m_pWndFileView = new CIconsView();
		break;

	case LFViewDetails:
		m_pWndFileView = new CDetailsView();
		break;

	case LFViewList:
		m_pWndFileView = new CListView();
		break;

	case LFViewCalendar:
		m_pWndFileView = new CCalendarView();
		break;

	case LFViewTimeline:
		m_pWndFileView = new CTimelineView();
		break;

	case LFViewGlobe:
		m_pWndFileView = new CGlobeView();
		break;

	case LFViewTagcloud:
		m_pWndFileView = new CTagcloudView();
		break;
	}

	// Exchange control
	CRect rect;

	if (pVictim)
	{
		pVictim->GetWindowRect(rect);
		ScreenToClient(rect);
	}

	m_pWndFileView->Create(this, FileViewID, rect, &m_LargeIcons, &m_wndInspectorPane, p_Filter, p_RawFiles, p_CookedFiles, pPersistentData);

	if ((GetFocus()==pVictim) || (GetTopLevelParent()==GetActiveWindow()))
		m_pWndFileView->SetFocus();

	if (pVictim)
	{
		pVictim->DestroyWindow();
		delete pVictim;
	}

	m_ViewID = ViewID;

	return TRUE;
}

void CMainView::SetHeaderButtons()
{
	ASSERT(p_OrganizeButton);
	ASSERT(p_ViewButton);

	const ATTRIBUTE Attr = m_pWndFileView ? m_pWndFileView->GetSortAttribute() : theApp.m_ContextViewSettings[m_Context].SortBy;
	p_OrganizeButton->SetValue(theApp.GetAttributeName(Attr, m_Context), FALSE);

	p_ViewButton->SetValue(CString((LPCSTR)IDM_SETVIEW_FIRST+m_ViewID));
}

void CMainView::SetHeader()
{
	if (!p_CookedFiles)
	{
		// Empty header
		m_wndHeaderArea.SetHeader();
	}
	else
	{
		// Hint
		WCHAR tmpStr[256];
		LFGetFileSummaryEx(tmpStr, 256, p_CookedFiles->m_FileSummary);

		CString Hint(tmpStr);

		// Use hint from search result
		LPCWSTR pHint = p_CookedFiles->m_Hint;

		// If we show all files, use store comments (if we have a valid store ID)
		LFStoreDescriptor StoreDescriptor;
		if ((m_Context==LFContextAllFiles) && m_StoreIDValid)
			if (LFGetStoreSettings(m_StoreID, StoreDescriptor)==LFOk)
			{
				wcscpy_s(p_RawFiles->m_Name, 256, StoreDescriptor.StoreName);
				wcscpy_s(p_CookedFiles->m_Name, 256, StoreDescriptor.StoreName);

				pHint = StoreDescriptor.Comments;
			}

		// Merge hint and file count/size
		if (pHint && (*pHint!=L'\0'))
		{
			Hint.Insert(0, (GetThreadLocale() & 0x1FF)==LANG_ENGLISH ? _T("—") : _T(" – "));
			Hint.Insert(0, pHint);
		}

		CPoint BitmapOffset;
		m_wndHeaderArea.SetHeader(p_CookedFiles->m_Name, Hint, theApp.m_IconFactory.GetHeaderBitmap(p_RawFiles, p_Filter, m_ViewID, BitmapOffset), BitmapOffset, FALSE);

		SetHeaderButtons();

		GetTopLevelParent()->SetWindowText(p_CookedFiles->m_Name);
	}

	// The header can change its height!
	AdjustLayout();
}

void CMainView::UpdateViewSettings()
{
	if (m_pWndFileView)
	{
		FVPersistentData Data;
		GetPersistentData(Data);

		if (!CreateFileView(theApp.m_ContextViewSettings[m_Context].View, &Data))
			m_pWndFileView->UpdateViewSettings(m_Context);

		SetHeaderButtons();
	}
}

void CMainView::UpdateSearchResult()
{
	FVPersistentData Data;
	GetPersistentData(Data, TRUE);

	UpdateSearchResult(p_Filter, p_RawFiles, p_CookedFiles, &Data, FALSE);
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
		m_StoreID = pFilter->Query.StoreID;
		m_StoreIDValid = !LFIsDefaultStoreID(m_StoreID);
	}

	if (!pCookedFiles)
	{
		if (m_pWndFileView)
			m_pWndFileView->UpdateSearchResult(NULL, NULL, NULL, NULL);
	}
	else
	{
		if (!CreateFileView(theApp.m_ContextViewSettings[m_Context=pCookedFiles->m_Context].View, pPersistentData))
		{
			m_pWndFileView->UpdateViewSettings(m_Context, TRUE);
			m_pWndFileView->UpdateSearchResult(pFilter, pRawFiles, pCookedFiles, pPersistentData);
		}

		m_DropTarget.SetStore(pFilter);
	}

	if (m_IsClipboard)
		m_DropTarget.SetSearchResult(pRawFiles);

	SetHeader();

	if (UpdateSelection)
		SelectionChanged();
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

BOOL CMainView::GetContextMenu(CMenu& Menu, INT Index)
{
	return m_pWndFileView ? m_pWndFileView->GetContextMenu(Menu, Index) : FALSE;
}

void CMainView::GetPersistentData(FVPersistentData& Data, BOOL ForReload) const
{
	if (m_pWndFileView)
	{
		m_pWndFileView->GetPersistentData(Data, ForReload);
	}
	else
	{
		ZeroMemory(&Data, sizeof(Data));
	}
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

	const INT MaxWidth = max(m_wndInspectorPane.GetMinWidth(), (rect.right-128)/2);
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

void CMainView::SelectNone()
{
	if (m_pWndFileView)
		m_pWndFileView->SelectNone();
}

LFTransactionList* CMainView::BuildTransactionList(BOOL All, BOOL ResolveLocations, BOOL IncludePIDL)
{
	LFTransactionList* pTransactionList = LFAllocTransactionList(p_RawFiles, All);

	if (ResolveLocations)
		ShowNotification(LFDoTransaction(pTransactionList, LFTransactionResolveLocations, NULL, IncludePIDL));

	return pTransactionList;
}

void CMainView::RemoveTransactedItems(LFTransactionList* pTransactionList)
{
	if (!p_RawFiles)
		return;

	// Unselect all files
	if (m_pWndFileView)
		m_pWndFileView->SendMessage(IDM_ITEMVIEW_SELECTNONE);

	// Remove items from raw search result
	for (UINT a=0; a<pTransactionList->m_ItemCount; a++)
		if (((*pTransactionList)[a].LastError==LFOk) && (*pTransactionList)[a].Processed)
			(*pTransactionList)[a].pItemDescriptor->RemoveFlag = TRUE;

	LFRemoveFlaggedItems(p_RawFiles);

	// Cook search result
	FVPersistentData Data;
	GetPersistentData(Data);
	GetOwner()->SendMessage(WM_COOKFILES, (WPARAM)&Data);
}

BOOL CMainView::DeleteFiles(BOOL Trash, BOOL All)
{
	LFTransactionList* pTransactionList = BuildTransactionList(All);

	if (Trash)
	{
		CWaitCursor WaitCursor;

		LFDoTransaction(pTransactionList, LFTransactionPutInTrash);
	}
	else
	{
		WorkerDeleteParameters Parameters;
		ZeroMemory(&Parameters, sizeof(Parameters));
		Parameters.pTransactionList = pTransactionList;

		LFDoWithProgress(WorkerDelete, &Parameters.Hdr, this);
	}

	RemoveTransactedItems(pTransactionList);

	// Show notification
	ShowNotification(pTransactionList->m_LastError);

	const BOOL Modified = pTransactionList->m_Modified;
	LFFreeTransactionList(pTransactionList);

	return Modified;
}

void CMainView::RecoverFiles(BOOL All)
{
	CWaitCursor WaitCursor;

	LFTransactionList* pTransactionList = BuildTransactionList(All);
	LFDoTransaction(pTransactionList, LFTransactionRecover);
	RemoveTransactedItems(pTransactionList);

	// Show notification
	ShowNotification(pTransactionList->m_LastError);

	LFFreeTransactionList(pTransactionList);
}

BOOL CMainView::UpdateItems(const LFVariantData* pValue1, const LFVariantData* pValue2, const LFVariantData* pValue3)
{
	CWaitCursor WaitCursor;

	LFTransactionList* pTransactionList = BuildTransactionList();
	LFDoTransaction(pTransactionList, LFTransactionUpdate, NULL, NULL, pValue1, pValue2, pValue3);

	if (m_pWndFileView && pTransactionList->m_Modified)
	{
		// Update folder colors
		if ((pValue1 && (pValue1->Attr==LFAttrColor)) || (pValue2 && (pValue2->Attr==LFAttrColor)) || (pValue3 && (pValue3->Attr==LFAttrColor)))
			LFUpdateFolderColors(p_CookedFiles, p_RawFiles);

		// Update search result
		UpdateSearchResult();
	}

	// Show notification
	ShowNotification(pTransactionList->m_LastError);

	const BOOL Changes = pTransactionList->m_Modified;
	LFFreeTransactionList(pTransactionList);

	return Changes;
}

void CMainView::SelectionChanged()
{
	m_wndInspectorPane.AggregateInitialize(m_Context);

	if (p_CookedFiles)
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			const LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[a];

			if (CFileView::IsItemSelected(pItemDescriptor))
				m_wndInspectorPane.AggregateAdd(pItemDescriptor, p_RawFiles);
		}

	m_wndInspectorPane.AggregateClose();
	m_wndTaskbar.PostMessage(WM_IDLEUPDATECMDUI);
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
	ON_WM_INITMENUPOPUP()

	ON_MESSAGE_VOID(WM_ADJUSTLAYOUT, OnAdjustLayout)
	ON_NOTIFY(IVN_SELECTIONCHANGED, FileViewID, OnSelectionChanged)
	ON_NOTIFY(IVN_BEGINDRAGANDDROP, FileViewID, OnBeginDragAndDrop)
	ON_MESSAGE(WM_RENAMEITEM, OnRenameItem)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoreAttributesChanged)

	ON_COMMAND(ID_PANE_INSPECTOR, OnToggleInspector)
	ON_UPDATE_COMMAND_UI(ID_PANE_INSPECTOR, OnUpdatePaneCommands)

	ON_BN_CLICKED(IDM_ORGANIZE, OnOrganizeButton)
	ON_COMMAND(IDM_ORGANIZE_OPTIONS, OnOrganizeOptions)
	ON_COMMAND_RANGE(IDM_SETORGANIZE_FIRST, IDM_SETORGANIZE_FIRST+LFAttributeCount-1, OnSetOrganize)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_ORGANIZE, IDM_ORGANIZE_OPTIONS, OnUpdateHeaderCommands)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_SETORGANIZE_FIRST, IDM_SETORGANIZE_FIRST+LFAttributeCount-1, OnUpdateSetOrganizeCommands)

	ON_BN_CLICKED(IDM_VIEW, OnViewButton)
	ON_COMMAND_RANGE(IDM_SETVIEW_FIRST, IDM_SETVIEW_FIRST+LFViewCount-1, OnSetView)
	ON_UPDATE_COMMAND_UI(IDM_VIEW, OnUpdateHeaderCommands)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_SETVIEW_FIRST, IDM_SETVIEW_FIRST+LFViewCount-1, OnUpdateSetViewCommands)

	ON_COMMAND(IDM_STORES_ADD, OnStoresAdd)
	ON_COMMAND(IDM_STORES_SYNCHRONIZE, OnStoresSynchronize)
	ON_COMMAND(IDM_STORES_RUNMAINTENANCE, OnStoresRunMaintenance)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORES_ADD, IDM_STORES_RUNMAINTENANCE, OnUpdateStoresCommands)

	ON_COMMAND(IDM_FONTS_SHOWINSTALLED, OnFontsShowInstalled)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FONTS_SHOWINSTALLED, IDM_FONTS_SHOWINSTALLED, OnUpdateFontsCommands)

	ON_COMMAND(IDM_NEW_CLEARNEW, OnNewClearNew)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_NEW_CLEARNEW, IDM_NEW_CLEARNEW, OnUpdateNewCommands)

	ON_COMMAND(IDM_TRASH_EMPTY, OnTrashEmpty)
	ON_COMMAND(IDM_TRASH_RECOVERALL, OnTrashRecoverAll)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_TRASH_EMPTY, IDM_TRASH_RECOVERALL, OnUpdateTrashCommands)

	ON_COMMAND(IDM_FILTERS_CREATENEW, OnFiltersCreateNew)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILTERS_CREATENEW, IDM_FILTERS_EDIT, OnUpdateFiltersCommands)

	ON_COMMAND(IDM_ITEM_OPEN, OnItemOpen)
	ON_COMMAND_RANGE(FIRSTSENDTO, LASTSENDTO, OnItemSendTo)
	ON_UPDATE_COMMAND_UI(IDM_ITEM_OPEN, OnUpdateItemCommands)

	ON_COMMAND(IDM_STORE_OPENNEWWINDOW, OnStoreOpenNewWindow)
	ON_COMMAND(IDM_STORE_OPENFILEDROP, OnStoreOpenFileDrop)
	ON_COMMAND(IDM_STORE_SYNCHRONIZE, OnStoreSynchronize)
	ON_COMMAND(IDM_STORE_MAKEDEFAULT, OnStoreMakeDefault)
	ON_COMMAND(IDM_STORE_SHORTCUT, OnItemShortcut)
	ON_COMMAND(IDM_STORE_DELETE, OnStoreDelete)
	ON_COMMAND(IDM_STORE_RENAME, OnStoreRename)
	ON_COMMAND(IDM_STORE_PROPERTIES, OnStoreProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORE_OPENFILEDROP, IDM_STORE_PROPERTIES, OnUpdateStoreCommands)

	ON_COMMAND(IDM_FILE_OPENWITH, OnFileOpenWith)
	ON_COMMAND(IDM_FILE_SHOWEXPLORER, OnFileShowExplorer)
	ON_COMMAND(IDM_FILE_EDIT, OnFileEdit)
	ON_COMMAND(IDM_FILE_REMEMBER, OnFileRemember)
	ON_COMMAND(IDM_FILE_REMOVEFROMCLIPBOARD, OnFileRemoveFromClipboard)
	ON_COMMAND(IDM_FILE_MAKETASK, OnFileMakeTask)
	ON_COMMAND(IDM_FILE_ARCHIVE, OnFileArchive)
	ON_COMMAND(IDM_FILE_COPY, OnFileCopy)
	ON_COMMAND(IDM_FILE_SHORTCUT, OnItemShortcut)
	ON_COMMAND(IDM_FILE_DELETE, OnFileDelete)
	ON_COMMAND(IDM_FILE_RENAME, OnFileRename)
	ON_COMMAND(IDM_FILE_PROPERTIES, OnFileProperties)
	ON_COMMAND(IDM_FILE_TASKDONE, OnFileTaskDone)
	ON_COMMAND(IDM_FILE_RECOVER, OnFileRecover)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILE_OPENWITH, IDM_FILE_RECOVER, OnUpdateFileCommands)

	ON_COMMAND_RANGE(IDM_FILE_MOVETOCONTEXT, IDM_FILE_MOVETOCONTEXT+LFContextCount-1, OnFileMoveToContext)
END_MESSAGE_MAP()

INT CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Taskbar
	if (!m_wndTaskbar.Create(this, m_LargeIcons, m_SmallIcons, IDB_TASKS_16, 1))
		return -1;

	m_wndTaskbar.AddButton(IDM_STORES_ADD, 0);
	m_wndTaskbar.AddButton(IDM_STORES_SYNCHRONIZE, 1);
	m_wndTaskbar.AddButton(IDM_FONTS_SHOWINSTALLED, 2, TRUE);
	m_wndTaskbar.AddButton(IDM_NEW_CLEARNEW, 3, TRUE);
	m_wndTaskbar.AddButton(IDM_FILE_TASKDONE, 4, TRUE);
	m_wndTaskbar.AddButton(IDM_TRASH_EMPTY, 5, TRUE);
	m_wndTaskbar.AddButton(IDM_TRASH_RECOVERALL, 6, TRUE);
	m_wndTaskbar.AddButton(IDM_FILE_RECOVER, 7);
	m_wndTaskbar.AddButton(IDM_FILTERS_CREATENEW, 8);
	m_wndTaskbar.AddButton(IDM_CALENDAR_PREVYEAR, 9, TRUE);
	m_wndTaskbar.AddButton(IDM_CALENDAR_NEXTYEAR, 10, TRUE);
	m_wndTaskbar.AddButton(IDM_CALENDAR_GOTOYEAR, 11);
	m_wndTaskbar.AddButton(IDM_GLOBE_JUMPTOLOCATION, 12, TRUE);
	m_wndTaskbar.AddButton(IDM_GLOBE_ZOOMIN, 13);
	m_wndTaskbar.AddButton(IDM_GLOBE_ZOOMOUT, 14);
	m_wndTaskbar.AddButton(IDM_GLOBE_AUTOSIZE, 15);
	m_wndTaskbar.AddButton(IDM_TAGCLOUD_SORTVALUE, 16, TRUE);
	m_wndTaskbar.AddButton(IDM_TAGCLOUD_SORTCOUNT, 17, TRUE);

	m_wndTaskbar.AddButton(IDM_ITEM_OPEN, 18, TRUE);

	m_wndTaskbar.AddButton(IDM_GLOBE_GOOGLEEARTH, 19, TRUE);
	m_wndTaskbar.AddButton(IDM_STORE_MAKEDEFAULT, 20);
	m_wndTaskbar.AddButton(IDM_STORE_PROPERTIES, 21);
	m_wndTaskbar.AddButton(IDM_FILE_REMEMBER, 22);
	m_wndTaskbar.AddButton(IDM_FILE_REMOVEFROMCLIPBOARD, 23, TRUE);
	m_wndTaskbar.AddButton(IDM_FILE_MAKETASK, 24);
	m_wndTaskbar.AddButton(IDM_FILE_ARCHIVE, 25);
	m_wndTaskbar.AddButton(IDM_FILE_DELETE, 26);
	m_wndTaskbar.AddButton(IDM_FILE_RENAME, 27);

	#define INSPECTORICONVISIBLE     28
	#define INSPECTORICONHIDDEN      29
	p_InspectorButton = m_wndTaskbar.AddButton(ID_PANE_INSPECTOR, theApp.m_ShowInspectorPane ? INSPECTORICONVISIBLE : INSPECTORICONHIDDEN, TRUE, TRUE);

	m_wndTaskbar.AddButton(IDM_BACKSTAGE_PURCHASE, 30, TRUE, TRUE);
	m_wndTaskbar.AddButton(IDM_BACKSTAGE_ENTERLICENSEKEY, 31, TRUE, TRUE);
	m_wndTaskbar.AddButton(IDM_BACKSTAGE_SUPPORT, 32, TRUE, TRUE);
	m_wndTaskbar.AddButton(IDM_BACKSTAGE_ABOUT, 33, TRUE, TRUE);

	// Drop target
	m_DropTarget.Register(this);

	// Explorer header
	if (!m_wndHeaderArea.Create(this, 2, TRUE))
		return -1;

	p_OrganizeButton = m_wndHeaderArea.AddButton(IDM_ORGANIZE);
	p_ViewButton = m_wndHeaderArea.AddButton(IDM_VIEW);

	// Inspector
	if (!m_wndInspectorPane.Create(this, 4, FALSE, theApp.m_InspectorPaneWidth, TRUE))
		return -1;

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

void CMainView::OnSize(UINT nType, INT cx, INT cy)
{
	CFrontstageWnd::OnSize(nType, cx, cy);

	AdjustLayout(SWP_NOACTIVATE | SWP_NOZORDER | SWP_NOREDRAW | SWP_NOCOPYBITS);
}

void CMainView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	(m_pWndFileView ? (CWnd*)m_pWndFileView : (CWnd*)&m_wndTaskbar)->SetFocus();
}

void CMainView::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/)
{
	SelectNone();
	SetFocus();
}

void CMainView::OnRButtonUp(UINT /*nFlags*/, CPoint point)
{
	SelectNone();

	ClientToScreen(&point);

	CMenu Menu;
	TrackPopupMenu(Menu, point, this, GetContextMenu(Menu, -1));
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

void CMainView::OnInitMenuPopup(CMenu* pMenuPopup, UINT nIndex, BOOL bSysMenu)
{
	ASSERT(pMenuPopup);

	CFrontstageWnd::OnInitMenuPopup(pMenuPopup, nIndex, bSysMenu);

	// Replace %u with actual file count
	CString strFileCount;
	strFileCount.Format(_T("%u"), m_wndInspectorPane.GetFileCount());

	for (UINT uItem=0; uItem<pMenuPopup->GetMenuItemCount(); uItem++)
	{
		WCHAR strMask[256];
		WCHAR strMenuString[256];

		// Get menu item info
		MENUITEMINFO MenuItemInfo;
		MenuItemInfo.cbSize = sizeof(MenuItemInfo);
		MenuItemInfo.fMask = MIIM_TYPE | MIIM_DATA;
		MenuItemInfo.dwTypeData = strMask;
		MenuItemInfo.cch = sizeof(strMask)/sizeof(WCHAR);

		pMenuPopup->GetMenuItemInfo(uItem, &MenuItemInfo, TRUE);

		// When a string contains %u: replace it
		if ((MenuItemInfo.fType==MFT_STRING) && wcsstr(strMask, L"%u"))
		{
			swprintf_s(strMenuString, 256, strMask, m_wndInspectorPane.GetFileCount());

			// Set menu item info
			MenuItemInfo.dwTypeData = strMenuString;
			pMenuPopup->SetMenuItemInfo(uItem, &MenuItemInfo, TRUE);
		}
	}
}


// Messages and notifications

void CMainView::OnAdjustLayout()
{
	if (!m_Resizing)
		AdjustLayout();
}

void CMainView::OnSelectionChanged(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	SelectionChanged();

	*pResult = 0;
}

void CMainView::OnBeginDragAndDrop(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	LFDataSource *pDataSource = NULL;

	if (m_Context==LFContextStores)
	{
		// Stores have no physical path that could be resolved via LFTransactionList
		const INT Index = GetSelectedItem();
		if (Index!=-1)
		{
			LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

			if (pItemDescriptor->Type & LFTypeShortcutAllowed)
				pDataSource = new LFDataSource(pItemDescriptor);
		}
	}
	else
	{
		// All other contexts
		LFTransactionList* pTransactionList = BuildTransactionList(FALSE, TRUE);

		if (!pTransactionList->m_LastError && pTransactionList->m_ItemCount)
			pDataSource = new LFDataSource(pTransactionList);

		LFFreeTransactionList(pTransactionList);
	}

	// Do drag and drop
	if (pDataSource)
	{
		// Disable local drop target
		m_DropTarget.SetDragSource(TRUE);

		LFDropSource DropSource(pDataSource);
		pDataSource->DoDragDrop(DROPEFFECT_COPY | DROPEFFECT_LINK, NULL, &DropSource);

		delete pDataSource;

		// Enable local drop target
		m_DropTarget.SetDragSource(FALSE);

		*pResult = 0;
	}
	else
	{
		*pResult = 1;
	}
}

LRESULT CMainView::OnRenameItem(WPARAM wParam, LPARAM lParam)
{
	LFTransactionList* pTransactionList = LFAllocTransactionList();
	LFAddTransactionItem(pTransactionList, (*p_CookedFiles)[(UINT)wParam]);

	LFVariantData VData;
	LFInitVariantData(VData, LFAttrFileName);

	wcsncpy_s(VData.UnicodeString, 256, (LPCWSTR)lParam, _TRUNCATE);
	VData.IsNull = FALSE;

	LFDoTransaction(pTransactionList, LFTransactionUpdate, NULL, NULL, &VData);

	if (pTransactionList->m_Modified)
	{
		FVPersistentData Data;
		GetPersistentData(Data);
		UpdateSearchResult(p_Filter, p_RawFiles, p_CookedFiles, &Data);
	}

	// Show notification
	ShowNotification(pTransactionList->m_LastError);
	LFFreeTransactionList(pTransactionList);

	return NULL;
}

LRESULT CMainView::OnStoreAttributesChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (p_RawFiles && p_CookedFiles && (m_Context<=LFLastQueryContext))
		SetHeader();

	return NULL;
}


// Inspector pane

void CMainView::OnToggleInspector()
{
	ASSERT(p_InspectorButton);

	theApp.m_ShowInspectorPane = m_ShowInspectorPane = !m_ShowInspectorPane;
	p_InspectorButton->SetIconID(m_ShowInspectorPane ? INSPECTORICONVISIBLE : INSPECTORICONHIDDEN);
	AdjustLayout();
}

void CMainView::OnUpdatePaneCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


// Header

void CMainView::OnUpdateHeaderCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}


// Organize

void CMainView::OnOrganizeButton()
{
	CMenu Menu;
	Menu.CreatePopupMenu();

	const SUBFOLDERATTRIBUTE SubfolderAttribute = LFGetSubfolderAttribute(p_Filter);

	for (UINT a=0; a<LFAttributeCount; a++)
		if (theApp.IsAttributeSortable(a, m_Context, SubfolderAttribute) && theApp.IsAttributeAdvertised(a, m_Context))
			Menu.AppendMenu(MF_STRING, IDM_SETORGANIZE_FIRST+a, _T("&")+CString(theApp.GetAttributeName(a, m_Context)));

	Menu.AppendMenu(MF_SEPARATOR);
	Menu.AppendMenu(MF_STRING, IDM_ORGANIZE_OPTIONS, CString((LPCSTR)IDS_CONTEXTMENU_MORE));

	m_wndHeaderArea.TrackPopupMenu(Menu, IDM_ORGANIZE);
}

void CMainView::OnOrganizeOptions()
{
	OrganizeDlg(this, m_Context).DoModal();
}

void CMainView::OnSetOrganize(UINT nID)
{
	theApp.SetContextSort(m_Context, nID-IDM_SETORGANIZE_FIRST, theApp.IsAttributeSortDescending(nID-IDM_SETORGANIZE_FIRST, m_Context));

	SetFocus();
}

void CMainView::OnUpdateSetOrganizeCommands(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(pCmdUI->m_nID-IDM_SETORGANIZE_FIRST==theApp.m_ContextViewSettings[m_Context].SortBy);
}


// View

void CMainView::OnViewButton()
{
	CMenu Menu;
	Menu.CreatePopupMenu();

	UINT AllowedViews = theApp.m_Attributes[theApp.m_ContextViewSettings[m_Context].SortBy].TypeProperties.AllowedViews;

	for (UINT a=0; a<LFViewCount; a++, AllowedViews>>=1)
		if ((AllowedViews & 1) && theApp.IsViewAllowed(m_Context, a))
		{
			const UINT nID = IDM_SETVIEW_FIRST+a;

			Menu.AppendMenu(MF_STRING, nID, _T("&")+CString((LPCSTR)nID));
		}

	m_wndHeaderArea.TrackPopupMenu(Menu, IDM_VIEW);
}

void CMainView::OnSetView(UINT nID)
{
	theApp.SetContextView(m_Context, nID-IDM_SETVIEW_FIRST);

	SetFocus();
}

void CMainView::OnUpdateSetViewCommands(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio((INT)(pCmdUI->m_nID-IDM_SETVIEW_FIRST)==m_ViewID);
}


// Stores

void CMainView::OnStoresAdd()
{
	// Allowed?
	if (LFNagScreen(this))
	{
		CWaitCursor WaitCursor;
		LFAddStoreDlg(this).DoModal();
	}
}

void CMainView::OnStoresSynchronize()
{
	LFRunSynchronizeStores(DEFAULTSTOREID(), this);
}

void CMainView::OnStoresRunMaintenance()
{
	DismissNotification();

	LFRunStoreMaintenance(this);
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


// Fonts

void CMainView::OnFontsShowInstalled()
{
	WCHAR Path[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_FONTS, NULL, SHGFP_TYPE_CURRENT, Path)))
		ShellExecute(GetSafeHwnd(), _T("open"), Path, NULL, NULL, SW_SHOWNORMAL);
}

void CMainView::OnUpdateFontsCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(m_Context==LFContextFonts);
}


// New

void CMainView::OnNewClearNew()
{
	RecoverFiles(TRUE);
}

void CMainView::OnUpdateNewCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(((m_Context==LFContextNew) && p_CookedFiles) ? p_CookedFiles->m_ItemCount : FALSE);
}


// Trash

void CMainView::OnTrashRecoverAll()
{
	RecoverFiles(TRUE);
}

void CMainView::OnTrashEmpty()
{
	if (DeleteFiles(FALSE, TRUE))
		theApp.PlayTrashSound();
}

void CMainView::OnUpdateTrashCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = (m_Context==LFContextTrash) && p_CookedFiles ? p_CookedFiles->m_ItemCount : FALSE;

	const INT Index = GetSelectedItem();
	if (Index!=-1)
		if (pCmdUI->m_nID==IDM_TRASH_RECOVERALL)
			bEnable = FALSE;

	pCmdUI->Enable(bEnable);
}


// Filters

void CMainView::OnFiltersCreateNew()
{
	if (LFEditFilterDlg(m_StoreID, this).DoModal()==IDOK)
		GetTopLevelParent()->PostMessage(WM_COMMAND, ID_NAV_RELOAD);
}

void CMainView::OnUpdateFiltersCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = (m_Context==LFContextFilters);

	const INT Index = GetSelectedItem();
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


// Items

void CMainView::OnItemOpen()
{
	const INT Index = GetSelectedItem();
	if (Index!=-1)
	{
		LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

		if (pItemDescriptor->pNextFilter)
		{
			// Navigate to item filter
			LFFilter* pFilter = pItemDescriptor->pNextFilter;
			pItemDescriptor->pNextFilter = NULL;

			GetTopLevelParent()->SendMessage(WM_NAVIGATETO, (WPARAM)pFilter, (LPARAM)pItemDescriptor);
		}
		else
		{
			// Only files have no navigable filter
			ASSERT(LFIsFile(pItemDescriptor));

			if (pItemDescriptor->Type & LFTypeMounted)
			{
				WCHAR Path[MAX_PATH];
				UINT Result;

				// Is it a .filter file?
				if (_stricmp(pItemDescriptor->CoreAttributes.FileFormat, "filter")==0)
				{
					// Yes, load filter and navigate to it
					LFFilter* pFilter = LFLoadFilter(pItemDescriptor);

					if (pFilter)
						GetTopLevelParent()->SendMessage(WM_NAVIGATETO, (WPARAM)pFilter);
				}
				else
				{
					// Retrieve physical file location
					if ((Result=LFGetFileLocation(pItemDescriptor, Path, MAX_PATH))==LFOk)
					{
						// Launch file
						if (ShellExecute(GetSafeHwnd(), _T("open"), Path, NULL, NULL, SW_SHOWNORMAL)==(HINSTANCE)SE_ERR_NOASSOC)
							OnFileOpenWith();
					}
					else
					{
						LFErrorBox(this, Result);
					}
				}
			}
		}
	}
}

void CMainView::OnItemSendTo(UINT nID)
{
	const SendToItemData* pSendToItemData = m_pWndFileView ? m_pWndFileView->GetSendToItemData(nID) : NULL;

	// Item data present?
	if (!pSendToItemData)
		return;

	if (pSendToItemData->IsStore)
	{
		// Destination is a store
		WorkerSendToParameters Parameters;
		ZeroMemory(&Parameters, sizeof(Parameters));

		if (strcmp(pSendToItemData->StoreID, CHOOSESTOREID)==0)
		{
			LFChooseStoreDlg dlg(this);
			if (dlg.DoModal()!=IDOK)
				return;

			Parameters.StoreID = dlg.m_StoreID;
		}
		else
		{
			Parameters.StoreID = MAKEABSOLUTESTOREID(pSendToItemData->StoreID);
		}

		// Start worker
		Parameters.pTransactionList = BuildTransactionList();

		LFDoWithProgress(WorkerSendTo, &Parameters.Hdr, this);

		// Show notification
		ShowNotification(Parameters.pTransactionList->m_LastError);

		LFFreeTransactionList(Parameters.pTransactionList);
	}
	else
	{
		// Destination is a shell object
		IShellFolder* pDesktop = NULL;
		if (SUCCEEDED(SHGetDesktopFolder(&pDesktop)))
		{
			LPITEMIDLIST pidlFQ;
			if (SUCCEEDED(pDesktop->ParseDisplayName(NULL, NULL, (LPWSTR)pSendToItemData->Path, NULL, &pidlFQ, NULL)))
			{
				IShellFolder* pParentFolder;
				LPCITEMIDLIST pidlRel;
				if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder, (LPVOID*)&pParentFolder, &pidlRel)))
				{
					IDropTarget* pDropTarget;
					if (SUCCEEDED(pParentFolder->GetUIObjectOf(GetSafeHwnd(), 1, &pidlRel, IID_IDropTarget, NULL, (LPVOID*)&pDropTarget)))
					{
						LFTransactionList* pTransactionList = BuildTransactionList(FALSE, TRUE);
						if (pTransactionList->m_ItemCount && !pTransactionList->m_LastError)
						{
							CWaitCursor WaitCursor;

							LFDataSource DataSource(pTransactionList);
							IDataObject* pDataObject = DataSource.GetDataObject();

							POINTL pt = { 0, 0 };
							DROPEFFECT dwEffect = DROPEFFECT_COPY;
							if (SUCCEEDED(pDropTarget->DragEnter(pDataObject, MK_LBUTTON, pt, &dwEffect)))
							{
								pDropTarget->Drop(pDataObject, MK_LBUTTON, pt, &dwEffect);
							}
							else
							{
								pDropTarget->DragLeave();
							}
						}

						LFFreeTransactionList(pTransactionList);
						pDropTarget->Release();
					}

					pParentFolder->Release();
				}

				theApp.GetShellManager()->FreeItem(pidlFQ);
			}

			pDesktop->Release();
		}
	}
}

void CMainView::OnItemShortcut()
{
	const INT Index = GetSelectedItem();
	if (Index!=-1)
	{
		const UINT Result = LFCreateDesktopShortcutForItem((*p_CookedFiles)[Index]);

		// Show notification
		if (Result)
		{
			ShowNotification(Result);
		}
		else
		{
			ShowNotification(ENT_READY, CString((LPCSTR)IDS_SHORTCUTCREATED));
		}
	}
}

void CMainView::OnUpdateItemCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;

	const INT Index = GetSelectedItem();
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

void CMainView::OnStoreOpenNewWindow()
{
	const INT Index = GetSelectedItem();
	if (Index!=-1)
		(new CMainWnd())->Create((*p_CookedFiles)[Index]->StoreID);
}

void CMainView::OnStoreOpenFileDrop()
{
	const INT Index = GetSelectedItem();
	if (Index!=-1)
		theApp.OpenFileDrop((*p_CookedFiles)[Index]->StoreID);

	// Iconize own window
	GetTopLevelParent()->ShowWindow(SW_MINIMIZE);
}

void CMainView::OnStoreSynchronize()
{
	const INT Index = GetSelectedItem();
	if (Index!=-1)
		LFRunSynchronizeStores((*p_CookedFiles)[Index]->StoreID, this);
}

void CMainView::OnStoreMakeDefault()
{
	const INT Index = GetSelectedItem();
	if (Index!=-1)
		ShowNotification(LFSetDefaultStore((*p_CookedFiles)[Index]->StoreID));
}

void CMainView::OnStoreDelete()
{
	const INT Index = GetSelectedItem();
	if (Index!=-1)
		LFDeleteStore((*p_CookedFiles)[Index]->StoreID, this);
}

void CMainView::OnStoreRename()
{
	const INT Index = GetSelectedItem();
	if ((Index!=-1) && m_pWndFileView)
		m_pWndFileView->EditLabel(Index);
}

void CMainView::OnStoreProperties()
{
	const INT Index = GetSelectedItem();
	if (Index!=-1)
		LFStorePropertiesDlg((*p_CookedFiles)[Index]->StoreID, this).DoModal();
}

void CMainView::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	BOOL bEnable = FALSE;

	if (m_Context==LFContextStores)
	{
		const INT Index = GetSelectedItem();
		if (Index!=-1)
		{
			const LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

			if (LFIsStore(pItemDescriptor))
				switch (pCmdUI->m_nID)
				{
				case IDM_STORE_SYNCHRONIZE:
					bEnable = ((pItemDescriptor->Type & (LFTypeSynchronizeAllowed | LFTypeMounted | LFTypeWriteable))==(LFTypeSynchronizeAllowed | LFTypeMounted | LFTypeWriteable));
					break;

				case IDM_STORE_MAKEDEFAULT:
					bEnable = !(pItemDescriptor->Type & LFTypeDefault);
					break;

				case IDM_STORE_SHORTCUT:
					bEnable = (pItemDescriptor->Type & LFTypeShortcutAllowed);
					break;

				case IDM_STORE_DELETE:
					bEnable = (pItemDescriptor->Type & LFTypeManageable);
					break;

				case IDM_STORE_RENAME:
					bEnable = (pItemDescriptor->Type & LFTypeManageable) && !m_pWndFileView->IsEditing();
					break;

				default:
					bEnable = TRUE;
				}
		}
	}

	pCmdUI->Enable(bEnable);
}


// File

void CMainView::OnFileOpenWith()
{
	const INT Index = GetSelectedItem();
	if (Index!=-1)
	{
		UINT Result;
		WCHAR Path[MAX_PATH];
		if ((Result=LFGetFileLocation((*p_CookedFiles)[Index], Path, MAX_PATH))==LFOk)
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

void CMainView::OnFileShowExplorer()
{
	const INT Index = GetSelectedItem();
	if (Index!=-1)
	{
		UINT Result;
		WCHAR Path[MAX_PATH];
		if ((Result=LFGetFileLocation((*p_CookedFiles)[Index], Path, MAX_PATH))==LFOk)
		{
			theApp.OpenFolderAndSelectItem(Path);
		}
		else
		{
			LFErrorBox(this, Result);
		}
	}
}

void CMainView::OnFileEdit()
{
	const INT Index = GetSelectedItem();
	if ((Index!=-1) && (_stricmp((*p_CookedFiles)[Index]->CoreAttributes.FileFormat, "filter")==0))
	{
		LFFilter* pFilter = LFLoadFilter((*p_CookedFiles)[Index]);

		if (LFEditFilterDlg(pFilter && !LFIsDefaultStoreID(pFilter->Query.StoreID) ? pFilter->Query.StoreID : m_StoreID, this, pFilter).DoModal()==IDOK)
			GetTopLevelParent()->PostMessage(WM_COMMAND, ID_NAV_RELOAD);

		LFFreeFilter(pFilter);
	}
}

void CMainView::OnFileRemember()
{
	ASSERT(p_RawFiles);

	CMainWnd* pClipboardWnd = theApp.GetClipboard();

	BOOL Changes = FALSE;
	BOOL First = TRUE;

	for (UINT a=0; a<p_RawFiles->m_ItemCount; a++)
	{
		const LFItemDescriptor* pItemDescriptor = (*p_RawFiles)[a];

		if (CFileView::IsItemSelected(pItemDescriptor))
			if (pClipboardWnd->AddClipItem(pItemDescriptor, First))
				Changes = TRUE;
	}

	if (Changes)
		pClipboardWnd->SendMessage(WM_COOKFILES);
}

void CMainView::OnFileRemoveFromClipboard()
{
	LFTransactionList* pTransactionList = BuildTransactionList();

	for (UINT a=0; a<pTransactionList->m_ItemCount; a++)
		(*pTransactionList)[a].Processed = TRUE;

	RemoveTransactedItems(pTransactionList);

	LFFreeTransactionList(pTransactionList);
}

void CMainView::OnFileMakeTask()
{
	// Default priority: 6 (normal/lime green)
	LFVariantData Priority;
	LFInitVariantData(Priority, LFAttrPriority);
	Priority.IsNull = FALSE;
	Priority.Rating = 4;

	// Default due time: none
	LFVariantData DueTime;
	LFInitVariantData(DueTime, LFAttrDueTime);

	if (LFMakeTaskDlg(&Priority, &DueTime, this).DoModal()==IDOK)
	{
		CWaitCursor WaitCursor;

		LFTransactionList* pTransactionList = BuildTransactionList();
		LFDoTransaction(pTransactionList, LFTransactionUpdateTask, NULL, NULL, &Priority, &DueTime);
		UpdateSearchResult();

		// Show notification
		ShowNotification(pTransactionList->m_LastError);

		LFFreeTransactionList(pTransactionList);
	}
}

void CMainView::OnFileArchive()
{
	CWaitCursor WaitCursor;

	LFTransactionList* pTransactionList = BuildTransactionList();
	LFDoTransaction(pTransactionList, LFTransactionArchive);
	RemoveTransactedItems(pTransactionList);

	// Show notification
	ShowNotification(pTransactionList->m_LastError);

	LFFreeTransactionList(pTransactionList);
}

void CMainView::OnFileCopy()
{
	LFTransactionList* pTransactionList = BuildTransactionList(FALSE, TRUE);
	if (pTransactionList->m_ItemCount && !pTransactionList->m_LastError)
	{
		CWaitCursor WaitCursor;

		LFDataSource* pDataSource = new LFDataSource(pTransactionList);

		pDataSource->SetClipboard();
		pDataSource->FlushClipboard();
		// pDataSource is now owned by clipboard, DO NOT DELETE!

		// Show notification
		WCHAR tmpStr[256];
		LFGetFileSummary(tmpStr, 256, pTransactionList->m_ItemCount);

		CString Text;
		Text.Format(IDS_FILESCOPIED, tmpStr);

		ShowNotification(ENT_READY, Text);
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
	const INT Index = GetSelectedItem();
	if ((Index!=-1) && m_pWndFileView)
		m_pWndFileView->EditLabel(Index);
}

void CMainView::OnFileProperties()
{
	if (!m_ShowInspectorPane)
	{
		ASSERT(p_InspectorButton);

		m_ShowInspectorPane = TRUE;
		p_InspectorButton->SetIconID(INSPECTORICONVISIBLE);
		AdjustLayout();
	}

	m_wndInspectorPane.SetFocus();
}

void CMainView::OnFileTaskDone()
{
	RecoverFiles();
}

void CMainView::OnFileRecover()
{
	RecoverFiles();
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
			bEnable = ((pItemDescriptor->Type & (LFTypeMask | LFTypeMounted))==(LFTypeFile | LFTypeMounted)) && !LFIsFilterFile(pItemDescriptor);

		break;

	case IDM_FILE_SHOWEXPLORER:
		if (pItemDescriptor)
			bEnable = ((pItemDescriptor->Type & (LFTypeMask | LFTypeMounted | LFTypeExplorerAllowed))==(LFTypeFile | LFTypeMounted | LFTypeExplorerAllowed)) && !LFIsFilterFile(pItemDescriptor);

		break;

	case IDM_FILE_EDIT:
		if (pItemDescriptor)
			bEnable = ((pItemDescriptor->Type & (LFTypeMask | LFTypeMounted))==(LFTypeFile | LFTypeMounted)) && LFIsFilterFile(pItemDescriptor);

		break;

	case IDM_FILE_REMEMBER:
		bEnable = m_wndInspectorPane.GetFileCount() && (m_Context!=LFContextClipboard) && (m_Context!=LFContextTrash);
		break;

	case IDM_FILE_REMOVEFROMCLIPBOARD:
		bEnable = m_wndInspectorPane.GetFileCount() && (m_Context==LFContextClipboard);
		break;

	case IDM_FILE_MAKETASK:
		bEnable = m_wndInspectorPane.GetFileCount() && (m_Context!=LFContextTasks) && (m_Context!=LFContextArchive) && (m_Context!=LFContextTrash);
		break;

	case IDM_FILE_ARCHIVE:
		bEnable = m_wndInspectorPane.GetFileCount() && (m_Context!=LFContextArchive) && (m_Context!=LFContextTrash);
		break;

	case IDM_FILE_COPY:
	case IDM_FILE_DELETE:
		bEnable = m_wndInspectorPane.GetFileCount();
		break;

	case IDM_FILE_SHORTCUT:
		if (pItemDescriptor && (m_Context!=LFContextArchive) && (m_Context!=LFContextTrash))
			bEnable = (pItemDescriptor->Type & (LFTypeMask | LFTypeMounted | LFTypeShortcutAllowed))==(LFTypeFile | LFTypeMounted | LFTypeShortcutAllowed);

		break;

	case IDM_FILE_RENAME:
		if (pItemDescriptor && (m_Context!=LFContextArchive) && (m_Context!=LFContextTrash))
			bEnable = ((pItemDescriptor->Type & (LFTypeMask | LFTypeMounted))==(LFTypeFile | LFTypeMounted));

		if (m_pWndFileView)
			bEnable &= !m_pWndFileView->IsEditing();

		break;

	case IDM_FILE_PROPERTIES:
		bEnable = m_wndInspectorPane.GetFileCount() && !m_ShowInspectorPane;
		break;

	case IDM_FILE_TASKDONE:
		bEnable = m_wndInspectorPane.GetFileCount() && (m_Context==LFContextTasks);
		break;

	case IDM_FILE_RECOVER:
		bEnable = m_wndInspectorPane.GetFileCount() && ((m_Context==LFContextArchive) || (m_Context==LFContextTrash));
		break;
	}

	pCmdUI->Enable(bEnable);
}


void CMainView::OnFileMoveToContext(UINT CmdID)
{
	CWaitCursor WaitCursor;

	LFTransactionList* pTransactionList = BuildTransactionList();
	LFDoTransaction(pTransactionList, LFTransactionUpdateUserContext, NULL, CmdID-IDM_FILE_MOVETOCONTEXT);
	RemoveTransactedItems(pTransactionList);

	// Show notification
	ShowNotification(pTransactionList->m_LastError);

	LFFreeTransactionList(pTransactionList);
}
