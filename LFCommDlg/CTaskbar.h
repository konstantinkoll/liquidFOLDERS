
// CTaskbar.h: Schnittstelle der Klasse CTaskbar
//

#pragma once
#include "CTaskButton.h"
#include <list>
using std::list;


// CTaskbar
//

class AFX_EXT_CLASS CTaskbar : public CWnd
{
public:
	CTaskbar();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	UINT GetPreferredHeight();

protected:
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnUpdateButtons();
	DECLARE_MESSAGE_MAP()

private:
	CBitmap BackBuffer;
	int BackBufferL;
	int BackBufferH;
	HBRUSH hBackgroundBrush;
	list<CTaskButton*> ButtonsLeft;
	list<CTaskButton*> ButtonsRight;
};
