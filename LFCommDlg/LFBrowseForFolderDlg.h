#pragma once
#include "LFDialog.h"
#include "CExplorerHeader.h"
#include "CExplorerTree.h"

class AFX_EXT_CLASS LFBrowseForFolderDlg : public LFDialog
{
public:
	LFBrowseForFolderDlg(BOOL OnlyFSObjects=TRUE, CWnd* pParentWnd=NULL, CString Caption=_T(""), CString Hint=_T(""));
	~LFBrowseForFolderDlg();

	virtual void DoDataExchange(CDataExchange* pDX);

	LPITEMIDLIST m_FolderPIDL;

protected:
	LFApplication* p_App;
	BOOL m_OnlyFSObjects;
	CString m_Caption;
	CString m_Hint;

	void AdjustLayout();

	afx_msg BOOL OnInitDialog();
	afx_msg LRESULT OnThemeChanged();
	DECLARE_MESSAGE_MAP()

private:
	CExplorerHeader m_wndExplorerHeader;
	CExplorerTree m_wndExplorerTree;
};
