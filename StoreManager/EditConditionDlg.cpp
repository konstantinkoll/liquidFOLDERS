
// EditConditionDlg.cpp: Implementierung der Klasse EditConditionDlg
//

#include "stdafx.h"
#include "EditConditionDlg.h"


// EditConditionDlg
//

EditConditionDlg::EditConditionDlg(CWnd* pParent)
	: LFAttributeListDlg(IDD_EDITCONDITION, pParent)
{
}

void EditConditionDlg::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
	{
		CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_COMPAREATTRIBUTE);
//		p_View->SortBy = (UINT)pList->GetItemData(pList->GetNextItem(-1, LVNI_SELECTED));
	}
}

void EditConditionDlg::TestAttribute(UINT attr, BOOL& add, BOOL& check)
{
	add = (attr!=LFAttrFileID) && (attr!=LFAttrStoreID);
	check = FALSE;
}


BEGIN_MESSAGE_MAP(EditConditionDlg, LFAttributeListDlg)
	ON_NOTIFY(NM_DBLCLK, IDC_COMPAREATTRIBUTE, OnDoubleClick)
END_MESSAGE_MAP()

BOOL EditConditionDlg::OnInitDialog()
{
	LFAttributeListDlg::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_EDITCONDITION);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Attribut-Liste füllen
	PopulateListCtrl(IDC_COMPAREATTRIBUTE, FALSE, 0);	// TODO

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void EditConditionDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	PostMessage(WM_COMMAND, (WPARAM)IDOK);
}
