
// LFGotoYearDlg.h: Schnittstelle der Klasse LFGotoYearDlg
//

#pragma once
#include "LFCommDlg.h"


// LFGotoYearDlg
//

class LFGotoYearDlg : public LFDialog
{
public:
	LFGotoYearDlg(CWnd* pParentWnd, UINT Year);

	virtual void DoDataExchange(CDataExchange* pDX);

	UINT m_Year;

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CMFCMaskedEdit m_wndEdit;
};
