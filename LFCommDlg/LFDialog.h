
#pragma once
#include "CGdiPlusBitmap.h"
#include "liquidFOLDERS.h"
#include "CGlassButton.h"
#include "CTransparentRadioButton.h"

class AFX_EXT_CLASS LFDialog : public CDialog
{
public:
	LFDialog(UINT nIDTemplate, CWnd* pParent=NULL);

	CBitmap* GetBackBuffer();

protected:
	CGdiPlusBitmapResource* backdrop;
	CGdiPlusBitmapResource* logo;
	UINT m_nIDTemplate;

	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);
	virtual void CheckLicenseKey(LFLicense* License=NULL);

	afx_msg BOOL OnInitDialog();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnDestroy();
	afx_msg void OnEnterLicenseKey();
	DECLARE_MESSAGE_MAP()

private:
	CBitmap BackBuffer;
	int BackBufferL;
	int BackBufferH;
	HICON m_hIconL;
	HICON m_hIconS;
};
