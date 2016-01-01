
// LFLicenseDlg.h: Schnittstelle der Klasse LFLicenseDlg
//

#pragma once
#include "LFDialog.h"


// LFLicenseDlg
//

class LFLicenseDlg : public LFDialog
{
public:
	LFLicenseDlg(CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL InitDialog();

	afx_msg void OnLoadLicense();
	afx_msg void OnChange();
	DECLARE_MESSAGE_MAP()
};
