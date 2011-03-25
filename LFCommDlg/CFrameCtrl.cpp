
// CFrameCtrl.cpp: Implementierung der Klasse CFrameCtrl
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CFrameCtrl
//

CFrameCtrl::CFrameCtrl()
	: CWnd()
{
}

BOOL CFrameCtrl::Create(CWnd* pParentWnd, CRect rect)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	return CWnd::Create(className, _T(""), WS_CHILD | WS_CLIPCHILDREN | WS_VISIBLE, rect, pParentWnd, 0);
}


BEGIN_MESSAGE_MAP(CFrameCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
END_MESSAGE_MAP()

BOOL CFrameCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CFrameCtrl::OnNcPaint()
{
	DrawControlBorder(this);
}
