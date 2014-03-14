
// DeleteFilesDlg.cpp: Implementierung der Klasse DeleteFilesDlg
//

#include "StdAfx.h"
#include "DeleteFilesDlg.h"
#include "Resource.h"


// DeleteFilesDlg
//

DeleteFilesDlg::DeleteFilesDlg(CWnd* pParentWnd)
	: LFDialog(IDD_DELETEFILES, pParentWnd, LFDS_UAC)
{
	m_Delete = TRUE;
}

void DeleteFilesDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Check(pDX, IDC_DELETESOURCE, m_Delete);
}


BEGIN_MESSAGE_MAP(DeleteFilesDlg, LFDialog)
END_MESSAGE_MAP()

BOOL DeleteFilesDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	// Checkbox setzen
	((CButton*)GetDlgItem(IDC_DELETESOURCE))->SetCheck(TRUE);

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
