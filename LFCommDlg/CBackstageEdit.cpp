
// CBackstageEdit.cpp: Implementierung der Klasse CBackstageEdit
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CBackstageEdit
//

#define BORDER     2

CBackstageEdit::CBackstageEdit()
	: CEdit()
{
	m_ClientAreaTopOffset = 0;
	m_Hover = FALSE;
}

BOOL CBackstageEdit::Create(CBackstageWnd* pParentWnd, UINT nID, CString EmptyHint)
{
	m_EmptyHint = EmptyHint;

	return CEdit::Create(WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL, CRect(0, 0, 0, 0), pParentWnd, nID);
}


BEGIN_MESSAGE_MAP(CBackstageEdit, CEdit)
	ON_WM_CREATE()
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
END_MESSAGE_MAP()

INT CBackstageEdit::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEdit::OnCreate(lpCreateStruct)==-1)
		return -1;

	SetFont(&LFGetApp()->m_DefaultFont);

	m_ClientAreaTopOffset = ((INT)CBackstageBar::GetPreferredHeight()-2*BORDER-LFGetApp()->m_DefaultFont.GetFontHeight()-1)/2;
	SetWindowPos(NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_NOREDRAW | SWP_NOCOPYBITS);

	return 0;
}

void CBackstageEdit::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS* lpncsp)
{
	lpncsp->rgrc[0].top += BORDER+m_ClientAreaTopOffset;
	lpncsp->rgrc[0].bottom -= BORDER;
	lpncsp->rgrc[0].left += 2+BORDER;
	lpncsp->rgrc[0].right -= BORDER;
}

LRESULT CBackstageEdit::OnNcHitTest(CPoint point)
{
	CRect rectWindow;
	GetWindowRect(rectWindow);

	return rectWindow.PtInRect(point) ? HTCLIENT : HTNOWHERE;
}

BOOL CBackstageEdit::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CBackstageEdit::OnPaint()
{
	CRect rect;
	GetWindowRect(rect);

	rect.bottom -= rect.top;
	rect.right -= rect.left;
	rect.left = rect.top = 0;

	CWindowDC pDC(this);

	CRect rectClient;
	GetClientRect(rectClient);
	rectClient.OffsetRect(BORDER+2, BORDER+m_ClientAreaTopOffset);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	HBITMAP hBitmap1 = CreateTransparentBitmap(rect.Width(), rect.Height());
	HBITMAP hOldBitmap1 = (HBITMAP)dc.SelectObject(hBitmap1);

	// Background
	FillRect(dc, rect, (HBRUSH)GetParent()->SendMessage(WM_CTLCOLORSTATIC, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd));

	// Border
	if (!IsCtrlThemed())
		dc.Draw3dRect(rect, 0x000000, 0x000000);

	// Inherited paint
	CDC dcPaint;
	dcPaint.CreateCompatibleDC(&pDC);
	dcPaint.SetBkMode(TRANSPARENT);

	HBITMAP hBitmap2 = CreateTransparentBitmap(rectClient.Width(), rectClient.Height());
	HBITMAP hOldBitmap2 = (HBITMAP)dcPaint.SelectObject(hBitmap2);

	CEdit::DefWindowProc(WM_PAINT, (WPARAM)dcPaint.m_hDC, NULL);

	dc.BitBlt(rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), &dcPaint, 0, 0, SRCCOPY);

	dcPaint.SelectObject(hOldBitmap2);
	DeleteObject(hBitmap2);

	// Hint
	if ((GetWindowTextLength()==0) && (GetFocus()!=this))
	{
		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_ItalicFont);
		dc.SetTextColor(IsCtrlThemed() ? m_Hover ? 0xF0F0F0 : 0x998981 : GetSysColor(COLOR_3DSHADOW));

		CRect rectText(rect.left+3+BORDER, rect.top+BORDER+m_ClientAreaTopOffset, rect.right-BORDER, rect.bottom);
		dc.DrawText(m_EmptyHint, rectText, DT_SINGLELINE | DT_END_ELLIPSIS | DT_TOP | DT_NOPREFIX);

		dc.SelectObject(pOldFont);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(hOldBitmap1);
	DeleteObject(hBitmap1);

	ValidateRect(rect);
}

void CBackstageEdit::OnMouseMove(UINT nFlags, CPoint point)
{
	CEdit::OnMouseMove(nFlags, point);

	if (!m_Hover)
	{
		m_Hover = TRUE;
		Invalidate();

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}
}

void CBackstageEdit::OnMouseLeave()
{
	m_Hover = FALSE;
	Invalidate();
}
