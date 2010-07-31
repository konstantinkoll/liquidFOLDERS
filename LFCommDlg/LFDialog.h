
#pragma once
#include "CGdiPlusBitmap.h"
#include "liquidFOLDERS.h"
#include "CGlassButton.h"
#include "CTransparentRadioButton.h"

#define LFDS_Blue         1
#define LFDS_White        2
#define LFDS_UAC          3

#define LFDS_Default      LFDS_Blue

class AFX_EXT_CLASS LFDialog : public CDialog
{
public:
	LFDialog(UINT nIDTemplate, UINT nIDStyle=LFDS_Default, CWnd* pParent=NULL);

	CBitmap* GetBackBuffer();

protected:
	CGdiPlusBitmapResource* backdrop;
	CGdiPlusBitmapResource* logo;
	UINT m_nIDTemplate;
	UINT m_nIDStyle;

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
	HICON hIconL;
	HICON hIconS;
	HICON hIconShield;
	int ShieldSize;
	int UACHeight;
};
