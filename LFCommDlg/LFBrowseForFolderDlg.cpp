#include "stdafx.h"
#include "CGlasWindow.h"
#include "LFBrowseForFolderDlg.h"
#include "resource.h"


LFBrowseForFolderDlg::LFBrowseForFolderDlg(BOOL OnlyFSObjects, CWnd* pParentWnd, CString Caption, CString Hint)
	: LFDialog(IDD_BROWSEFORFOLDER, LFDS_White, pParentWnd)
{
	m_OnlyFSObjects = OnlyFSObjects;
	m_Caption = Caption;
	m_Hint = Hint;
}

void LFBrowseForFolderDlg::AdjustLayout()
{
	if (!IsWindow(m_wndExplorerTree.GetSafeHwnd()))
		return;

	CRect rect;
	GetClientRect(rect);

	UINT ExplorerHeight = 0;
	if (IsWindow(m_wndExplorerHeader))
	{
		ExplorerHeight = m_wndExplorerHeader.GetPreferredHeight();
		m_wndExplorerHeader.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);
	}

	CRect borders(0, 0, 7, 7);
	MapDialogRect(&borders);

	CRect btn;
	GetDlgItem(IDOK)->GetWindowRect(&btn);
	ScreenToClient(&btn);
	const UINT Line = btn.top-borders.Height()-2;
	m_wndExplorerTree.SetWindowPos(NULL, rect.left+borders.Width(), rect.top+ExplorerHeight, rect.Width()-borders.Width(), Line-ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(LFBrowseForFolderDlg, LFDialog)
	ON_WM_THEMECHANGED()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
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

	m_wndExplorerTree.Create(this, IDC_SHELLTREE);
	m_wndExplorerTree.SetBkColor(0xFFFFFF);
	m_wndExplorerTree.SetTextColor(0x000000);
	m_wndExplorerTree.SetFlags(SHCONTF_FOLDERS);
	m_wndExplorerTree.EnableShellContextMenu(TRUE);
	m_wndExplorerTree.SetFocus();

	OnThemeChanged();
	AdjustLayout();

	return FALSE;  // TRUE zurückgeben, wenn der Fokus nicht auf ein Steuerelement gesetzt wird
}

LRESULT LFBrowseForFolderDlg::OnThemeChanged()
{
	BOOL Themed = (p_App->m_ThemeLibLoaded) ? p_App->zIsThemeActive() : FALSE;
	m_wndExplorerHeader.SetDesign(Themed ? GWD_THEMED : GWD_DEFAULT);

	return LFDialog::OnThemeChanged();
}

void LFBrowseForFolderDlg::OnSize(UINT nType, int cx, int cy)
{
	LFDialog::OnSize(nType, cx, cy);
	AdjustLayout();
}

void LFBrowseForFolderDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	LFDialog::OnGetMinMaxInfo(lpMMI);

	lpMMI->ptMinTrackSize.x = max(lpMMI->ptMinTrackSize.x, 200);
	lpMMI->ptMinTrackSize.y = max(lpMMI->ptMinTrackSize.y, 400);
}
