
// CMainView.cpp: Implementierung der Klasse CMainView
//

#include "stdafx.h"
#include "CMainView.h"
#include "CCalendarView.h"
#include "CGlobeView.h"
#include "CListView.h"
#include "CTagcloudView.h"
#include "CTimelineView.h"


// CMainView
//

#define FileViewID     3

CMainView::CMainView()
{
	p_wndFileView = NULL;
	p_RawFiles = p_CookedFiles = NULL;
	m_Context = m_ViewID = -1;
}

CMainView::~CMainView()
{
	if (p_wndFileView)
	{
		p_wndFileView->DestroyWindow();
		delete p_wndFileView;
	}
}

INT CMainView::Create(CWnd* _pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), dwStyle, rect, _pParentWnd, nID);
}

BOOL CMainView::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// The file view gets the command first
	if (p_wndFileView)
		if (p_wndFileView->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
			return TRUE;

	// Check application commands
	if (theApp.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

BOOL CMainView::CreateFileView(UINT ViewID, INT FocusItem)
{
	CFileView* pNewView = NULL;

	switch (ViewID)
	{
	case LFViewLargeIcons:
	case LFViewSmallIcons:
	case LFViewList:
	case LFViewDetails:
	case LFViewTiles:
	case LFViewSearchResult:
	case LFViewPreview:
		if ((m_ViewID<LFViewLargeIcons) || (m_ViewID>LFViewPreview))
		{
			pNewView = new CListView();
			((CListView*)pNewView)->Create(this, FileViewID, p_CookedFiles, FocusItem);
		}
		break;
	case LFViewCalendar:
		if (m_ViewID!=LFViewCalendar)
		{
			pNewView = new CCalendarView();
			((CCalendarView*)pNewView)->Create(this, FileViewID, p_CookedFiles, FocusItem);
		}
		break;
	case LFViewGlobe:
		if (m_ViewID!=LFViewGlobe)
		{
			pNewView = new CGlobeView();
			((CGlobeView*)pNewView)->Create(this, FileViewID, p_CookedFiles, FocusItem);
		}
		break;
	case LFViewTagcloud:
		if (m_ViewID!=LFViewTagcloud)
		{
			pNewView = new CTagcloudView();
			((CTagcloudView*)pNewView)->Create(this, FileViewID, p_CookedFiles, FocusItem);
		}
		break;
	case LFViewTimeline:
		if (m_ViewID!=LFViewTimeline)
		{
			pNewView = new CTimelineView();
			((CTimelineView*)pNewView)->Create(this, FileViewID, p_CookedFiles, FocusItem);
		}
		break;
	}

	m_ViewID = ViewID;

	// Exchange view
	if (pNewView)
	{
		if (p_wndFileView)
		{
			p_wndFileView->DestroyWindow();
			delete p_wndFileView;
		}

		p_wndFileView = pNewView;
		p_wndFileView->SetOwner(GetOwner());
		p_wndFileView->SetFocus();
		AdjustLayout();
	}

	return (pNewView!=NULL);
}

void CMainView::SetHeader()
{
	if (!p_CookedFiles)
	{
		m_wndExplorerHeader.SetText(_T(""), _T(""));
	}
	else
	{
		CString Hint;
		CString Mask;
		WCHAR tmpBuf[256];

		if (m_Context==LFContextStores)
		{
			ENSURE(Mask.LoadString(p_CookedFiles->m_ItemCount==1 ? IDS_STORES_SINGULAR : IDS_STORES_PLURAL));
			Hint.Format(Mask, p_CookedFiles->m_StoreCount);
		}
		else
		{
			ENSURE(Mask.LoadString(p_CookedFiles->m_FileCount==1 ? IDS_FILES_SINGULAR : IDS_FILES_PLURAL));
			LFINT64ToString(p_CookedFiles->m_FileSize, tmpBuf, 256);
			Hint.Format(Mask, p_CookedFiles->m_FileCount);
			Hint.Append(_T(" ("));
			Hint.Append(tmpBuf);
			Hint.Append(_T(")"));
		}

		if (m_Context==LFContextStoreHome)
		{
			LFStoreDescriptor s;
			if (LFGetStoreSettings(p_RawFiles->m_StoreID, &s)==LFOk)
			{
				wcscpy_s(p_RawFiles->m_Name, 256, s.StoreName);
				wcscpy_s(p_CookedFiles->m_Name, 256, s.StoreName);

				if (s.Comment[0]!=L'\0')
				{
					Hint.Insert(0, _T(" – "));
					Hint.Insert(0, s.Comment);
				}
			}
		}

		m_wndExplorerHeader.SetColors(m_Context<=LFContextClipboard ? 0x126E00 : 0x993300, (COLORREF)-1, FALSE);
		m_wndExplorerHeader.SetText(p_CookedFiles->m_Name, Hint);
	}
}

void CMainView::UpdateViewOptions(INT Context)
{
	if (((Context==m_Context) || (Context==-1)) && (p_wndFileView))
		if (!CreateFileView(theApp.m_Views[Context].Mode, GetFocusItem()))
			p_wndFileView->UpdateViewOptions(Context);
}

void CMainView::UpdateSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, INT FocusItem)
{
	p_RawFiles = pRawFiles;
	p_CookedFiles = pCookedFiles;

	if (FocusItem<0)
		FocusItem = 0;

	if (!pCookedFiles)
	{
		if (p_wndFileView)
			p_wndFileView->UpdateSearchResult(NULL, -1);
	}
	else
	{
		m_Context = pCookedFiles->m_Context;

		if (!CreateFileView(theApp.m_Views[pCookedFiles->m_Context].Mode, FocusItem))
		{
			p_wndFileView->UpdateViewOptions(m_Context);
			p_wndFileView->UpdateSearchResult(pCookedFiles, FocusItem);
		}
	}

	SetHeader();
	OnUpdateSelection();
}

void CMainView::ShowCaptionBar(LPCWSTR Icon, UINT res, UINT Command)
{
	// TODO
	((CMainFrame*)GetParent())->ShowCaptionBar(Icon, res, Command);
}

void CMainView::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
	m_wndTaskbar.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	const UINT ExplorerHeight = m_wndExplorerHeader.GetPreferredHeight();
	m_wndExplorerHeader.SetWindowPos(NULL, rect.left, rect.top+TaskHeight, rect.Width(), ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	if (p_wndFileView)
		p_wndFileView->SetWindowPos(NULL, rect.left, rect.top+TaskHeight+ExplorerHeight, rect.Width(), rect.Height()-ExplorerHeight-TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}

INT CMainView::GetFocusItem()
{
	return p_wndFileView ? p_wndFileView->GetFocusItem() : -1;
}

INT CMainView::GetSelectedItem()
{
	return p_wndFileView ? p_wndFileView->GetSelectedItem() : -1;
}

INT CMainView::GetNextSelectedItem(INT n)
{
	return p_wndFileView ? p_wndFileView->GetNextSelectedItem(n) : -1;
}

void CMainView::SelectNone()
{
	if (p_wndFileView)
		p_wndFileView->SendMessage(WM_SELECTNONE);
}

void CMainView::ExecuteContextMenu(CHAR Drive, LPCSTR verb)
{
	WCHAR Path[4] = L" :\\";
	Path[0] = Drive;

	LPITEMIDLIST pidlFQ = SHSimpleIDListFromPath(Path);
	LPCITEMIDLIST pidlRel = NULL;

	IShellFolder* pParentFolder = NULL;
	if (FAILED(SHBindToParent(pidlFQ, IID_IShellFolder, (void**)&pParentFolder, &pidlRel)))
		return;

	IContextMenu* pcm = NULL;
	if (SUCCEEDED(pParentFolder->GetUIObjectOf(GetSafeHwnd(), 1, &pidlRel, IID_IContextMenu, NULL, (void**)&pcm)))
	{
		HMENU hPopup = CreatePopupMenu();
		if (hPopup)
		{
			UINT uFlags = CMF_NORMAL | CMF_EXPLORE;
			if (SUCCEEDED(pcm->QueryContextMenu(hPopup, 0, 1, 0x6FFF, uFlags)))
			{
				CWaitCursor wait;

				CMINVOKECOMMANDINFO cmi;
				cmi.cbSize = sizeof(CMINVOKECOMMANDINFO);
				cmi.fMask = 0;
				cmi.hwnd = GetSafeHwnd();
				cmi.lpVerb = verb;
				cmi.lpParameters = NULL;
				cmi.lpDirectory = NULL;
				cmi.nShow = SW_SHOWNORMAL;
				cmi.dwHotKey = 0;
				cmi.hIcon = NULL;

				pcm->InvokeCommand(&cmi);
			}
		}
	}
}

void CMainView::AddTransactionItem(LFTransactionList* tl, LFItemDescriptor* item, UINT UserData)
{
	switch (item->Type & LFTypeMask)
	{
	case LFTypeFile:
	case LFTypeStore:
		LFAddItemDescriptor(tl, item, UserData);
		break;
	case LFTypeVirtual:
		if ((item->FirstAggregate!=-1) && (item->LastAggregate!=-1))
			for (INT a=item->FirstAggregate; a<=item->LastAggregate; a++)
				LFAddItemDescriptor(tl, p_RawFiles->m_Items[a], UserData);
		break;
	}
}

LFTransactionList* CMainView::BuildTransactionList(BOOL All)
{
	LFTransactionList* tl = NULL;

	if ((p_RawFiles) && (p_CookedFiles))
	{
		tl = LFAllocTransactionList();

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
	}

	return tl;
}

void CMainView::RemoveTransactedItems(LFTransactionList* tl)
{
	if (!p_RawFiles)
		return;

	for (UINT a=0; a<tl->m_ItemCount; a++)
		if (tl->m_Items[a].LastError==LFOk)
			tl->m_Items[a].Item->DeleteFlag = true;

	LFRemoveFlaggedItemDescriptors(p_RawFiles);
	// TODO
	//UpdateHistory();
	GetOwner()->SendMessage(WM_COOKFILES, GetFocusItem());
}

BOOL CMainView::UpdateTrashFlag(BOOL Trash, BOOL All)
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
	LFTransactionUpdate(tl, GetOwner()->GetSafeHwnd(), &value1, &value2);
	RemoveTransactedItems(tl);

	if (tl->m_LastError>LFCancel)
		ShowCaptionBar(IDI_ERROR, tl->m_LastError);

	BOOL changes = tl->m_Changes;
	LFFreeTransactionList(tl);
	return changes;
}

BOOL CMainView::DeleteFiles(BOOL All)
{
	LFTransactionList* tl = BuildTransactionList(All);
	LFTransactionDelete(tl);
	RemoveTransactedItems(tl);

	if (tl->m_LastError>LFCancel)
		ShowCaptionBar(IDI_ERROR, tl->m_LastError);

	BOOL changes = tl->m_Changes;
	LFFreeTransactionList(tl);
	return changes;
}

BOOL CMainView::UpdateItems(LFVariantData* value1, LFVariantData* value2, LFVariantData* value3)
{
	LFTransactionList* tl = BuildTransactionList();
	LFTransactionUpdate(tl, GetOwner()->GetSafeHwnd(), value1, value2, value3);

	if (p_wndFileView)
	{
		BOOL deselected = FALSE;

		for (UINT a=0; a<tl->m_ItemCount; a++)
			if (tl->m_Items[a].LastError!=LFOk)
			{
				p_wndFileView->SelectItem(tl->m_Items[a].UserData, FALSE, TRUE);
				deselected = TRUE;
			}

		if (tl->m_Changes)
			UpdateSearchResult(p_RawFiles, p_CookedFiles, GetFocusItem());
		if (deselected)
			OnUpdateSelection();
	}

	if (tl->m_LastError>LFCancel)
		ShowCaptionBar(IDI_ERROR, tl->m_LastError);

	BOOL changes = tl->m_Changes;
	LFFreeTransactionList(tl);
	return changes;
}


BEGIN_MESSAGE_MAP(CMainView, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE_VOID(WM_UPDATESELECTION, OnUpdateSelection)
	ON_MESSAGE(WM_RENAMEITEM, OnRenameItem)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoreAttributesChanged)

	ON_COMMAND(IDM_STORES_CREATENEW, OnStoresCreateNew)
	ON_COMMAND(IDM_STORES_MAINTAINALL, OnStoresMaintainAll)
	ON_COMMAND(IDM_STORES_BACKUP, OnStoresBackup)
	ON_COMMAND(IDM_STORES_HIDEEMPTYDRIVES, OnStoresHideEmptyDrives)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORES_CREATENEW, IDM_STORES_BACKUP, OnUpdateStoresCommands)
	ON_UPDATE_COMMAND_UI(IDM_STORES_HIDEEMPTYDRIVES, OnUpdateStoresCommands)

	ON_COMMAND(IDM_HOME_HIDEEMPTYDOMAINS, OnHomeHideEmptyDomains)
	ON_COMMAND(IDM_HOME_IMPORTFOLDER, OnHomeImportFolder)
	ON_COMMAND(IDM_HOME_MAINTAIN, OnHomeMaintain)
	ON_COMMAND(IDM_HOME_PROPERTIES, OnHomeProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_HOME_HIDEEMPTYDOMAINS, IDM_HOME_PROPERTIES, OnUpdateHomeCommands)

	ON_COMMAND(IDM_HOUSEKEEPING_REGISTER, OnHousekeepingRegister)
	ON_COMMAND(IDM_HOUSEKEEPING_SEND, OnHousekeepingSend)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_HOUSEKEEPING_REGISTER, IDM_HOUSEKEEPING_SEND, OnUpdateHousekeepingCommands)

	ON_COMMAND(IDM_TRASH_EMPTY, OnTrashEmpty)
	ON_COMMAND(IDM_TRASH_RESTOREALL, OnTrashRestoreAll)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_TRASH_EMPTY, IDM_TRASH_RESTOREALL, OnUpdateTrashCommands)

	ON_UPDATE_COMMAND_UI(IDM_ITEM_OPEN, OnUpdateItemCommands)

	ON_COMMAND(IDM_DRIVE_CREATENEWSTORE, OnDriveCreateNewStore)
	ON_COMMAND(IDM_DRIVE_PROPERTIES, OnDriveProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_DRIVE_CREATENEWSTORE, IDM_DRIVE_PROPERTIES, OnUpdateDriveCommands)

	ON_COMMAND(IDM_STORE_MAKEDEFAULT, OnStoreMakeDefault)
	ON_COMMAND(IDM_STORE_MAKEHYBRID, OnStoreMakeHybrid)
	ON_COMMAND(IDM_STORE_IMPORTFOLDER, OnStoreImportFolder)
	ON_COMMAND(IDM_STORE_MAINTAIN, OnStoreMaintain)
	ON_COMMAND(IDM_STORE_DELETE, OnStoreDelete)
	ON_COMMAND(IDM_STORE_RENAME, OnStoreRename)
	ON_COMMAND(IDM_STORE_PROPERTIES, OnStoreProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORE_MAKEDEFAULT, IDM_STORE_PROPERTIES, OnUpdateStoreCommands)

	ON_COMMAND(IDM_FILE_OPENWITH, OnFileOpenWith)
	ON_COMMAND(IDM_FILE_REMEMBER, OnFileRemember)
	ON_COMMAND(IDM_FILE_REMOVE, OnFileRemove)
	ON_COMMAND(IDM_FILE_DELETE, OnFileDelete)
	ON_COMMAND(IDM_FILE_RENAME, OnFileRename)
	ON_COMMAND(IDM_FILE_SEND, OnFileSend)
	ON_COMMAND(IDM_FILE_RESTORE, OnFileRestore)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILE_OPENWITH, IDM_FILE_RESTORE, OnUpdateFileCommands)
