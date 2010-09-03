
// CExplorerTree.cpp: Implementierung der Klasse CExplorerTree
//

#include "stdafx.h"
#include "CExplorerTree.h"
#include "LFApplication.h"


// CExplorerTree
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

CExplorerTree::CExplorerTree()
	: CMFCShellTreeCtrl()
{
}

BOOL CExplorerTree::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP | TVS_HASBUTTONS | TVS_NOTOOLTIPS;
	CRect rect;
	rect.SetRectEmpty();
	return CMFCShellTreeCtrl::Create(dwStyle, rect, pParentWnd, nID);
}


BEGIN_MESSAGE_MAP(CExplorerTree, CMFCShellTreeCtrl)
	ON_WM_CREATE()
END_MESSAGE_MAP()

int CExplorerTree::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMFCShellTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	LFApplication* pApp = (LFApplication*)AfxGetApp();
	if ((pApp->m_ThemeLibLoaded) && (pApp->OSVersion>=OS_Vista))
		pApp->zSetWindowTheme(GetSafeHwnd(), L"explorer", NULL);

	if (pApp->OSVersion==OS_XP)
		ModifyStyle(0, TVS_HASLINES);

	LOGFONT lf;
	pApp->m_DefaultFont.GetLogFont(&lf);
	SetItemHeight((SHORT)(max(abs(lf.lfHeight), GetSystemMetrics(SM_CYSMICON))+(pApp->OSVersion<OS_Vista ? 2 : 6)));

	return 0;
}
