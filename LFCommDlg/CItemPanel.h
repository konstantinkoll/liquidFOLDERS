
// CItemPanel.h: Schnittstelle der Klasse CItemPanel
//

#pragma once
#include "LFCore.h"


// CItemPanel
//

class CItemPanel : public CWnd
{
public:
	CItemPanel();

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	void Empty();
	void SetItem(CString Text, CImageList* pIcons=NULL, INT nID=-1, BOOL Ghosted=FALSE);
	BOOL SetItem(const CHAR* pStoreID);
	BOOL SetItem(LPITEMIDLIST pidlFQ, LPCWSTR Path=NULL, UINT nID=0, LPCWSTR Hint=NULL);
	BOOL SetItem(LPCWSTR Path, UINT nID=0, LPCWSTR Hint=NULL);

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnMouseLeave();
	afx_msg void OnMouseHover(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()

	CSize m_IconSize;
	CImageList* p_Icons;
	INT m_IconID;
	BOOL m_Ghosted;
	CString m_Text;
	UINT m_Lines;

private:
	BOOL m_Hover;
};
