
// LFDeleteStoreDlg.cpp: Implementierung der Klasse LFDeleteStoreDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFDeleteStoreDlg.h"


// LFDeleteStoreDlg
//

LFDeleteStoreDlg::LFDeleteStoreDlg(const LPCSTR pStoreID, CWnd* pParentWnd)
	: LFDialog(IDD_DELETESTORE, pParentWnd, TRUE, TRUE)
{
	ASSERT(pStoreID);

	strcpy_s(m_StoreID, LFKeySize, pStoreID);
}

BOOL LFDeleteStoreDlg::InitDialog()
{
	// Store prüfen
	OnStoresChanged(NULL, NULL);

	// Radiobutton setzen
	((CButton*)GetDlgItem(IDC_KEEP))->SetCheck(TRUE);

	// Fette Überschrift
	GetDlgItem(IDC_CAPTION)->SetFont(&LFGetApp()->m_DialogBoldFont);

	// Nuke
	if ((m_Store.Mode & LFStoreModeBackendMask)>LFStoreModeBackendInternal)
		GetDlgItem(IDC_NUKEMESSAGE)->ShowWindow(SW_HIDE);

	// Titelleiste
	CString Mask;
	GetWindowText(Mask);

	CString Caption;
	Caption.Format(Mask, m_Store.StoreName);
	SetWindowText(Caption);

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFDeleteStoreDlg, LFDialog)
	ON_BN_CLICKED(IDC_KEEP, OnUpdateOkButton)
	ON_BN_CLICKED(IDC_DELETE, OnUpdateOkButton)
	ON_REGISTERED_MESSAGE(LFGetApp()->p_MessageIDs->StoresChanged, OnStoresChanged)
END_MESSAGE_MAP()

void LFDeleteStoreDlg::OnUpdateOkButton()
{
	GetDlgItem(IDOK)->EnableWindow(GetCheckedRadioButton(IDC_KEEP, IDC_DELETE)==IDC_DELETE);
}

LRESULT LFDeleteStoreDlg::OnStoresChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (LFGetStoreSettings(m_StoreID, m_Store)!=LFOk)
	{
		// Prevent desktop dimming and sound
		m_UAC = FALSE;

		OnCancel();
	}

	return NULL;
}
