
// ReportDlg.cpp: Implementierung der Klasse ReportDlg
//

#include "stdafx.h"
#include "ReportDlg.h"
#include "resource.h"


// ReportDlg
//

ReportDlg::ReportDlg(CWnd* pParent, ReportList* Successful, ReportList* WithErrors)
	: CDialog(IDD_REPORT, pParent)
{
	ASSERT(Successful);
	ASSERT(WithErrors);

	m_Lists[0] = Successful;
	m_Lists[1] = WithErrors;
}


BEGIN_MESSAGE_MAP(ReportDlg, CDialog)
END_MESSAGE_MAP()

BOOL ReportDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Symbol für dieses Dialogfeld festlegen. Wird automatisch erledigt
	// wenn das Hauptfenster der Anwendung kein Dialogfeld ist
	HICON hIcon = theApp.LoadIcon(IDD_REPORT);
	SetIcon(hIcon, TRUE);		// Großes Symbol verwenden
	SetIcon(hIcon, FALSE);		// Kleines Symbol verwenden

	// Tabs setzen
	CTabCtrl* tabs = (CTabCtrl*)GetDlgItem(IDC_TABS);

	m_Icons.Create(IDB_WARNING);
	tabs->SetImageList(&m_Icons);

	for (UINT a=0; a<2; a++)
	{
		CString mask;
		ENSURE(mask.LoadString(IDS_REPORTTAB0+a));

		CString tmpStr;
		tmpStr.Format(mask, m_Lists[a]->m_ItemCount);

		tabs->InsertItem(a, tmpStr, (int)a-1);
	}

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}
