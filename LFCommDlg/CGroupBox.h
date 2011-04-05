
// CGroupBox: Schnittstelle der Klasse CGroupBox
//

#pragma once


// CGroupBox
//

class AFX_EXT_CLASS CGroupBox : public CStatic
{
public:
	CGroupBox();

	virtual void PreSubclassWindow();

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
