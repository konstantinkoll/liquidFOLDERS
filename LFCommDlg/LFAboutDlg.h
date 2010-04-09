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
	int AllowEmptyDrives;

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

	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};
