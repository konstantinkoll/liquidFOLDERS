
// LFBrowseForFolderDlg.cpp: Implementierung der Klasse LFBrowseForFolderDlg
//

#include "stdafx.h"
#include "CGlasWindow.h"
#include "LFBrowseForFolderDlg.h"
#include "resource.h"


// LFBrowseForFolderDlg
//

LFBrowseForFolderDlg::LFBrowseForFolderDlg(BOOL OnlyFSObjects, CString RootPath, CWnd* pParentWnd, CString Caption, CString Hint)
	: LFDialog(IDD_BROWSEFORFOLDER, LFDS_White, pParentWnd)
{
	p_App = (LFApplication*)AfxGetApp();
	m_OnlyFSObjects = OnlyFSObjects;
	m_RootPath = RootPath;
	m_Caption = Caption;
	m_Hint = Hint;
	m_FolderPIDL = NULL;
	m_FolderPath[0] = L'\0';
}

LFBrowseForFolderDlg::~LFBrowseForFolderDlg()
{
	if (m_FolderPIDL)
		p_App->GetShellManager()->FreeItem(m_FolderPIDL);
}

void LFBrowseForFolderDlg::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_SHELLTREE, m_wndExplorerTree);

	if (pDX->m_bSaveAndValidate)
	{
		m_FolderPIDL = p_App->GetShellManager()->CopyItem(m_wndExplorerTree.GetSelectedPIDL());
		SHGetPathFromIDList(m_FolderPIDL, m_FolderPath);
	}
}

void LFBrowseForFolderDlg::AdjustLayout()
{
	if (!IsWindow(m_wndExplorerTree))
		return;

	CRect rect;
	GetLayoutRect(rect);

	UINT ExplorerHeight = 0;
	if (IsWindow(m_wndExplorerHeader))
	{
		ExplorerHeight = m_wndExplorerHeader.GetPreferredHeight();
		m_wndExplorerHeader.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);
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
		m_wndExplorerHeader.Create(this, IDC_EXPLORERHEADER);
		m_wndExplorerHeader.SetText(m_Caption, m_Hint, FALSE);
		m_wndExplorerHeader.SetLineStyle(FALSE, FALSE);
	}

	if ((p_App->m_ThemeLibLoaded) && (p_App->OSVersion>=OS_Vista))
	{
		p_App->zSetWindowTheme(m_wndExplorerTree, L"EXPLORER", NULL);
		m_wndExplorerTree.ModifyStyle(0, TVS_TRACKSELECT);
	}

	LOGFONT lf;
	p_App->m_DefaultFont.GetLogFont(&lf);
	m_wndExplorerTree.SetItemHeight((SHORT)(max(abs(lf.lfHeight), GetSystemMetrics(SM_CYSMICON))+(p_App->OSVersion<OS_Vista ? 2 : 6)));
	m_wndExplorerTree.SetFont(&p_App->m_DefaultFont, FALSE);
	m_wndExplorerTree.SetOnlyFilesystem(TRUE);
	m_wndExplorerTree.PopulateTree();

	AdjustLayout();

	return TRUE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

void LFBrowseForFolderDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	LFDialog::OnGetMinMaxInfo(lpMMI);

	lpMMI->ptMinTrackSize.x = max(lpMMI->ptMinTrackSize.x, 200);
	lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, 300);
}

void LFBrowseForFolderDlg::OnSelectionChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	if (m_OnlyFSObjects)
	{
		NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;

		LPAFX_SHELLITEMINFO pInfo = (LPAFX_SHELLITEMINFO)pNMTreeView->itemNew.lParam;

		TCHAR path[MAX_PATH];
		GetDlgItem(IDOK)->EnableWindow(SHGetPathFromIDList(pInfo->pidlFQ, path));
	}
}
