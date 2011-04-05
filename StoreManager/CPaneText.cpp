
// CPaneText.cpp: Implementierung der Klasse CPaneText
//

#include "stdafx.h"
#include "CPaneText.h"
#include "LFCommDlg.h"


// CPaneText
//

CPaneText::CPaneText()
	: CWnd()
{
}

BOOL CPaneText::Create(CWnd* pParentWnd, UINT nID, CString _text)
{
	m_Text = _text;

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

void CPaneText::SetText(CString _text)
{
	m_Text = _text;
	Invalidate();
}


BEGIN_MESSAGE_MAP(CPaneText, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
END_MESSAGE_MAP()

BOOL CPaneText::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CPaneText::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	dc.SetTextColor(afxGlobalData.clrBarText);
	dc.SetBkColor(IsCtrlThemed() ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));
	dc.FillSolidRect(rect, dc.GetBkColor());

	CFont* pOldFont = (CFont*)dc.SelectStockObject(DEFAULT_GUI_FONT);

	dc.DrawText(m_Text, rect, DT_SINGLELINE | DT_END_ELLIPSIS | DT_BOTTOM);
	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CPaneText::OnSetFocus(CWnd* /*pOldWnd*/)
{
	CWnd* pParent = GetParent();

	if (pParent)
		pParent->SetFocus();
}
