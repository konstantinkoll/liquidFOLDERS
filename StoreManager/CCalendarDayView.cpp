
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

	int heightDay = 30;

	m_CalendarHeaderCtrl.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), heightDay, SWP_NOACTIVATE | SWP_NOZORDER);
	m_FileList.SetWindowPos(NULL, rectClient.left, rectClient.top + heightDay, rectClient.Width(), rectClient.Height() - heightDay, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CCalendarDayView::Create(CWnd* _pParentWnd, LFSearchResult* _result)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, NULL, NULL, NULL);

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	CRect rect;
	rect.SetRectEmpty();
	CWnd::Create(className, _T(""), dwStyle, rect, _pParentWnd, AFX_IDW_PANE_FIRST);

	CFileView::Create(_result, LFViewCalendarDay);
}

void CCalendarDayView::SetSearchResult(LFSearchResult* _result)
{
	m_FileList.SetRedraw(FALSE);
	m_FileList.ItemChanged = 1;

	// Items
	m_FileList.DeleteAllItems();

	if (_result)
	{
		UINT puColumns[] = { LFAttrFileTime, LFAttrFileSize, LFAttrHint };
		LVITEM lvi;
		ZeroMemory(&lvi, sizeof(lvi));
		lvi.mask = LVIF_TEXT | LVIF_IMAGE | /*LVIF_GROUPID |*/ LVIF_COLUMNS | LVIF_STATE;
		lvi.cColumns = 3;
		lvi.puColumns = puColumns;

		for (UINT a=0; a<_result->m_Count; a++)
		{
			lvi.iItem = a;
			lvi.pszText = (LPWSTR)_result->m_Files[a]->AttributeStrings[LFAttrFileName];
			lvi.iImage = _result->m_Files[a]->IconID-1;
			//lvi.iGroupId = _result->m_Files[a]->CategoryID;
			lvi.state = (_result->m_Files[a]->Type & LFTypeGhosted) ? LVIS_CUT : 0;
			lvi.stateMask = LVIS_CUT;
			m_FileList.InsertItem(&lvi);
		}
	}

	// Sortierung
	m_FileList.SetHeader(TRUE, FALSE);

	m_FileList.ItemChanged = 0;
	m_FileList.SetRedraw(TRUE);
	m_CalendarHeaderCtrl.SetText(L"Hallo Du");
}

void CCalendarDayView::SetViewOptions(UINT /*_ViewID*/, BOOL Force)
{
	m_FileList.SetRedraw(FALSE);

	// Font and icons
	if (Force || (pViewParameters->GrannyMode!=m_ViewParameters.GrannyMode))
	{
		SetFont(&theApp.m_Fonts[FALSE][pViewParameters->GrannyMode]);

		CHeaderCtrl * pHdrCtrl = m_FileList.GetHeaderCtrl();
		if (pHdrCtrl)
		{
			pHdrCtrl->SetFont(NULL);
			pHdrCtrl->SetFont(&theApp.m_Fonts[FALSE][FALSE]);
		}

		CImageList* icons = pViewParameters->GrannyMode ? &theApp.m_Icons24: &theApp.m_Icons16;
		m_FileList.SetImageList(icons, LVSIL_NORMAL);
		m_FileList.SetImageList(icons, LVSIL_SMALL);
	}

	// Colors
	if (Force || (pViewParameters->Background!=m_ViewParameters.Background) || (theApp.m_nAppLook!=RibbonColor))
	{
		COLORREF back;
		COLORREF text;
		COLORREF highlight;
		theApp.GetBackgroundColors(pViewParameters->Background, &back, &text, &highlight);

		m_CalendarHeaderCtrl.SetColors(highlight, back, pViewParameters->Background==ChildBackground_Ribbon ? afxGlobalData.clrBarDkShadow : GetSysColor(COLOR_3DSHADOW));
		m_FileList.SetBkColor(back);
		m_FileList.SetTextBkColor(CLR_NONE);
		m_FileList.SetTextColor(text);

		if (theApp.osInfo.dwMajorVersion==5)
		{
			LVGROUPMETRICS metrics;
			ZeroMemory(&metrics, sizeof(LVGROUPMETRICS));
			metrics.cbSize = sizeof(LVGROUPMETRICS);
			metrics.mask = LVGMF_TEXTCOLOR;
			metrics.crHeader = text;
			m_FileList.SetGroupMetrics(&metrics);
		}
	}

	// Categories
	//if (Force || (pViewParameters->ShowCategories!=m_ViewParameters.ShowCategories))
	//	m_FileList.EnableGroupView(pViewParameters->ShowCategories && (!m_FileList.OwnerData));

	// Full row select
	if (Force || (pViewParameters->FullRowSelect!=m_ViewParameters.FullRowSelect))
		m_FileList.SetExtendedStyle(FileListExtendedStyles | (pViewParameters->FullRowSelect ? LVS_EX_FULLROWSELECT : 0));

	// View
	if (Force)
		m_FileList.SetView(LV_VIEW_DETAILS);

	// Header
	m_FileList.SetHeader(FALSE, FALSE);

	m_FileList.SetRedraw(TRUE);
	m_FileList.Invalidate();
}

CMenu* CCalendarDayView::GetContextMenu()
{
	CMenu* menu = new CMenu();
	menu->LoadMenu(IDM_DETAILS);
	return menu;
}


BEGIN_MESSAGE_MAP(CCalendarDayView, CAbstractListView)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int CCalendarDayView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct) == -1)
		return -1;

	m_CalendarHeaderCtrl.Create(this, 2);

	m_FileList.Create(this, this, FALSE);
	//m_FileList.EnableGroupView(TRUE);

	return 0;
}

void CCalendarDayView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}
