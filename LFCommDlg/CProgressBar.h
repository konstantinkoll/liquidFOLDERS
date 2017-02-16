
// CProgressBar: Schnittstelle der Klasse CProgressBar
//

#pragma once


// CProgressBar
//

class CProgressBar : public CStatic
{
public:
	CProgressBar();

	virtual void PreSubclassWindow();

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
