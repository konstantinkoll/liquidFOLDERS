
// LFEditTimeDlg.h: Schnittstelle der Klasse LFEditTimeDlg
//

#pragma once
#include "LFCommDlg.h"


// LFEditTimeDlg
//

class LFEditTimeDlg : public LFDialog
{
public:
	LFEditTimeDlg(CWnd* pParentWnd, LFVariantData* pData);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	LFVariantData* p_Data;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnUseTime();
	DECLARE_MESSAGE_MAP()

private:
	CMonthCalCtrl m_wndCalendar;
	CDateTimeCtrl m_wndTime;
	BOOL m_UseTime;
};
