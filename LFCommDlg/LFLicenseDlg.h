#pragma once
#include "CGdiPlusBitmap.h"
#include "liquidFOLDERS.h"
#include "LFDialog.h"

class AFX_EXT_CLASS LFLicenseDlg : public LFDialog
{
public:
	LFLicenseDlg(CWnd* pParent=NULL);

protected:
	CGdiPlusBitmapResource* icon;
	CGlassButton m_OkButton;
	CGlassButton m_CancelButton;
	CGlassButton m_LoadButton;

	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg void OnLoadLicense();
	DECLARE_MESSAGE_MAP()
};
