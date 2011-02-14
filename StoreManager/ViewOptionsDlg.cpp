
// ViewOptionsDlg.cpp: Implementierung der Klasse ViewOptionsDlg
//

#include "stdafx.h"
#include "ViewOptionsDlg.h"
#include "LFCore.h"


// ViewOptionsDlg
//

ViewOptionsDlg::ViewOptionsDlg(CWnd* pParentWnd, UINT Context)
	: ChooseDetailsDlg(pParentWnd, Context, IDD_VIEWOPTIONS)
{
}

void ViewOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	ChooseDetailsDlg::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_VIEWMODES, m_wndViewList);

	if (pDX->m_bSaveAndValidate)
	{
		CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_VIEWMODES);
		p_View->Mode = (UINT)pList->GetItemData(pList->GetNextItem(-1, LVNI_SELECTED));
	}
}


BEGIN_MESSAGE_MAP(ViewOptionsDlg, ChooseDetailsDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_VIEWMODES, OnViewModeChange)
	ON_NOTIFY(NM_DBLCLK, IDC_VIEWMODES, OnDoubleClick)
END_MESSAGE_MAP()

BOOL ViewOptionsDlg::OnInitDialog()
{
	ChooseDetailsDlg::OnInitDialog();

	// Icons
	m_ViewIcons.Create(IDB_VIEWS, NULL, 0, 10, 32, 32);

	// View-Liste
	for (INT a=0; a<2; a++)
	{
		CString tmpStr;
		ENSURE(tmpStr.LoadString(IDS_VIEWGROUP1+a));

		m_wndViewList.AddCategory(a, tmpStr, _T(""), TRUE);
	}

	m_wndViewList.SetImageList(&m_ViewIcons, LVSIL_NORMAL);
	m_wndViewList.SetIconSpacing(68, 64);
	m_wndViewList.EnableGroupView(TRUE);

	// Items
	LV_ITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_GROUPID | LVIF_PARAM | LVIF_STATE;
	lvi.iGroupId = 0;

	for (UINT a=0; a<LFViewCount; a++)
	{
		if (theApp.m_AllowedViews[m_Context]->IsSet(a))
		{
			CString tmpStr;
			ENSURE(tmpStr.LoadString(IDM_VIEW_FIRST+a));

			lvi.lParam = (LPARAM)a;
			lvi.pszText = tmpStr.GetBuffer();
			lvi.iImage = a;
			lvi.state = lvi.stateMask = (a==p_View->Mode) ? LVIS_SELECTED | LVIS_FOCUSED : 0;
			m_wndViewList.InsertItem(&lvi);
		}

		if (a==LFViewPreview)
			lvi.iGroupId++;
	}

	// Aktuelles View auswählen
	NM_LISTVIEW nmlv;
	LRESULT res;
	nmlv.lParam = p_View->Mode;
	nmlv.uNewState = LVIS_SELECTED;
	OnViewModeChange((NMHDR*)&nmlv, &res);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void ViewOptionsDlg::OnViewModeChange(NMHDR* pNMHDR, LRESULT* pResult)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	// Attribute
	if (pNMListView->uNewState & LVIS_SELECTED)
	{
		INT vm = (INT)pNMListView->lParam;
		BOOL enable = (vm==LFViewDetails);

		m_ShowAttributes.EnableWindow(enable);
		GetDlgItem(IDC_CHECKALL)->EnableWindow(enable);
		GetDlgItem(IDC_UNCHECKALL)->EnableWindow(enable);

		NM_LISTVIEW nmlv;
		LRESULT res;
		nmlv.iItem = m_ShowAttributes.GetNextItem(-1, LVIS_SELECTED);
		nmlv.uNewState = LVIS_SELECTED;
		OnSelectionChange((NMHDR*)&nmlv, &res);
	}

	*pResult = 0;
}

void ViewOptionsDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	PostMessage(WM_COMMAND, (WPARAM)IDOK);
}
