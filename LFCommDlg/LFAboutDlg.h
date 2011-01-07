
// LFAboutDlg.h: Schnittstelle der Klasse LFAboutDlg
//

#pragma once
#include "CGdiPlusBitmap.h"
#include "liquidFOLDERS.h"
#include "LFDialog.h"


// LFAboutDlg
//

struct LFAboutDlgParameters
{
	// Vom Aufrufer zu setzen
	CString AppName;
	CString Build;
	CGdiPlusBitmapResource* Icon;
	INT TextureSize;
	INT MaxTextureSize;

	// Von LFCommDlg gesetzt
	CString Version;
	CString Copyright;
};

class AFX_EXT_CLASS LFAboutDlg : public LFDialog
{
public:
	LFAboutDlg(LFAboutDlgParameters* pParameters, CWnd* pParent=NULL);

protected:
	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);
	virtual void CheckLicenseKey(LFLicense* License=NULL);
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

	LFAboutDlgParameters* p_Parameters;
};
