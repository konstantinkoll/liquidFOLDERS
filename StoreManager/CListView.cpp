
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
}

CListView::~CListView()
{
}

void CListView::Create(CWnd* pParentWnd, LFSearchResult* _result, UINT _ViewID, int _FocusItem)
{
	m_HasCategories = _result->m_HasCategories;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS);

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	CRect rect;
	rect.SetRectEmpty();
	CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, AFX_IDW_PANE_FIRST);

	CFileView::Create(_result, _ViewID, _FocusItem, FALSE, FALSE);
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
			if (result)
				m_FileList.SetItemState(FocusItem, LVIS_FOCUSED, LVIS_FOCUSED);
			m_FileList.SetItemCountEx(_result->m_ItemCount, 0);
			if (!result)
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
				lvi.cColumns = (_result->m_Context==LFContextStoreHome) ? _result->m_Items[a]->Description[0]!='\0' ? 4 : 2 : 3;
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
			UINT cmd = 0;
			UINT icon = 0;
			CString footerStr;
			CString btnStr;

			switch (_result->m_Context)
			{
			case LFContextStores:
				if (!_result->m_StoreCount)
				{
					cmd = ID_STORE_NEW;
					icon = IDI_STORE_Default-1;
					footerStr.LoadString(IDS_NOSTORES);
				}
				break;
			case LFContextStoreHome:
				if (_result->m_HidingItems)
				{
					cmd = ID_NAV_RELOAD_SHOWALL;
					icon = IDI_FLD_Default-1;
				}
				break;
			}

			if (cmd)
			{
				CString tmpStr;
				ENSURE(tmpStr.LoadString(cmd));

				int pos = tmpStr.Find('\n');
				if (pos!=-1)
				{
					if (footerStr.IsEmpty())
						footerStr = tmpStr.Mid(0, pos-1);
					if (btnStr.IsEmpty())
						btnStr = tmpStr.Mid(pos+1);
				}

				m_FileList.SetFooterText(footerStr);
				m_FileList.InsertFooterButton(0, btnStr, NULL, icon, cmd);
				IListViewFooterCallback* i = NULL;
				if (m_xFooterCallback.QueryInterface(IID_IListViewFooterCallback, (LPVOID*)&i)==NOERROR)
					m_FileList.ShowFooter(i);
			}
		}
	}

	m_FileList.ItemChanged = 0;
}

void CListView::SetViewOptions(UINT _ViewID, BOOL Force)
{
	// Font
	if (Force)
	{
		m_FileList.SetFont(&theApp.m_DefaultFont);

		if (_ViewID==LFViewDetails)
		{
			CHeaderCtrl * pHdrCtrl = m_FileList.GetHeaderCtrl();
			if (pHdrCtrl)
			{
				pHdrCtrl->SetFont(NULL);
				pHdrCtrl->SetFont(&theApp.m_DefaultFont);
			}
		}
	}

	// Categories
	if (Force || (_ViewID!=ViewID))
		m_FileList.EnableGroupView((!m_FileList.OwnerData) && (_ViewID!=LFViewList));

	// Farbe
	if (Force)
		OnSysColorChange();

	// Icons
	if (Force || (_ViewID!=ViewID))
	{
		CImageList* icons = NULL;
		int nImageList = LVSIL_NORMAL;

		switch (_ViewID)
		{
		case LFViewLargeIcons:
		case LFViewPreview:
			icons = &theApp.m_Icons128;
			m_FileList.SetIconSpacing(140, 140+(int)(GetFontHeight()*2.5));
			break;
		case LFViewSmallIcons:
			m_FileList.SetIconSpacing(32+(int)(GetFontHeight()*8), (int)(GetFontHeight()*8));
		case LFViewTiles:
			icons = &theApp.m_Icons48;
			break;
		default:
			icons = &theApp.m_Icons16;
			nImageList = LVSIL_SMALL;
		}
		m_FileList.SetImageList(icons, nImageList);
	}

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
			if ((theApp.OSVersion==OS_XP) && (m_FileList.OwnerData))  // Only for virtual lists on Windows XP
			{
				tvi.dwMask |= LVTVIM_LABELMARGIN;
				tvi.rcLabelMargin.bottom = (int)(GetFontHeight()*1.3);
				tvi.rcLabelMargin.top = -18;
				tvi.rcLabelMargin.left = 1;
				tvi.rcLabelMargin.right = 1;
			}
			m_FileList.SetTileViewInfo(&tvi);
			m_FileList.SetView(LV_VIEW_LIST);
			m_FileList.SetSelectedColumn(-1);
			iView = LV_VIEW_TILE;
		}

		m_FileList.ModifyStyle(_ViewID!=LFViewList ? LVS_ALIGNLEFT : 0, _ViewID==LFViewList ? LVS_ALIGNLEFT : 0);
		if (theApp.OSVersion==OS_XP)
			m_FileList.SetExtendedStyle(m_FileList.GetExtendedStyle() & !LVS_EX_BORDERSELECT | FileListExtendedStyles | (_ViewID==LFViewPreview ? LVS_EX_BORDERSELECT : 0));

		m_FileList.SetView(iView);
		m_FileList.CreateColumns();
		m_FileList.EnsureVisible(FocusItem, FALSE);
	}
	else
		if (_ViewID==LFViewDetails)
			m_FileList.SetHeader();

	// Full row select
	if (Force || (_ViewID!=ViewID) || (pViewParameters->FullRowSelect!=m_ViewParameters.FullRowSelect))
		if (_ViewID==LFViewDetails)
			m_FileList.SetExtendedStyle(m_FileList.GetExtendedStyle() & !LVS_EX_FULLROWSELECT | FileListExtendedStyles | (pViewParameters->FullRowSelect ? LVS_EX_FULLROWSELECT : 0));
}


BEGIN_MESSAGE_MAP(CListView, CAbstractListView)
	ON_WM_CREATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()

int CListView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct)==-1)
		return -1;

	if (!m_FileList.Create(this, !m_HasCategories))
		return -1;

	m_FileList.SetImageList(&theApp.m_Icons16, LVSIL_FOOTER);

	if (m_HasCategories)
	{
		LVGROUP lvg;
		ZeroMemory(&lvg, sizeof(lvg));
		lvg.cbSize = sizeof(lvg);
		lvg.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN;
		lvg.uAlign = LVGA_HEADER_LEFT;
		if (theApp.OSVersion>=OS_Vista)
		{
			lvg.mask |= LVGF_STATE;
			lvg.state = LVGS_COLLAPSIBLE;
			lvg.stateMask = 0;
		}

		for (UINT a=0; a<LFItemCategoryCount; a++)
		{
			lvg.iGroupId = a;
			lvg.pszHeader = theApp.m_ItemCategories[a]->Name;

			if (theApp.OSVersion>=OS_Vista)
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

STDMETHODIMP CListView::XFooterCallback::OnButtonClicked(int /*itemIndex*/, LPARAM lParam, PINT pRemoveFooter)
{
	METHOD_PROLOGUE(CListView, FooterCallback);
	pThis->GetParentFrame()->PostMessage(WM_COMMAND, (WPARAM)lParam);
	*pRemoveFooter = (lParam==ID_NAV_RELOAD_SHOWALL);
	return S_OK;
}

STDMETHODIMP CListView::XFooterCallback::OnDestroyButton(int /*itemIndex*/, LPARAM /*lParam*/)
{
	METHOD_PROLOGUE(CListView, FooterCallback);
	return S_OK;
}