END_MESSAGE_MAP()

INT CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Task bar
	if (!m_wndTaskbar.Create(this, IDB_TASKS, 1))
		return -1;

	m_wndTaskbar.SetOwner(GetOwner());

	m_wndTaskbar.AddButton(IDM_STORES_CREATENEW, 0);
	m_wndTaskbar.AddButton(IDM_STORES_MAINTAINALL, 1, TRUE);
	m_wndTaskbar.AddButton(IDM_HOME_IMPORTFOLDER, 2);
	m_wndTaskbar.AddButton(IDM_HOUSEKEEPING_REGISTER, 3);
	m_wndTaskbar.AddButton(IDM_HOUSEKEEPING_SEND, 28, TRUE);
	m_wndTaskbar.AddButton(IDM_TRASH_EMPTY, 4, TRUE);
	m_wndTaskbar.AddButton(IDM_TRASH_RESTOREALL, 5);
	m_wndTaskbar.AddButton(IDM_GLOBE_JUMPTOLOCATION, 6);
	m_wndTaskbar.AddButton(IDM_GLOBE_ZOOMIN, 7);
	m_wndTaskbar.AddButton(IDM_GLOBE_ZOOMOUT, 8);
	m_wndTaskbar.AddButton(IDM_GLOBE_AUTOSIZE, 9);
	m_wndTaskbar.AddButton(IDM_TAGCLOUD_SORTVALUE, 10);
	m_wndTaskbar.AddButton(IDM_TAGCLOUD_SORTCOUNT, 11);
	m_wndTaskbar.AddButton(IDM_ITEM_OPEN, 12);
	m_wndTaskbar.AddButton(IDM_GLOBE_GOOGLEEARTH, 13);
	m_wndTaskbar.AddButton(IDM_DRIVE_PROPERTIES, 14);
	m_wndTaskbar.AddButton(IDM_STORE_DELETE, 15);
	m_wndTaskbar.AddButton(IDM_STORE_RENAME, 16);
	m_wndTaskbar.AddButton(IDM_STORE_PROPERTIES, 17);
	m_wndTaskbar.AddButton(IDM_STORE_MAKEDEFAULT, 18);
	m_wndTaskbar.AddButton(IDM_STORE_IMPORTFOLDER, 2);
	m_wndTaskbar.AddButton(IDM_FILE_REMEMBER, 19);
	m_wndTaskbar.AddButton(IDM_FILE_REMOVE, 20);
	m_wndTaskbar.AddButton(IDM_FILE_DELETE, 21);
	m_wndTaskbar.AddButton(IDM_FILE_RENAME, 22);
	m_wndTaskbar.AddButton(IDM_FILE_SEND, 23);
	m_wndTaskbar.AddButton(IDM_FILE_RESTORE, 24);
	m_wndTaskbar.AddButton(ID_APP_NEWFILEDROP, 25, TRUE);

	m_wndTaskbar.AddButton(ID_APP_PURCHASE, 26, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ENTERLICENSEKEY, 27, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_SUPPORT, 28, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ABOUT, 29, TRUE, TRUE);

	// Explorer header
	if (!m_wndExplorerHeader.Create(this, 2))
		return -1;

	return 0;
}

