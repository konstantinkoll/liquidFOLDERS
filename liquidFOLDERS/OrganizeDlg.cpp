
// OrganizeDlg.cpp: Implementierung der Klasse OrganizeDlg
//

#include "stdafx.h"
#include "OrganizeDlg.h"


// OrganizeDlg
//

OrganizeDlg::OrganizeDlg(UINT Context, CWnd* pParentWnd)
	: LFAttributeListDlg(IDD_ORGANIZE, pParentWnd)
{
	p_ViewParameters = &theApp.m_Views[Context];
	m_Context = Context;
}

void OrganizeDlg::DoDataExchange(CDataExchange* pDX)
{
	LFAttributeListDlg::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_SORTATTRIBUTE, m_wndSortAttribute);
	DDX_Check(pDX, IDC_AUTODIRS, p_ViewParameters->AutoDirs);
	DDX_Control(pDX, IDC_SORTDIRECTION, m_wndSortDirection);

	if (pDX->m_bSaveAndValidate)
	{
		const CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_SORTATTRIBUTE);
		p_ViewParameters->SortBy = (UINT)pList->GetItemData(pList->GetNextItem(-1, LVNI_SELECTED));

		p_ViewParameters->Descending = m_wndSortDirection.GetCurSel();
	}
}

void OrganizeDlg::TestAttribute(UINT Attr, BOOL& Add, BOOL& Check)
{
	Add = LFIsAttributeAllowed(theApp.m_Contexts[m_Context], Attr) && (theApp.m_Attributes[Attr].Sortable);
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
	PopulateListCtrl(IDC_SORTATTRIBUTE, FALSE, p_ViewParameters->SortBy);

	// Combobox füllen
	m_wndSortDirection.SetCurSel(p_ViewParameters->Descending ? 1 : 0);

	// Ggf. Elemente deaktivieren
	if ((p_ViewParameters->Mode>LFViewPreview) || (!theApp.m_Contexts[m_Context].AllowGroups))
		GetDlgItem(IDC_AUTODIRS)->EnableWindow(FALSE);

	if (p_ViewParameters->Mode>LFViewPreview)
		GetDlgItem(IDC_SORTDIRECTION)->EnableWindow(FALSE);

	return TRUE;
}


BEGIN_MESSAGE_MAP(OrganizeDlg, LFAttributeListDlg)
	ON_NOTIFY(NM_DBLCLK, IDC_SORTATTRIBUTE, OnDoubleClick)
END_MESSAGE_MAP()

void OrganizeDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	PostMessage(WM_COMMAND, (WPARAM)IDOK);
}
