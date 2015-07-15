
// LFStoreMaintenanceDlg.h: Schnittstelle der Klasse LFStoreMaintenanceDlg
//

#pragma once
#include "LFCore.h"
#include "CMaintenanceReport.h"


// LFStoreMaintenanceDlg
//

class LFStoreMaintenanceDlg : public LFDialog
{
public:
	LFStoreMaintenanceDlg(LFMaintenanceList* pMaintenanceList, CWnd* pParentWnd=NULL);
	~LFStoreMaintenanceDlg();

	virtual void AdjustLayout();

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	DECLARE_MESSAGE_MAP()

	CHeaderArea m_wndHeaderArea;
	CMaintenanceReport m_wndMaintenanceReport;
	LFMaintenanceList* m_pMaintenanceList;
};
