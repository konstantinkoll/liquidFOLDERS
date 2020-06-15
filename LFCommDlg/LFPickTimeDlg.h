
// LFPickTimeDlg.h: Schnittstelle der Klasse LFPickTimeDlg
//

#pragma once
#include "CInspectorGrid.h"


// LFPickTimeDlg
//

class LFPickTimeDlg : public CAttributePickDlg
{
public:
	LFPickTimeDlg(LFVariantData* pVData, ITEMCONTEXT Context=LFContextAllFiles, CWnd* pParentWnd=NULL, UINT nIDTemplate=IDD_PICKTIME, BOOL UseTime=FALSE, BOOL UseDate=TRUE);

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

private:
	BOOL m_UseTime;
};
