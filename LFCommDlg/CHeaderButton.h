
// CHeaderButton.h: Schnittstelle der Klasse CHeaderButton
//

#pragma once
#include "CHoverButton.h"


// CHeaderButton
//

#define WM_ADJUSTLAYOUT     WM_USER+2
#define WM_GETMENU          WM_USER+3

class CHeaderButton : public CHoverButton
{
public:
	CHeaderButton();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	BOOL Create(CWnd* pParentWnd, UINT nID, CString Caption, CString Hint);
	void SetValue(LPCWSTR Value, BOOL ShowDropdown=TRUE, BOOL Repaint=TRUE);
	void GetPreferredSize(LPSIZE lpSize, INT& CaptionWidth);
	void GetCaption(CString& Caption, INT& CaptionWidth) const;

protected:
	afx_msg void OnPaint();
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	DECLARE_MESSAGE_MAP()

private:
	CString m_Caption;
	CString m_Hint;
	CString m_Value;
	INT m_CaptionWidth;
	BOOL m_ShowDropdown;
};
