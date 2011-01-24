
// GoToYearDlg.cpp: Implementierung der Klasse GoToYearDlg
//

#include "stdafx.h"
#include "GoToYearDlg.h"


// GoToYearDlg
//

GoToYearDlg::GoToYearDlg(CWnd* pParentWnd, UINT Year)
	: CDialog(IDD_GOTOYEAR, pParentWnd)
{
	m_Year = Year;
}

void GoToYearDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_YEAR, m_wndEdit);

	if (pDX->m_bSaveAndValidate)
	{
		CString tmpStr;
		m_wndEdit.GetWindowText(tmpStr);

		BOOL Ok = FALSE;
		UINT Year;
		if (swscanf_s(tmpStr, _T("%d"), &Year)==1)
			if ((Year>=1900) && (Year<=2100))
			{
				m_Year = Year;
				Ok = TRUE;
			}

		if (!Ok)
		{
			CString tmpCaption;
			CString tmpMessage;
			GetWindowText(tmpCaption);
			ENSURE(tmpMessage.LoadString(IDS_ILLEGALYEAR));

			MessageBox(tmpMessage, tmpCaption, MB_ICONERROR);
			pDX->Fail();
		}
	}
}


BEGIN_MESSAGE_MAP(GoToYearDlg, CDialog)
END_MESSAGE_MAP()

BOOL GoToYearDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_GOTOYEAR);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Eingabezeile
	CString tmpStr;
	tmpStr.Format(_T("%d"), m_Year);
	m_wndEdit.SetWindowText(tmpStr);
	m_wndEdit.SetLimitText(4);
	m_wndEdit.SetValidChars(_T("0123456789"));
	m_wndEdit.SetSel(0, 3);

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
