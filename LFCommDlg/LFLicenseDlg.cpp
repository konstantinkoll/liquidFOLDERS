
// LFLicenseDlg.cpp: Implementierung der Klasse LFLicenseDlg
//

#include "stdafx.h"
#include "LFLicenseDlg.h"
#include "LFCore.h"
#include "Resource.h"


// LFLicenseDlg
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

LFLicenseDlg::LFLicenseDlg(CWnd* pParent)
	: LFDialog(IDD_ENTERLICENSEKEY, LFDS_Default, pParent)
{
}

void LFLicenseDlg::DoDataExchange(CDataExchange* pDX)
{
	LFDialog::DoDataExchange(pDX);

	if (pDX->m_bSaveAndValidate)
	{
		CString key;
		GetDlgItem(IDC_LICENSEKEY)->GetWindowText(key);

		p_App->WriteGlobalString(_T("License"), key);

		CString caption;
		CString message;
		if (LFIsLicensed(NULL, true))
		{
			ENSURE(caption.LoadString(IDS_LICENSEVALID_CAPTION));
			ENSURE(message.LoadString(IDS_LICENSEVALID_MSG));
			MessageBox(message, caption, MB_ICONINFORMATION);
		}
		else
		{
			ENSURE(caption.LoadString(IDS_ERROR));
			ENSURE(message.LoadString(IDS_INVALIDLICENSE));
			MessageBox(message, caption, MB_ICONWARNING);

			pDX->Fail();
		}
	}
}

void LFLicenseDlg::OnLoadLicense()
{
	CString tmpStr;
	ENSURE(tmpStr.LoadString(IDS_LICFILEFILTER));
	tmpStr += _T(" (*.lic)|*.lic||");

	CFileDialog dlg(TRUE, _T(".lic"), NULL, OFN_DONTADDTORECENT | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST, tmpStr, this);
	if (dlg.DoModal()==IDOK)
	{
		CString key;

		CStdioFile f;
		if (!f.Open(dlg.GetPathName(), CFile::modeRead | CFile::shareDenyWrite))
		{
			LFErrorBox(LFDriveNotReady);
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
				LFErrorBox(LFDriveNotReady);
			}

			f.Close();

			GetDlgItem(IDC_LICENSEKEY)->SetWindowText(key);
			GetDlgItem(IDOK)->SetFocus();
		}
	}
}


BEGIN_MESSAGE_MAP(LFLicenseDlg, LFDialog)
	ON_BN_CLICKED(IDC_LOADLICENSE, OnLoadLicense)
END_MESSAGE_MAP()

BOOL LFLicenseDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	m_Icon.Load(IDB_KEY, _T("PNG"), LFCommDlgDLL.hResource);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFLicenseDlg::OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect)
{
	LFDialog::OnEraseBkgnd(dc, g, rect);

	INT l = m_Icon.m_pBitmap->GetWidth();
	INT h = m_Icon.m_pBitmap->GetHeight();
	g.DrawImage(m_Icon.m_pBitmap, 16, 16, l, h);
}
