
// CGlassWindow: Schnittstelle der Klasse CGlassWindow
//

#pragma once
#include "liquidFOLDERS.h"
#include "LFApplication.h"


// CGlassWindow
//

#define GWD_DEFAULT            1
#define GWD_THEMED             2
#define GWD_AERO               3

#define WM_OPENDROPDOWN        WM_USER+1
#define WM_CLOSEDROPDOWN       WM_USER+2

class AFX_EXT_CLASS CGlassWindow : public CWnd
{
public:
	CGlassWindow();

	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual void AdjustLayout();
	virtual void PostNcDestroy();

	BOOL Create(DWORD dwStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, LPCTSTR lpszPlacementPrefix=_T(""), CSize sz=CSize(0, 0));
	void ToggleFullScreen();
	void UseGlasBackground(MARGINS Margins);
	void GetLayoutRect(LPRECT lpRect) const;
	void DrawFrameBackground(CDC* pDC, CRect rect);
	UINT GetDesign();
	CWnd* RegisterPopupWindow(CWnd* pPopupWnd);

	HTHEME hTheme;

protected:
	LFApplication* p_App;
	CWnd* p_PopupWindow;
	CList<CWnd*> m_GlasChildren;
	CString m_PlacementPrefix;
	WINDOWPLACEMENT m_WindowPlacement;
	BOOL m_IsAeroWindow;
	BOOL m_Active;
	HACCEL hAccelerator;
	MARGINS m_Margins;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSysColorChange();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnCompositionChanged();
	afx_msg LRESULT OnDisplayChange(WPARAM wParam, LPARAM lParam);
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg LRESULT OnWakeup(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	DECLARE_MESSAGE_MAP()

private:
	void SetTheme();
};
