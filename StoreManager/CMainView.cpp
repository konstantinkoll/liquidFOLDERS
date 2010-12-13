
// CMainView.cpp: Implementierung der Klasse CMainView
//

#include "stdafx.h"
#include "CMainView.h"
#include "CListView.h"
#include "CGlobeView.h"
#include "CTagcloudView.h"


// CMainView
//

CMainView::CMainView()
{
	p_wndFileView = NULL;
	p_RawFiles = p_CookedFiles = NULL;
	m_Context = m_ViewID = -1;
	m_ShowHeader = FALSE;
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
			((CListView*)pNewView)->Create(this, 3, p_CookedFiles, FocusItem);
		}
		break;
	case LFViewGlobe:
		if (m_ViewID!=LFViewGlobe)
		{
			pNewView = new CGlobeView();
			((CGlobeView*)pNewView)->Create(this, 3, p_CookedFiles, FocusItem);
		}
		break;
	case LFViewTagcloud:
		if (m_ViewID!=LFViewTagcloud)
		{
			pNewView = new CTagcloudView();
			((CTagcloudView*)pNewView)->Create(this, 3, p_CookedFiles, FocusItem);
		}
		break;
	/*case LFViewCalendarYear:
		if (m_ViewID!=LFViewCalendarYear)
		{
			pNewView = new CCalendarYearView();
			((CCalendarYearView*)pNewView)->Create(this, p_CookedFiles, FocusItem);
		}
		break;
	case LFViewCalendarDay:
		if (m_ViewID!=LFViewCalendarDay)
		{
			pNewView = new CCalendarDayView();
			((CCalendarDayView*)pNewView)->Create(this, p_CookedFiles, FocusItem);
		}
		break;
	case LFViewTimeline:
		if (m_ViewID!=LFViewTimeline)
		{
			pNewView = new CTimelineView();
			((CTimelineView*)pNewView)->Create(this, p_CookedFiles, FocusItem);
		}
		break;*/
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

	if (!pCookedFiles)
	{
		// Header
		m_wndExplorerHeader.SetText(_T(""), _T(""));

		// View
		if (p_wndFileView)
			p_wndFileView->UpdateSearchResult(NULL, -1);
	}
	else
	{
		// Context
		m_Context = pCookedFiles->m_Context;

		// Header
		BOOL ShowHeader = m_ShowHeader;
		CString Hint;
		CString Mask;
		WCHAR tmpBuf[256];

		switch (m_Context)
		{
		case LFContextStores:
			ENSURE(Mask.LoadString(pCookedFiles->m_ItemCount==1 ? IDS_STORES_SINGULAR : IDS_STORES_PLURAL));
			Hint.Format(Mask, pCookedFiles->m_StoreCount);
			m_ShowHeader = TRUE;
			break;
		case LFContextStoreHome:
		case LFContextClipboard:
		case LFContextHousekeeping:
		case LFContextTrash:
		case LFContextSubfolderDay:
		case LFContextSubfolderLocation:
			ENSURE(Mask.LoadString(pCookedFiles->m_FileCount==1 ? IDS_FILES_SINGULAR : IDS_FILES_PLURAL));
			LFINT64ToString(pCookedFiles->m_FileSize, tmpBuf, 256);
			Hint.Format(Mask, pCookedFiles->m_FileCount, tmpBuf);
			m_ShowHeader = TRUE;
			break;
		default:
			m_ShowHeader = FALSE;
		}

		if (m_ShowHeader)
		{
			m_wndExplorerHeader.SetColors(m_Context<=LFContextClipboard ? 0x126E00 : 0x993300, (COLORREF)-1, FALSE);
			m_wndExplorerHeader.SetText(pCookedFiles->m_Name, Hint);
		}

		// View
		if (!CreateFileView(theApp.m_Views[pCookedFiles->m_Context].Mode, FocusItem))
		{
			p_wndFileView->UpdateViewOptions(m_Context);
			p_wndFileView->UpdateSearchResult(pCookedFiles, FocusItem);

			if (ShowHeader!=m_ShowHeader)
				AdjustLayout();
		}
	}

	OnUpdateSelection();
}

