
// LFMakeTaskDlg.h: Schnittstelle der Klasse LFMakeTaskDlg
//

#pragma once
#include "LFPickTimeDlg.h"


// LFMakeTaskDlg
//

class LFMakeTaskDlg : public LFPickTimeDlg
{
public:
	LFMakeTaskDlg(LFVariantData* pVDataPriority, LFVariantData* pVDataDueTime, CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnUseDate();
	DECLARE_MESSAGE_MAP()

	CPropertyEdit m_wndPriority;

private:
	LFVariantData* p_VDataPriority;
};
