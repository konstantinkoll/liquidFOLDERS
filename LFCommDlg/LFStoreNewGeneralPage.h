
// LFStoreNewGeneralPage.h: Schnittstelle der Klasse LFStoreNewGeneralPage
//

#pragma once
#include "CIconCtrl.h"


// LFStoreNewGeneralPage
//

class LFStoreNewGeneralPage : public CPropertyPage
{
public:
	virtual void DoDataExchange(CDataExchange* pDX);

	CIconCtrl m_wndIcon;
	CEdit m_wndStoreName;
	CEdit m_wndStoreComment;
	CButton m_wndMakeSearchable;
};
