
#pragma once


class AFX_EXT_CLASS CSimpleTooltip : public CWnd
{
public:
	CSimpleTooltip();
	virtual ~CSimpleTooltip();

	virtual BOOL Create(CWnd* _pWndParent=NULL);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void Track(CRect rect, const CString& strText);
	void Hide();
	void Deactivate();

protected:
	CWnd* pWndParent;
	CString m_strText;
	CRect m_rect;

	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
