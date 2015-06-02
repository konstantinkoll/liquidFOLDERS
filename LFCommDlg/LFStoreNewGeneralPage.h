
// LFStoreNewGeneralPage.h: Schnittstelle der Klasse LFStoreNewGeneralPage
//

#pragma once
#include "LFCommDlg.h"


// LFStoreNewGeneralPage
//

class LFStoreNewGeneralPage : public CPropertyPage
{
public:
	virtual void DoDataExchange(CDataExchange* pDX);

	CIconCtrl m_wndIcon;
	CEdit m_wndStoreName;
	CEdit m_wndStoreComment;
	CButton m_wndMakeDefault;
	CButton m_wndMakeSearchable;
};
