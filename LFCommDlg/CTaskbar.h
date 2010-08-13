
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

	virtual LRESULT DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	BOOL Create(CWnd* pParentWnd, UINT ResID, UINT nID);
	UINT GetPreferredHeight();
	CTaskButton* AddButton(UINT nID, CString Text, int IconID, BOOL bAddRight=FALSE, BOOL bOnlyIcon=FALSE);
	void AdjustLayout();

protected:
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnIdleUpdateCmdUI();
	DECLARE_MESSAGE_MAP()

private:
	CMFCToolBarImages Icons;
	CBitmap BackBuffer;
	int BackBufferL;
	int BackBufferH;
	HBRUSH hBackgroundBrush;
	list<CTaskButton*> ButtonsLeft;
	list<CTaskButton*> ButtonsRight;
};
