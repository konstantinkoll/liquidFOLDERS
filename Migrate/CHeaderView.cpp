
// CHeaderView.cpp: Implementierung der Klasse CHeaderView
//

#include "stdafx.h"
#include "CHeaderView.h"
#include "Migrate.h"
#include "LFCore.h"


// CHeaderView
//

CHeaderView::CHeaderView()
{
#ifdef _DEBUG
	for (UINT a=0; a<cAttrColumns; a++)
		widths[a] = 100;
#else
	ZeroMemory(widths, sizeof(widths));
#endif
}

int CHeaderView::Create(CWnd* _pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	rect.SetRectEmpty();
	int res = CWnd::Create(className, _T("Header"), dwStyle, rect, _pParentWnd, nID);

	rect.top = 1;
	rect.bottom = 20;
	rect.right = -1;

	CFont* fnt = CFont::FromHandle((HFONT)GetStockObject(DEFAULT_GUI_FONT));

	for (UINT a=0; a<cAttrColumns; a++)
	{
		rect.left = rect.right+18;
		rect.right = rect.left+widths[a]-18;
		cbxs[a].Create(dwStyle | CBS_DROPDOWNLIST | CBS_SORT, rect, this, a);
		cbxs[a].SetFont(fnt);
		cbxs[a].AddString(L"Test");
	}

	return res;
}

int CHeaderView::GetPreferredHeight()
{
	CRect rectCombo;
	cbxs[0].GetWindowRect(rectCombo);

	return rectCombo.Height()+2;
}

BEGIN_MESSAGE_MAP(CHeaderView, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_SETCURSOR()
END_MESSAGE_MAP()

BOOL CHeaderView::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	pDC->FillSolidRect(rect, GetSysColor(COLOR_WINDOW));

	return TRUE;
}

BOOL CHeaderView::OnSetCursor(CWnd* /*pWnd*/, UINT /*nHitTest*/, UINT /*message*/)
{
	SetCursor(LoadCursor(NULL, IDC_ARROW));
	return TRUE;
}
