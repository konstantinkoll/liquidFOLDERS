
// LFStorePropertiesToolsPage.h: Schnittstelle der Klasse LFStorePropertiesToolsPage
//

#pragma once
#include "LFCommDlg.h"


// LFStorePropertiesToolsPage
//

class LFStorePropertiesToolsPage : public CPropertyPage
{
public:
	LFStorePropertiesToolsPage(LFStoreDescriptor* pStore);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	LFStoreDescriptor* p_Store;
	CIconCtrl m_IconMaintenance;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnRunBackup();
	DECLARE_MESSAGE_MAP()

private:
	CString m_Mask;
};