BOOL CMainView::OnEraseBkgnd(CDC* /*pDC*/)
{
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

	CMenu* pMenu = p_wndFileView->GetBackgroundContextMenu();
	if (pMenu)
	{
		CMenu* pPopup = pMenu->GetSubMenu(0);
		ASSERT_VALID(pPopup);

		if (m_Context==LFContextStores)
			pPopup->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);

		CString tmpStr;
		switch (m_ViewID)
		{
		case LFViewDetails:
			ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_AUTOSIZEALL));
			pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_DETAILS_AUTOSIZEALL, tmpStr);
			break;
		}

		CString mask;
		ENSURE(mask.LoadString(IDS_CONTEXTMENU_VIEWOPTIONS));
		tmpStr.Format(mask, theApp.m_Contexts[m_Context]->Name);
		pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, ID_APP_VIEWOPTIONS, tmpStr);

		pPopup->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);

		switch (m_Context)
		{
		case LFContextStores:
			ENSURE(tmpStr.LoadString(IDM_STORES_HIDEEMPTYDRIVES));
			pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_STORES_HIDEEMPTYDRIVES, tmpStr);
			break;
		case LFContextStoreHome:
			ENSURE(tmpStr.LoadString(IDM_HOME_HIDEEMPTYDOMAINS));
			pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_HOME_HIDEEMPTYDOMAINS, tmpStr);
			break;
		}

		if (m_ViewID==LFViewTagcloud)
		{
			ENSURE(tmpStr.LoadString(IDM_TAGCLOUD_SORT));
			pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION | MF_POPUP, (UINT_PTR)LoadMenu(NULL, MAKEINTRESOURCE(IDM_TAGCLOUD_SORT)), tmpStr);
		}

		if (theApp.m_Contexts[m_Context]->AllowGroups)
		{
			ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_AUTODIRS));
			pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, ID_VIEW_AUTODIRS, tmpStr);
		}

		ENSURE(mask.LoadString(IDS_CONTEXTMENU_SORTOPTIONS));
		tmpStr.Format(mask, theApp.m_Contexts[m_Context]->Name);
		pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, ID_APP_SORTOPTIONS, tmpStr);

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetOwner(), NULL);
		delete pMenu;
	}
}

