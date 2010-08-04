#pragma once
#include "CGdiPlusBitmap.h"
#include "liquidFOLDERS.h"
#include "LFDialog.h"

struct LFAboutDlgParameters
{
	// Vom Aufrufer zu setzen
	CString appname;
	CString build;
	CGdiPlusBitmapResource* icon;
	int TextureSize;
	int MaxTextureSize;
	int RibbonColor;
	BOOL HideEmptyDrives;
	BOOL HideEmptyDomains;

	// Von LFCommDlg gesetzt
	CString version;
	CString copyright;
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

	LFAboutDlgParameters* parameters;
};
