
// CCalendarDayView.cpp: Implementierung der Klasse CCalendarDayView
//

#include "stdafx.h"
#include "CCalendarDayView.h"
#include "Resource.h"
#include "StoreManager.h"
#include "LFCore.h"


// CCalendarDayView
//

CCalendarDayView::CCalendarDayView()
{
	RibbonColor = 0;
}

CCalendarDayView::~CCalendarDayView()
{
}

void CCalendarDayView::AdjustLayout()
{
	CRect rectClient;
	GetClientRect(rectClient);

	INT heightDay = 30;

	m_CalendarHeaderCtrl.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), heightDay, SWP_NOACTIVATE | SWP_NOZORDER);
	m_FileList.SetWindowPos(NULL, rectClient.left, rectClient.top + heightDay, rectClient.Width(), rectClient.Height() - heightDay, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CCalendarDayView::Create(CWnd* _pParentWnd, LFSearchResult* _result, INT _FocusItem)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS);

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	CRect rect;
	rect.SetRectEmpty();
	CWnd::Create(className, _T(""), dwStyle, rect, _pParentWnd, AFX_IDW_PANE_FIRST);

	CFileView::Create(_result, LFViewCalendarDay, _FocusItem, FALSE, FALSE);
}

void CCalendarDayView::SetSearchResult(LFSearchResult* _result)
{
	m_FileList.SetRedraw(FALSE);
	m_FileList.ItemChanged = 1;

	// Items
	m_FileList.DeleteAllItems();

	if (_result)
	{
		UINT puColumns[] = { LFAttrFileTime, LFAttrFileSize, LFAttrDescription };
		LVITEM lvi;
		ZeroMemory(&lvi, sizeof(lvi));
		lvi.mask = LVIF_TEXT | LVIF_IMAGE | /*LVIF_GROUPID |*/ LVIF_COLUMNS | LVIF_STATE;
		lvi.cColumns = 3;
		lvi.puColumns = puColumns;

		for (UINT a=0; a<_result->m_ItemCount; a++)
		{
			lvi.iItem = a;
			lvi.pszText = (LPWSTR)_result->m_Items[a]->CoreAttributes.FileName;
			lvi.iImage = _result->m_Items[a]->IconID-1;
			//lvi.iGroupId = _result->m_Items[a]->CategoryID;
			lvi.state = (_result->m_Items[a]->Type & LFTypeGhosted) ? LVIS_CUT : 0;
			lvi.stateMask = LVIS_CUT;
			m_FileList.InsertItem(&lvi);
		}
	}

	// Sortierung
	m_FileList.SetHeader(TRUE, FALSE);

	m_FileList.ItemChanged = 0;
	m_FileList.SetRedraw(TRUE);
	m_CalendarHeaderCtrl.SetText(_T("Test (current day along with controls will appear here)"));
}

void CCalendarDayView::SetViewOptions(UINT /*_ViewID*/, BOOL Force)
{
	// Font
	if (Force)
	{
		m_FileList.SetFont(&theApp.m_DefaultFont);

		CHeaderCtrl * pHdrCtrl = m_FileList.GetHeaderCtrl();
		if (pHdrCtrl)
		{
			pHdrCtrl->SetFont(NULL);
			pHdrCtrl->SetFont(&theApp.m_DefaultFont);
		}
	}

	// Categories
	//if (Force || (pViewParameters->ShowCategories!=m_ViewParameters.ShowCategories) || (_ViewID!=ViewID))
	//	m_FileList.EnableGroupView(pViewParameters->ShowCategories && (!m_FileList.OwnerData) && (_ViewID!=LFViewList));

	// Full row select
	if (Force || (pViewParameters->FullRowSelect!=m_ViewParameters.FullRowSelect))
		m_FileList.SetExtendedStyle(FileListExtendedStyles | (pViewParameters->FullRowSelect ? LVS_EX_FULLROWSELECT : 0));

	// View
	if (Force)
	{
		m_FileList.SetView(LV_VIEW_DETAILS);
		m_FileList.CreateColumns();
	}
	else
	{
		m_FileList.SetHeader();
	}

	// Farbe
	if (Force)
		OnSysColorChange();

	// Icons
	if (Force)
	{
		m_FileList.SetImageList(&theApp.m_Icons16, LVSIL_NORMAL);
		m_FileList.SetImageList(&theApp.m_Icons16, LVSIL_SMALL);
	}
}


BEGIN_MESSAGE_MAP(CCalendarDayView, CAbstractListView)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

INT CCalendarDayView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct)==-1)
		return -1;

	if (!m_CalendarHeaderCtrl.Create(this, 2))
		return -1;

	if (!m_FileList.Create(this, FALSE))
		return -1;

	//m_FileList.EnableGroupView(TRUE);

	return 0;
}

void CCalendarDayView::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}