void CMainView::OnUpdateSelection()
{
	INT idx = GetNextSelectedItem(-1);
	m_FilesSelected = FALSE;

	while (idx>=0)
	{
		LFItemDescriptor* item = p_CookedFiles->m_Items[idx];

		m_FilesSelected |= ((item->Type & LFTypeMask)==LFTypeFile) ||
						(((item->Type & LFTypeMask)==LFTypeVirtual) && (item->FirstAggregate!=-1) && (item->LastAggregate!=-1));

		idx = GetNextSelectedItem(idx);
	}

	m_wndTaskbar.PostMessage(WM_IDLEUPDATECMDUI);

	// TODO
	((CMainFrame*)GetParent())->OnUpdateSelection();
}

LRESULT CMainView::OnRenameItem(WPARAM wParam, LPARAM lParam)
{
	LFTransactionList* tl = LFAllocTransactionList();
	LFAddItemDescriptor(tl, p_CookedFiles->m_Items[(UINT)wParam]);

	LFVariantData value;
	value.Attr = LFAttrFileName;
	value.Type = LFTypeUnicodeString;
	value.IsNull = false;
	wcscpy_s(value.UnicodeString, 256, (WCHAR*)lParam);

	LFTransactionUpdate(tl, GetSafeHwnd(), &value);

	if (tl->m_Changes)
		UpdateSearchResult(p_RawFiles, p_CookedFiles, GetFocusItem());

	if (tl->m_LastError>LFCancel)
		ShowCaptionBar(IDI_ERROR, tl->m_LastError);

	BOOL changes = tl->m_Changes;
	LFFreeTransactionList(tl);
	return changes;
}

