
// LFEditTimeDlg.h: Schnittstelle der Klasse LFEditTimeDlg
//

#pragma once
#include "LFCore.h"
#include "LFApplication.h"
#include "CStorePanel.h"
#include "CInspectorGrid.h"
#include "CFrameCtrl.h"


// LFEditTimeDlg
//

class AFX_EXT_CLASS LFEditTimeDlg : public CDialog
{
public:
	LFEditTimeDlg(CWnd* pParentWnd, UINT Attr);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	LFApplication* p_App;
	UINT m_Attr;

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CMonthCalCtrl m_wndCalendar;
	CFrameCtrl m_FrameCtrl;
};
