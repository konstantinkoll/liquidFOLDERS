
// CExplorerNotification.h: Schnittstelle der Klasse CExplorerNotification
//

#pragma once
#include "LFCommDlg.h"


// CExplorerNotification
//

#define ENT_READY        1
#define ENT_INFO         2
#define ENT_WARNING      3
#define ENT_SHIELD       5
#define ENT_ERROR        6

class AFX_EXT_CLASS CExplorerNotification : public CWnd
{
public:
	CExplorerNotification();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	UINT GetPreferredHeight();
	void SetNotification(UINT Type, CString Text, UINT Command=0);
	void DismissNotification();

protected:
	BOOL m_Dismissed;

	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

private:
	LFApplication* p_App;
	COLORREF m_FirstCol;
	COLORREF m_SecondCol;
	HICON hIcon;
	INT m_IconCX;
	INT m_IconCY;
	CString m_Text;
	UINT m_Command;
};
