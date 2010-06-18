#pragma once
#include "CGdiPlusBitmap.h"
#include "liquidFOLDERS.h"
#include "LFDialog.h"

struct LFAboutDlgParameters
{
	// Vom Aufrufer zu setzen
	CString appname;
	CString build;
	CString caption;
	CGdiPlusBitmapResource* icon;
	int TextureSize;
	int MaxTextureSize;
	int RibbonColor;
	BOOL HideEmptyDrives;
	BOOL HideEmptyDomains;

	// Von LFAbout gesetzt
	CString version;
	CString copyright;
};

class AFX_EXT_CLASS LFAboutDlg : public LFDialog
{
public:
	LFAboutDlg(LFAboutDlgParameters* pParameters, CWnd* pParent=NULL);

protected:
	LFAboutDlgParameters* parameters;
	CGlassButton m_OkButton;
	CGlassButton m_CancelButton;
	CGlassButton m_LicenseButton;
	CTransparentRadioButton m_Texture[LFTexture8192-LFTextureAuto+1];
	CTransparentRadioButton m_HideEmptyDrives;
	CTransparentRadioButton m_HideEmptyDomains;

	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);
	virtual void CheckLicenseKey(LFLicense* License=NULL);
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
