#pragma once
#include "LFDialog.h"

class AFX_EXT_CLASS LFWelcomeDlg : public LFDialog
{
public:
	LFWelcomeDlg(CWnd* pParentWnd=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	void CreateStore(int ID);

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