LRESULT CMainView::OnStoreAttributesChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if ((p_RawFiles) && (p_CookedFiles) && (m_Context==LFContextStoreHome))
		SetHeader();

	return NULL;
}


// Stores

void CMainView::OnStoresCreateNew()
{
	LFStoreDescriptor* s = LFAllocStoreDescriptor();

	LFStoreNewDlg dlg(this, s);
	if (dlg.DoModal()==IDOK)
		LFErrorBox(LFCreateStore(s, dlg.MakeDefault));

	LFFreeStoreDescriptor(s);
}

void CMainView::OnStoresMaintainAll()
{
	LFMaintenanceList* ml = LFStoreMaintenance();
	LFErrorBox(ml->m_LastError);

	LFStoreMaintenanceDlg dlg(ml, this);
	dlg.DoModal();

	LFFreeMaintenanceList(ml);
}

void CMainView::OnStoresBackup()
{
	LFBackupStores(this);
}

void CMainView::OnStoresHideEmptyDrives()
{
	theApp.m_HideEmptyDrives = !theApp.m_HideEmptyDrives;
	theApp.Reload(LFContextStores);
}

void CMainView::OnUpdateStoresCommands(CCmdUI* pCmdUI)
{
	BOOL b = (p_CookedFiles) && (m_Context==LFContextStores);

	switch (pCmdUI->m_nID)
	{
	case IDM_STORES_HIDEEMPTYDRIVES:
		pCmdUI->SetCheck(theApp.m_HideEmptyDrives);
	case IDM_STORES_CREATENEW:
		break;
	default:
		b &= (LFGetStoreCount()>0);
	}

	pCmdUI->Enable(b);
}


