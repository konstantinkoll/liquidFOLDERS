
// CHeaderView.h: Schnittstelle der Klasse CHeaderView
//

#pragma once
#define cAttrColumns               15


// CHeaderView
//

class CHeaderView : public CWnd
{
public:
	CHeaderView();
	virtual ~CHeaderView();

	void Create(CWnd* _pParentWnd, UINT nID);
	int GetPreferredHeight();

protected:
	UINT widths[cAttrColumns];
	CComboBox cbxs[cAttrColumns];

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	DECLARE_MESSAGE_MAP()
};
