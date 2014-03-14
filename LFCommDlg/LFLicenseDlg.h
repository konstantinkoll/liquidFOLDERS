
// LFLicenseDlg.h: Schnittstelle der Klasse LFLicenseDlg
//

#pragma once
#include "CGdiPlusBitmap.h"
#include "liquidFOLDERS.h"
#include "LFDialog.h"


// LFLicenseDlg
//

class AFX_EXT_CLASS LFLicenseDlg : public LFDialog
{
public:
	LFLicenseDlg(CWnd* pParentWnd=NULL);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg void OnLoadLicense();
	afx_msg void OnChange();
	DECLARE_MESSAGE_MAP()
};
