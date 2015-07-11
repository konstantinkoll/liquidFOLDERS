
// LFStorePropertiesGeneralPage.h: Schnittstelle der Klasse LFStorePropertiesGeneralPage
//

#pragma once
#include "CIconCtrl.h"
#include "LFCore.h"


// LFStorePropertiesGeneralPage
//

class LFStorePropertiesGeneralPage : public CPropertyPage
{
public:
	LFStorePropertiesGeneralPage(LFStoreDescriptor* pStore, BOOL* pStoreValid);

	virtual void DoDataExchange(CDataExchange* pDX);

	CEdit m_wndStoreName;
	CEdit m_wndStoreComment;
	CButton m_wndMakeDefault;
	CButton m_wndMakeSearchable;

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg LRESULT OnUpdateStore(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	LFStoreDescriptor* p_Store;
	BOOL* p_StoreValid;
	CIconCtrl m_wndIcon;
};
