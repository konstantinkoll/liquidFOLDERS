
// LFDeleteStoreDlg.cpp: Implementierung der Klasse LFDeleteStoreDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFDeleteStoreDlg.h"


// LFDeleteStoreDlg
//

LFDeleteStoreDlg::LFDeleteStoreDlg(CHAR* StoreID, CWnd* pParentWnd)
	: LFDialog(IDD_DELETESTORE, pParentWnd, TRUE)
{
	ASSERT(StoreID);

	strcpy_s(m_StoreID, LFKeySize, StoreID);
}


BEGIN_MESSAGE_MAP(LFDeleteStoreDlg, LFDialog)
	ON_BN_CLICKED(IDC_KEEP, SetOkButton)
	ON_BN_CLICKED(IDC_DELETE, SetOkButton)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoresChanged, OnStoresChanged)
END_MESSAGE_MAP()

BOOL LFDeleteStoreDlg::OnInitDialog()
{
	// Store prüfen
	SendMessage(LFGetApp()->p_MessageIDs->StoresChanged);

	LFDialog::OnInitDialog();

	// Radiobutton setzen
	((CButton*)GetDlgItem(IDC_KEEP))->SetCheck(TRUE);

	// Fette Überschrift
	GetDlgItem(IDC_CAPTION)->SetFont(&LFGetApp()->m_BoldFont);

	// Titelleiste
	CString text;
	GetWindowText(text);

	CString Caption;
	Caption.Format(text, m_Store.StoreName);
	SetWindowText(Caption);

	return TRUE;
}

void LFDeleteStoreDlg::SetOkButton()
{
	GetDlgItem(IDOK)->EnableWindow(GetCheckedRadioButton(IDC_KEEP, IDC_DELETE)==IDC_DELETE);
}

LRESULT LFDeleteStoreDlg::OnStoresChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (LFGetStoreSettings(m_StoreID, &m_Store)!=LFOk)
	{
		m_UAC = FALSE;
		EndDialog(IDCANCEL);
	}

	return NULL;
}
