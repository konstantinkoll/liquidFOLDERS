
// LFStoreMaintenanceDlg.h: Schnittstelle der Klasse LFStoreMaintenanceDlg
//

#pragma once
#include "liquidFOLDERS.h"


// LFStoreMaintenanceDlg
//

typedef DynArray<LFML_Item*> CMaintenanceReportList;

class AFX_EXT_CLASS LFStoreMaintenanceDlg : public CDialog
{
public:
	LFStoreMaintenanceDlg(LFMaintenanceList* ml, CWnd* pParentWnd=NULL);

protected:
	CMaintenanceReportList m_Lists[2];
	int m_Page;

	void SetPage(int page);

	afx_msg BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnTabChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

private:
	CImageList m_Icons;
};
