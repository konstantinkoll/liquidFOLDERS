
// LFEditTimeDlg.h: Schnittstelle der Klasse LFEditTimeDlg
//

#pragma once
#include "LFCore.h"
#include "LFApplication.h"
#include "CInspectorGrid.h"
#include "CFrameCtrl.h"


// LFEditTimeDlg
//

class AFX_EXT_CLASS LFEditTimeDlg : public CDialog
{
public:
	LFEditTimeDlg(CWnd* pParentWnd, LFVariantData* pData);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	LFApplication* p_App;
	LFVariantData* p_Data;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnUseTime();
	DECLARE_MESSAGE_MAP()

private:
	CMonthCalCtrl m_wndCalendar;
	CFrameCtrl m_FrameCtrl;
	CDateTimeCtrl m_wndTime;
	BOOL m_UseTime;
};
