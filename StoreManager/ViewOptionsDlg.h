#pragma once
#include "StoreManager.h"
#include "CAttributeListDialog.h"

class ViewOptionsDlg : public CAttributeListDialog
{
public:
	ViewOptionsDlg(CWnd* pParentWnd, UINT _RibbonColor, LFViewParameters* _view, int _context, LFSearchResult* files);
	virtual ~ViewOptionsDlg();

	UINT RibbonColor;
	int context;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	CImageListTransparent* m_pViewIcons;
	LFViewParameters* view;
	BOOL HasCategories;
	CButton* ShowCategories;
	CListCtrl* ShowAttributes;
	CString BackgroundText;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnViewModeChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnCheckAll();
	afx_msg void OnUncheckAll();
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