// Home

void CMainView::OnHomeHideEmptyDomains()
{
	theApp.m_HideEmptyDomains = !theApp.m_HideEmptyDomains;
	theApp.Reload(LFContextStoreHome);
}

void CMainView::OnHomeImportFolder()
{
	if (p_CookedFiles)
		LFImportFolder(p_CookedFiles->m_StoreID, this);
}

void CMainView::OnHomeMaintain()
{
	if (p_CookedFiles)
	{
		LFMaintenanceList* ml = LFStoreMaintenance(p_CookedFiles->m_StoreID);
		LFErrorBox(ml->m_LastError);

		LFStoreMaintenanceDlg dlg(ml, this);
		dlg.DoModal();

		LFFreeMaintenanceList(ml);
	}
}

void CMainView::OnHomeProperties()
{
	if (p_CookedFiles)
	{
		LFStorePropertiesDlg dlg(p_CookedFiles->m_StoreID, this);
		dlg.DoModal();
	}
}

void CMainView::OnUpdateHomeCommands(CCmdUI* pCmdUI)
{
	BOOL b = (p_CookedFiles) && (m_Context==LFContextStoreHome);

	if (pCmdUI->m_nID==IDM_HOME_HIDEEMPTYDOMAINS)
		pCmdUI->SetCheck(theApp.m_HideEmptyDomains);

	pCmdUI->Enable(b);
}


