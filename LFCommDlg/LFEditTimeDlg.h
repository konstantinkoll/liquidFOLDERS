
// LFEditTimeDlg.h: Schnittstelle der Klasse LFEditTimeDlg
//

#pragma once
#include "LFDialog.h"


// LFEditTimeDlg
//

class LFEditTimeDlg : public LFDialog
{
public:
	LFEditTimeDlg(LFVariantData* pVData, CWnd* pParentWnd=NULL, UINT nIDTemplate=IDD_EDITTIME, BOOL UseTime=FALSE, BOOL UseDate=TRUE);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	void EnableControls();

	afx_msg void OnUseTime();
	DECLARE_MESSAGE_MAP()

	LFVariantData* p_VData;

	CMonthCalCtrl m_wndCalendar;
	CDateTimeCtrl m_wndTime;
	BOOL m_UseDate;
	BOOL m_UseTime;
};
