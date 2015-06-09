
// LFStoreDeleteDlg.cpp: Implementierung der Klasse LFStoreDeleteDlg
//

#include "StdAfx.h"
#include "LFStoreDeleteDlg.h"
#include "Resource.h"


// LFStoreDeleteDlg
//

LFStoreDeleteDlg::LFStoreDeleteDlg(CHAR* StoreID, CWnd* pParentWnd)
	: LFDialog(IDD_STOREDELETE, pParentWnd, TRUE)
{
	ASSERT(StoreID);

	strcpy_s(m_Key, LFKeySize, StoreID);
}


BEGIN_MESSAGE_MAP(LFStoreDeleteDlg, LFDialog)
	ON_BN_CLICKED(IDC_KEEP, SetOkButton)
	ON_BN_CLICKED(IDC_DELETE, SetOkButton)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoresChanged, OnStoresChanged)
END_MESSAGE_MAP()

BOOL LFStoreDeleteDlg::OnInitDialog()
{
	// Store prüfen
	SendMessage(p_App->p_MessageIDs->StoresChanged);

	LFDialog::OnInitDialog();

	// Radiobutton setzen
	((CButton*)GetDlgItem(IDC_KEEP))->SetCheck(TRUE);

	// Fette Überschrift
	GetDlgItem(IDC_CAPTION)->SetFont(&p_App->m_BoldFont);

	// Titelleiste
	CString text;
	GetWindowText(text);

	CString caption;
	caption.Format(text, m_Store.StoreName);
	SetWindowText(caption);

	return TRUE;
}

void LFStoreDeleteDlg::SetOkButton()
{
	GetDlgItem(IDOK)->EnableWindow(GetCheckedRadioButton(IDC_KEEP, IDC_DELETE)==IDC_DELETE);
}

LRESULT LFStoreDeleteDlg::OnStoresChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (LFGetStoreSettings(m_Key, &m_Store)!=LFOk)
	{
		m_UAC = FALSE;
		EndDialog(IDCANCEL);
	}

	return NULL;
}
