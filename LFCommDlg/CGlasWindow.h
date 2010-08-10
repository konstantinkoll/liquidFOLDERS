
// CGlasWindow: Schnittstelle der Klasse CGlasWindow
//

#pragma once
#include "liquidFOLDERS.h"
#include "LFApplication.h"


// CGlasWindow
//

class AFX_EXT_CLASS CGlasWindow : public CWnd
{
public:
	CGlasWindow();
	~CGlasWindow();

	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void AdjustLayout();

	void UseGlasBackground(MARGINS Margins);
	void GetLayoutRect(LPRECT lpRect) const;
	void DrawFrameBackground(CDC* pDC, CRect rect);

	BOOL m_IsAeroWindow;
	HTHEME hTheme;

protected:
	LFApplication* p_App;
	MARGINS m_Margins;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnCompositionChanged();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	DECLARE_MESSAGE_MAP()
};
