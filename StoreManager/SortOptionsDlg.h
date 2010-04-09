#pragma once
#include "StoreManager.h"
#include "CAttributeListDialog.h"

class SortOptionsDlg : public CAttributeListDialog
{
public:
	SortOptionsDlg(CWnd* pParent, LFViewParameters* _view, int _context, BOOL _IsClipboard);
	virtual ~SortOptionsDlg();

	int context;
	BOOL IsClipboard;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	LFViewParameters* view;

	afx_msg BOOL OnInitDialog();
	afx_msg void SetAttrGroupBox();
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};
