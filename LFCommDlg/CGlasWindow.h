
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

	void UseGlasBackground(MARGINS Margins);
	void GetLayoutRect(LPRECT lpRect) const;

	BOOL m_IsAeroWindow;
	HTHEME hTheme;

protected:
	LFApplication* p_App;
	MARGINS m_Margins;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnCompositionChanged();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS *lpncsp);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcPaint();
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	DECLARE_MESSAGE_MAP()
};
