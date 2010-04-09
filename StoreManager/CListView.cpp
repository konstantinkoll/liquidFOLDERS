
// CListView.cpp: Implementierung der Klasse CListView
//

#include "stdafx.h"
#include "CListView.h"
#include "Resource.h"
#include "StoreManager.h"
#include "LFCore.h"


// CListView

CListView::CListView()
{
	RibbonColor = 0;
	ViewID = LFViewAutomatic;
}

CListView::~CListView()
{
	DestroyWindow();
}

void CListView::Create(CWnd* pParentWnd, LFSearchResult* _result, UINT _ViewID)
{
	PreSubclassWindow();
	m_FileList.Create(pParentWnd, this, !_result->m_HasCategories);
	m_hWnd = m_FileList.m_hWnd;
	m_pfnSuper = m_FileList.m_pfnSuper;

	if (_result->m_HasCategories)
	{
		LVGROUP lvg;
		ZeroMemory(&lvg, sizeof(lvg));
		lvg.cbSize = sizeof(lvg);
		lvg.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN;
		lvg.uAlign = LVGA_HEADER_LEFT;
		if (theApp.osInfo.dwMajorVersion>=6)
		{
			lvg.mask |= LVGF_STATE;
			lvg.state = LVGS_COLLAPSIBLE;
			lvg.stateMask = 0;
		}

		for (UINT a=0; a<LFItemCategoryCount; a++)
		{
			lvg.iGroupId = a;
			lvg.pszHeader = theApp.m_ItemCategories[a]->Name;

			if (theApp.osInfo.dwMajorVersion>=6)
			{
				lvg.pszSubtitle = theApp.m_ItemCategories[a]->Hint;
				if (*lvg.pszSubtitle=='\0')
				{
					lvg.mask &= ~LVGF_SUBTITLE;
				}
				else
				{
					lvg.mask |= LVGF_SUBTITLE;
				}
			}

			m_FileList.InsertGroup(a, &lvg);
		}
	}

	CFileView::Create(_result, _ViewID);
}

void CListView::SetSearchResult(LFSearchResult* _result)
{
	m_FileList.SetRedraw(FALSE);
	m_FileList.ItemChanged = 1;

	// Items
	if (m_FileList.OwnerData)
	{
		if (_result)
		{
			m_FileList.SetItemCountEx(_result->m_Count, 0);
		}
		else
		{
			m_FileList.SetItemCountEx(0, 0);
		}
	}
	else
	{
		m_FileList.DeleteAllItems();

		if (_result)
		{
			// Change or append items
			UINT puColumns[] = { 1, 2, 3 };
			LVITEM lvi;
			ZeroMemory(&lvi, sizeof(lvi));
			lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_GROUPID | LVIF_COLUMNS | LVIF_STATE;
			lvi.cColumns = 3;
			lvi.puColumns = puColumns;

			for (UINT a=0; a<_result->m_Count; a++)
			{
				lvi.iItem = a;
				lvi.pszText = (LPWSTR)_result->m_Files[a]->AttributeStrings[LFAttrFileName];
				lvi.iImage = _result->m_Files[a]->IconID-1;
				lvi.iGroupId = _result->m_Files[a]->CategoryID;
				lvi.state = ((_result->m_Files[a]->Type & LFTypeGhosted) ? LVIS_CUT : 0) |
							(((int)a==FocusItem) ? LVIS_FOCUSED : 0);
				lvi.stateMask = LVIS_CUT | LVIS_FOCUSED;
				m_FileList.InsertItem(&lvi);
			}
		}
	}

	// Sortierung
	if (ViewID==LFViewDetails)
		m_FileList.SetHeader(TRUE);

	m_FileList.ItemChanged = 0;
	m_FileList.SetRedraw(TRUE);
}

