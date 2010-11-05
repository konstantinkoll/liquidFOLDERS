
// LFEditTagsDlg.h: Schnittstelle der Klasse LFEditTagsDlg
//

#pragma once
#include "LFCore.h"
#include "LFApplication.h"
#include "CTagList.h"


// LFEditTagsDlg
//

class AFX_EXT_CLASS LFEditTagsDlg : public CDialog
{
public:
	LFEditTagsDlg(CWnd* pParentWnd, CString _Tags, CHAR* _StoreID);

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
	CHAR StoreID[LFKeySize];
	BOOL StoreIDValid;
	CEdit m_TagEdit;
	CTagList m_TagList;
};
