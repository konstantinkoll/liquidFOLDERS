
// CTaskButton.h: Schnittstelle der Klasse CTaskButton
//

#pragma once


// CTaskButton
//

#define ID_UPDATEBUTTONS             900

class AFX_EXT_CLASS CTaskButton : public CButton
{
public:
	CTaskButton();

	void Create(CString Caption, CString Tooltip, CMFCToolBarImages* Icons, int IconID, CWnd* pParentWnd, UINT nID);
	int GetPreferredWidth();

protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEnable(BOOL bEnable);
	DECLARE_MESSAGE_MAP()

private:
	CString m_Caption;
	CString m_Tooltip;
	CMFCToolBarImages* m_Icons;
	int m_IconID;
	BOOL m_Hover;

	void CreateRoundRectangle(CRect rect, int rad, GraphicsPath& path);
};
