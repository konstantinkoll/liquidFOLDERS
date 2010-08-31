
// CDropdownSelector.h: Schnittstelle der Klasse CDropdownSelector
//

#pragma once
#include "CGlasWindow.h"
#include "CExplorerList.h"
#include "CBottomArea.h"


// CDropdownWindow
//

#define CXDropdownListIconSpacing     90

class AFX_EXT_CLASS CDropdownWindow : public CWnd
{
public:
	CDropdownWindow();

	virtual void AdjustLayout();

	BOOL Create(CWnd* pOwnerWnd, UINT _DialogResID=0);
	void SetDesign(UINT _Design);
	void AddCategory(int ID, CString name);

protected:
	CExplorerList m_wndList;
	CBottomArea m_wndBottomArea;
	UINT m_DialogResID;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	DECLARE_MESSAGE_MAP()
};


// CDropdownSelector
//

class AFX_EXT_CLASS CDropdownSelector : public CWnd
{
public:
	CDropdownSelector();

	virtual void CreateDropdownWindow();

	BOOL Create(CString EmptyHint, CGlasWindow* pParentWnd, UINT nID);
	void SetEmpty(BOOL Repaint=TRUE);
	void SetItem(CString Caption, HICON hIcon, CString DisplayName, BOOL Repaint=TRUE);
	UINT GetPreferredHeight();

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

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
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
	DECLARE_MESSAGE_MAP()

private:
	HTHEME hTheme;
};
