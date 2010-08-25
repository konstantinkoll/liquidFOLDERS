
#pragma once


class AFX_EXT_CLASS LFTooltip : public CWnd
{
public:
	LFTooltip();

	virtual BOOL Create(CWnd* pWndParent);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void Track(CPoint point, HICON hIcon, CSize szIcon, const CString& strCaption, CString strText);
	void Deactivate();

protected:
	void Hide();

	HICON m_Icon;
	CSize m_szIcon;
	CString m_strCaption;
	CString m_strText;

	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
