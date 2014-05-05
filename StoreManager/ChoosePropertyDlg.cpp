
// ChoosePropertyDlg.cpp: Implementierung der Klasse ChoosePropertyDlg
//

#include "stdafx.h"
#include "ChoosePropertyDlg.h"
#include "resource.h"


// ChoosePropertyDlg
//

ChoosePropertyDlg::ChoosePropertyDlg(CWnd* pParentWnd, INT Attr)
	: LFAttributeListDlg(IDD_CHOOSEPROPERTY, pParentWnd)
{
	m_Attr = Attr;
}

void ChoosePropertyDlg::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
	{
		CListCtrl* pList = (CListCtrl*)GetDlgItem(IDC_ATTRIBUTES);
		m_Attr = (UINT)pList->GetItemData(pList->GetNextItem(-1, LVNI_SELECTED));
	}
}

void ChoosePropertyDlg::TestAttribute(UINT attr, BOOL& add, BOOL& check)
{
	add = (!theApp.m_Attributes[attr].ReadOnly) && (attr!=LFAttrFileName) && (theApp.m_Attributes[attr].Type!=LFTypeGeoCoordinates);
	check = FALSE;
}


BEGIN_MESSAGE_MAP(ChoosePropertyDlg, LFAttributeListDlg)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_NOTIFY(NM_DBLCLK, IDC_ATTRIBUTES, OnDoubleClick)
END_MESSAGE_MAP()

BOOL ChoosePropertyDlg::OnInitDialog()
{
	LFAttributeListDlg::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_CHOOSEPROPERTY);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Attribut-Liste füllen
	PopulateListCtrl(IDC_ATTRIBUTES, FALSE, m_Attr);

	// "Reset" deaktivieren
	if (m_Attr==-1)
		GetDlgItem(IDC_RESET)->EnableWindow(FALSE);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void ChoosePropertyDlg::OnReset()
{
	m_Attr = -1;
	EndDialog(IDC_RESET);
}

void ChoosePropertyDlg::OnDoubleClick(NMHDR* /*pNMHDR*/, LRESULT* /*pResult*/)
{
	PostMessage(WM_COMMAND, (WPARAM)IDOK);
}
