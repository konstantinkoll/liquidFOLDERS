
// CExplorerList: Schnittstelle der Klasse CExplorerList
//

#pragma once
#include "liquidFOLDERS.h"
#include "LFApplication.h"
#include "LFCore.h"


// CExplorerList
//

class AFX_EXT_CLASS CExplorerList : public CListCtrl
{
public:
	CExplorerList();

	virtual void PreSubclassWindow();

	void AddCategory(INT ID, CString Name, CString Hint=_T(""), BOOL Collapsable=FALSE);
	void AddItemCategories();
	void AddColumn(INT ID, CString Name);
	void AddColumn(INT ID, UINT Attr);
	void AddStoreColumns();
	void SetSearchResult(LFSearchResult* result);
	void SetMenus(UINT _ItemMenuID=0, BOOL _HighlightFirst=FALSE, UINT _BackgroundMenuID=0);

protected:
	LFApplication* p_App;
	LFSearchResult* p_Result;
	HTHEME hTheme;
	UINT m_ItemMenuID;
	BOOL m_HighlightFirst;
	UINT m_BackgroundMenuID;

	afx_msg INT OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint pos);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	DECLARE_MESSAGE_MAP()
};
