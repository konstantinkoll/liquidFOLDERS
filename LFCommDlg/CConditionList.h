
// CConditionList: Schnittstelle der Klasse CConditionList
//

#pragma once
#include "liquidFOLDERS.h"
#include "LFApplication.h"
#include "CImageListTransparent.h"


// CConditionList
//

class AFX_EXT_CLASS CConditionList : public CListCtrl
{
public:
	CConditionList();

	virtual void PreSubclassWindow();
	virtual BOOL SetWindowPos(const CWnd* pWndInsertAfter, INT x, INT y, INT cx, INT cy, UINT nFlags);

	void InsertItem(LFFilterCondition* c);
	void SetItem(INT nItem, LFFilterCondition* c);
	void SetMenus(UINT ItemMenuID=0, BOOL HighlightFirst=FALSE, UINT BackgroundMenuID=0);

protected:
	LFApplication* p_App;
	HTHEME hTheme;
	UINT m_ItemMenuID;
	BOOL m_HighlightFirst;
	UINT m_BackgroundMenuID;
	INT m_LastWidth;

	virtual void Init();

	void SetTileSize(INT cx);

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

private:
	CImageListTransparent m_AttributeIcons16;
	CImageListTransparent m_AttributeIcons32;
	CString m_Compare[LFFilterCompareCount];

	void ConditionToItem(LFFilterCondition* c, LVITEM& lvi);
	void FinishItem(INT idx, LFFilterCondition* c);
};
