
// CBackstageWnd: Schnittstelle der Klasse CBackstageWnd
//

#pragma once
#include "CBackstageShadow.h"
#include "CBackstageSidebar.h"
#include "CBackstageWidgets.h"


// CBackstageWnd
//

#define BACKSTAGEGRIPPER           4
#define BACKSTAGERADIUS            11
#define BACKSTAGEBORDER            9
#define BACKSTAGECAPTIONMARGIN     4

#define SWP_NOCLIENTSIZE     0x0800
#define SWP_NOCLIENTMOVE     0x1000

class CBackstageWnd : public CWnd
{
public:
	CBackstageWnd(BOOL IsDialog=FALSE, BOOL WantsBitmap=FALSE);

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	virtual BOOL OnCmdMsg(UINT nID, INT nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	virtual BOOL GetLayoutRect(LPRECT lpRect) const;
	virtual void PostNcDestroy();

	BOOL Create(DWORD dwStyle, LPCTSTR lpszClassName, LPCTSTR lpszWindowName, LPCTSTR lpszPlacementPrefix=_T(""), const CSize& Size=CSize(0, 0), BOOL ShowCaption=FALSE);
	void SetSidebar(CBackstageSidebar* pSidebarWnd);
	void GetCaptionButtonMargins(LPSIZE lpSize) const;
	void HideSidebar();

protected:
	virtual INT GetCaptionHeight(BOOL IncludeBottomMargin=TRUE) const;
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);
	virtual void PaintOnBackground(CDC& dc, Graphics& g, const CRect& rectLayout);
	virtual void PaintBackground(CPaintDC& pDC, CRect rect);

	void UpdateBackground();
	void PaintCaption(CPaintDC& pDC, CRect& rect);
	void InvalidateCaption();
	void UpdateRegion(INT cx=-1, INT cy=-1);
	void AdjustLayout(UINT nFlags=SWP_NOACTIVATE | SWP_NOZORDER);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnNcCalcSize(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcPaint();
	afx_msg BOOL OnNcActivate(BOOL bActive);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg LRESULT OnGetTitleBarInfoEx(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg LRESULT OnSetFont(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnCompositionChanged();
	HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnWindowPosChanging(WINDOWPOS* lpwndpos);
	afx_msg void OnWindowPosChanged(WINDOWPOS* lpwndpos);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnInitMenuPopup(CMenu* pPopupMenu, UINT nIndex, BOOL bSysMenu);
	afx_msg LRESULT OnLicenseActivated(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWakeup(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);

	afx_msg void OnBackstageToggleSidebar();
	afx_msg void OnUpdateBackstageCommands(CCmdUI* pCmdUI);
	DECLARE_MESSAGE_MAP()

	CString m_PlacementPrefix;
	HACCEL hAccelerator;
	CBackstageSidebar* m_pSidebarWnd;
	BOOL m_WantsBitmap;
	BOOL m_ShowSidebar;
	BOOL m_SidebarAlwaysVisible;
	BOOL m_ShowCaption;
	BOOL m_ShowExpireCaption;
	INT m_BottomDivider;
	INT m_BackBufferL;
	INT m_BackBufferH;
	HBRUSH hBackgroundBrush;
	CBackstageShadow m_wndShadow;
	CBackstageWidgets m_wndWidgets;

private:
	void PrepareBitmaps();

	INT m_RegionWidth;
	INT m_RegionHeight;
	BOOL m_IsDialog;
};
