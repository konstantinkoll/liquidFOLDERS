
// GoToYearDlg.h: Schnittstelle der Klasse GoToYearDlg
//

#pragma once
#include "StoreManager.h"


// GoToYearDlg
//

class GoToYearDlg : public CDialog
{
public:
	GoToYearDlg(CWnd* pParentWnd, UINT Year);

	virtual void DoDataExchange(CDataExchange* pDX);

	UINT m_Year;

protected:
	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CMFCMaskedEdit m_wndEdit;
};
