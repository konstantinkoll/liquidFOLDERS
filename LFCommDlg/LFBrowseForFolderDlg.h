
// LFBrowseForFolderDlg.h: Schnittstelle der Klasse LFBrowseForFolderDlg
//

#pragma once
#include "LFDialog.h"
#include "CHeaderArea.h"
#include "CShellTree.h"


// LFBrowseForFolderDlg
//

class LFBrowseForFolderDlg : public LFDialog
{
public:
	LFBrowseForFolderDlg(const CString& Caption, const CString& Hint, CWnd* pParentWnd=NULL, BOOL OnlyFSObjects=TRUE, const CString& RootPath=_T(""));

	LPITEMIDLIST m_FolderPIDL;
	WCHAR m_FolderPath[MAX_PATH];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void AdjustLayout(const CRect& rectLayout, UINT nFlags);
	virtual BOOL InitDialog();

	afx_msg void OnDestroy();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnSelectionChanged(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()

	CString m_RootPath;
	BOOL m_OnlyFSObjects;
	CString m_Caption;
	CString m_Hint;

private:
	static BOOL IsValidPath(ExplorerTreeItemData* pData);

	CHeaderArea m_wndHeaderArea;
	CShellTree m_wndExplorerTree;
};
