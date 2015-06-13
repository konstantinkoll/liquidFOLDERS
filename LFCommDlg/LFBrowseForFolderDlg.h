
// LFBrowseForFolderDlg.h: Schnittstelle der Klasse LFBrowseForFolderDlg
//

#pragma once
#include "LFDialog.h"
#include "CHeaderArea.h"
#include "CExplorerTree.h"


// LFBrowseForFolderDlg
//

class LFBrowseForFolderDlg : public LFDialog
{
public:
	LFBrowseForFolderDlg(CWnd* pParentWnd=NULL, CString Caption=_T(""), CString Hint=_T(""), BOOL OnlyFSObjects=TRUE, BOOL ShowDeleteSource=FALSE, CString RootPath=_T(""));
	~LFBrowseForFolderDlg();

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void AdjustLayout();

	LPITEMIDLIST m_FolderPIDL;
	WCHAR m_FolderPath[MAX_PATH];
	BOOL m_DeleteSource;

protected:
	afx_msg BOOL OnInitDialog();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	CString m_RootPath;
	BOOL m_OnlyFSObjects;
	BOOL m_ShowDeleteSource;
	CString m_Caption;
	CString m_Hint;

private:
	CHeaderArea m_wndHeaderArea;
	CExplorerTree m_wndExplorerTree;
	CButton m_wndDeleteSource;
};
