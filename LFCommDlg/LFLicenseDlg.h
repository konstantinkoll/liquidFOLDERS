
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
	LFLicenseDlg(CWnd* pParent=NULL);

protected:
	CGdiPlusBitmapResource* icon;

	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnLoadLicense();
	DECLARE_MESSAGE_MAP()
};
