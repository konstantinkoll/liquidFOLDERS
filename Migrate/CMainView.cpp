
// CMainView.cpp: Implementierung der Klasse CMainView
//

#include "stdafx.h"
#include "CMainView.h"
#include "Resource.h"
#include "Migrate.h"
#include "LFCore.h"


// CMainView
//

CMainView::CMainView()
{
	m_IsRootSet = FALSE;
}

int CMainView::Create(CWnd* _pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), dwStyle, rect, _pParentWnd, nID);
}

BOOL CMainView::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	return GetParent()->OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

void CMainView::ClearRoot()
{
	m_IsRootSet = m_SelectedCanExpand = m_SelectedHasPropSheet = m_SelectedCanRename = m_SelectedCanDelete = FALSE;

	CString caption;
	CString hint;
	ENSURE(caption.LoadString(IDS_NOROOT_CAPTION));
	ENSURE(hint.LoadString(IDS_NOROOT_HINT));

	m_wndExplorerHeader.SetText(caption, hint);

	m_wndTree.ClearRoot();
	m_wndTree.EnableWindow(FALSE);
}

void CMainView::SetRoot(LPITEMIDLIST pidl, BOOL Update, BOOL ExpandAll)
{
	m_IsRootSet = TRUE;
	m_SelectedCanExpand = m_SelectedHasPropSheet = m_SelectedCanRename = m_SelectedCanDelete = FALSE;

	CString caption;
	CString hint;
	SHFILEINFO sfi;
	if (SUCCEEDED(SHGetFileInfo((wchar_t*)pidl, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_TYPENAME)))
	{
		caption = sfi.szDisplayName;
		hint = sfi.szTypeName;
	}

	m_wndExplorerHeader.SetText(caption, hint);

	m_wndTree.SetRoot(pidl, Update, ExpandAll);
	m_wndTree.EnableWindow(TRUE);
}

void CMainView::PopulateMigrationList(CMigrationList* ml, LFItemDescriptor* it)
{
	if (m_IsRootSet)
		m_wndTree.PopulateMigrationList(ml, it);
}

void CMainView::UncheckMigrated(CReportList* rl)
{
	if (m_IsRootSet)
		m_wndTree.UncheckMigrated(rl);
}

BOOL CMainView::FoldersChecked()
{
	return m_IsRootSet ? m_wndTree.FoldersChecked() : FALSE;
}

void CMainView::AdjustLayout()
{
	CRect rect;
	GetClientRect(rect);

	const UINT TaskHeight = m_wndTaskbar.GetPreferredHeight();
	m_wndTaskbar.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	const UINT ExplorerHeight = m_wndExplorerHeader.GetPreferredHeight();
	m_wndExplorerHeader.SetWindowPos(NULL, rect.left, rect.top+TaskHeight, rect.Width(), ExplorerHeight, SWP_NOACTIVATE | SWP_NOZORDER);

	m_wndTree.SetWindowPos(NULL, rect.left, rect.top+TaskHeight+ExplorerHeight, rect.Width(), rect.Height()-ExplorerHeight-TaskHeight, SWP_NOACTIVATE | SWP_NOZORDER);
}


BEGIN_MESSAGE_MAP(CMainView, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_VIEW_AUTOSIZEALL, OnAutosizeAll)
	ON_COMMAND(ID_VIEW_SELECTROOT, OnSelectRoot)
	ON_COMMAND(ID_VIEW_SELECTROOT_TASKBAR, OnSelectRoot)
	ON_COMMAND(ID_VIEW_EXPAND, OnExpand)
	ON_COMMAND(ID_VIEW_OPEN, OnOpen)
	ON_COMMAND(ID_VIEW_RENAME, OnRename)
	ON_COMMAND(ID_VIEW_DELETE, OnDelete)
	ON_COMMAND(ID_VIEW_PROPERTIES, OnProperties)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_SELECTROOT, ID_VIEW_PROPERTIES, OnUpdateTaskbar)
	ON_WM_CONTEXTMENU()
	ON_NOTIFY(TVN_SELCHANGED, 3, OnSelectionChanged)
END_MESSAGE_MAP()

