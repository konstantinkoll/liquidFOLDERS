
// LFStoreNewVolumeDlg.h: Schnittstelle der Klasse LFStoreNewVolume
//

#pragma once
#include "liquidFOLDERS.h"
#include "CExplorerTree.h"
#include "CIconCtrl.h"


// LFStoreNewVolumeDlg
//

class AFX_EXT_CLASS LFStoreNewVolumeDlg : public CDialog
{
public:
	LFStoreNewVolumeDlg(CWnd* pParentWnd, CHAR Drive, LFStoreDescriptor* pStore);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	CIconCtrl m_IconHybrid;
	CIconCtrl m_IconExternal;
	CExplorerTree m_PathTree;
	CHAR m_Drive;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnShellChange(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	LFStoreDescriptor* m_pStore;
	ULONG m_ulSHChangeNotifyRegister;
};
