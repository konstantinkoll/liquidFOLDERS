
// LFStorePropertiesIndexPage.h: Schnittstelle der Klasse LFStorePropertiesIndexPage
//

#pragma once
#include "LFCommDlg.h"


// LFStorePropertiesIndexPage
//

class LFStorePropertiesIndexPage : public CPropertyPage
{
public:
	LFStorePropertiesIndexPage(LFStoreDescriptor* pStore, BOOL* pStoreValid);

protected:
	LFStoreDescriptor* p_Store;
	BOOL* p_StoreValid;

	afx_msg BOOL OnInitDialog();
	afx_msg LRESULT OnUpdateStore(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
