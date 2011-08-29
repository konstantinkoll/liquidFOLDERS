
// CMainView.cpp: Implementierung der Klasse CMainView
//

#include "stdafx.h"
#include "CMainView.h"
#include "CCalendarView.h"
#include "CGlobeView.h"
#include "CListView.h"
#include "CTagcloudView.h"
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

#ifdef _DEBUG
DWORD WINAPI WorkerTest(void* lParam)
{
	CoInitialize(NULL);
	LFWorkerParameters* wp = (LFWorkerParameters*)lParam;

	LFProgress p;
	ZeroMemory(&p, sizeof(p));
	p.ProgressState = LFProgressWorking;
	p.MinorCount = 5;

	for (UINT a=0; a<p.MinorCount; a++)
	{
		swprintf(p.Object, 256, _T("Item %d"), a+1);
		if (SendMessage(wp->hWnd, WM_UPDATEPROGRESS, (WPARAM)&p, NULL))
			break;

		Sleep(1000);

		p.MinorCurrent++;
		if (SendMessage(wp->hWnd, WM_UPDATEPROGRESS, (WPARAM)&p, NULL))
			break;
	}

	CoUninitialize();
	PostMessage(wp->hWnd, WM_COMMAND, (WPARAM)IDOK, NULL);
	return 0;
}
#endif


// CMainView
//

#define FileViewID     4

CMainView::CMainView()
{
	p_wndFilter = NULL;
	p_wndFileView = NULL;
	p_Filter = NULL;
	p_RawFiles = p_CookedFiles = NULL;
	p_FilterButton = p_InspectorButton = NULL;
	p_OrganizeButton = p_ViewButton = NULL;
	m_Context = m_ViewID = -1;
	m_Resizing = m_StoreIDValid = FALSE;
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

	// Check Filter
	if (p_wndFilter)
		if (p_wndFilter->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
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
	case LFViewContent:
	case LFViewPreview:
		if ((m_ViewID<LFViewLargeIcons) || (m_ViewID>LFViewPreview))
		{
			pNewView = new CListView();
			((CListView*)pNewView)->Create(this, FileViewID, p_CookedFiles, Data);
		}
		break;
	case LFViewCalendar:
		if (m_ViewID!=LFViewCalendar)
		{
			pNewView = new CCalendarView();
			((CCalendarView*)pNewView)->Create(this, FileViewID, p_CookedFiles, Data);
		}
		break;
	case LFViewGlobe:
		if (m_ViewID!=LFViewGlobe)
		{
			pNewView = new CGlobeView();
			((CGlobeView*)pNewView)->Create(this, FileViewID, p_CookedFiles, Data);
		}
		break;
	case LFViewTagcloud:
		if (m_ViewID!=LFViewTagcloud)
		{
			pNewView = new CTagcloudView();
			((CTagcloudView*)pNewView)->Create(this, FileViewID, p_CookedFiles, Data);
		}
		break;
	}

	m_ViewID = ViewID;

	// Exchange view
	if (pNewView)
	{
		CFileView* pVictim = p_wndFileView;

		p_wndFileView = pNewView;
		p_wndFileView->SetOwner(GetOwner());
		p_wndFileView->SetFocus();
		AdjustLayout();

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

	p_OrganizeButton->SetValue(theApp.m_Attributes[theApp.m_Views[m_Context].SortBy]->Name, FALSE);

	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDM_VIEW_FIRST+m_ViewID));
	p_ViewButton->SetValue(tmpStr);
}

