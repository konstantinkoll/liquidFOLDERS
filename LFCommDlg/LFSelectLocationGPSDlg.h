
// LFSelectLocationGPSDlg.h: Schnittstelle der Klasse LFSelectLocationGPSDlg
//

#pragma once
#include "CMapCtrl.h"
#include "LFDialog.h"


// LFSelectLocationGPSDlg
//

class LFSelectLocationGPSDlg : public LFDialog
{
public:
	LFSelectLocationGPSDlg(const LFGeoCoordinates& Location, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	LFGeoCoordinates m_Location;

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnUpdateEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnLatitudeChanged();
	afx_msg void OnLongitudeChanged();

	afx_msg void OnIATA();
	afx_msg void OnReset();
	afx_msg void OnUpdateCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

private:
	CMapCtrl m_wndMap;
};
