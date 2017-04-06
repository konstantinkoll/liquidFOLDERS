
// GlobeOptionsDlg.h: Schnittstelle der Klasse GlobeOptionsDlg
//

#pragma once
#include "liquidFOLDERS.h"


// GlobeOptionsDlg
//

class GlobeOptionsDlg : public LFDialog
{
public:
	GlobeOptionsDlg(UINT Context, CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	UINT m_Context;
	LFContextViewSettings* p_ContextViewSettings;

	CComboBox m_wndModelQuality;
	CComboBox m_wndTextureQuality;

private:
	static void AddQuality(CComboBox& wndCombobox, UINT nResID);
};
