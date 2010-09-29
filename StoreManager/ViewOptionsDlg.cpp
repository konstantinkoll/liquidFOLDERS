
// ViewOptionsDlg.cpp: Implementierung der Klasse ViewOptionsDlg
//

#include "stdafx.h"
#include "ViewOptionsDlg.h"
#include "LFCore.h"


// ViewOptionsDlg
//

ViewOptionsDlg::ViewOptionsDlg(CWnd* pParentWnd, LFViewParameters* View, UINT Context)
	: ChooseDetailsDlg(pParentWnd, View, Context, IDD_VIEWOPTIONS)
{
}

void ViewOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	ChooseDetailsDlg::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_FULLROWSELECT, p_View->FullRowSelect);

	if (pDX->m_bSaveAndValidate)
	{
		CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_VIEWMODES);
		p_View->Mode = (UINT)pList->GetItemData(pList->GetNextItem(-1, LVNI_SELECTED));
	}
}


BEGIN_MESSAGE_MAP(ViewOptionsDlg, ChooseDetailsDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_VIEWMODES, OnViewModeChange)
	ON_NOTIFY(NM_DBLCLK, IDC_VIEWMODES, OnDoubleClick)
	ON_COMMAND(IDC_CHECKALL, OnCheckAll)
	ON_COMMAND(IDC_UNCHECKALL, OnUncheckAll)
END_MESSAGE_MAP()

BOOL ViewOptionsDlg::OnInitDialog()
{
	ChooseDetailsDlg::OnInitDialog();

	// View-Liste füllen
	CStringW tmpStr;

	CListCtrl* l = (CListCtrl*)GetDlgItem(IDC_VIEWMODES);

	LVGROUP lvg;
	ZeroMemory(&lvg, sizeof(lvg));
	lvg.cbSize = sizeof(lvg);
	lvg.mask = LVGF_HEADER | LVGF_GROUPID | LVGF_ALIGN;
	ENSURE(tmpStr.LoadString(IDS_VIEWGROUP1));
	lvg.pszHeader = tmpStr.GetBuffer();
	lvg.cchHeader = (int)wcslen(lvg.pszHeader);
	lvg.uAlign = LVGA_HEADER_LEFT;
	lvg.state = LVGS_COLLAPSIBLE;
	if (theApp.OSVersion>=OS_Vista)
	{
		lvg.mask |= LVGF_STATE;
		lvg.stateMask = LVGS_COLLAPSIBLE;
	}
	l->InsertGroup(lvg.iGroupId, &lvg);

	for (int a=0; a<2; a++)
	{
		lvg.iGroupId = a;
		ENSURE(tmpStr.LoadString(IDS_VIEWGROUP1+a));
		lvg.pszHeader = tmpStr.GetBuffer();
		l->InsertGroup(lvg.iGroupId, &lvg);
	}

	m_ViewIcons.Create(IDB_RIBBONVIEW_16, NULL, 1, 12);
	l->SetImageList(&m_ViewIcons, LVSIL_SMALL);
	l->SetIconSpacing(68, 30);
	l->EnableGroupView(TRUE);

	LV_ITEM lvi;
	ZeroMemory(&lvi, sizeof(lvi));
	lvi.mask = LVIF_TEXT | LVIF_IMAGE | LVIF_GROUPID | LVIF_PARAM | LVIF_STATE;
	lvi.iGroupId = 0;

	for (UINT a=0; a<LFViewCount; a++)
	{
		if (theApp.m_AllowedViews[m_Context]->IsSet(a))
		{
			CString tmpStr = theApp.GetCommandName(ID_APP_VIEW_LARGEICONS+a);
			lvi.lParam = (LPARAM)a;
			lvi.pszText = tmpStr.GetBuffer();
			lvi.iImage = a;
			lvi.state = lvi.stateMask = (a==p_View->Mode) ? LVIS_SELECTED | LVIS_FOCUSED : 0;
			l->InsertItem(&lvi);
		}

		if ((a==5) || (a==8))
			lvi.iGroupId++;
	}

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
		int vm = (int)pNMListView->lParam;
		BOOL enable = (vm==LFViewDetails) || (vm==LFViewCalendarDay);

		m_ShowAttributes.EnableWindow(enable);
		GetDlgItem(IDC_CHECKALL)->EnableWindow(enable);
		GetDlgItem(IDC_UNCHECKALL)->EnableWindow(enable);
		GetDlgItem(IDC_FULLROWSELECT)->EnableWindow(enable);

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
