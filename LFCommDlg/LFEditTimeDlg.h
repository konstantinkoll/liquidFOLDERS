
// LFEditTimeDlg.h: Schnittstelle der Klasse LFEditTimeDlg
//

#pragma once
#include "LFDialog.h"


// LFEditTimeDlg
//

class LFEditTimeDlg : public LFDialog
{
public:
	LFEditTimeDlg(LFVariantData* pData, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnUseTime();
	DECLARE_MESSAGE_MAP()

	LFVariantData* p_Data;

private:
	CMonthCalCtrl m_wndCalendar;
	CDateTimeCtrl m_wndTime;
	BOOL m_UseTime;
};
