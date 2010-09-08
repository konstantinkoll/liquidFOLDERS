
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

	void EnableTheming();
	void AddCategory(int ID, CString name, CString hint=_T(""));
	void AddItemCategories();
	void AddColumn(int ID, CString name);
	void AddColumn(int ID, UINT attr);
	void AddStoreColumns();
	void SetSearchResult(LFSearchResult* result);
	BOOL SupportsFooter();
	void ShowFooter(IListViewFooterCallback* pCallbackObject);
	void RemoveFooter();
	void SetFooterText(LPCWSTR pText);
	void InsertFooterButton(int insertAt, LPCWSTR pText, LPCWSTR pUnknown, UINT iconIndex, LONG lParam);

protected:
	LFApplication* p_App;
	IListViewFooter* p_FooterHandler;
	HTHEME hTheme;

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	DECLARE_MESSAGE_MAP()
};