void CMainView::SetHeader()
{
	if (p_wndFilter && p_CookedFiles)
		p_wndFilter->SetStoreID(m_StoreIDValid ? m_StoreID : "");

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

		if ((m_Context==LFContextStoreHome) && (m_StoreIDValid))
		{
			LFStoreDescriptor s;
			if (LFGetStoreSettings(m_StoreID, &s)==LFOk)
			{
				wcscpy_s(p_RawFiles->m_Name, 256, s.StoreName);
				wcscpy_s(p_CookedFiles->m_Name, 256, s.StoreName);

				if (s.Comment[0]!=L'\0')
				{
					Hint.Insert(0, (GetThreadLocale() & 0x1FF)==LANG_ENGLISH ? _T("—") : _T(" – "));
					Hint.Insert(0, s.Comment);
				}
			}
		}

		m_wndExplorerHeader.SetColors(m_Context<=LFContextClipboard ? 0x126E00 : 0x993300, (COLORREF)-1, FALSE);
		m_wndExplorerHeader.SetText(p_CookedFiles->m_Name, Hint, FALSE);
		SetHeaderButtons();
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
			p_wndFileView->UpdateSearchResult(NULL, NULL);

		RevokeDragDrop(m_wndExplorerHeader.GetSafeHwnd());
		RevokeDragDrop(p_wndFileView->GetSafeHwnd());
	}
	else
	{
		m_Context = pCookedFiles->m_Context;

		if (!CreateFileView(theApp.m_Views[pCookedFiles->m_Context].Mode, Data))
		{
			p_wndFileView->UpdateViewOptions(m_Context);
			p_wndFileView->UpdateSearchResult(pCookedFiles, Data);
		}

		m_DropTarget.SetFilter(pFilter);
		RegisterDragDrop(m_wndExplorerHeader.GetSafeHwnd(), &m_DropTarget);
		RegisterDragDrop(p_wndFileView->GetSafeHwnd(), &m_DropTarget);
	}

	if (m_IsClipboard)
		m_DropTarget.SetSearchResult(pRawFiles);

	SetHeader();
	if (UpdateSelection)
		OnUpdateSelection();
}

void CMainView::UpdateFooter()
{
	if (p_wndFileView)
		p_wndFileView->UpdateFooter();
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
	WCHAR* Message = LFGetErrorText(ResID);
	ShowNotification(Type, Message, Command);
	free(Message);
}

