
// CGlassButton.h: Schnittstelle der Klasse CGlassButton
//

#pragma once


// CGlassButton
//

class AFX_EXT_CLASS CGlassButton : public CButton
{
public:
	CGlassButton();
	virtual ~CGlassButton();

protected:
	afx_msg void OnPaint();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	BOOL m_Hover;

	void CreateRoundRectangle(CRect rect, int rad, GraphicsPath& path);
	void CreateBottomRadialPath(CRect rect, GraphicsPath& path);
};