// Housekeeping

void CMainView::OnHousekeepingRegister()
{
	MessageBox(_T("Coming soon!"));
}

void CMainView::OnHousekeepingSend()
{
	MessageBox(_T("Coming soon!"));
}

void CMainView::OnUpdateHousekeepingCommands(CCmdUI* pCmdUI)
{
	BOOL b = (p_CookedFiles) && (m_Context==LFContextHousekeeping);

	if ((p_CookedFiles) && (pCmdUI->m_nID==IDM_HOUSEKEEPING_REGISTER))
		b &= (p_CookedFiles->m_ItemCount>0);

	pCmdUI->Enable(b);
}


// Trash

void CMainView::OnTrashEmpty()
{
	if (DeleteFiles(TRUE))
		theApp.PlayTrashSound();
}

void CMainView::OnTrashRestoreAll()
{
	UpdateTrashFlag(FALSE, TRUE);
}

void CMainView::OnUpdateTrashCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(((m_Context==LFContextTrash) && (p_CookedFiles)) ? p_CookedFiles->m_FileCount : FALSE);
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
				((item->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeFile);
			break;
		}
	}

	pCmdUI->Enable(b);
}


// Drive

void CMainView::OnDriveCreateNewStore()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		LFStoreDescriptor* s = LFAllocStoreDescriptor();

		LFStoreNewDriveDlg dlg(this, p_CookedFiles->m_Items[idx]->CoreAttributes.FileID[0], s);
		if (dlg.DoModal()==IDOK)
			LFErrorBox(LFCreateStore(s, FALSE), GetSafeHwnd());

		LFFreeStoreDescriptor(s);
	}
}

void CMainView::OnDriveProperties()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
		ExecuteContextMenu(p_CookedFiles->m_Items[idx]->CoreAttributes.FileID[0], "properties");
}

void CMainView::OnUpdateDriveCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		LFItemDescriptor* item = p_CookedFiles->m_Items[idx];
		switch (pCmdUI->m_nID)
		{
		case IDM_DRIVE_CREATENEWSTORE:
			b = ((item->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeDrive);
			break;
		case IDM_DRIVE_PROPERTIES:
			b = ((item->Type & LFTypeMask)==LFTypeDrive);
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

void CMainView::OnStoreMakeHybrid()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
		LFErrorBox(LFMakeHybridStore(p_CookedFiles->m_Items[idx]->StoreID, NULL), GetSafeHwnd());
}

void CMainView::OnStoreImportFolder()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
		LFImportFolder(p_CookedFiles->m_Items[idx]->StoreID, this);
}

