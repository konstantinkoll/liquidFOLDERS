
// CPaneText.h: Schnittstelle der Klasse CPaneText
//

// CPaneText
//

class CPaneText : public CWnd
{
public:
	CPaneText();

	BOOL Create(CWnd* pParentWnd, UINT nID, CString _text=_T(""));
	void SetText(CString _text=_T(""));

protected:
	CString m_Text;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, INT cx, INT cy);
	DECLARE_MESSAGE_MAP()
};
