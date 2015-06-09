
// CCategory: Schnittstelle der Klasse CCategory
//

#pragma once


// CCategory
//

class CCategory : public CStatic
{
public:
	CCategory();

	virtual void PreSubclassWindow();

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
