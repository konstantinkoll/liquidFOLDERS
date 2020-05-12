
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

		VERIFY(SHGetPathFromIDList(m_FolderPIDL, m_FolderPath));
	}
}

BOOL LFBrowseForFolderDlg::IsValidPath(ExplorerTreeItemData* pData)
{
	// Check item data
	if (!pData)
		return FALSE;

	// Get path
	WCHAR Path[MAX_PATH];
	if (!SHGetPathFromIDList(pData->pidlFQ, Path))
		return FALSE;

	// Check volume type
	ASSERT((Path[0]>=L'A') && (Path[0]<='Z'));
	if ((LFGetLogicalVolumes() & (1<<(Path[0]-L'A')))==0)
		return FALSE;

	// Subfolder
	if (wcslen(Path)>3)
		return TRUE;

	// Check for boot volume
	WCHAR WindowsDirectory[MAX_PATH];
	UINT szPath = GetWindowsDirectory(WindowsDirectory, MAX_PATH);
	if (!szPath || (szPath>MAX_PATH-1))
		return FALSE;

	// Only allow root directory on non-boot volumes
	return Path[0]!=WindowsDirectory[0];
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

		GetDlgItem(IDOK)->EnableWindow(IsValidPath((ExplorerTreeItemData*)pNMTreeView->itemNew.lParam));
	}
}
