
// LFWelcomeDlg.h: Schnittstelle der Klasse LFWelcomeDlg
//

#pragma once
#include "LFDialog.h"


// LFWelcomeDlg
//

class AFX_EXT_CLASS LFWelcomeDlg : public LFDialog
{
public:
	LFWelcomeDlg(CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	void CreateStore(INT ID);

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
