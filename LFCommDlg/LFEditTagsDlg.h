#pragma once
#include "LFCore.h"
#include "LFApplication.h"
#include "CTagList.h"

class AFX_EXT_CLASS LFEditTagsDlg : public CDialog
{
public:
	LFEditTagsDlg(CWnd* pParentWnd, CString _Tags, char* _StoreID);
	virtual ~LFEditTagsDlg();

	virtual void DoDataExchange(CDataExchange* pDX);

	CString m_Tags;

protected:
	LFApplication* p_App;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddTags();
	DECLARE_MESSAGE_MAP()

private:
	char StoreID[LFKeySize];
	BOOL StoreIDValid;
	CEdit m_TagEdit;
	CTagList m_TagList;
};
