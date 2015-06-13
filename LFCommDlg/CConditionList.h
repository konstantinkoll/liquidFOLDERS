
// CConditionList: Schnittstelle der Klasse CConditionList
//

#pragma once
#include "LF.h"
#include "CImageListTransparent.h"


// CConditionList
//

class CConditionList : public CListCtrl
{
public:
	CConditionList();

	virtual void PreSubclassWindow();
	virtual BOOL SetWindowPos(const CWnd* pWndInsertAfter, INT x, INT y, INT cx, INT cy, UINT nFlags);

	void InsertItem(LFFilterCondition* c);
	void SetItem(INT nItem, LFFilterCondition* c);
	void SetMenus(UINT BackgroundMenuID=0);

protected:
	virtual void Init();

	void SetTileSize(INT cx);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

	HTHEME hTheme;
	UINT m_BackgroundMenuID;
	INT m_LastWidth;

private:
	void ConditionToItem(LFFilterCondition* c, LVITEM& lvi);
	void FinishItem(INT Index, LFFilterCondition* c);

	CImageListTransparent m_AttributeIcons16;
	CImageListTransparent m_AttributeIcons32;
	CString m_Compare[LFFilterCompareCount];
};
