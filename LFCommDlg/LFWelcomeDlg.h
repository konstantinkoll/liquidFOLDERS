#pragma once
#include "LFDialog.h"

class AFX_EXT_CLASS LFWelcomeDlg : public LFDialog
{
public:
	LFWelcomeDlg(CWnd* pParentWnd=NULL);

protected:
	CGlassButton m_OkButton;
	CGlassButton m_CancelButton;
	CGlassButton m_LicenseButton;

	virtual void DoDataExchange(CDataExchange* pDX);
	void CreateStore(int ID);

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
