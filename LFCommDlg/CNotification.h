
// CNotification.h: Schnittstelle der Klasse CNotification
//

#pragma once
#include "CFrontstageWnd.h"
#include "CHoverButton.h"


// CNotification
//

#define ENT_READY       1
#define ENT_INFO        2
#define ENT_WARNING     3
#define ENT_SHIELD      4
#define ENT_ERROR       5

class CNotification : public CFrontstageWnd
{
public:
	CNotification();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	UINT GetPreferredHeight() const;
	void SetNotification(UINT Type, const CString& Text, UINT Command=0);
	void DismissNotification();

protected:
	void AdjustLayout();
	UINT GetTimerLength() const;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
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
	HICON m_hIcon;
	INT m_IconSize;
	INT m_GradientHeight;
	BOOL m_CloseHover;
	BOOL m_ClosePressed;
	COLORREF m_FirstCol;
	COLORREF m_SecondCol;
	CString m_Text;
	CRect m_RectClose;
	CHoverButton m_wndCommandButton;
	CString m_CommandText;
	UINT m_Command;
	UINT m_RightMargin;
};

inline UINT CNotification::GetTimerLength() const
{
	return 2000+40*m_Text.GetLength();
}
