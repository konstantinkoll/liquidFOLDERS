
// LFStoreNewDriveDlg.h: Schnittstelle der Klasse LFStoreNewDrive
//

#pragma once
#include "liquidFOLDERS.h"
#include "CExplorerTree.h"
#include "CIconCtrl.h"


// LFStoreNewDriveDlg
//

class AFX_EXT_CLASS LFStoreNewDriveDlg : public CDialog
{
public:
	LFStoreNewDriveDlg(CWnd* pParentWnd, char Drive, LFStoreDescriptor* pStore);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	CIconCtrl m_IconHybrid;
	CIconCtrl m_IconExternal;
	CExplorerTree m_PathTree;
	char m_Drive;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnShellChange(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	LFStoreDescriptor* m_pStore;
	ULONG m_ulSHChangeNotifyRegister;
};
