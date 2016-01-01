
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

	UINT m_Year;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

private:
	CMFCMaskedEdit m_wndEdit;
};
