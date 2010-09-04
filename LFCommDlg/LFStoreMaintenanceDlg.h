
// LFStoreMaintenanceDlg.h: Schnittstelle der Klasse LFStoreMaintenanceDlg
//

#pragma once
#include "liquidFOLDERS.h"


// LFStoreMaintenanceDlg
//

struct LFMaintenanceDlgParameters
{
	UINT Repaired;
	UINT NoAccess;
	UINT NoFreeSpace;
	UINT RepairError;
};

class AFX_EXT_CLASS LFStoreMaintenanceDlg : public CDialog
{
public:
	LFStoreMaintenanceDlg(LFMaintenanceDlgParameters* pParameters, CWnd* pParentWnd=NULL);

protected:
	LFMaintenanceDlgParameters* parameters;

	void SetNumber(UINT ID, UINT Number);

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
