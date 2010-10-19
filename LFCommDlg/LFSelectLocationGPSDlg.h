
// LFSelectLocationGPSDlg.h: Schnittstelle der Klasse LFSelectLocationGPSDlg
//

#pragma once
#include "LFCore.h"
#include "LFCommDlg.h"
#include "CMapSelectionCtrl.h"


// LFSelectLocationGPSDlg
//

class AFX_EXT_CLASS LFSelectLocationGPSDlg : public CDialog
{
public:
	LFSelectLocationGPSDlg(CWnd* pParentWnd, const LFGeoCoordinates Location);

	virtual void DoDataExchange(CDataExchange* pDX);

	LFGeoCoordinates m_Location;

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnUpdateEdit(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnReset();
	afx_msg void OnLatitudeChanged();
	afx_msg void OnLongitudeChanged();
	DECLARE_MESSAGE_MAP()

private:
	CMapSelectionCtrl m_Map;
};
