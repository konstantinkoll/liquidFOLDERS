
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
	DECLARE_MESSAGE_MAP()
};
