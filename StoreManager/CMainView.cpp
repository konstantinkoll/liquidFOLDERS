
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
	if ((Context==m_ContextID) && (p_wndFileView))
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
		CString Caption;
		CString Hint;
		CString Mask;
		WCHAR tmpBuf[256];

		switch (m_ContextID)
		{
		case LFContextStores:
			ENSURE(Mask.LoadString(Result->m_ItemCount==1 ? IDS_ITEMS_SINGULAR : IDS_ITEMS_PLURAL));
			Hint.Format(Mask, Result->m_ItemCount);
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
			m_wndExplorerHeader.SetText(Caption, Hint);
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


/*BOOL CMainView::OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}*/

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
	//ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_SELECTROOT, ID_VIEW_PROPERTIES, OnUpdateTaskbar)
	ON_WM_CONTEXTMENU()
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

void CMainView::OnUpdateTaskbar(CCmdUI* pCmdUI)
{
	pCmdUI->Enable(TRUE);
}

void CMainView::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
{
	if ((pos.x==-1) && (pos.y==-1))
	{
		pos.x = pos.y = 0;
		ClientToScreen(&pos);
	}

/*	CMenu menu;
	ENSURE(menu.LoadMenu(IDM_BACKGROUND));

	CMenu* popup = menu.GetSubMenu(0);
	ASSERT(popup);

	if (!m_IsRootSet)
		popup->EnableMenuItem(ID_VIEW_AUTOSIZEALL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this, NULL);*/
}