int CMainView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Task bar
	if (!m_wndTaskbar.Create(this, IDB_TASKS, 1))
		return -1;

	m_wndTaskbar.AddButton(ID_VIEW_SELECTROOT_TASKBAR, 0);
	m_wndTaskbar.AddButton(ID_VIEW_EXPAND, 1);
	m_wndTaskbar.AddButton(ID_VIEW_OPEN, 2, TRUE);
	m_wndTaskbar.AddButton(ID_VIEW_RENAME, 3);
	m_wndTaskbar.AddButton(ID_VIEW_DELETE, 4);
	m_wndTaskbar.AddButton(ID_VIEW_PROPERTIES, 5);
	m_wndTaskbar.AddButton(ID_APP_NEWSTOREMANAGER, 6, TRUE);

	m_wndTaskbar.AddButton(ID_APP_PURCHASE, 7, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ENTERLICENSEKEY, 8, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_SUPPORT, 9, TRUE, TRUE);
	m_wndTaskbar.AddButton(ID_APP_ABOUT, 10, TRUE, TRUE);

	// Explorer header
	if (!m_wndExplorerHeader.Create(this, 2))
		return -1;

	// Tree
	if (!m_wndTree.Create(this, 3))
		return -1;

	ClearRoot();
	return 0;
}

BOOL CMainView::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMainView::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CMainView::OnSetFocus(CWnd* /*pOldWnd*/)
{
	if (m_wndTree.IsWindowEnabled())
	{
		m_wndTree.SetFocus();
	}
	else
	{
		m_wndTaskbar.SetFocus();
	}
}

void CMainView::OnAutosizeAll()
{
	m_wndTree.AutosizeColumns();
}

void CMainView::OnSelectRoot()
{
	GetOwner()->SendMessage(WM_COMMAND, ID_VIEW_SELECTROOT);
}

void CMainView::OnExpand()
{
	m_wndTree.ExpandFolder();
}

void CMainView::OnOpen()
{
	m_wndTree.OpenFolder();
}

void CMainView::OnRename()
{
	m_wndTree.EditLabel();
}

void CMainView::OnDelete()
{
	m_wndTree.DeleteFolder();
}

void CMainView::OnProperties()
{
	m_wndTree.ShowProperties();
}

void CMainView::OnUpdateTaskbar(CCmdUI* pCmdUI)
{
	switch (pCmdUI->m_nID)
	{
	case ID_VIEW_SELECTROOT:
		pCmdUI->Enable(TRUE);
		break;
	case ID_VIEW_SELECTROOT_TASKBAR:
		pCmdUI->Enable(!m_IsRootSet);
		break;
	case ID_VIEW_EXPAND:
		pCmdUI->Enable(m_IsRootSet && m_SelectedCanExpand);
		break;
	case ID_VIEW_RENAME:
		pCmdUI->Enable(m_IsRootSet && m_SelectedCanRename);
		break;
	case ID_VIEW_DELETE:
		pCmdUI->Enable(m_IsRootSet && m_SelectedCanDelete);
		break;
	case ID_VIEW_PROPERTIES:
		pCmdUI->Enable(m_IsRootSet && m_SelectedHasPropSheet);
		break;
	default:
		pCmdUI->Enable(m_IsRootSet);
	}
}

void CMainView::OnContextMenu(CWnd* /*pWnd*/, CPoint pos)
{
	if ((pos.x==-1) && (pos.y==-1))
	{
		pos.x = pos.y = 0;
		ClientToScreen(&pos);
	}

	CMenu menu;
	ENSURE(menu.LoadMenu(IDM_BACKGROUND));

	CMenu* popup = menu.GetSubMenu(0);
	ASSERT(popup);

	if (!m_IsRootSet)
		popup->EnableMenuItem(ID_VIEW_AUTOSIZEALL, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

	popup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, this, NULL);
}

void CMainView::OnSelectionChanged(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	tagTreeView* pNMTreeView = (tagTreeView*)pNMHDR;

	if (pNMTreeView->pCell)
	{
		m_SelectedCanExpand = (pNMTreeView->pCell->Flags & CF_CANEXPAND);
		m_SelectedHasPropSheet = (pNMTreeView->pCell->Flags & CF_HASPROPSHEET);
		m_SelectedCanRename = (pNMTreeView->pCell->Flags & CF_CANRENAME);
		m_SelectedCanDelete = (pNMTreeView->pCell->Flags & CF_CANDELETE);

		return;
	}

	m_SelectedCanExpand = m_SelectedHasPropSheet = m_SelectedCanRename = m_SelectedCanDelete = FALSE;
}
