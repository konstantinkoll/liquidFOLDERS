#pragma once
#include "LFDialog.h"
#include "CExplorerHeader.h"
#include "CExplorerTree.h"

class AFX_EXT_CLASS LFBrowseForFolderDlg : public LFDialog
{
public:
	LFBrowseForFolderDlg(CWnd* pParentWnd=NULL);

protected:
	void AdjustLayout();

	afx_msg BOOL OnInitDialog();
	afx_msg LRESULT OnThemeChanged();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	DECLARE_MESSAGE_MAP()

private:
	CExplorerHeader m_wndExplorerHeader;
	CExplorerTree m_wndExplorerTree;
};
