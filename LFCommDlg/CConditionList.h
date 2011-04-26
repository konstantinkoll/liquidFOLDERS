
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

	void SetMenus(UINT _ItemMenuID=0, BOOL _HighlightFirst=FALSE, UINT _BackgroundMenuID=0);

protected:
	LFApplication* p_App;
	HTHEME hTheme;
	UINT m_ItemMenuID;
	BOOL m_HighlightFirst;
	UINT m_BackgroundMenuID;

	virtual void Init();

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()

private:
	CImageListTransparent m_AttributeIcons16;
	CImageListTransparent m_AttributeIcons32;
};
