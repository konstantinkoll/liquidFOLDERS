
// CMainView.cpp: Implementierung der Klasse CMainView
//

#include "stdafx.h"
#include "CMainView.h"
#include "CListView.h"
#include "CGlobeView.h"
#include "CTagcloudView.h"


CString MakeHex(BYTE* x, UINT bCount)
{
	CString tmpStr;
	for (UINT a=0; a<bCount; a++)
	{
		CString digit;
		digit.Format(_T("%.2x"), x[a]);
		tmpStr += digit;
		if (a<bCount-1)
			tmpStr += _T(",");
	}
	return tmpStr;
}

void CEscape(CString &s)
{
	for (INT a = s.GetLength()-1; a>=0; a--)
		if ((s[a]==L'\\') || (s[a]==L'\"'))
			s.Insert(a, L'\\');
}


// CMainView
//

CMainView::CMainView()
{
	p_wndFileView = NULL;
	p_Result = NULL;
	m_ContextID = m_ViewID = -1;
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
			((CListView*)pNewView)->Create(this, 3, p_Result, FocusItem);
		}
		break;
	case LFViewGlobe:
		if (m_ViewID!=LFViewGlobe)
		{
			pNewView = new CGlobeView();
			((CGlobeView*)pNewView)->Create(this, 3, p_Result, FocusItem);
		}
		break;
	case LFViewTagcloud:
		if (m_ViewID!=LFViewTagcloud)
		{
			pNewView = new CTagcloudView();
			((CTagcloudView*)pNewView)->Create(this, 3, p_Result, FocusItem);
		}
		break;
	/*case LFViewCalendarYear:
		if (m_ViewID!=LFViewCalendarYear)
		{
			pNewView = new CCalendarYearView();
			((CCalendarYearView*)pNewView)->Create(this, p_Result, FocusItem);
		}
		break;
	case LFViewCalendarDay:
		if (m_ViewID!=LFViewCalendarDay)
		{
			pNewView = new CCalendarDayView();
			((CCalendarDayView*)pNewView)->Create(this, p_Result, FocusItem);
		}
		break;
	case LFViewTimeline:
		if (m_ViewID!=LFViewTimeline)
		{
			pNewView = new CTimelineView();
			((CTimelineView*)pNewView)->Create(this, p_Result, FocusItem);
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
		p_wndFileView->SetOwner(GetParent());	// TODO
		p_wndFileView->SetFocus();
		AdjustLayout();
	}

	return (pNewView!=NULL);
}

void CMainView::UpdateViewOptions(INT Context)
{
	if (((Context==m_ContextID) || (Context==-1)) && (p_wndFileView))
		if (!CreateFileView(theApp.m_Views[Context].Mode, GetFocusItem()))
			p_wndFileView->UpdateViewOptions(Context);
}

void CMainView::UpdateSearchResult(LFSearchResult* Result, INT FocusItem)
{
	p_Result = Result;

	if (!Result)
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
		m_ContextID = Result->m_Context;

		// Header
		BOOL ShowHeader = m_ShowHeader;
		CString Hint;
		CString Mask;
		WCHAR tmpBuf[256];

		switch (m_ContextID)
		{
		case LFContextStores:
			ENSURE(Mask.LoadString(Result->m_ItemCount==1 ? IDS_STORES_SINGULAR : IDS_STORES_PLURAL));
			Hint.Format(Mask, Result->m_StoreCount);
			m_ShowHeader = TRUE;
			break;
		case LFContextStoreHome:
		case LFContextClipboard:
		case LFContextHousekeeping:
		case LFContextTrash:
		case LFContextSubfolderDay:
		case LFContextSubfolderLocation:
			ENSURE(Mask.LoadString(Result->m_FileCount==1 ? IDS_FILES_SINGULAR : IDS_FILES_PLURAL));
			LFINT64ToString(Result->m_FileSize, tmpBuf, 256);
			Hint.Format(Mask, Result->m_FileCount, tmpBuf);
			m_ShowHeader = TRUE;
			break;
		default:
			m_ShowHeader = FALSE;
		}

		if (m_ShowHeader)
		{
			m_wndExplorerHeader.SetColors(m_ContextID<=LFContextClipboard ? 0x126E00 : 0x993300, (COLORREF)-1, FALSE);
			m_wndExplorerHeader.SetText(Result->m_Name, Hint);
		}

		// View
		if (!CreateFileView(theApp.m_Views[p_Result->m_Context].Mode, FocusItem))
		{
			p_wndFileView->UpdateViewOptions(m_ContextID);
			p_wndFileView->UpdateSearchResult(Result, FocusItem);

			if (ShowHeader!=m_ShowHeader)
				AdjustLayout();
		}
	}
}

void CMainView::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	// TODO
	//const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
	const UINT TaskHeight = 0;
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

/*	ON_COMMAND(ID_STORE_NEW, OnStoreNew)
	ON_COMMAND(ID_STORE_NEWINTERNAL, OnStoreNewInternal)
	ON_COMMAND(ID_STORE_NEWDRIVE, OnStoreNewDrive)*/
	ON_COMMAND(IDM_STORE_MAKEDEFAULT, OnStoreMakeDefault)
	ON_COMMAND(IDM_STORE_MAKEHYBRID, OnStoreMakeHybrid)
	ON_COMMAND(IDM_STORE_IMPORTFOLDER, OnStoreImportFolder)
	ON_COMMAND(IDM_STORE_MAINTAIN, OnStoreMaintain)
	ON_COMMAND(IDM_STORE_DELETE, OnStoreDelete)
	ON_COMMAND(IDM_STORE_RENAME, OnStoreRename)
	ON_COMMAND(IDM_STORE_PROPERTIES, OnStoreProperties)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORE_MAKEDEFAULT, IDM_STORE_PROPERTIES, OnUpdateStoreCommands)

	ON_COMMAND(IDM_STORES_CREATENEW, OnStoresCreateNew)
	ON_COMMAND(IDM_STORES_MAINTAINALL, OnStoresMaintainAll)
	ON_COMMAND(IDM_STORES_BACKUP, OnStoresBackup)
	ON_UPDATE_COMMAND_UI_RANGE(IDM_STORES_CREATENEW, IDM_STORES_BACKUP, OnUpdateStoresCommands)
