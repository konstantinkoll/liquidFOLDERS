
// CExplorerNotification.h: Schnittstelle der Klasse CExplorerNotification
//

#pragma once
#include "CFrontstageWnd.h"
#include "CHoverButton.h"


// CExplorerNotification
//

#define ENT_READY       1
#define ENT_INFO        2
#define ENT_WARNING     3
#define ENT_SHIELD      4
#define ENT_ERROR       5

class CExplorerNotification : public CFrontstageWnd
{
public:
	CExplorerNotification();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	UINT GetPreferredHeight() const;
	void SetNotification(UINT Type, const CString& Text, UINT Command=0);
	void DismissNotification();

protected:
	void AdjustLayout();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnButtonClicked();
	DECLARE_MESSAGE_MAP()

	BOOL m_Dismissed;

private:
	COLORREF m_FirstCol;
	COLORREF m_SecondCol;
	HICON hIcon;
	INT m_IconCX;
	INT m_IconCY;
	INT m_GradientCY;
	CString m_Text;
	CRect m_RectClose;
	BOOL m_CloseHover;
	BOOL m_ClosePressed;
	CHoverButton m_wndCommandButton;
	CString m_CommandText;
	UINT m_Command;
	UINT m_RightMargin;
};
