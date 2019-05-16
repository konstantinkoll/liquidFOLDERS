
// LFDeleteStoreDlg.cpp: Implementierung der Klasse LFDeleteStoreDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFDeleteStoreDlg.h"


// LFDeleteStoreDlg
//

LFDeleteStoreDlg::LFDeleteStoreDlg(const ABSOLUTESTOREID& StoreID, CWnd* pParentWnd)
	: LFDialog(IDD_DELETESTORE, pParentWnd, TRUE, TRUE)
{
	m_StoreID = StoreID;
}

BOOL LFDeleteStoreDlg::InitDialog()
{
	// Store prüfen
	OnStoresChanged(NULL, NULL);

	// Radiobutton setzen
	((CButton*)GetDlgItem(IDC_KEEP))->SetCheck(TRUE);

	// Fette Überschrift
	GetDlgItem(IDC_CAPTION)->SetFont(&LFGetApp()->m_DialogBoldFont);

	// Nuke messages
	const BOOL Nuke = (m_StoreDescriptor.Mode & LFStoreModeBackendMask)<=LFStoreModeBackendInternal;

	GetDlgItem(IDC_NUKEMESSAGE1)->ShowWindow(Nuke ? SW_SHOW : SW_HIDE);
	GetDlgItem(IDC_NUKEMESSAGE2)->ShowWindow(Nuke ? SW_HIDE : SW_SHOW);

	// Titelleiste
	CString Mask;
	GetWindowText(Mask);

	CString Caption;
	Caption.Format(Mask, m_StoreDescriptor.StoreName);
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
	if (LFGetStoreSettings(m_StoreID, m_StoreDescriptor)!=LFOk)
	{
		// Prevent desktop dimming and sound
		m_UAC = FALSE;

		OnCancel();
	}

	return NULL;
}
