
// GlobeOptionsDlg.h: Schnittstelle der Klasse GlobeOptionsDlg
//

#pragma once
#include "liquidFOLDERS.h"


// GlobeOptionsDlg
//

class GlobeOptionsDlg : public LFDialog
{
public:
	GlobeOptionsDlg(LFViewParameters* pViewParameters, CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	LFViewParameters* p_ViewParameters;

private:
	static void AddQuality(CComboBox& wndCombobox, UINT nResID);

	CComboBox m_wndModelQuality;
	CComboBox m_wndTextureQuality;
};
