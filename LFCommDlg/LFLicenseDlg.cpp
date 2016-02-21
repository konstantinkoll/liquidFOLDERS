
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
		CString LicenseKey;
		GetDlgItem(IDC_LICENSEKEY)->GetWindowText(LicenseKey);

		LFGetApp()->WriteGlobalString(_T("License"), LicenseKey);

		if (LFIsLicensed(NULL, TRUE))
		{
			::PostMessage(HWND_BROADCAST, LFGetApp()->m_LicenseActivatedMsg, NULL, NULL);

			CString Caption((LPCSTR)IDS_LICENSEVALID_CAPTION);
			CString Message((LPCSTR)IDS_LICENSEVALID_MSG);
			LFMessageBox(this, Message, Caption, MB_ICONINFORMATION | MB_OK);
		}
		else
		{
			CString Caption((LPCSTR)IDS_ERROR);
			CString Message((LPCSTR)IDS_INVALIDLICENSE);
			LFMessageBox(this, Message, Caption, MB_ICONWARNING | MB_OK);

			pDX->Fail();
		}
	}
}

BOOL LFLicenseDlg::InitDialog()
{
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
		CString LicenseKey;

		CStdioFile f;
		if (!f.Open(dlg.GetPathName(), CFile::modeRead | CFile::shareDenyWrite))
		{
			LFErrorBox(this, LFDriveNotReady);
		}
		else
		{
			try
			{
				CString Line;

				UINT cLines = 0;
				while ((f.ReadString(Line)) && (cLines++<128))
					LicenseKey.Append(Line+_T("\r\n"));
			}
			catch(CFileException ex)
			{
				LFErrorBox(this, LFDriveNotReady);
			}

			f.Close();

			GetDlgItem(IDC_LICENSEKEY)->SetWindowText(LicenseKey);
			GetDlgItem(IDOK)->EnableWindow(TRUE);
			GetDlgItem(IDOK)->SetFocus();
		}
	}
}

void LFLicenseDlg::OnChange()
{
	CString LicenseKey;
	GetDlgItem(IDC_LICENSEKEY)->GetWindowText(LicenseKey);

	GetDlgItem(IDOK)->EnableWindow(!LicenseKey.IsEmpty());
}
