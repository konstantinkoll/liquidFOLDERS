
// LFStoreNewDriveDlg.h: Schnittstelle der Klasse LFStoreNewDrive
//

#pragma once
#include "liquidFOLDERS.h"
#include "CIconCtrl.h"


// LFStoreNewDriveDlg
//

class AFX_EXT_CLASS LFStoreNewDriveDlg : public CDialog
{
public:
	LFStoreNewDriveDlg(CWnd* pParentWnd, char Drive, LFStoreDescriptor* _store);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnMediaChanged(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	LFStoreDescriptor* store;

	CIconCtrl m_IconHybrid;
	CIconCtrl m_IconExternal;
	ULONG m_ulSHChangeNotifyRegister;
	char m_Drive;
};
