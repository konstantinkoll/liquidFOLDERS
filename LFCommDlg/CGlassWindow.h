
// CGlassWindow: Schnittstelle der Klasse CGlassWindow
//

#pragma once


// CGlassWindow
//

#define GWD_DEFAULT     1
#define GWD_THEMED      2
#define GWD_AERO        3

class CGlassWindow : public CWnd
{
public:
	CGlassWindow();

	virtual LRESULT DefWindowProc(UINT Message, WPARAM wParam, LPARAM lParam);
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

protected:
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
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg LRESULT OnWakeup(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);
	DECLARE_MESSAGE_MAP()

	CString m_PlacementPrefix;
	WINDOWPLACEMENT m_WindowPlacement;
	BOOL m_Active;
	HACCEL hAccelerator;
	HTHEME hTheme;
	CList<CWnd*> m_GlasChildren;
	BOOL m_IsAeroWindow;
	MARGINS m_Margins;

private:
	void SetTheme();
};