void CMainView::AdjustLayout()
{
	if (!IsWindow(m_wndTaskbar))
		return;
	if (!IsWindow(m_wndExplorerNotification))
		return;
	if (!IsWindow(m_wndExplorerHeader))
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
	INT FilterWidth = 0;
	INT InspectorWidth = 0;
	if (MaxWidth>0)
	{
		if (p_wndFilter)
		{
			theApp.m_FilterWidth = max(32, p_wndFilter->GetPreferredWidth());
			FilterWidth = min(MaxWidth, (INT)theApp.m_FilterWidth);

			if (m_ShowFilterPane)
			{
				p_wndFilter->SetMaxWidth(MaxWidth);
				p_wndFilter->SetWindowPos(NULL, rect.left, rect.top+TaskHeight, FilterWidth, rect.Height()-TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER | SWP_SHOWWINDOW);
			}
			else
			{
				p_wndFilter->ShowWindow(SW_HIDE);
				FilterWidth = 0;
			}
		}

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

	const UINT ExplorerHeight = m_wndExplorerHeader.GetPreferredHeight();
	m_wndExplorerHeader.SetWindowPos(NULL, rect.left+FilterWidth, rect.top+TaskHeight, rect.Width()-FilterWidth-InspectorWidth, ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	if (p_wndFileView)
		p_wndFileView->SetWindowPos(NULL, rect.left+FilterWidth, rect.top+TaskHeight+ExplorerHeight, rect.Width()-FilterWidth-InspectorWidth, rect.Height()-ExplorerHeight-TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);

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

void CMainView::SetFilter(LFFilter* f)
{
	if (p_wndFilter)
		p_wndFilter->SetFilter(f);
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

void CMainView::AddFileIDItem(LFFileIDList* il, LFItemDescriptor* item)
{
	switch (item->Type & LFTypeMask)
	{
	case LFTypeFile:
		LFAddFileID(il, item->StoreID, item->CoreAttributes.FileID, NULL);
		break;
	case LFTypeVirtual:
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
		if (il->m_Items[a].LastError==LFOk)
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
		if (tl->m_Items[a].LastError==LFOk)
			tl->m_Items[a].Item->DeleteFlag = true;

	LFRemoveFlaggedItemDescriptors(p_RawFiles);

	FVPersistentData Data;
	GetPersistentData(Data);
	GetOwner()->SendMessage(WM_COOKFILES, (WPARAM)&Data);
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
		ShowNotification(ENT_ERROR, tl->m_LastError);

	BOOL Changes = tl->m_Changes;
	LFFreeTransactionList(tl);
	return Changes;
}

BOOL CMainView::DeleteFiles(BOOL All)
{
	LFTransactionList* tl = BuildTransactionList(All);
	LFTransactionDelete(tl, false);
	RemoveTransactedItems(tl);

	if (tl->m_LastError>LFCancel)
		ShowNotification(ENT_ERROR, tl->m_LastError);

	BOOL Changes = tl->m_Changes;
	LFFreeTransactionList(tl);
	return Changes;
}

BOOL CMainView::UpdateItems(LFVariantData* Value1, LFVariantData* Value2, LFVariantData* Value3)
{
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
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE_VOID(WM_ADJUSTLAYOUT, OnAdjustLayout)
	ON_MESSAGE_VOID(WM_UPDATESELECTION, OnUpdateSelection)
	ON_MESSAGE_VOID(WM_BEGINDRAGDROP, OnBeginDragDrop)
	ON_MESSAGE(WM_RENAMEITEM, OnRenameItem)
	ON_MESSAGE(WM_SENDTO, OnSendTo)
	ON_REGISTERED_MESSAGE(theApp.p_MessageIDs->StoreAttributesChanged, OnStoreAttributesChanged)

	ON_COMMAND(ID_PANE_FILTER, OnToggleFilter)
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

	ON_COMMAND(IDM_STORES_CREATENEW, OnStoresCreateNew)
	ON_COMMAND(IDM_STORES_MAINTAINALL, OnStoresMaintainAll)
	ON_COMMAND(IDM_STORES_BACKUP, OnStoresBackup)
	ON_COMMAND(IDM_STORES_SHOWEMPTYVOLUMES, OnStoresShowEmptyVolumes)
	ON_COMMAND(IDM_STORES_REPAIRCORRUPTEDINDEX, OnStoresMaintainAll)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORES_CREATENEW, IDM_STORES_BACKUP, OnUpdateStoresCommands)
	ON_UPDATE_COMMAND_UI(IDM_STORES_SHOWEMPTYVOLUMES, OnUpdateStoresCommands)

	ON_COMMAND(IDM_HOME_SHOWEMPTYDOMAINS, OnHomeShowEmptyDomains)
	ON_COMMAND(IDM_HOME_SHOWSTATISTICS, OnHomeShowStatistics)
	ON_COMMAND(IDM_HOME_IMPORTFOLDER, OnHomeImportFolder)
	ON_COMMAND(IDM_HOME_MAINTAIN, OnHomeMaintain)
	ON_COMMAND(IDM_HOME_PROPERTIES, OnHomeProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_HOME_SHOWEMPTYDOMAINS, IDM_HOME_PROPERTIES, OnUpdateHomeCommands)

	ON_COMMAND(IDM_HOUSEKEEPING_REGISTER, OnHousekeepingRegister)
	ON_COMMAND(IDM_HOUSEKEEPING_SEND, OnHousekeepingSend)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_HOUSEKEEPING_REGISTER, IDM_HOUSEKEEPING_SEND, OnUpdateHousekeepingCommands)

	ON_COMMAND(IDM_TRASH_EMPTY, OnTrashEmpty)
	ON_COMMAND(IDM_TRASH_RESTOREALL, OnTrashRestoreAll)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_TRASH_EMPTY, IDM_TRASH_RESTOREALL, OnUpdateTrashCommands)

	ON_UPDATE_COMMAND_UI(IDM_ITEM_OPEN, OnUpdateItemCommands)

	ON_COMMAND(IDM_VOLUME_CREATENEWSTORE, OnVolumeCreateNewStore)
	ON_COMMAND(IDM_VOLUME_FORMAT, OnVolumeFormat)
	ON_COMMAND(IDM_VOLUME_EJECT, OnVolumeEject)
	ON_COMMAND(IDM_VOLUME_PROPERTIES, OnVolumeProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_VOLUME_CREATENEWSTORE, IDM_VOLUME_PROPERTIES, OnUpdateDriveCommands)

	ON_COMMAND(IDM_STORE_MAKEDEFAULT, OnStoreMakeDefault)
	ON_COMMAND(IDM_STORE_MAKEHYBRID, OnStoreMakeHybrid)
	ON_COMMAND(IDM_STORE_IMPORTFOLDER, OnStoreImportFolder)
	ON_COMMAND(IDM_STORE_MAINTAIN, OnStoreMaintain)
	ON_COMMAND(IDM_STORE_SHORTCUT, OnStoreShortcut)
	ON_COMMAND(IDM_STORE_DELETE, OnStoreDelete)
	ON_COMMAND(IDM_STORE_RENAME, OnStoreRename)
	ON_COMMAND(IDM_STORE_PROPERTIES, OnStoreProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORE_MAKEDEFAULT, IDM_STORE_PROPERTIES, OnUpdateStoreCommands)

	ON_COMMAND(IDM_FILE_OPENWITH, OnFileOpenWith)
	ON_COMMAND(IDM_FILE_REMEMBER, OnFileRemember)
	ON_COMMAND(IDM_FILE_REMOVE, OnFileRemove)
	ON_COMMAND(IDM_FILE_COPY, OnFileCopy)
	ON_COMMAND(IDM_FILE_SHORTCUT, OnFileShortcut)
	ON_COMMAND(IDM_FILE_DELETE, OnFileDelete)
	ON_COMMAND(IDM_FILE_RENAME, OnFileRename)
	ON_COMMAND(IDM_FILE_RESTORE, OnFileRestore)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_FILE_OPENWITH, IDM_FILE_RESTORE, OnUpdateFileCommands)
END_MESSAGE_MAP()

INT CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Taskbar
	if (!m_wndTaskbar.Create(this, IDB_TASKS, 1))
		return -1;

	m_wndTaskbar.SetOwner(GetOwner());

	m_wndTaskbar.AddButton(IDM_STORES_CREATENEW, 0);
	m_wndTaskbar.AddButton(IDM_HOME_IMPORTFOLDER, 1, TRUE);
	m_wndTaskbar.AddButton(IDM_HOUSEKEEPING_REGISTER, 2, TRUE);
	m_wndTaskbar.AddButton(IDM_HOUSEKEEPING_SEND, 33, TRUE);
	m_wndTaskbar.AddButton(IDM_TRASH_EMPTY, 3, TRUE);
	m_wndTaskbar.AddButton(IDM_TRASH_RESTOREALL, 4, TRUE);
	m_wndTaskbar.AddButton(IDM_CALENDAR_PREVYEAR, 5, TRUE);
	m_wndTaskbar.AddButton(IDM_CALENDAR_NEXTYEAR, 6, TRUE);
	m_wndTaskbar.AddButton(IDM_CALENDAR_GOTOYEAR, 7);
	m_wndTaskbar.AddButton(IDM_GLOBE_JUMPTOLOCATION, 8, TRUE);
	m_wndTaskbar.AddButton(IDM_GLOBE_ZOOMIN, 9);
	m_wndTaskbar.AddButton(IDM_GLOBE_ZOOMOUT, 10);
	m_wndTaskbar.AddButton(IDM_GLOBE_AUTOSIZE, 11);
	m_wndTaskbar.AddButton(IDM_TAGCLOUD_SORTVALUE, 12);
	m_wndTaskbar.AddButton(IDM_TAGCLOUD_SORTCOUNT, 13);
	m_wndTaskbar.AddButton(IDM_ITEM_OPEN, 14);
	m_wndTaskbar.AddButton(IDM_GLOBE_GOOGLEEARTH, 15, TRUE);
	m_wndTaskbar.AddButton(IDM_VOLUME_PROPERTIES, 16);
	m_wndTaskbar.AddButton(IDM_STORE_DELETE, 17);
	m_wndTaskbar.AddButton(IDM_STORE_RENAME, 18);
	m_wndTaskbar.AddButton(IDM_STORE_PROPERTIES, 19);
	m_wndTaskbar.AddButton(IDM_STORE_MAKEDEFAULT, 20);
	m_wndTaskbar.AddButton(IDM_STORE_IMPORTFOLDER, 1);
	m_wndTaskbar.AddButton(IDM_FILE_REMEMBER, 21);
	m_wndTaskbar.AddButton(IDM_FILE_REMOVE, 22);
	m_wndTaskbar.AddButton(IDM_FILE_DELETE, 23);
	m_wndTaskbar.AddButton(IDM_FILE_RENAME, 24);
	m_wndTaskbar.AddButton(IDM_FILE_RESTORE, 25);
	m_wndTaskbar.AddButton(ID_APP_NEWFILEDROP, 26, TRUE);

	#define FilterIconVisible     28
	#define FilterIconHidden      27
	p_FilterButton = m_wndTaskbar.AddButton(ID_PANE_FILTER, theApp.m_ShowFilterPane ? FilterIconVisible : FilterIconHidden, TRUE, TRUE);

	#define InspectorIconVisible     30
	#define InspectorIconHidden      29
	p_InspectorButton = m_wndTaskbar.AddButton(ID_PANE_INSPECTOR, theApp.m_ShowInspectorPane ? InspectorIconVisible : InspectorIconHidden, TRUE, TRUE);

	m_wndTaskbar.AddButton(ID_APP_PURCHASE, 31, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ENTERLICENSEKEY, 32, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_SUPPORT, 33, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ABOUT, 34, TRUE, TRUE);

	// Filter
	if (!m_IsClipboard)
	{
		p_wndFilter = new CFilterWnd();
		if (!p_wndFilter->Create(TRUE, theApp.m_FilterWidth, this, 2))
			return -1;

		p_wndFilter->SetOwner(GetOwner());
		m_ShowFilterPane = theApp.m_ShowFilterPane;
	}
	else
	{
		m_ShowFilterPane = FALSE;
	}

	// Drop target
	m_DropTarget.SetOwner(GetOwner());

	// Explorer header
	if (!m_wndExplorerHeader.Create(this, 3))
		return -1;

	m_wndExplorerHeader.SetOwner(GetOwner());

	p_OrganizeButton = m_wndExplorerHeader.AddButton(IDM_ORGANIZE);
	p_ViewButton = m_wndExplorerHeader.AddButton(IDM_VIEW);

	// Inspector
	if (!m_wndInspector.Create(FALSE, theApp.m_InspectorWidth, this, 5))
		return -1;

	m_wndInspector.SetOwner(GetOwner());
	m_ShowInspectorPane = theApp.m_ShowInspectorPane;

	// Explorer notification
	if (!m_wndExplorerNotification.Create(this, 6))
		return -1;

	return 0;
}

void CMainView::OnDestroy()
{
	if (p_wndFilter)
	{
		p_wndFilter->DestroyWindow();
		delete p_wndFilter;
	}

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
		pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_VIEW_OPTIONS, tmpStr);

		pPopup->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);

		switch (m_Context)
		{
		case LFContextStores:
			ENSURE(tmpStr.LoadString(IDM_STORES_SHOWEMPTYVOLUMES));
			pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_STORES_SHOWEMPTYVOLUMES, tmpStr);
			break;
		case LFContextStoreHome:
			ENSURE(tmpStr.LoadString(IDM_HOME_SHOWEMPTYDOMAINS));
			pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_HOME_SHOWEMPTYDOMAINS, tmpStr);
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
			pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_ORGANIZE_TOGGLEAUTODIRS, tmpStr);
		}

		ENSURE(mask.LoadString(IDS_CONTEXTMENU_SORTOPTIONS));
		tmpStr.Format(mask, theApp.m_Contexts[m_Context]->Name);
		pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_ORGANIZE_OPTIONS, tmpStr);

		if (p_wndFileView->MultiSelectAllowed())
		{
			pPopup->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);

			ENSURE(tmpStr.LoadString(IDS_CONTEXTMENU_SELECTALL));
			pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_SELECTALL, tmpStr);
		}

		HBITMAP hBmp = theApp.SetContextMenuIcon(pPopup, IDM_STORES_CREATENEW, IDI_STORE_Internal);

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetOwner(), NULL);
		delete pMenu;

		DeleteObject(hBmp);
	}
}

