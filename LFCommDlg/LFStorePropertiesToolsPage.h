
// LFStorePropertiesToolsPage.h: Schnittstelle der Klasse LFStorePropertiesToolsPage
//

#pragma once
#include "LFCommDlg.h"


// LFStorePropertiesToolsPage
//

class LFStorePropertiesToolsPage : public CPropertyPage
{
public:
	LFStorePropertiesToolsPage(LFStoreDescriptor* pStore, BOOL* pStoreValid);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	LFStoreDescriptor* p_Store;
	BOOL* p_StoreValid;
	CIconCtrl m_wndIconMaintenance;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnRunMaintenance();
	afx_msg void OnRunSynchronize();
	afx_msg void OnRunBackup();
	afx_msg LRESULT OnUpdateStore(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	CString m_MaskMaintenance;
	CString m_MaskSynchronized;
};
