
// LFStoreDeleteDlg.cpp: Implementierung der Klasse LFStoreDeleteDlg
//

#include "StdAfx.h"
#include "LFStoreDeleteDlg.h"
#include "Resource.h"


// LFStoreDeleteDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFStoreDeleteDlg::LFStoreDeleteDlg(CWnd* pParentWnd, WCHAR* _StoreName)
	: LFDialog(IDD_STOREDELETE, pParentWnd, LFDS_UAC)
{
	StoreName = _StoreName;
}


BEGIN_MESSAGE_MAP(LFStoreDeleteDlg, LFDialog)
	ON_BN_CLICKED(IDC_KEEP, SetOkButton)
	ON_BN_CLICKED(IDC_DELETE, SetOkButton)
END_MESSAGE_MAP()

BOOL LFStoreDeleteDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	// Titelleiste
	CString text;
	GetWindowText(text);
	CString caption;
	caption.Format(text, StoreName);
	SetWindowText(caption);

	// Radiobutton setzen
	((CButton*)GetDlgItem(IDC_KEEP))->SetCheck(TRUE);

	// Fette Überschrift
	LOGFONT lf;
	if (GetFont()->GetLogFont(&lf))
	{
		lf.lfWeight = FW_BOLD;
		BoldFont.CreateFontIndirect(&lf);
		GetDlgItem(IDC_CAPTION)->SetFont(&BoldFont, false);
	}

	return TRUE;
}

void LFStoreDeleteDlg::SetOkButton()
{
	GetDlgItem(IDOK)->EnableWindow(GetCheckedRadioButton(IDC_KEEP, IDC_DELETE)==IDC_DELETE);
}
