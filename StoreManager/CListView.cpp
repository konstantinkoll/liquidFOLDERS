
// CListView.cpp: Implementierung der Klasse CListView
//

#include "stdafx.h"
#include "CListView.h"
#include "Resource.h"
#include "StoreManager.h"
#include "LFCore.h"
#include "LFCommDlg.h"


// CListView

CListView::CListView()
{
	RibbonColor = 0;
	ViewID = LFViewAutomatic;
}

CListView::~CListView()
{
}

void CListView::Create(CWnd* pParentWnd, LFSearchResult* _result, UINT _ViewID)
{
	result = _result;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, NULL, NULL, NULL);

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	CRect rect;
	rect.SetRectEmpty();
	CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, AFX_IDW_PANE_FIRST);

	CFileView::Create(_result, _ViewID);
}

void CListView::AdjustLayout()
{
	CRect rectClient;
	GetClientRect(rectClient);

	m_FileList.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

void CListView::SetSearchResult(LFSearchResult* _result)
{
	m_FileList.ItemChanged = 1;

	// Items
	if (m_FileList.OwnerData)
	{
		if (_result)
		{
			m_FileList.SetItemCountEx(_result->m_ItemCount, 0);
			m_FileList.SetItemState(FocusItem, LVIS_FOCUSED, LVIS_FOCUSED);
			m_FileList.EnsureVisible(FocusItem, FALSE);
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
			UINT puColumns[3];
			if (_result->m_Context==LFContextStoreHome)
			{
				puColumns[0] = 1;
				puColumns[1] = 3;
				puColumns[2] = 4;
			}
			else
			{
				puColumns[0] = 1;
				puColumns[1] = 2;
				puColumns[2] = 3;
			}

			LVITEM lvi;
			ZeroMemory(&lvi, sizeof(lvi));
			lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_GROUPID | LVIF_COLUMNS | LVIF_STATE;
			lvi.puColumns = puColumns;

			for (UINT a=0; a<_result->m_ItemCount; a++)
			{
				lvi.iItem = a;
				lvi.cColumns = (_result->m_Context==LFContextStoreHome) ? _result->m_Items[a]->Hint[0]!='\0' ? 4 : 2 : 3;
				lvi.pszText = (LPWSTR)_result->m_Items[a]->CoreAttributes.FileName;
				lvi.iImage = _result->m_Items[a]->IconID-1;
				lvi.iGroupId = _result->m_Items[a]->CategoryID;
				lvi.state = ((_result->m_Items[a]->Type & LFTypeGhosted) ? LVIS_CUT : 0) |
							(((int)a==FocusItem) ? LVIS_FOCUSED : 0);
				lvi.stateMask = LVIS_CUT | LVIS_FOCUSED;
				m_FileList.InsertItem(&lvi);
			}
		}
	}

	// Sortierung
	if (ViewID==LFViewDetails)
		m_FileList.SetHeader(TRUE);

	// Footer
	if (m_FileList.SupportsFooter())
	{
		m_FileList.RemoveFooter();

		if (_result)
		{
			if ((_result->m_Context==LFContextStoreHome) && (_result->m_HidingItems))
			{
				CString tmpStr1 = _T("?");
				CString tmpStr2 = _T("?");
				tmpStr1.LoadString(ID_NAV_RELOAD_SHOWALL);

				int pos = tmpStr1.Find('\n');
				if (pos!=-1)
				{
					tmpStr2 = tmpStr1.Mid(pos+1);
					tmpStr1.Delete(pos, tmpStr1.GetLength()-pos);
				}

				m_FileList.SetFooterText(tmpStr1);
				m_FileList.InsertFooterButton(0, tmpStr2, NULL, IDI_FLD_Default-1, ID_NAV_RELOAD_SHOWALL);
				IListViewFooterCallback* i = NULL;
				if (m_xFooterCallback.QueryInterface(IID_IListViewFooterCallback, (LPVOID*)&i)==NOERROR)
					m_FileList.ShowFooter(i);
			}

			m_FileList.EnsureVisible(FocusItem, FALSE);
		}
	}

	m_FileList.ItemChanged = 0;
}

void CListView::SetViewOptions(UINT _ViewID, BOOL Force)
{
	// Font
	if (Force || (pViewParameters->GrannyMode!=m_ViewParameters.GrannyMode))
	{
		m_FileList.SetFont(&theApp.m_Fonts[FALSE][pViewParameters->GrannyMode]);

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
		m_FileList.SetTextBkColor(back);
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
			tvi.cLines = 3;
			tvi.dwFlags = LVTVIF_FIXEDWIDTH;
			tvi.dwMask = LVTVIM_COLUMNS | LVTVIM_TILESIZE;
			tvi.sizeTile.cx = 240;
			if ((theApp.osInfo.dwMajorVersion==5) && (m_FileList.OwnerData))  // Only for virtual lists on Windows XP
			{
				tvi.dwMask |= LVTVIM_LABELMARGIN;
				tvi.rcLabelMargin.bottom = (int)(GetFontHeight(pViewParameters->GrannyMode)*1.3);
				tvi.rcLabelMargin.top = -18;
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
		m_FileList.CreateColumns();
	}
	else
		if (_ViewID==LFViewDetails)
			m_FileList.SetHeader();

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
			m_FileList.SetIconSpacing(32+(int)(GetFontHeight(pViewParameters->GrannyMode)*8), (int)(GetFontHeight(pViewParameters->GrannyMode)*8));
		case LFViewTiles:
			icons = pViewParameters->GrannyMode ? &theApp.m_Icons64 : &theApp.m_Icons48;
			break;
		default:
			icons = pViewParameters->GrannyMode ? &theApp.m_Icons24: &theApp.m_Icons16;
		}
		m_FileList.SetImageList(icons, LVSIL_NORMAL);
		m_FileList.SetImageList(icons, LVSIL_SMALL);
	}
}


BEGIN_MESSAGE_MAP(CListView, CAbstractListView)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int CListView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct)==-1)
		return -1;

	if (!m_FileList.Create(this, !result->m_HasCategories))
		return -1;

	m_FileList.SetImageList(&theApp.m_Icons16, LVSIL_FOOTER);

	if (result->m_HasCategories)
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
				if (*lvg.pszSubtitle==L'\0')
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

	return 0;
}

void CListView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}


BEGIN_INTERFACE_MAP(CListView, CAbstractListView)
	INTERFACE_PART(CListView, IID_IUnknown, FooterCallback)
	INTERFACE_PART(CListView, IID_IListViewFooterCallback, FooterCallback)
END_INTERFACE_MAP()

IMPLEMENT_IUNKNOWN(CListView, FooterCallback)

STDMETHODIMP CListView::XFooterCallback::OnButtonClicked(int /*itemIndex*/, LPARAM /*lParam*/, PINT pRemoveFooter)
{
	METHOD_PROLOGUE(CListView, FooterCallback);
	pThis->GetParentFrame()->PostMessage(WM_COMMAND, ID_NAV_RELOAD_SHOWALL);
	*pRemoveFooter = TRUE;
	return S_OK;
}

STDMETHODIMP CListView::XFooterCallback::OnDestroyButton(int /*itemIndex*/, LPARAM /*lParam*/)
{
	METHOD_PROLOGUE(CListView, FooterCallback);
	return S_OK;
}