void CListView::SetViewOptions(UINT _ViewID, BOOL Force)
{
	m_FileList.SetRedraw(FALSE);

	// Font
	if (Force || (pViewParameters->GrannyMode!=m_ViewParameters.GrannyMode))
	{
		SetFont(&theApp.m_Fonts[FALSE][pViewParameters->GrannyMode]);

		if (_ViewID==LFViewDetails)
		{
			CHeaderCtrl * pHdrCtrl = m_FileList.GetHeaderCtrl();
			if (pHdrCtrl)
			{
				pHdrCtrl->SetFont(NULL);
				pHdrCtrl->SetFont(&theApp.m_Fonts[FALSE][FALSE]);
			}
		}
	}

	// Colors
	if (Force || (pViewParameters->Background!=m_ViewParameters.Background) || (theApp.m_nAppLook!=RibbonColor))
	{
		COLORREF back;
		COLORREF text;
		COLORREF highlight;
		theApp.GetBackgroundColors(pViewParameters->Background, &back, &text, &highlight);

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
	if (Force || (pViewParameters->ShowCategories!=m_ViewParameters.ShowCategories) || (_ViewID!=ViewID))
		m_FileList.EnableGroupView(pViewParameters->ShowCategories && (!m_FileList.OwnerData) && (_ViewID!=LFViewList));

	// Full row select
	if (Force || (pViewParameters->FullRowSelect!=m_ViewParameters.FullRowSelect))
		m_FileList.SetExtendedStyle(FileListExtendedStyles | (pViewParameters->FullRowSelect ? LVS_EX_FULLROWSELECT : 0));

	// View
	if (Force || (_ViewID!=ViewID))
	{
		int iView = LV_VIEW_ICON;
		switch (_ViewID)
		{
		case LFViewList:
			iView = LV_VIEW_LIST;
			break;
		case LFViewDetails:
			iView = LV_VIEW_DETAILS;
			break;
		case LFViewTiles:
			LVTILEVIEWINFO tvi;
			ZeroMemory(&tvi, sizeof(tvi));
			tvi.cbSize = sizeof(LVTILEVIEWINFO);
			tvi.cLines = 5;
			tvi.dwFlags = LVTVIF_AUTOSIZE;
			tvi.dwMask = LVTVIM_COLUMNS;
			if ((theApp.osInfo.dwMajorVersion==5) && (m_FileList.OwnerData))  // Only for virtual lists on Windows XP
			{
				tvi.dwMask |= LVTVIM_LABELMARGIN;
				tvi.rcLabelMargin.bottom = (int)(GetFontHeight(pViewParameters->GrannyMode)*1.7);
				tvi.rcLabelMargin.top = -24;
				tvi.rcLabelMargin.left = 1;
				tvi.rcLabelMargin.right = 1;
			}
			m_FileList.SetTileViewInfo(&tvi);
			m_FileList.SetView(LV_VIEW_LIST);
			m_FileList.SetSelectedColumn(-1);
			iView = LV_VIEW_TILE;
		}

		ModifyStyle(LVS_ALIGNLEFT, _ViewID==LFViewList ? LVS_ALIGNLEFT : 0);
		m_FileList.SetView(iView);

		if (iView!=LV_VIEW_DETAILS)
			m_FileList.CreateColumns();
	}

	// Icons
	if (Force || (_ViewID!=ViewID) || (pViewParameters->GrannyMode!=m_ViewParameters.GrannyMode))
	{
		CImageList* icons = NULL;
		switch (_ViewID)
		{
		case LFViewLargeIcons:
		case LFViewPreview:
			icons = &theApp.m_Icons128;
			m_FileList.SetIconSpacing(140, 140+(int)(GetFontHeight(pViewParameters->GrannyMode)*2.5));
			break;
		case LFViewSmallIcons:
			m_FileList.SetIconSpacing(32+(int)(GetFontHeight(pViewParameters->GrannyMode)*8), 48+(int)(GetFontHeight(pViewParameters->GrannyMode)*2.5));
		case LFViewTiles:
			icons = pViewParameters->GrannyMode ? &theApp.m_Icons64 : &theApp.m_Icons48;
			break;
		default:
			icons = pViewParameters->GrannyMode ? &theApp.m_Icons24: &theApp.m_Icons16;
		}
		m_FileList.SetImageList(icons, LVSIL_NORMAL);
		m_FileList.SetImageList(icons, LVSIL_SMALL);
	}

	// Header
	if (_ViewID==LFViewDetails)
		m_FileList.SetHeader();

	m_FileList.SetRedraw(TRUE);
	m_FileList.Invalidate();
}

CMenu* CListView::GetContextMenu()
{
	CMenu* menu;

	if (m_FileList.GetView()==LV_VIEW_DETAILS)
	{
		menu = new CMenu();
		menu->LoadMenu(IDM_DETAILS);
	}
	else
	{
		menu = NULL;
	}

	return menu;
}


LRESULT CListView::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT lResult = 0;

	if (!m_FileList.OnWndMsg(message, wParam, lParam, &lResult))
		lResult = DefWindowProc(message, wParam, lParam);

	return lResult;
}

BOOL CListView::OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pLResult)
{
	return m_FileList.OnChildNotify(message, wParam, lParam, pLResult);
}
