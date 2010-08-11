
// CGlasWindow: Schnittstelle der Klasse CGlasWindow
//

#pragma once
#include "liquidFOLDERS.h"
#include "LFApplication.h"


// CGlasWindow
//

#define GWD_DEFAULT            1
#define GWD_THEMED             2
#define GWD_AERO               3

class AFX_EXT_CLASS CGlasWindow : public CWnd
{
public:
	CGlasWindow();
	~CGlasWindow();

	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual void AdjustLayout();

	BOOL Create(DWORD dwStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, const RECT& rect, CWnd* pParentWnd=NULL, UINT nID=0);
	BOOL PreTranslateMessage(MSG* pMsg);
	void UseGlasBackground(MARGINS Margins);
	void GetLayoutRect(LPRECT lpRect) const;
	void DrawFrameBackground(CDC* pDC, CRect rect);
	UINT GetDesign();

	HTHEME hTheme;

protected:
	LFApplication* p_App;
	BOOL m_IsAeroWindow;
	MARGINS m_Margins;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnCompositionChanged();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnActivateApp(BOOL bActive, DWORD dwThreadID);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	DECLARE_MESSAGE_MAP()
};
