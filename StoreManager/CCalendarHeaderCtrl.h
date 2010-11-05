
// CCalendarHeaderCtrl.h: Schnittstelle der Klasse CCalendarHeaderCtrl
//

#pragma once
#include "CFileView.h"


// CCalendarHeaderCtrl
//

class CCalendarHeaderCtrl : public CWnd
{
public:
	CCalendarHeaderCtrl();
	virtual ~CCalendarHeaderCtrl();

	BOOL Create(CFileView* pParentWnd, UINT nID);
	void SetText(CString _Text);
	void SetColors(COLORREF _TextCol, COLORREF _BackCol, COLORREF _LineCol);

protected:
	COLORREF m_TextCol;
	COLORREF m_BackCol;
	COLORREF m_LineCol;
	CString m_Text;


	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	DECLARE_MESSAGE_MAP()
};
