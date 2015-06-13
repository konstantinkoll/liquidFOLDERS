
// LFGotoYearDlg.h: Schnittstelle der Klasse LFGotoYearDlg
//

#pragma once
#include "LFDialog.h"


// LFGotoYearDlg
//

class LFGotoYearDlg : public LFDialog
{
public:
	LFGotoYearDlg(UINT Year, CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

	UINT m_Year;

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CMFCMaskedEdit m_wndEdit;
};
