
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
	LFBrowseForFolderDlg(BOOL OnlyFSObjects=TRUE, CString RootPath=_T(""), CWnd* pParentWnd=NULL, CString Caption=_T(""), CString Hint=_T(""));
	~LFBrowseForFolderDlg();

	virtual void DoDataExchange(CDataExchange* pDX);

	LPITEMIDLIST m_FolderPIDL;
	TCHAR m_FolderPath[MAX_PATH];

protected:
	LFApplication* p_App;
	CString m_RootPath;
	BOOL m_OnlyFSObjects;
	CString m_Caption;
	CString m_Hint;

	void AdjustLayout();

	afx_msg BOOL OnInitDialog();
	afx_msg void OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

private:
	CExplorerHeader m_wndExplorerHeader;
	CExplorerTree m_wndExplorerTree;
};
