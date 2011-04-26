
// CLabel.h: Schnittstelle der Klasse CLabel
//

#pragma once


// CLabel
//

class AFX_EXT_CLASS CLabel : public CWnd
{
public:
	CLabel();

	BOOL Create(CWnd* pParentWnd, UINT nID, CString Text=_T(""));
	void SetText(CString Text);
	INT GetPreferredHeight();

protected:
	LFApplication* p_App;
	INT m_FontHeight;
	CString m_Text;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
