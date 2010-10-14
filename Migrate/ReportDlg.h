
// ReportDlg.h: Schnittstelle der Klasse ReportDlg
//

#pragma once
#include "LFCommDlg.h"
#include "Migrate.h"
#include "CMigrationList.h"


// ReportDlg
//

class ReportDlg : public CDialog
{
public:
	ReportDlg(CWnd* pParent, CReportList* Successful, CReportList* WithErrors);

	virtual void DoDataExchange(CDataExchange* pDX);

	BOOL m_UncheckMigrated;

protected:
	CReportList* m_Lists[2];
	int m_Page;

	void SetPage(int page);

	afx_msg BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTabChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

private:
	CImageListTransparent m_Icons;
};
