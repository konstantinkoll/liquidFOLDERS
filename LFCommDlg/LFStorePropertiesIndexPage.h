
// LFStorePropertiesIndexPage.h: Schnittstelle der Klasse LFStorePropertiesIndexPage
//

#pragma once
#include "LF.h"


// LFStorePropertiesIndexPage
//

class LFStorePropertiesIndexPage : public CPropertyPage
{
public:
	LFStorePropertiesIndexPage(LFStoreDescriptor* pStore, BOOL* pStoreValid);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg LRESULT OnUpdateStore(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	LFStoreDescriptor* p_Store;
	BOOL* p_StoreValid;
};
