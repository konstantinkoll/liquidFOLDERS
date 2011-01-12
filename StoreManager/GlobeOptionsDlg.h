
// GlobeOptionsDlg.h: Schnittstelle der Klasse GlobeOptionsDlg
//

#pragma once
#include "StoreManager.h"


// GlobeOptionsDlg
//

class GlobeOptionsDlg : public CDialog
{
public:
	GlobeOptionsDlg(CWnd* pParent, LFViewParameters* View, UINT Context);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	LFViewParameters* p_View;
	UINT m_Context;

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
