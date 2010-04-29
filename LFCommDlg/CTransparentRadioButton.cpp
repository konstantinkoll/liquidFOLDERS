
// CTransparentRadioButton.cpp: Implementierung der Klasse CTransparentRadioButton
//

#include "stdafx.h"
#include "CTransparentRadioButton.h"
#include "LFDialog.h"


// CTransparentRadioButton
//

CTransparentRadioButton::CTransparentRadioButton()
	: CButton()
{
	m_Hover = FALSE;
	p_App = (LFApplication*)AfxGetApp();

	if (p_App->m_ThemeLibLoaded)
	{
		hTheme = p_App->zOpenThemeData(GetSafeHwnd(), VSCLASS_BUTTON);
	}
	else
	{
		hTheme = NULL;
	}
}

CTransparentRadioButton::~CTransparentRadioButton()
{
}


BEGIN_MESSAGE_MAP(CTransparentRadioButton, CButton)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
	ON_WM_THEMECHANGED()
	ON_WM_MOUSEMOVE()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
END_MESSAGE_MAP()

void CTransparentRadioButton::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap buffer;
	buffer.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&buffer);

	// Background
	CRect rectWindow;
	GetWindowRect(rectWindow);
	GetParent()->ScreenToClient(rectWindow);

	CDC dcBack;
	dcBack.CreateCompatibleDC(&dc);
	CBitmap* oldBitmap = (CBitmap*)dcBack.SelectObject(((LFDialog*)GetParent())->GetBackBuffer());
	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &dcBack, rectWindow.left, rectWindow.top, SRCCOPY);
	dcBack.SelectObject(oldBitmap);

	// Icon
	CRect rectIcon;
	rectIcon.CopyRect(rect);
	rectIcon.left--;
	rectIcon.right = rectIcon.left+rect.Height();

	if (hTheme)
	{
		int uiStyle = RBS_UNCHECKEDDISABLED;
		if (IsWindowEnabled())
			uiStyle = (GetState() & 4) ? RBS_UNCHECKEDPRESSED : m_Hover ? RBS_UNCHECKEDHOT : RBS_UNCHECKEDNORMAL;
		p_App->zDrawThemeBackground(hTheme, dc.m_hDC, BP_RADIOBUTTON, uiStyle + (GetCheck() ? 4 : 0), rectIcon, rectIcon);
	}
	else
	{
		UINT uiStyle = DFCS_BUTTONRADIO | (GetCheck() ? DFCS_CHECKED : 0) | (GetState() ? DFCS_PUSHED : 0) | (IsWindowEnabled() ? 0 : DFCS_INACTIVE);
		dc.DrawFrameControl(rectIcon, DFC_BUTTON, uiStyle);
	}

	// Text
	CRect rectText;
	rectText.CopyRect(rect);
	rectText.left += rect.Height();
	CFont* pOldFont = (CFont*)dc.SelectStockObject(DEFAULT_GUI_FONT);
	CString tmpStr;
	GetWindowText(tmpStr);
	dc.SetTextColor(IsWindowEnabled() ? 0x000000 : 0x7B7770);
	const UINT textflags = DT_SINGLELINE | DT_LEFT | DT_END_ELLIPSIS | DT_VCENTER;
	dc.DrawText(tmpStr, rectText, textflags | DT_CALCRECT);
	rectText.OffsetRect(0, (rect.Height()-rectText.Height())/2);
	dc.DrawText(tmpStr, rectText, textflags);
	dc.SelectObject(pOldFont);

	// Focus
	if (GetFocus()==this)
	{
		rectText.InflateRect(1, 0);
		rectText.top = rect.top;
		rectText.bottom = rect.bottom;
		dc.DrawFocusRect(rectText);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

BOOL CTransparentRadioButton::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CTransparentRadioButton::OnDestroy()
{
	if (hTheme)
		p_App->zCloseThemeData(hTheme);

	CButton::OnDestroy();
}

LRESULT CTransparentRadioButton::OnThemeChanged()
{
	if (p_App->m_ThemeLibLoaded)
	{
		if (hTheme)
			p_App->zCloseThemeData(hTheme);

		hTheme = p_App->zOpenThemeData(m_hWnd, VSCLASS_BUTTON);
	}

	return TRUE;
}

void CTransparentRadioButton::OnMouseMove(UINT nFlags, CPoint point)
{
	CButton::OnMouseMove(nFlags, point);

	if (!m_Hover)
	{
		m_Hover = TRUE;
		Invalidate();

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = GetSafeHwnd();
		TrackMouseEvent(&tme);
	}
}

LRESULT CTransparentRadioButton::OnMouseLeave(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	m_Hover = FALSE;
	Invalidate();

	return NULL;
}
