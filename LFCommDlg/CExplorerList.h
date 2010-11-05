
// CExplorerList: Schnittstelle der Klasse CExplorerList
//

#pragma once
#include "liquidFOLDERS.h"
#include "LFApplication.h"
#include "LFCore.h"
#include "IListViewFooter.h"


// CExplorerList
//

#define LVSIL_FOOTER                4

class AFX_EXT_CLASS CExplorerList : public CListCtrl
{
public:
	CExplorerList();

	void AddCategory(INT ID, CString name, CString hint=_T(""));
	void AddItemCategories();
	void AddColumn(INT ID, CString name);
	void AddColumn(INT ID, UINT attr);
	void AddStoreColumns();
	void SetSearchResult(LFSearchResult* result);
	void SetMenus(UINT _ItemMenuID=0, BOOL _HighlightFirst=FALSE, UINT _BackgroundMenuID=0);
	BOOL SupportsFooter();
	void ShowFooter(IListViewFooterCallback* pCallbackObject);
	void RemoveFooter();
	void SetFooterText(LPCWSTR pText);
	void InsertFooterButton(INT insertAt, LPCWSTR pText, LPCWSTR pUnknown, UINT iconIndex, LONG lParam);

protected:
	LFApplication* p_App;
	LFSearchResult* p_Result;
	IListViewFooter* p_FooterHandler;
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
