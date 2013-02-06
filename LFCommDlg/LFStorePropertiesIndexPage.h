
// LFStorePropertiesIndexPage.h: Schnittstelle der Klasse LFStorePropertiesIndexPage
//

#pragma once
#include "LFCommDlg.h"


// LFStorePropertiesIndexPage
//

class LFStorePropertiesIndexPage : public CPropertyPage
{
public:
	LFStorePropertiesIndexPage(LFStoreDescriptor* pStore);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	LFStoreDescriptor* p_Store;

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