void CMainView::OnAdjustLayout()
{
	if (!m_Resizing)
		AdjustLayout();
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
			(((item->Type & LFTypeMask)==LFTypeVirtual) && (item->FirstAggregate!=-1) && (item->LastAggregate!=-1));

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
			LFItemDescriptor* i = p_CookedFiles->m_Items[idx];
			if (i->Type & LFTypeStore)
			{
				m_DropTarget.SetDragging(TRUE);

				LFStoreDataObject* pDataObject = new LFStoreDataObject(i);
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
		CHAR StoreID[LFKeySize];
		strcpy_s(StoreID, LFKeySize, pItemData->StoreID);

		if (strcmp(StoreID, "CHOOSE")==0)
		{
			LFChooseStoreDlg dlg(this, LFCSD_Normal);
			if (dlg.DoModal()!=IDOK)
				return NULL;

			strcpy_s(StoreID, LFKeySize, dlg.m_StoreID);
		}

		LFFileIDList* il = BuildFileIDList();
		LFTransactionImport(StoreID, il, false);
		LFErrorBox(il->m_LastError, GetSafeHwnd());
		LFFreeFileIDList(il);
	}
	else
	{
		IShellFolder* pDesktop = NULL;
		if (SUCCEEDED(SHGetDesktopFolder(&pDesktop)))
		{
			LPITEMIDLIST pidlFQ;
			if (SUCCEEDED(pDesktop->ParseDisplayName(NULL, NULL, pItemData->Path, NULL, &pidlFQ, NULL)))
			{
				IShellFolder* pParent = NULL;
				LPCITEMIDLIST pidlRel;
				if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder, (void**)&pParent, &pidlRel)))
				{
					IDropTarget* pDropTarget = NULL;
					if (SUCCEEDED(pParent->GetUIObjectOf(GetSafeHwnd(), 1, &pidlRel, IID_IDropTarget, NULL, (void**)&pDropTarget)))
					{
						LFTransactionList* tl = BuildTransactionList(FALSE, TRUE);
						if (tl->m_ItemCount)
						{
							CWaitCursor wait;
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

					pParent->Release();
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
	if ((p_RawFiles) && (p_CookedFiles) && (m_Context==LFContextStoreHome))
		SetHeader();

	return NULL;
}


// Panes

void CMainView::OnToggleFilter()
{
	ASSERT(p_FilterButton);

	theApp.m_ShowFilterPane = m_ShowFilterPane = !m_ShowFilterPane;
	p_FilterButton->SetIconID(m_ShowFilterPane ? FilterIconVisible : FilterIconHidden);
	AdjustLayout();
}

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
		pCmdUI->Enable((theApp.m_Contexts[m_Context]->AllowGroups) && (theApp.m_Views[m_Context].Mode<=LFViewPreview));
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
#define AppendAttribute(hMenu, attr) if (theApp.m_Contexts[m_Context]->AllowedAttributes->IsSet(attr)) { tmpStr = theApp.m_Attributes[attr]->Name; AppendMenu(hMenu, MF_STRING, IDM_ORGANIZE_FIRST+attr, _T("&")+tmpStr); }
#define AppendSeparator(hMenu, attr) if (theApp.m_Contexts[m_Context]->AllowedAttributes->IsSet(attr)) { AppendMenu(hMenu, MF_SEPARATOR, 0, NULL); }
#define AppendPopup(nID) if (GetMenuItemCount(hPopupMenu)) { ENSURE(tmpStr.LoadString(nID)); AppendMenu(hMenu, MF_POPUP | MF_STRING, (UINT_PTR)hPopupMenu, tmpStr); } else { DestroyMenu(hPopupMenu); }
		AppendAttribute(hMenu, LFAttrFileName);
		AppendAttribute(hMenu, LFAttrTitle);
		AppendAttribute(hMenu, LFAttrComments);

		hPopupMenu = CreatePopupMenu();
		AppendAttribute(hPopupMenu, LFAttrCreationTime);
		AppendAttribute(hPopupMenu, LFAttrRecordingTime);
		AppendAttribute(hPopupMenu, LFAttrAddTime);
		AppendAttribute(hPopupMenu, LFAttrFileTime);
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

		if (theApp.m_Contexts[m_Context]->AllowGroups)
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
		theApp.m_Views[m_Context].Descending = theApp.m_Attributes[nID]->PreferDescendingSort;
		theApp.UpdateSortOptions(m_Context);
	}

	SetFocus();
}

void CMainView::OnUpdateSortCommands(CCmdUI* pCmdUI)
{
	UINT Attr = pCmdUI->m_nID-IDM_ORGANIZE_FIRST;

	pCmdUI->Enable(theApp.m_Contexts[m_Context]->AllowedAttributes->IsSet(Attr));
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

void CMainView::OnStoresCreateNew()
{
	LFStoreDescriptor* s = LFAllocStoreDescriptor();

	LFStoreNewDlg dlg(this, s);
	if (dlg.DoModal()==IDOK)
		LFErrorBox(LFCreateStore(s, dlg.MakeDefault), GetSafeHwnd());

	LFFreeStoreDescriptor(s);
}

void CMainView::OnStoresMaintainAll()
{
	CWaitCursor wait;

	LFMaintenanceList* ml = LFStoreMaintenance();
	LFErrorBox(ml->m_LastError, GetSafeHwnd());

	LFStoreMaintenanceDlg dlg(ml, this);
	dlg.DoModal();

	LFFreeMaintenanceList(ml);
}

void CMainView::OnStoresBackup()
{
	LFBackupStores(this);
}

void CMainView::OnStoresShowEmptyVolumes()
{
	theApp.m_ShowEmptyVolumes = !theApp.m_ShowEmptyVolumes;
	theApp.Reload(LFContextStores);
}

void CMainView::OnUpdateStoresCommands(CCmdUI* pCmdUI)
{
	BOOL b = (p_CookedFiles) && (m_Context==LFContextStores);

	switch (pCmdUI->m_nID)
	{
	case IDM_STORES_SHOWEMPTYVOLUMES:
		pCmdUI->SetCheck(theApp.m_ShowEmptyVolumes);
	case IDM_STORES_CREATENEW:
		break;
	default:
		b &= (LFGetStoreCount()>0);
	}

	pCmdUI->Enable(b);
}


// Home

void CMainView::OnHomeShowEmptyDomains()
{
	theApp.m_ShowEmptyDomains = !theApp.m_ShowEmptyDomains;
	theApp.Reload(LFContextStoreHome);
}

void CMainView::OnHomeShowStatistics()
{
	theApp.m_ShowStatistics = !theApp.m_ShowStatistics;
	theApp.UpdateFooter(LFContextStoreHome);
}

void CMainView::OnHomeImportFolder()
{
	if (m_StoreIDValid)
		LFImportFolder(m_StoreID, this, TRUE);
}

void CMainView::OnHomeMaintain()
{
	if (m_StoreIDValid)
	{
		CWaitCursor wait;

		LFMaintenanceList* ml = LFStoreMaintenance(m_StoreID);
		LFErrorBox(ml->m_LastError, GetSafeHwnd());

		LFStoreMaintenanceDlg dlg(ml, this);
		dlg.DoModal();

		LFFreeMaintenanceList(ml);
	}
}

void CMainView::OnHomeProperties()
{
	if (m_StoreIDValid)
	{
		LFStorePropertiesDlg dlg(m_StoreID, this);
		dlg.DoModal();
	}
}

void CMainView::OnUpdateHomeCommands(CCmdUI* pCmdUI)
{
	BOOL b = (p_CookedFiles) && (m_Context==LFContextStoreHome);

	switch (pCmdUI->m_nID)
	{
	case IDM_HOME_SHOWEMPTYDOMAINS:
		pCmdUI->SetCheck(theApp.m_ShowEmptyDomains);
		break;
	case IDM_HOME_SHOWSTATISTICS:
		pCmdUI->SetCheck(theApp.m_ShowStatistics);
		break;
	}

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
				((item->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeFile) ||
				((item->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeVolume);
			break;
		}
	}

	pCmdUI->Enable(b);
}


// Drive

void CMainView::OnVolumeCreateNewStore()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		LFStoreDescriptor* s = LFAllocStoreDescriptor();

		LFStoreNewVolumeDlg dlg(this, p_CookedFiles->m_Items[idx]->CoreAttributes.FileID[0], s);
		if (dlg.DoModal()==IDOK)
			LFErrorBox(LFCreateStore(s, FALSE), GetSafeHwnd());

		LFFreeStoreDescriptor(s);
	}
}

