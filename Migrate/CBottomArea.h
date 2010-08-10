
// CBottomArea.h: Schnittstelle der Klasse CBottomArea
//


// CBottomArea
//

class CBottomArea : public CDialogBar
{
public:
	CBottomArea();

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
};
