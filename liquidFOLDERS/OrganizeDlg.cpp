
// OrganizeDlg.cpp: Implementierung der Klasse OrganizeDlg
//

#include "stdafx.h"
#include "OrganizeDlg.h"


// OrganizeDlg
//

OrganizeDlg::OrganizeDlg(UINT Context, CWnd* pParentWnd)
	: LFAttributeListDlg(IDD_ORGANIZE, pParentWnd)
{
	p_View = &theApp.m_Views[Context];
	m_Context = Context;
}

void OrganizeDlg::DoDataExchange(CDataExchange* pDX)
{
	LFAttributeListDlg::DoDataExchange(pDX);

	DDX_Check(pDX, IDC_AUTODIRS, p_View->AutoDirs);
	DDX_Control(pDX, IDC_SORTDIRECTION, m_wndSortDirection);

	if (pDX->m_bSaveAndValidate)
	{
		const CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_SORTATTRIBUTE);
		p_View->SortBy = (UINT)pList->GetItemData(pList->GetNextItem(-1, LVNI_SELECTED));

		p_View->Descending = m_wndSortDirection.GetCurSel();
	}
}

void OrganizeDlg::TestAttribute(UINT Attr, BOOL& Add, BOOL& Check)
{
	Add = LFIsAttributeAllowed(theApp.m_Contexts[m_Context], Attr) && (theApp.m_Attributes[Attr].Sortable);
	Check = FALSE;
}


BEGIN_MESSAGE_MAP(OrganizeDlg, LFAttributeListDlg)
	ON_NOTIFY(NM_DBLCLK, IDC_SORTATTRIBUTE, OnDoubleClick)
END_MESSAGE_MAP()

BOOL OrganizeDlg::OnInitDialog()
{
	LFAttributeListDlg::OnInitDialog();

	// Titelleiste
	CString text;
	GetWindowText(text);
	CString Caption;
	Caption.Format(text, theApp.m_Contexts[m_Context].Name);
	SetWindowText(Caption);

	// Attribut-Liste füllen
	PopulateListCtrl(IDC_SORTATTRIBUTE, FALSE, p_View->SortBy);

	// Combobox füllen
	ENSURE(text.LoadString(IDS_ASCENDING));
	m_wndSortDirection.AddString(text);
	ENSURE(text.LoadString(IDS_DESCENDING));
	m_wndSortDirection.AddString(text);

	m_wndSortDirection.SetCurSel(p_View->Descending ? 1 : 0);

	// Ggf. Elemente deaktivieren
	if ((p_View->Mode>LFViewPreview) || (!theApp.m_Contexts[m_Context].AllowGroups))
		GetDlgItem(IDC_AUTODIRS)->EnableWindow(FALSE);

	if (p_View->Mode>LFViewPreview)
		GetDlgItem(IDC_SORTDIRECTION)->EnableWindow(FALSE);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void OrganizeDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	PostMessage(WM_COMMAND, (WPARAM)IDOK);
}
