
// CDropdownSelector.h: Schnittstelle der Klasse CDropdownSelector
//

#pragma once
#include "CGlasWindow.h"
#include "CExplorerList.h"
#include "CBottomArea.h"
#include "LFTooltip.h"


// CDropdownListCtrl
//

class AFX_EXT_CLASS CDropdownListCtrl : public CExplorerList
{
public:
	CDropdownListCtrl();

protected:
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};


// CDropdownWindow
//

#define CXDropdownListIconSpacing     90

class AFX_EXT_CLASS CDropdownWindow : public CWnd
{
public:
	CDropdownWindow();

	virtual void AdjustLayout();

	BOOL Create(CWnd* pOwnerWnd, UINT _DialogResID=0);

protected:
	CDropdownListCtrl m_wndList;
	CBottomArea m_wndBottomArea;
	UINT m_DialogResID;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	DECLARE_MESSAGE_MAP()
};


// CDropdownSelector
//

#define WM_SETITEM        WM_USER+4
#define NM_SELCHANGED     WM_USER+5
#define NM_SELUPDATE      WM_USER+6

class AFX_EXT_CLASS CDropdownSelector : public CWnd
{
public:
	CDropdownSelector();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void CreateDropdownWindow();
	virtual void SetEmpty(BOOL Repaint=TRUE);
	virtual void GetTooltipData(HICON& hIcon, CSize& size, CString& caption, CString& hint);

	BOOL Create(CString EmptyHint, CString Caption, CGlasWindow* pParentWnd, UINT nID);
	void SetItem(HICON hIcon, CString DisplayName, BOOL Repaint=TRUE, UINT NotifyCode=NM_SELCHANGED);
	UINT GetPreferredHeight();
	BOOL IsEmpty();

protected:
	CString m_EmptyHint;
	CString m_Caption;
	CString m_DisplayName;
	HICON m_Icon;
	BOOL m_IsEmpty;
	BOOL m_Hover;
	BOOL m_Pressed;
	BOOL m_Dropped;
	LFApplication* p_App;
	CDropdownWindow* p_DropWindow;
	LFTooltip m_TooltipCtrl;

	void NotifyOwner(UINT NotifyCode);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg LRESULT OnOpenDropdown(WPARAM wParam=NULL, LPARAM lParam=NULL);
	afx_msg LRESULT OnCloseDropdown(WPARAM wParam=NULL, LPARAM lParam=NULL);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pOldWnd);
	DECLARE_MESSAGE_MAP()

private:
	HTHEME hTheme;
};
