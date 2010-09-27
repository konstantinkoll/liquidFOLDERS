
// SortOptionsDlg.cpp: Implementierung der Klasse SortOptionsDlg
//

#include "stdafx.h"
#include "SortOptionsDlg.h"
#include "LFCore.h"


// SortOptionsDlg
//

SortOptionsDlg::SortOptionsDlg(CWnd* pParent, LFViewParameters* View, UINT Context)
	: LFAttributeListDlg(IDD_SORTOPTIONS, pParent)
{
	ASSERT(View);
	p_View = View;
	m_Context = Context;
}

void SortOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Radio(pDX, IDC_ASCENDING, p_View->Descending);
	DDX_Check(pDX, IDC_AUTODIRS, p_View->AutoDirs);

	if (pDX->m_bSaveAndValidate)
	{
		CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_SORTATTRIBUTE);
		p_View->SortBy = (UINT)pList->GetItemData(pList->GetNextItem(-1, LVNI_SELECTED));
	}
}

void SortOptionsDlg::TestAttribute(UINT attr, BOOL& add, BOOL& check)
{
	add = (theApp.m_Contexts[m_Context]->AllowedAttributes->IsSet(attr)) && (theApp.m_Attributes[attr]->Sortable);
	check = FALSE;
}


BEGIN_MESSAGE_MAP(SortOptionsDlg, LFAttributeListDlg)
	ON_BN_CLICKED(IDC_AUTODIRS, OnSetAttrGroupBox)
	ON_NOTIFY(NM_DBLCLK, IDC_SORTATTRIBUTE, OnDoubleClick)
END_MESSAGE_MAP()

BOOL SortOptionsDlg::OnInitDialog()
{
	LFAttributeListDlg::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_SORTOPTIONS);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Titelleiste
	CString text;
	GetWindowText(text);
	CString caption;
	caption.Format(text, theApp.m_Contexts[m_Context]->Name);
	SetWindowText(caption);

	// Attribut-Liste füllen
	PopulateListCtrl(IDC_SORTATTRIBUTE, FALSE, p_View->SortBy);

	// Ggf. Elemente deaktivieren
	if (p_View->Mode>LFViewPreview)
	{
		GetDlgItem(IDC_ASCENDING)->EnableWindow(FALSE);
		GetDlgItem(IDC_DESCENDING)->EnableWindow(FALSE);
	}

	if ((p_View->Mode>LFViewPreview) || (!theApp.m_Contexts[m_Context]->AllowGroups))
	{
		GetDlgItem(IDC_AUTODIRS)->EnableWindow(FALSE);

		CString tmpStr;
		ENSURE(tmpStr.LoadString(p_View->AutoDirs ? IDS_SUBFOLDERS_MANDATORY : IDS_SUBFOLDERS_NOTAVAIL));
		GetDlgItem(IDC_SUBFOLDERHINT)->SetWindowText(tmpStr);
	}

	// Gruppenbox
	OnSetAttrGroupBox();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void SortOptionsDlg::OnSetAttrGroupBox()
{
	CString tmpStr;
	ENSURE(tmpStr.LoadString(IsDlgButtonChecked(IDC_AUTODIRS) ? IDS_GROUPBOX_GROUP : IDS_GROUPBOX_SORT));
	GetDlgItem(IDC_ATTRBOX)->SetWindowText(tmpStr);
}

void SortOptionsDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	PostMessage(WM_COMMAND, (WPARAM)IDOK);
}
