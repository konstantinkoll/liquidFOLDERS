
// GlobeOptionsDlg.h: Schnittstelle der Klasse GlobeOptionsDlg
//

#pragma once
#include "liquidFOLDERS.h"


// GlobeOptionsDlg
//

class GlobeOptionsDlg : public LFDialog
{
public:
	GlobeOptionsDlg(LFViewParameters* pViewParameters, UINT Context, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnViewport();
	DECLARE_MESSAGE_MAP()

	LFViewParameters* p_ViewParameters;
	UINT m_Context;

private:
	CComboBox m_wndTextureSize;
	CButton m_wndViewport;
};
