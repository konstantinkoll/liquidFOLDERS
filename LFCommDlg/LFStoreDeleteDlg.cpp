#include "StdAfx.h"
#include "LFStoreDeleteDlg.h"
#include "Resource.h"
#include <mmsystem.h>


// LFStoreDeleteDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFStoreDeleteDlg::LFStoreDeleteDlg(CWnd* pParentWnd, wchar_t* _StoreName)
	: CDialog(IDD_STOREDELETE, pParentWnd)
{
	StoreName = _StoreName;
}

LFStoreDeleteDlg::~LFStoreDeleteDlg()
{
}

BEGIN_MESSAGE_MAP(LFStoreDeleteDlg, CDialog)
	ON_BN_CLICKED(IDC_KEEP, SetOkButton)
	ON_BN_CLICKED(IDC_DELETE, SetOkButton)
END_MESSAGE_MAP()

BOOL LFStoreDeleteDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = LoadIcon(LFCommDlgDLL.hResource, MAKEINTRESOURCE(IDD_STOREDELETE));
	SetIcon(hIcon, FALSE);
	SetIcon(hIcon, TRUE);

	// Titelleiste
	CString text;
	GetWindowText(text);
	CString caption;
	caption.Format(text, StoreName);
	SetWindowText(caption);

	// Radiobutton setzen
	((CButton*)GetDlgItem(IDC_KEEP))->SetCheck(TRUE);

	// Sound
	PlaySoundA("SystemExclamation", NULL, SND_ALIAS | SND_ASYNC | SND_NOWAIT);

	return TRUE;
}

void LFStoreDeleteDlg::SetOkButton()
{
	GetDlgItem(IDOK)->EnableWindow(GetCheckedRadioButton(IDC_KEEP, IDC_DELETE)==IDC_DELETE);
}
