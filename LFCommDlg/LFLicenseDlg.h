
// LFLicenseDlg.h: Schnittstelle der Klasse LFLicenseDlg
//

#pragma once
#include "LF.h"
#include "LFDialog.h"


// LFLicenseDlg
//

class LFLicenseDlg : public LFDialog
{
public:
	LFLicenseDlg(CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnLoadLicense();
	afx_msg void OnChange();
	DECLARE_MESSAGE_MAP()
};
