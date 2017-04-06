
// OrganizeDlg.cpp: Implementierung der Klasse OrganizeDlg
//

#include "stdafx.h"
#include "OrganizeDlg.h"


// OrganizeDlg
//

OrganizeDlg::OrganizeDlg(UINT Context, CWnd* pParentWnd)
	: LFAttributeListDlg(IDD_ORGANIZE, pParentWnd)
{
	p_ContextViewSettings = &theApp.m_ContextViewSettings[m_Context=Context];
}

void OrganizeDlg::DoDataExchange(CDataExchange* pDX)
{
	LFAttributeListDlg::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SORTATTRIBUTE, m_wndSortAttribute);
	DDX_Control(pDX, IDC_SORTDIRECTION, m_wndSortDirection);

	if (pDX->m_bSaveAndValidate)
		theApp.SetContextSort(m_Context,
			(UINT)m_wndSortAttribute.GetItemData(m_wndSortAttribute.GetNextItem(-1, LVNI_SELECTED)),
			m_wndSortDirection.GetCurSel());
}

void OrganizeDlg::TestAttribute(UINT Attr, BOOL& Add, BOOL& Check)
{
	Add = theApp.IsAttributeAvailable(m_Context, Attr) && (theApp.m_Attributes[Attr].TypeProperties.Sortable) && (theApp.m_Attributes[Attr].AttrProperties.DefaultView!=(UINT)-1);

	Check = FALSE;
}

BOOL OrganizeDlg::InitDialog()
{
	LFAttributeListDlg::InitDialog();

	// Titelleiste
	CString Text;
	GetWindowText(Text);

	CString Caption;
	Caption.Format(Text, theApp.m_Contexts[m_Context].Name);

	SetWindowText(Caption);

	// Attribut-Liste füllen
	PopulateListCtrl(IDC_SORTATTRIBUTE, FALSE, p_ContextViewSettings->SortBy);

	// Combobox füllen
	m_wndSortDirection.SetCurSel(p_ContextViewSettings->Descending ? 1 : 0);

	// Ggf. Elemente deaktivieren
	if (p_ContextViewSettings->View>LFViewDetails)
		m_wndSortDirection.EnableWindow(FALSE);

	return TRUE;
}


BEGIN_MESSAGE_MAP(OrganizeDlg, LFAttributeListDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_SORTATTRIBUTE, OnItemChanged)
	ON_NOTIFY(NM_DBLCLK, IDC_SORTATTRIBUTE, OnDoubleClick)
END_MESSAGE_MAP()

void OrganizeDlg::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && ((pNMListView->uOldState & LVIS_SELECTED) || (pNMListView->uNewState & LVIS_SELECTED)))
	{
		INT Index = m_wndSortAttribute.GetNextItem(-1, LVNI_SELECTED);
		if (Index!=-1)
		{
			const UINT Attr = (UINT)m_wndSortAttribute.GetItemData(Index);

			m_wndSortDirection.SetCurSel(theApp.m_Attributes[Attr].AttrProperties.DefaultDescending ? 1 : 0);
		}
	}
}

void OrganizeDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	PostMessage(WM_COMMAND, (WPARAM)IDOK);
}
