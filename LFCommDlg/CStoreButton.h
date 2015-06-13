
// CStoreButton.h: Schnittstelle der Klasse CStoreButton
//

#pragma once


// CStoreButton
//

class CStoreButton : public CButton
{
public:
	CStoreButton();

	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);

	void SetStoreType(UINT StoreType);

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnMouseLeave(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	DECLARE_MESSAGE_MAP()

private:
	WCHAR m_Caption[256];
	CImageList* p_Icons;
	INT m_IconSize;
	INT m_IconID;
	BOOL m_Hover;
};

void DDX_StoreButton(CDataExchange* pDX, int nIDC, CStoreButton& rControl, UINT SourceType);