END_MESSAGE_MAP()

INT CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Task bar
	if (!m_wndTaskbar.Create(this, 0, 1))
		return -1;

/*	m_wndTaskbar.AddButton(ID_VIEW_SELECTROOT_TASKBAR, 0);
	m_wndTaskbar.AddButton(ID_VIEW_EXPAND, 1);
	m_wndTaskbar.AddButton(ID_VIEW_OPEN, 2, TRUE);
	m_wndTaskbar.AddButton(ID_VIEW_RENAME, 3);
	m_wndTaskbar.AddButton(ID_VIEW_DELETE, 4);
	m_wndTaskbar.AddButton(ID_VIEW_PROPERTIES, 5);
	m_wndTaskbar.AddButton(ID_APP_NEWSTOREMANAGER, 6, TRUE);

	m_wndTaskbar.AddButton(ID_APP_PURCHASE, 7, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ENTERLICENSEKEY, 8, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_SUPPORT, 9, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ABOUT, 10, TRUE, TRUE);*/

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

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, GetOwner(), NULL);
		delete pMenu;
	}
}


// Store
//

void CMainView::OnStoreMakeDefault()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
		LFErrorBox(LFMakeDefaultStore(p_Result->m_Items[idx]->StoreID, NULL), m_hWnd);
}

void CMainView::OnStoreMakeHybrid()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
		LFErrorBox(LFMakeHybridStore(p_Result->m_Items[idx]->StoreID, NULL), m_hWnd);
}

void CMainView::OnStoreImportFolder()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
		LFImportFolder(p_Result->m_Items[idx]->StoreID, this);
}

void CMainView::OnStoreMaintain()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		LFMaintenanceList* ml = LFStoreMaintenance(p_Result->m_Items[idx]->StoreID);
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
		LFErrorBox(((LFApplication*)AfxGetApp())->DeleteStore(p_Result->m_Items[idx], this));
}

void CMainView::OnStoreProperties()
{
	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		LFStorePropertiesDlg dlg(p_Result->m_Items[idx]->StoreID, this);
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
}

void CMainFrame::OnStoreMaintenance()
{
	LFMaintenanceList* ml = LFStoreMaintenance();
	LFErrorBox(ml->m_LastError);

	LFStoreMaintenanceDlg dlg(ml, this);
	dlg.DoModal();

	LFFreeMaintenanceList(ml);

	OnNavigateReload();
}

