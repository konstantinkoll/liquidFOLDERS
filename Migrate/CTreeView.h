
// CTreeView.h: Schnittstelle der Klasse CTreeView
//

#pragma once


// CTreeView
//

class CTreeView : public CWnd
{
public:
	CTreeView();
	~CTreeView();

	void Create(CWnd* _pParentWnd, UINT nID);

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()
};
