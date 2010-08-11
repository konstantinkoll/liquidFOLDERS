
// CExplorerHeader.h: Schnittstelle der Klasse CExplorerHeader
//


// CExplorerHeader
//

class AFX_EXT_CLASS CExplorerHeader : public CWnd
{
public:
	CExplorerHeader();
	virtual ~CExplorerHeader();

	BOOL Create(CWnd* pParentWnd, UINT nID);
	void SetText(CString _Caption, CString _Hint, BOOL Repaint=TRUE);
	void SetColors(COLORREF CaptionCol, COLORREF HintCol, COLORREF BackCol, COLORREF LineCol, BOOL Repaint=TRUE);
	void SetLineStyle(BOOL GradientLine, BOOL Repaint=TRUE);
	UINT GetPreferredHeight();

protected:
	COLORREF m_CaptionCol;
	COLORREF m_HintCol;
	COLORREF m_BackCol;
	COLORREF m_LineCol;
	CString m_Caption;
	CString m_Hint;
	CBitmap m_Background;
	HBRUSH m_hBackgroundBrush;
	BOOL m_GradientLine;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