void CMainFrame::OnStoreBackup()
{
	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_REGFILEFILTER));
	tmpStr += _T(" (*.reg)|*.reg||");

	CFileDialog dlg(FALSE, _T(".reg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, tmpStr, this);

	if (dlg.DoModal()==IDOK)
	{
		CStdioFile f;
		if (!f.Open(dlg.GetFileName(), CFile::modeCreate | CFile::modeWrite))
		{
			LFErrorBox(LFDriveNotReady, GetSafeHwnd());
		}
		else
		{
			try
			{
				f.WriteString(_T("Windows Registry Editor Version 5.00\n"));

				for (UINT a=0; a<CookedFiles->m_ItemCount; a++)
				{
					LFItemDescriptor* i = CookedFiles->m_Items[a];
					if ((i->Type & LFTypeStore) && (i->CategoryID<=LFItemCategoryHybridStores))
					{
						LFStoreDescriptor s;
						if (LFGetStoreSettings(i->StoreID, &s)==LFOk)
						{
							// Header
							tmpStr = _T("\n[HKEY_CURRENT_USER\\Software\\liquidFOLDERS\\Stores\\");
							tmpStr += s.StoreID;
							f.WriteString(tmpStr+_T("]\n"));

							// Name
							tmpStr = s.StoreName;
							CEscape(tmpStr);
							f.WriteString(_T("\"Name\"=\"")+tmpStr+_T("\"\n"));

							// Mode
							tmpStr.Format(_T("\"Mode\"=dword:%.8x\n"), s.StoreMode);
							f.WriteString(tmpStr);

							// AutoLocation
							tmpStr.Format(_T("\"AutoLocation\"=dword:%.8x\n"), s.AutoLocation);
							f.WriteString(tmpStr);

							if (!s.AutoLocation)
							{
								// Path
								tmpStr = s.DatPath;
								CEscape(tmpStr);
								f.WriteString(_T("\"Path\"=\"")+tmpStr+_T("\"\n"));
							}

							// GUID
							f.WriteString(_T("\"GUID\"=hex:")+MakeHex((BYTE*)&s.guid, sizeof(s.guid))+_T("\n"));

							// CreationTime
							f.WriteString(_T("\"CreationTime\"=hex:")+MakeHex((BYTE*)&s.CreationTime, sizeof(s.CreationTime))+_T("\n"));

							// FileTime
							f.WriteString(_T("\"FileTime\"=hex:")+MakeHex((BYTE*)&s.FileTime, sizeof(s.FileTime))+_T("\n"));
						}
					}
				}
			}
			catch(CFileException ex)
			{
				LFErrorBox(LFDriveNotReady, GetSafeHwnd());
			}

			f.Close();
		}
	}
}*/


void CMainView::OnUpdateStoreCommands(CCmdUI* pCmdUI)
{
	BOOL b = FALSE;

	INT idx = GetSelectedItem();
	if (idx!=-1)
	{
		LFItemDescriptor* i = p_Result->m_Items[idx];
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


// Stores
//

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
	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_REGFILEFILTER));
	tmpStr += _T(" (*.reg)|*.reg||");

	CFileDialog dlg(FALSE, _T(".reg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, tmpStr, this);

	if (dlg.DoModal()==IDOK)
	{
		CStdioFile f;
		if (!f.Open(dlg.GetFileName(), CFile::modeCreate | CFile::modeWrite))
		{
			LFErrorBox(LFDriveNotReady, GetSafeHwnd());
		}
		else
		{
			try
			{
				f.WriteString(_T("Windows Registry Editor Version 5.00\n"));

				for (UINT a=0; a<p_Result->m_ItemCount; a++)
				{
					LFItemDescriptor* i = p_Result->m_Items[a];
					if ((i->Type & LFTypeStore) && (i->CategoryID<=LFItemCategoryHybridStores))
					{
						LFStoreDescriptor s;
						if (LFGetStoreSettings(i->StoreID, &s)==LFOk)
						{
							// Header
							tmpStr = _T("\n[HKEY_CURRENT_USER\\Software\\liquidFOLDERS\\Stores\\");
							tmpStr += s.StoreID;
							f.WriteString(tmpStr+_T("]\n"));

							// Name
							tmpStr = s.StoreName;
							CEscape(tmpStr);
							f.WriteString(_T("\"Name\"=\"")+tmpStr+_T("\"\n"));

							// Mode
							tmpStr.Format(_T("\"Mode\"=dword:%.8x\n"), s.StoreMode);
							f.WriteString(tmpStr);

							// AutoLocation
							tmpStr.Format(_T("\"AutoLocation\"=dword:%.8x\n"), s.AutoLocation);
							f.WriteString(tmpStr);

							if (!s.AutoLocation)
							{
								// Path
								tmpStr = s.DatPath;
								CEscape(tmpStr);
								f.WriteString(_T("\"Path\"=\"")+tmpStr+_T("\"\n"));
							}

							// GUID
							f.WriteString(_T("\"GUID\"=hex:")+MakeHex((BYTE*)&s.guid, sizeof(s.guid))+_T("\n"));

							// CreationTime
							f.WriteString(_T("\"CreationTime\"=hex:")+MakeHex((BYTE*)&s.CreationTime, sizeof(s.CreationTime))+_T("\n"));

							// FileTime
							f.WriteString(_T("\"FileTime\"=hex:")+MakeHex((BYTE*)&s.FileTime, sizeof(s.FileTime))+_T("\n"));
						}
					}
				}
			}
			catch(CFileException ex)
			{
				LFErrorBox(LFDriveNotReady, m_hWnd);
			}

			f.Close();
		}
	}
}

void CMainView::OnUpdateStoresCommands(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(pCmdUI->m_nID==IDM_STORES_CREATENEW || LFGetStoreCount());
}
