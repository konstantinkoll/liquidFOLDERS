
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

protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEnable(BOOL bEnable);
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_Hover;

	void CreateRoundRectangle(CRect rect, int rad, GraphicsPath& path);
	void CreateBottomRadialPath(CRect rect, GraphicsPath& path);
};
