#pragma once
#include "StoreManager.h"
#include "CAttributeListDialog.h"

class ChooseDetailsDlg : public CAttributeListDialog
{
public:
	ChooseDetailsDlg(CWnd* pParentWnd, LFViewParameters* _view, int _context);
	virtual ~ChooseDetailsDlg();

	UINT RibbonColor;
	int context;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	LFViewParameters* view;
	CListCtrl* ShowAttributes;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnSelectionChange(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMoveUp();
	afx_msg void OnMoveDown();
	afx_msg void OnCheckAll();
	afx_msg void OnUncheckAll();
	DECLARE_MESSAGE_MAP()

protected:
	void SwapItems(int FocusItem, int NewPos);
};
