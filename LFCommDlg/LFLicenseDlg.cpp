
// LFLicenseDlg.cpp: Implementierung der Klasse LFLicenseDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFLicenseDlg
//

LFLicenseDlg::LFLicenseDlg(CWnd* pParentWnd)
	: LFDialog(IDD_LICENSE, pParentWnd)
{
}

void LFLicenseDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	if (pDX->m_bSaveAndValidate)
	{
		CString Key;
		GetDlgItem(IDC_LICENSEKEY)->GetWindowText(Key);

		LFGetApp()->WriteGlobalString(_T("License"), Key);

		CString Caption;
		CString Message;
		if (LFIsLicensed(NULL, TRUE))
		{
			::PostMessage(HWND_BROADCAST, LFGetApp()->m_LicenseActivatedMsg, NULL, NULL);

			ENSURE(Caption.LoadString(IDS_LICENSEVALID_CAPTION));
			ENSURE(Message.LoadString(IDS_LICENSEVALID_MSG));
			LFMessageBox(this, Message, Caption, MB_ICONINFORMATION);
		}
		else
		{
			ENSURE(Caption.LoadString(IDS_ERROR));
			ENSURE(Message.LoadString(IDS_INVALIDLICENSE));
			LFMessageBox(this, Message, Caption, MB_ICONWARNING);

			pDX->Fail();
		}
	}
}

BOOL LFLicenseDlg::InitDialog()
{
	LFDialog::InitDialog();

	GetDlgItem(IDC_INSTRUCTIONS)->SetFont(&LFGetApp()->m_DefaultFont);

	return TRUE;
}


BEGIN_MESSAGE_MAP(LFLicenseDlg, LFDialog)
	ON_BN_CLICKED(IDC_LOADLICENSE, OnLoadLicense)
	ON_EN_CHANGE(IDC_LICENSEKEY, OnChange)
END_MESSAGE_MAP()

void LFLicenseDlg::OnLoadLicense()
{
	CString tmpStr((LPCSTR)IDS_LICFILEFILTER);
	tmpStr += _T(" (*.lic)|*.lic||");

	CFileDialog dlg(TRUE, _T(".lic"), NULL, OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, tmpStr, this);
	if (dlg.DoModal()==IDOK)
	{
		CString key;

		CStdioFile f;
		if (!f.Open(dlg.GetPathName(), CFile::modeRead | CFile::shareDenyWrite))
		{
			LFErrorBox(this, LFDriveNotReady);
		}
		else
		{
			try
			{
				CString line;

				UINT lines = 0;
				while ((f.ReadString(line)) && (lines++<128))
					key.Append(line+_T("\r\n"));
			}
			catch(CFileException ex)
			{
				LFErrorBox(this, LFDriveNotReady);
			}

			f.Close();

			GetDlgItem(IDC_LICENSEKEY)->SetWindowText(key);
			GetDlgItem(IDOK)->EnableWindow(TRUE);
			GetDlgItem(IDOK)->SetFocus();
		}
	}
}

void LFLicenseDlg::OnChange()
{
	CString Key;
	GetDlgItem(IDC_LICENSEKEY)->GetWindowText(Key);

	GetDlgItem(IDOK)->EnableWindow(!Key.IsEmpty());
}
