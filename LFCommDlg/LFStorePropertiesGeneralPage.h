
// LFStorePropertiesGeneralPage.h: Schnittstelle der Klasse LFStorePropertiesGeneralPage
//

#pragma once
#include "LFCommDlg.h"


// LFStorePropertiesGeneralPage
//

class LFStorePropertiesGeneralPage : public CPropertyPage
{
public:
	LFStorePropertiesGeneralPage(LFStoreDescriptor* pStore);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	LFStoreDescriptor* p_Store;
	CIconCtrl m_Icon;

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
