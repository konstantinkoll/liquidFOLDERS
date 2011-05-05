
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
	DDX_Control(pDX, IDC_COMPAREATTRIBUTE, m_wndAttribute);
	DDX_Control(pDX, IDC_PROPERTY, m_wndEdit);

	if (pDX->m_bSaveAndValidate)
	{
//		p_View->SortBy = (UINT)pList->GetItemData(pList->GetNextItem(-1, LVNI_SELECTED));
	}
}

void EditConditionDlg::TestAttribute(UINT attr, BOOL& add, BOOL& check)
{
	add = (attr!=LFAttrFileID) && (attr!=LFAttrStoreID);
	check = FALSE;
}


BEGIN_MESSAGE_MAP(EditConditionDlg, LFAttributeListDlg)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_COMPAREATTRIBUTE, OnItemChanged)
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

	// Property
	

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void EditConditionDlg::OnItemChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	NM_LISTVIEW* pNMListView = (NM_LISTVIEW*)pNMHDR;

	if ((pNMListView->uChanged & LVIF_STATE) && ((pNMListView->uOldState & LVIS_SELECTED) || (pNMListView->uNewState & LVIS_SELECTED)))
		m_wndEdit.SetAttribute((UINT)m_wndAttribute.GetItemData(m_wndAttribute.GetNextItem(-1, LVNI_SELECTED)));
}
