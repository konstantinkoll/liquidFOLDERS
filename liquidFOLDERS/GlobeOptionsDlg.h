
// GlobeOptionsDlg.h: Schnittstelle der Klasse GlobeOptionsDlg
//

#pragma once
#include "liquidFOLDERS.h"


// GlobeOptionsDlg
//

class GlobeOptionsDlg : public LFDialog
{
public:
	GlobeOptionsDlg(CWnd* pParentWnd, LFViewParameters* View, UINT Context);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	LFViewParameters* p_View;
	UINT m_Context;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnViewport();
	DECLARE_MESSAGE_MAP()

private:
	CComboBox m_wndTextureSize;
	CButton m_wndViewport;
};
