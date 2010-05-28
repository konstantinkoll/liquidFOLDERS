#pragma once
#include "LFCore.h"
#include "liquidFOLDERS.h"
#include "CExplorerList.h"
#include <list>

class AFX_EXT_CLASS LFChooseDefaultStoreDlg : public CDialog
{
public:
	LFChooseDefaultStoreDlg(CWnd* pParentWnd);
	virtual ~LFChooseDefaultStoreDlg();

	char StoreID[LFKeySize];

protected:
	HICON m_icStore;
	HICON m_icDefaultStore;
	CExplorerList m_List;
	CImageList* m_Icons;
	LFSearchResult* result;

	virtual void DoDataExchange(CDataExchange* pDX);

	void AddColumn(CListCtrl* l, UINT attr, UINT no);

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDestroy();
	afx_msg LRESULT OnThemeChanged();
	afx_msg LRESULT UpdateStores(WPARAM wParam, LPARAM lParam);
	afx_msg void OnGetDispInfo(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void LFChooseDefaultStoreDlg::OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNewStore();
	DECLARE_MESSAGE_MAP()

private:
	wchar_t m_StrBuffer[256];
};