void CMainView::OnStoreMaintain()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		LFMaintenanceList* ml = LFStoreMaintenance(p_CookedFiles->m_Items[idx]->StoreID);
		LFErrorBox(ml->m_LastError);

		LFStoreMaintenanceDlg dlg(ml, this);
		dlg.DoModal();

		LFFreeMaintenanceList(ml);
	}
}

void CMainView::OnStoreRename()
{
	INT idx = GetSelectedItem();
	if ((idx!=-1) && (p_wndFileView))
		p_wndFileView->EditLabel(idx);
}

void CMainView::OnStoreDelete()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
		LFErrorBox(((LFApplication*)AfxGetApp())->DeleteStore(p_CookedFiles->m_Items[idx], this));
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

	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		LFItemDescriptor* item = p_CookedFiles->m_Items[idx];
		if ((item->Type & LFTypeMask)==LFTypeStore)
			switch (pCmdUI->m_nID)
			{
			case IDM_STORE_MAKEDEFAULT:
				b = (item->CategoryID==LFItemCategoryInternalStores) && (!(item->Type & LFTypeDefaultStore));
				break;
			case IDM_STORE_MAKEHYBRID:
				b = (item->CategoryID==LFItemCategoryExternalStores) && (!(item->Type & LFTypeNotMounted));
				break;
			case IDM_STORE_IMPORTFOLDER:
				b = !(item->Type & LFTypeNotMounted);
				break;
			case IDM_STORE_RENAME:
				b = !p_wndFileView->IsEditing();
				break;
			default:
				b = TRUE;
			}
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
		UINT res = LFGetFileLocation(p_CookedFiles->m_Items[idx], Path, MAX_PATH, true);
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

void CMainView::OnFileRemember()
{
	CMainFrame* pClipboard = theApp.GetClipboard();
	BOOL changes = FALSE;

	INT idx = GetNextSelectedItem(-1);
	while (idx!=-1)
	{
		LFItemDescriptor* item = p_CookedFiles->m_Items[idx];
		switch (item->Type & LFTypeMask)
		{
		case LFTypeVirtual:
			if ((item->FirstAggregate!=-1) && (item->LastAggregate!=-1))
				for (INT a=item->FirstAggregate; a<=item->LastAggregate; a++)
					if (pClipboard->AddClipItem(p_RawFiles->m_Items[a]))
						changes = TRUE;
			break;
		case LFTypeFile:
			if (pClipboard->AddClipItem(item))
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
	RemoveTransactedItems(tl);
	LFFreeTransactionList(tl);
}

void CMainView::OnFileDelete()
{
	if (p_CookedFiles)
		if (p_CookedFiles->m_Context==LFContextTrash)
		{
			DeleteFiles();
		}
		else
		{
			UpdateTrashFlag(TRUE);
		}
}

void CMainView::OnFileRename()
{
	INT idx = GetSelectedItem();
	if ((idx!=-1) && (p_wndFileView))
		p_wndFileView->EditLabel(idx);
}

void CMainView::OnFileSend()
{
	MessageBox(_T("Coming soon!"));
}

void CMainView::OnFileRestore()
{
	UpdateTrashFlag(FALSE);
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
			b = ((item->Type & LFTypeMask)==LFTypeFile);
		break;
	case IDM_FILE_REMEMBER:
		b = m_FilesSelected && (m_Context!=LFContextClipboard) && (m_Context!=LFContextTrash);
		break;
	case IDM_FILE_REMOVE:
		b = m_FilesSelected && (m_Context==LFContextClipboard);
		break;
	case IDM_FILE_DELETE:
		b = m_FilesSelected;
		break;
	case IDM_FILE_RENAME:
		if ((item) && (m_Context!=LFContextTrash))
			b = ((item->Type & LFTypeMask)==LFTypeFile);
		if (p_wndFileView)
			b &= !p_wndFileView->IsEditing();
		break;
	case IDM_FILE_SEND:
		b = m_FilesSelected && (m_Context!=LFContextTrash);
		break;
	case IDM_FILE_RESTORE:
		b = m_FilesSelected && (m_Context==LFContextTrash);
		break;
	}

	pCmdUI->Enable(b);
}