void CMainView::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
	m_wndTaskbar.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	const UINT ExplorerHeight = m_ShowHeader ? m_wndExplorerHeader.GetPreferredHeight() : 0;
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


BEGIN_MESSAGE_MAP(CMainView, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_WM_CONTEXTMENU()
	ON_MESSAGE_VOID(WM_UPDATESELECTION, OnUpdateSelection)

	ON_COMMAND(IDM_STORES_CREATENEW, OnStoresCreateNew)
	ON_COMMAND(IDM_STORES_MAINTAINALL, OnStoresMaintainAll)
	ON_COMMAND(IDM_STORES_BACKUP, OnStoresBackup)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORES_CREATENEW, IDM_STORES_BACKUP, OnUpdateStoresCommands)
	ON_UPDATE_COMMAND_UI(IDM_STORES_HIDEEMPTYDRIVES, OnUpdateStoresCommands)

	ON_COMMAND(IDM_HOME_IMPORTFOLDER, OnHomeImportFolder)
	ON_COMMAND(IDM_HOME_MAINTAIN, OnHomeMaintain)
	ON_COMMAND(IDM_HOME_PROPERTIES, OnHomeProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_HOME_HIDEEMPTYDOMAINS, IDM_HOME_PROPERTIES, OnUpdateHomeCommands)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_HOUSEKEEPING_REGISTER, IDM_HOUSEKEEPING_SEND, OnUpdateHousekeepingCommands)

	ON_UPDATE_COMMAND_UI_RANGE(IDM_TRASH_EMPTY, IDM_TRASH_RESTORESELECTED, OnUpdateTrashCommands)

	ON_UPDATE_COMMAND_UI(IDM_ITEM_OPEN, OnUpdateItemCommands)

	ON_COMMAND(IDM_STORE_MAKEDEFAULT, OnStoreMakeDefault)
	ON_COMMAND(IDM_STORE_MAKEHYBRID, OnStoreMakeHybrid)
	ON_COMMAND(IDM_STORE_IMPORTFOLDER, OnStoreImportFolder)
	ON_COMMAND(IDM_STORE_MAINTAIN, OnStoreMaintain)
	ON_COMMAND(IDM_STORE_DELETE, OnStoreDelete)
	ON_COMMAND(IDM_STORE_RENAME, OnStoreRename)
	ON_COMMAND(IDM_STORE_PROPERTIES, OnStoreProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORE_MAKEDEFAULT, IDM_STORE_PROPERTIES, OnUpdateStoreCommands)
END_MESSAGE_MAP()

INT CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Task bar
	if (!m_wndTaskbar.Create(this, IDB_TASKS, 1))
		return -1;

	m_wndTaskbar.AddButton(IDM_STORES_CREATENEW, 0);
	m_wndTaskbar.AddButton(IDM_STORES_MAINTAINALL, 1, TRUE);
	m_wndTaskbar.AddButton(IDM_HOME_IMPORTFOLDER, 2);
	m_wndTaskbar.AddButton(IDM_HOUSEKEEPING_REGISTER, 3);
	m_wndTaskbar.AddButton(IDM_HOUSEKEEPING_SEND, 28, TRUE);
	m_wndTaskbar.AddButton(IDM_TRASH_EMPTY, 4, TRUE);
	m_wndTaskbar.AddButton(IDM_TRASH_RESTOREALL, 5);
	m_wndTaskbar.AddButton(IDM_TRASH_RESTORESELECTED, 6);
	m_wndTaskbar.AddButton(IDM_ITEM_OPEN, 7);
	m_wndTaskbar.AddButton(IDM_STORE_DELETE, 8);
	m_wndTaskbar.AddButton(IDM_STORE_RENAME, 9);
	m_wndTaskbar.AddButton(IDM_STORE_PROPERTIES, 10);
	m_wndTaskbar.AddButton(IDM_STORE_MAKEDEFAULT, 11);
	m_wndTaskbar.AddButton(IDM_STORE_IMPORTFOLDER, 2);
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

