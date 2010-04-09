
// CMainView.h: Schnittstelle der Klasse CMainView
//

#pragma once
#include "LFCommDlg.h"
#include "CHeaderView.h"
#include "CTreeView.h"


// CMainView
//

class CMainView : public CWnd
{
public:
	CMainView();
	virtual ~CMainView();

	CString Root;

	void Create(CWnd* _pParentWnd);
	void ClearRoot();
	void SetRoot(CString _Root);

protected:
	CHeaderView m_wndHeader;
	CTreeView m_wndTree;
	CGdiPlusBitmapResource* logo;
	CFont fntHint;
	CString Hint;
	BOOL IsRootSet;

	BOOL PaintEmpty(CDC* pDC, Graphics* g, CRect& rect);
	void AdjustLayout();

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	DECLARE_MESSAGE_MAP()

private:
	void DrawBorder(CDC* pDC, CRect& rect);
};
