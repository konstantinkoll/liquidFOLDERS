
// LFMakeTaskDlg.h: Schnittstelle der Klasse LFMakeTaskDlg
//

#pragma once
#include "LFEditTimeDlg.h"


// LFMakeTaskDlg
//

class LFMakeTaskDlg : public LFEditTimeDlg
{
public:
	LFMakeTaskDlg(LFVariantData* pDataPriority, LFVariantData* pDataDueTime, CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnUseDate();
	DECLARE_MESSAGE_MAP()

	LFVariantData* p_Data;

	CPropertyEdit m_wndPriority;
};