void CMainView::OnVolumeFormat()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		CHAR Drive = p_CookedFiles->m_Items[idx]->CoreAttributes.FileID[0];
		if (LFStoresOnVolume(Drive))
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
			ExecuteContextMenu(Drive, "format");
		}
	}
}

void CMainView::OnVolumeEject()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
		ExecuteContextMenu(p_CookedFiles->m_Items[idx]->CoreAttributes.FileID[0], "eject");
}

void CMainView::OnVolumeProperties()
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
		case IDM_VOLUME_CREATENEWSTORE:
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
		CWaitCursor wait;

		LFMaintenanceList* ml = LFStoreMaintenance(p_CookedFiles->m_Items[idx]->StoreID);
		LFErrorBox(ml->m_LastError, GetSafeHwnd());

		LFStoreMaintenanceDlg dlg(ml, this);
		dlg.DoModal();

		LFFreeMaintenanceList(ml);
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
		LFErrorBox(((LFApplication*)AfxGetApp())->DeleteStore(p_CookedFiles->m_Items[idx], this), GetSafeHwnd());
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
			case IDM_STORE_SHORTCUT:
				b = (item->CategoryID!=LFItemCategoryExternalStores);
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
	CMainWnd* pClipboard = theApp.GetClipboard();
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

void CMainView::OnFileCopy()
{
	LFTransactionList* tl = BuildTransactionList(FALSE, TRUE);
	if (tl->m_ItemCount)
	{
		CWaitCursor wait;
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
		CWaitCursor wait;
		for (UINT a=0; a<tl->m_ItemCount; a++)
			if ((tl->m_Items[a].LastError==LFOk) && (tl->m_Items[a].Processed))
				CreateShortcut(&tl->m_Items[a]);
	}

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
	case IDM_FILE_COPY:
	case IDM_FILE_SHORTCUT:
	case IDM_FILE_DELETE:
		b = m_FilesSelected;
		break;
	case IDM_FILE_RENAME:
		if ((item) && (m_Context!=LFContextTrash))
			b = ((item->Type & LFTypeMask)==LFTypeFile);
		if (p_wndFileView)
			b &= !p_wndFileView->IsEditing();
		break;
	case IDM_FILE_RESTORE:
		b = m_FilesSelected && (m_Context==LFContextTrash);
		break;
	}

	pCmdUI->Enable(b);
}
