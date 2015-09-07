
// LFGotoYearDlg.cpp: Implementierung der Klasse LFGotoYearDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFGotoYearDlg
//

LFGotoYearDlg::LFGotoYearDlg(UINT Year, CWnd* pParentWnd)
	: LFDialog(IDD_GOTOYEAR, pParentWnd)
{
	m_Year = Year;
}

void LFGotoYearDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_YEAR, m_wndEdit);

	if (pDX->m_bSaveAndValidate)
	{
		CString tmpStr;
		m_wndEdit.GetWindowText(tmpStr);

		BOOL Ok = FALSE;
		UINT Year;
		if (swscanf_s(tmpStr, _T("%u"), &Year)==1)
			if ((Year>=1900) && (Year<=2100))
			{
				m_Year = Year;
				Ok = TRUE;
			}

		if (!Ok)
		{
			CString Caption;
			GetWindowText(Caption);

			CString Message((LPCSTR)IDS_ILLEGALYEAR);

			LFMessageBox(this, Message, Caption, MB_ICONERROR);

			pDX->Fail();
		}
	}
}


BEGIN_MESSAGE_MAP(LFGotoYearDlg, LFDialog)
END_MESSAGE_MAP()

BOOL LFGotoYearDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	// Eingabezeile
	CString tmpStr;
	tmpStr.Format(_T("%u"), m_Year);
	m_wndEdit.SetWindowText(tmpStr);
	m_wndEdit.SetLimitText(4);
	m_wndEdit.SetValidChars(_T("0123456789"));
	m_wndEdit.SetSel(0, 3);

	return FALSE;
}
