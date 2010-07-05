#pragma once
#include "LFCore.h"
#include "LFApplication.h"
#include "CTagList.h"

class AFX_EXT_CLASS LFEditTagsDlg : public CDialog
{
public:
	LFEditTagsDlg(CWnd* pParentWnd, CString _Tags);
	virtual ~LFEditTagsDlg();

	virtual void DoDataExchange(CDataExchange* pDX);

	CString m_Tags;

protected:
	LFApplication* p_App;

	afx_msg BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()

private:
	CTagList m_TagList;
};
