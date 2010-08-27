
// CDropdownSelector.h: Schnittstelle der Klasse CDropdownSelector
//

#pragma once
#include "CGlasWindow.h"


// CDropdownSelector
//

class AFX_EXT_CLASS CDropdownSelector : public CWnd
{
public:
	CDropdownSelector();

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
