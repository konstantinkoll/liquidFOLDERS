
// LFStoreNewGeneralPage.h: Schnittstelle der Klasse LFStoreNewGeneralPage
//

#pragma once
#include "LFCommDlg.h"


// LFStoreNewGeneralPage
//

class LFStoreNewGeneralPage : public CPropertyPage
{
public:
	LFStoreNewGeneralPage(CHAR Volume);

	virtual void DoDataExchange(CDataExchange* pDX);

	CIconCtrl m_wndIcon;
	CEdit m_wndStoreName;
	CEdit m_wndStoreComment;
	CButton m_wndMakeDefault;
	CButton m_wndMakeSearchable;

protected:
	CHAR m_Volume;

	afx_msg BOOL OnInitDialog();
	afx_msg LRESULT OnUpdateStore(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};
