
// LFBrowseForFolderDlg.h: Schnittstelle der Klasse LFBrowseForFolderDlg
//

#pragma once
#include "LFDialog.h"
#include "CExplorerHeader.h"
#include "CExplorerTree.h"


// LFBrowseForFolderDlg
//

class AFX_EXT_CLASS LFBrowseForFolderDlg : public LFDialog
{
public:
	LFBrowseForFolderDlg(BOOL OnlyFSObjects=TRUE, BOOL ShowDeleteSource=FALSE, CString RootPath=_T(""), CWnd* pParentWnd=NULL, CString Caption=_T(""), CString Hint=_T(""));
	~LFBrowseForFolderDlg();

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void AdjustLayout();

	LPITEMIDLIST m_FolderPIDL;
	WCHAR m_FolderPath[MAX_PATH];
	BOOL m_DeleteSource;

protected:
	CString m_RootPath;
	BOOL m_OnlyFSObjects;
	BOOL m_ShowDeleteSource;
	CString m_Caption;
	CString m_Hint;

	afx_msg BOOL OnInitDialog();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

private:
	CExplorerHeader m_wndExplorerHeader;
	CExplorerTree m_wndExplorerTree;
	CButton m_wndDeleteSource;
};