void CMainView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (!p_wndFileView)
		return;

	CMenu* pMenu = p_wndFileView->GetBackgroundContextMenu();
	if (pMenu)
	{
		CMenu* pPopup = pMenu->GetSubMenu(0);
		ASSERT_VALID(pPopup);

		CString tmpStr;
		if (m_Context==LFContextStores)
		{
			pPopup->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);

			ENSURE(tmpStr.LoadString(IDM_STORES_HIDEEMPTYDRIVES));
			pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, IDM_STORES_HIDEEMPTYDRIVES, tmpStr);
		}

		CString mask;
		ENSURE(mask.LoadString(IDS_CONTEXTMENU_VIEWOPTIONS));
		tmpStr.Format(mask, theApp.m_Contexts[m_Context]->Name);
		pPopup->InsertMenu(0, MF_STRING | MF_BYPOSITION, ID_APP_VIEWOPTIONS, tmpStr);

		pPopup->InsertMenu(0, MF_SEPARATOR | MF_BYPOSITION);

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

	// TODO
	((CMainFrame*)GetParent())->OnUpdateSelection();
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

void CMainView::OnUpdateHousekeepingCommands(CCmdUI* pCmdUI)
{
	BOOL b = (p_CookedFiles) && (m_Context==LFContextHousekeeping);

	if ((p_CookedFiles) && (pCmdUI->m_nID==IDM_HOUSEKEEPING_REGISTER))
		b &= (p_CookedFiles->m_ItemCount>0);

	pCmdUI->Enable(b);
}


// Trash

void CMainView::OnUpdateTrashCommands(CCmdUI* pCmdUI)
{
	BOOL b = (p_CookedFiles) && (m_Context==LFContextTrash);
	
	if (pCmdUI->m_nID==IDM_TRASH_RESTORESELECTED)
		b &= FALSE; //TODO

	pCmdUI->Enable(b);
}


// Item

void CMainView::OnUpdateItemCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		LFItemDescriptor* i = p_CookedFiles->m_Items[idx];
		switch (pCmdUI->m_nID)
		{
		case IDM_ITEM_OPEN:
			b = (i->NextFilter!=NULL) ||
				((i->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeDrive) ||
				((i->Type & (LFTypeMask | LFTypeNotMounted))==LFTypeFile);
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
		LFErrorBox(LFMakeDefaultStore(p_CookedFiles->m_Items[idx]->StoreID, NULL), m_hWnd);
}

void CMainView::OnStoreMakeHybrid()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
		LFErrorBox(LFMakeHybridStore(p_CookedFiles->m_Items[idx]->StoreID, NULL), m_hWnd);
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



/*

void CMainFrame::OnStoreNewDrive(CHAR drv)
{
	INT i = m_wndMainView.GetSelectedItem();

	if (i!=-1)
	{
		LFStoreDescriptor* s = LFAllocStoreDescriptor();

		LFStoreNewDriveDlg dlg(this, drv, s);
		if (dlg.DoModal()==IDOK)
			LFErrorBox(LFCreateStore(s, FALSE), m_hWnd);

		LFFreeStoreDescriptor(s);
	}
}

void CMainFrame::OnStoreNewDrive()
{
	INT i = m_wndMainView.GetSelectedItem();

	if (i!=-1)
		OnStoreNewDrive(CookedFiles->m_Items[i]->CoreAttributes.FileID[0]);
}*/


void CMainView::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		LFItemDescriptor* i = p_CookedFiles->m_Items[idx];
		if ((i->Type & LFTypeMask)==LFTypeStore)
			switch (pCmdUI->m_nID)
			{
			case IDM_STORE_MAKEDEFAULT:
				b = (i->CategoryID==LFItemCategoryInternalStores) && (!(i->Type & LFTypeDefaultStore));
				break;
			case IDM_STORE_MAKEHYBRID:
				b = (i->CategoryID==LFItemCategoryExternalStores) && (!(i->Type & LFTypeNotMounted));
				break;
			case IDM_STORE_IMPORTFOLDER:
				b = !(i->Type & LFTypeNotMounted);
				break;
			default:
				b = TRUE;
			}
	}

	pCmdUI->Enable(b);
}
