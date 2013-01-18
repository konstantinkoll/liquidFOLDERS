
// LFDialog.h: Schnittstelle der Klasse LFDialog
//


#pragma once
#include "CGdiPlusBitmap.h"
#include "CGroupBox.h"
#include "liquidFOLDERS.h"
#include "LFApplication.h"


// LFDialog
//

#define LFDS_Blue         1
#define LFDS_White        2
#define LFDS_UAC          3

#define LFDS_Default      LFDS_Blue

class AFX_EXT_CLASS LFDialog : public CDialog
{
public:
	LFDialog(UINT nIDTemplate, UINT Design=LFDS_Default, CWnd* pParent=NULL);

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void AdjustLayout();

	void GetLayoutRect(LPRECT lpRect) const;
	UINT GetDesign() const;

protected:
	LFApplication* p_App;
	UINT m_nIDTemplate;
	UINT m_Design;

	virtual void OnEraseBkgnd(CDC& dc, Graphics& g, CRect& rect);
	virtual void CheckLicenseKey(LFLicense* License=NULL);

	CWnd* GetBottomWnd() const;
	void SetBottomLeftControl(CWnd* pChildWnd);
	void SetBottomLeftControl(UINT nID);
	void AddBottomRightControl(CWnd* pChildWnd);
	void AddBottomRightControl(UINT nID);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSysColorChange();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg void OnEnterLicenseKey();
	DECLARE_MESSAGE_MAP()

private:
	CGdiPlusBitmapResource* m_pBackdrop;
	CGroupBox m_GroupBox1;
	CGroupBox m_GroupBox2;
	HICON hIconL;
	HICON hIconS;
	HICON hIconShield;
	INT m_ShieldSize;
	INT m_UACHeight;
	CBitmap m_BackBuffer;
	INT m_BackBufferL;
	INT m_BackBufferH;
	HBRUSH hBackgroundBrush;
	CWnd* p_BottomLeftControl;
	CList<CWnd*> m_BottomRightControls;
	CPoint m_LastSize;
};
