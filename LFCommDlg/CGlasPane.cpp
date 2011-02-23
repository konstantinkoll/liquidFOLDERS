
// CGlasPane.cpp: Implementierung der Klasse CGlasPane
//

#include "stdafx.h"
#include "CGlasPane.h"


// CGlasPane
//

CGlasPane::CGlasPane()
	: CWnd()
{
}

BOOL CGlasPane::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::CreateEx(WS_EX_CONTROLPARENT, className, _T(""), dwStyle, rect, pParentWnd, nID);
}


BEGIN_MESSAGE_MAP(CGlasPane, CWnd)
END_MESSAGE_MAP()
