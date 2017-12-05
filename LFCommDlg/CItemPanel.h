
// CItemPanel.h: Schnittstelle der Klasse CItemPanel
//

#pragma once
#include "CFrontstageWnd.h"
#include "LFCore.h"


// CItemPanel
//

class CItemPanel : public CFrontstageWnd
{
public:
	CItemPanel();

	void Reset();
	void SetItem(CString Text, CImageList* pIcons=NULL, INT nID=-1, BOOL Ghosted=FALSE);
	BOOL SetItem(const LPCSTR pStoreID);
	BOOL SetItem(LPITEMIDLIST pidlFQ, LPCWSTR Path=NULL, UINT nID=0, LPCWSTR Hint=NULL);
	BOOL SetItem(LPCWSTR Path, UINT nID=0, LPCWSTR Hint=NULL);

protected:
	virtual INT ItemAtPosition(CPoint point) const;
	virtual void InvalidateItem(INT Index);
	virtual void ShowTooltip(const CPoint& point);

	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

	CSize m_IconSize;
	CImageList* p_Icons;
	INT m_IconID;
	BOOL m_Ghosted;
	CString m_Text;
	UINT m_Lines;
};
