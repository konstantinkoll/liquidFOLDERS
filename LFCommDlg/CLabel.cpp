
// CLabel.cpp: Implementierung der Klasse CLabel
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CLabel
//

CLabel::CLabel()
	: CWnd()
{
}

BOOL CLabel::Create(CWnd* pParentWnd, UINT nID, CString Text)
{
	m_Text = Text;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_DISABLED;
	CRect rect;
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

void CLabel::SetText(CString Text)
{
	m_Text = Text;
	Invalidate();
}


BEGIN_MESSAGE_MAP(CLabel, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CLabel::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CLabel::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	HBRUSH brush = (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORBTN, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd);
	if (brush)
		FillRect(dc, rect, brush);

	CFont* pOldFont = (CFont*)dc.SelectStockObject(DEFAULT_GUI_FONT);
	dc.DrawText(m_Text, rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_LEFT | DT_VCENTER);
	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}
