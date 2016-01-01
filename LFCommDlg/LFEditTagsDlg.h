
// LFEditTagsDlg.h: Schnittstelle der Klasse LFEditTagsDlg
//

#pragma once
#include "CTagList.h"


// LFEditTagsDlg
//

class LFEditTagsDlg : public CDialog
{
public:
	LFEditTagsDlg(CString Tags, CWnd* pParentWnd=NULL, CHAR* StoreID=NULL);

	CString m_Tags;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	afx_msg void OnDoubleClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnAddTags();
	DECLARE_MESSAGE_MAP()

private:
	CHAR m_StoreID[LFKeySize];
	BOOL m_StoreIDValid;
	CEdit m_TagEdit;
	CTagList m_TagList;
};
