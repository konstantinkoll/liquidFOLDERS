
// LFBrowseForFolderDlg.cpp: Implementierung der Klasse LFBrowseForFolderDlg
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFBrowseForFolderDlg
//

LFBrowseForFolderDlg::LFBrowseForFolderDlg(CWnd* pParentWnd, CString Caption, CString Hint, BOOL OnlyFSObjects, BOOL ShowDeleteSource, CString RootPath)
	: LFDialog(IDD_BROWSEFORFOLDER, pParentWnd)
{
	m_Caption = Caption;
	m_Hint = Hint;
	m_OnlyFSObjects = OnlyFSObjects;
	m_ShowDeleteSource = ShowDeleteSource;
	m_RootPath = RootPath;
	m_FolderPIDL = NULL;
	m_FolderPath[0] = L'\0';
	m_DeleteSource = FALSE;
}

LFBrowseForFolderDlg::~LFBrowseForFolderDlg()
{
	if (m_FolderPIDL)
		LFGetApp()->GetShellManager()->FreeItem(m_FolderPIDL);
}

void LFBrowseForFolderDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_SHELLTREE, m_wndExplorerTree);
	DDX_Control(pDX, IDC_DELETESOURCE, m_wndDeleteSource);

	if (pDX->m_bSaveAndValidate)
	{
		m_FolderPIDL = LFGetApp()->GetShellManager()->CopyItem(m_wndExplorerTree.GetSelectedPIDL());
		SHGetPathFromIDList(m_FolderPIDL, m_FolderPath);
		m_DeleteSource = m_wndDeleteSource.GetCheck();
	}
}

void LFBrowseForFolderDlg::AdjustLayout()
{
	if (!IsWindow(m_wndExplorerTree))
		return;

	CRect rect;
	GetLayoutRect(rect);

	UINT ExplorerHeight = 0;
	if (IsWindow(m_wndHeaderArea))
	{
		ExplorerHeight = m_wndHeaderArea.GetPreferredHeight();
		m_wndHeaderArea.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	}

	CRect borders(0, 0, 7, 7);
	MapDialogRect(&borders);

	m_wndExplorerTree.SetWindowPos(NULL, rect.left+borders.Width(), rect.top+ExplorerHeight, rect.Width()-borders.Width(), rect.Height()-ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(LFBrowseForFolderDlg, LFDialog)
	ON_WM_GETMINMAXINFO()
	ON_NOTIFY(TVN_SELCHANGED, IDC_SHELLTREE, OnSelectionChanged)
END_MESSAGE_MAP()

BOOL LFBrowseForFolderDlg::OnInitDialog()
{
	LFDialog::OnInitDialog();

	if ((!m_Caption.IsEmpty()) || (!m_Hint.IsEmpty()))
	{
		m_wndHeaderArea.Create(this, IDC_HEADERAREA);
		m_wndHeaderArea.SetText(m_Caption, m_Hint, FALSE);
	}

	if ((LFGetApp()->m_ThemeLibLoaded) && (LFGetApp()->OSVersion>=OS_Vista))
	{
		LFGetApp()->zSetWindowTheme(m_wndExplorerTree, L"EXPLORER", NULL);
		m_wndExplorerTree.ModifyStyle(0, TVS_TRACKSELECT);
	}

	m_wndDeleteSource.ShowWindow(m_ShowDeleteSource ? SW_SHOW : SW_HIDE);
	m_wndDeleteSource.EnableWindow(m_ShowDeleteSource);
	SetBottomLeftControl(&m_wndDeleteSource);

	m_wndExplorerTree.SetOnlyFilesystem(TRUE);
	m_wndExplorerTree.PopulateTree();

	AdjustLayout();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
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

		ExplorerTreeItemData* pItem = (ExplorerTreeItemData*)pNMTreeView->itemNew.lParam;

		WCHAR path[MAX_PATH];
		GetDlgItem(IDOK)->EnableWindow(pItem ? SHGetPathFromIDList(pItem->pidlFQ, path) : FALSE);
	}
}
