
// LFBrowseForFolderDlg.cpp: Implementierung der Klasse LFBrowseForFolderDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFBrowseForFolderDlg
//

LFBrowseForFolderDlg::LFBrowseForFolderDlg(const CString& Caption, const CString& Hint, CWnd* pParentWnd, BOOL OnlyFSObjects, const CString& RootPath)
	: LFDialog(IDD_BROWSEFORFOLDER, pParentWnd)
{
	m_Caption = Caption;
	m_Hint = Hint;
	m_OnlyFSObjects = OnlyFSObjects;
	m_RootPath = RootPath;
	m_FolderPIDL = NULL;
	m_FolderPath[0] = L'\0';
}

void LFBrowseForFolderDlg::DoDataExchange(CDataExchange* pDX)
{
	if (pDX->m_bSaveAndValidate)
	{
		m_FolderPIDL = LFGetApp()->GetShellManager()->CopyItem(m_wndExplorerTree.GetSelectedPIDL());
		SHGetPathFromIDList(m_FolderPIDL, m_FolderPath);
	}
}

void LFBrowseForFolderDlg::AdjustLayout(const CRect& rectLayout, UINT nFlags)
{
	LFDialog::AdjustLayout(rectLayout, nFlags);

	UINT ExplorerHeight = 0;
	if (IsWindow(m_wndHeaderArea))
	{
		ExplorerHeight = m_wndHeaderArea.GetPreferredHeight();
		m_wndHeaderArea.SetWindowPos(NULL, rectLayout.left, rectLayout.top, rectLayout.Width(), ExplorerHeight, nFlags);
	}

	if (IsWindow(m_wndExplorerTree))
	{
		const INT BorderLeft = (LFGetApp()->OSVersion==OS_XP) ? BACKSTAGEBORDER-3 : BACKSTAGEBORDER-2;
		m_wndExplorerTree.SetWindowPos(NULL, rectLayout.left+BorderLeft, rectLayout.top+ExplorerHeight, rectLayout.Width()-BorderLeft, m_BottomDivider-rectLayout.top-ExplorerHeight, nFlags);
	}
}

BOOL LFBrowseForFolderDlg::InitDialog()
{
	// Header
	if (!m_Caption.IsEmpty() || !m_Hint.IsEmpty())
	{
		m_wndHeaderArea.Create(this, IDC_HEADERAREA);
		m_wndHeaderArea.SetHeader(m_Caption, m_Hint, NULL, CPoint(0, 0), FALSE);
	}

	// Explorer tree
	m_wndExplorerTree.Create(this, IDC_SHELLTREE);

	if (LFGetApp()->m_ThemeLibLoaded && (LFGetApp()->OSVersion>=OS_Vista))
	{
		LFGetApp()->zSetWindowTheme(m_wndExplorerTree, L"EXPLORER", NULL);
		m_wndExplorerTree.ModifyStyle(0, TVS_TRACKSELECT);
	}

	m_wndExplorerTree.SetOnlyFilesystem(TRUE);
	m_wndExplorerTree.PopulateTree();
	m_wndExplorerTree.SetFocus();

	return FALSE;
}


BEGIN_MESSAGE_MAP(LFBrowseForFolderDlg, LFDialog)
	ON_WM_DESTROY()
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(TVN_SELCHANGED, IDC_SHELLTREE, OnSelectionChanged)
END_MESSAGE_MAP()

void LFBrowseForFolderDlg::OnDestroy()
{
	if (m_FolderPIDL)
		LFGetApp()->GetShellManager()->FreeItem(m_FolderPIDL);

	LFDialog::OnDestroy();
}

void LFBrowseForFolderDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	LFDialog::OnGetMinMaxInfo(lpMMI);

	lpMMI->ptMinTrackSize.x = max(lpMMI->ptMinTrackSize.x, 350);
	lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, 300);
}

void LFBrowseForFolderDlg::OnSelectionChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	if (m_OnlyFSObjects)
	{
		NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

		BOOL Enable = FALSE;

		ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)pNMTreeView->itemNew.lParam;
		if (pItem)
		{
			WCHAR Path[MAX_PATH];
			if (SHGetPathFromIDList(pItem->pidlFQ, Path))
				Enable = (wcslen(Path)>3);
		}

		GetDlgItem(IDOK)->EnableWindow(Enable);
	}
}
