
// CBottomArea.h: Schnittstelle der Klasse CBottomArea
//


// CBottomArea
//

class CBottomArea : public CDialogBar
{
public:
	CBottomArea();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpcs);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	DECLARE_MESSAGE_MAP()

private:
	HBRUSH hBackgroundBrush;
};
