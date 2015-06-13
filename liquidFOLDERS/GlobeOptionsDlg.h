
// GlobeOptionsDlg.h: Schnittstelle der Klasse GlobeOptionsDlg
//

#pragma once
#include "liquidFOLDERS.h"


// GlobeOptionsDlg
//

class GlobeOptionsDlg : public LFDialog
{
public:
	GlobeOptionsDlg(LFViewParameters* View, UINT Context, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnViewport();
	DECLARE_MESSAGE_MAP()

	LFViewParameters* p_View;
	UINT m_Context;

private:
	CComboBox m_wndTextureSize;
	CButton m_wndViewport;
};
