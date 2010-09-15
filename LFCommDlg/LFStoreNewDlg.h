
// LFStoreNewDlg.h: Schnittstelle der Klasse LFStoreNew
//

#pragma once
#include "liquidFOLDERS.h"
#include "CExplorerTree.h"
#include "CIconCtrl.h"


// LFStoreNewDlg
//

class AFX_EXT_CLASS LFStoreNewDlg : public CDialog
{
public:
	LFStoreNewDlg(CWnd* pParentWnd, LFStoreDescriptor* pStore);

	virtual void DoDataExchange(CDataExchange* pDX);

	bool MakeDefault;

protected:
	void SetOkButton();
	void PopulateTreeCtrl();

	CIconCtrl m_IconInternal;
	CIconCtrl m_IconHybrid;
	CIconCtrl m_IconExternal;
	CExplorerTree m_PathTree;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnSetInternalIcon();
	afx_msg void OnSetOptions();
	afx_msg void OnSelChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnShellChange(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	LFStoreDescriptor* m_pStore;
};
