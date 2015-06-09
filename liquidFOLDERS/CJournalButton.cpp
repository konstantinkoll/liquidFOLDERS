
// CJournalButton.cpp: Implementierung der Klasse CJournalButton
//

#include "stdafx.h"
#include "CJournalButton.h"
#include "Resource.h"


// CJournalUI
//

CJournalUI::CJournalUI()
	: CCmdUI()
{
	m_Enabled = FALSE;
}

void CJournalUI::Enable(BOOL bOn)
{
	m_Enabled = bOn;
}


// CJournalButton
//

#define XOffset      (m_IsLarge ? 0 : 1)
#define YOffset      (m_IsLarge ? 0 : -1)
#define GetHover     ((point.x>(INT)m_ButtonWidth/2+XOffset) ? 1 : 0)

CJournalButton::CJournalButton()
	: CWnd()
{
	m_CmdEnabled[0] = m_CmdEnabled[1] = FALSE;
	m_Hover = m_Pressed = -1;
}

BOOL CJournalButton::Create(UINT SuggestedHeight, CGlassWindow* pParentWnd, UINT nID)
{
	m_IsLarge = (SuggestedHeight>=32);

	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, LoadCursor(NULL, IDC_ARROW));

	const DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), dwStyle, rect, pParentWnd, nID);
}

UINT CJournalButton::GetPreferredHeight()
{
	return m_Frame.m_pBitmap->GetHeight();
}

UINT CJournalButton::GetPreferredWidth()
{
	return m_Frame.m_pBitmap->GetWidth();
}

void CJournalButton::DrawLeft(Graphics& g, CGdiPlusBitmap* pBmp)
{
	g.DrawImage(pBmp->m_pBitmap, XOffset, YOffset, 0, 0, m_ButtonWidth/2, m_ButtonHeight, UnitPixel);
}

void CJournalButton::DrawRight(Graphics& g, CGdiPlusBitmap* pBmp)
{
	g.DrawImage(pBmp->m_pBitmap, XOffset+m_ButtonWidth/2+1, YOffset, m_ButtonWidth/2, 0, m_ButtonWidth-m_ButtonWidth/2, m_ButtonHeight, UnitPixel);
}


BEGIN_MESSAGE_MAP(CJournalButton, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONUP()
	ON_MESSAGE_VOID(WM_IDLEUPDATECMDUI, OnIdleUpdateCmdUI)
END_MESSAGE_MAP()

INT CJournalButton::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	m_Frame.Load(m_IsLarge ? IDB_JOURNAL_FRAME1 : IDB_JOURNAL_FRAME0, _T("PNG"));
	m_Normal.Load(m_IsLarge ? IDB_JOURNAL_NORMAL1 : IDB_JOURNAL_NORMAL0, _T("PNG"));
	m_Hot.Load(m_IsLarge ? IDB_JOURNAL_HOT1 : IDB_JOURNAL_HOT0, _T("PNG"));
	m_Pushed.Load(m_IsLarge ? IDB_JOURNAL_PRESSED1 : IDB_JOURNAL_PRESSED0, _T("PNG"));
	m_Disabled.Load(m_IsLarge ? IDB_JOURNAL_DISABLED1 : IDB_JOURNAL_DISABLED0, _T("PNG"));

	m_ButtonWidth = m_Normal.m_pBitmap->GetWidth();
	m_ButtonHeight = m_Normal.m_pBitmap->GetHeight();

	return 0;
}

void CJournalButton::OnDestroy()
{
	CWnd::OnDestroy();
}

BOOL CJournalButton::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CJournalButton::OnPaint()
{
	CPaintDC pDC(this);

	CRect rectClient;
	GetClientRect(rectClient);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	BITMAPINFO dib = { 0 };
	dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib.bmiHeader.biWidth = rectClient.Width();
	dib.bmiHeader.biHeight = -rectClient.Height();
	dib.bmiHeader.biPlanes = 1;
	dib.bmiHeader.biBitCount = 32;
	dib.bmiHeader.biCompression = BI_RGB;

	HBITMAP hBmp = CreateDIBSection(dc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBmp);

	Graphics g(dc);

	CGlassWindow* pCtrlSite = (CGlassWindow*)GetParent();
	pCtrlSite->DrawFrameBackground(&dc, rectClient);

	g.DrawImage(m_Frame.m_pBitmap, 0, 0);

	DrawLeft(g, m_CmdEnabled[0] ? ((m_Pressed==0) && (m_Hover==0)) ? &m_Pushed : ((m_Hover==0) && (m_Pressed!=1)) ? &m_Hot : &m_Normal : &m_Disabled);
	DrawRight(g, m_CmdEnabled[1] ? ((m_Pressed==1) && (m_Hover==1)) ? &m_Pushed : ((m_Hover==1) && (m_Pressed!=0)) ? &m_Hot : &m_Normal : &m_Disabled);

	pDC.BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(hOldBitmap);
	DeleteObject(hBmp);
}

void CJournalButton::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	if (m_Hover==-1)
	{
		Invalidate();

		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(TRACKMOUSEEVENT);
		tme.dwFlags = TME_LEAVE;
		tme.hwndTrack = m_hWnd;
		TrackMouseEvent(&tme);
	}

	CRect rect;
	GetClientRect(rect);

	INT ID = rect.PtInRect(point) ? GetHover : -1;
	if (m_Hover!=ID)
	{
		m_Hover = ID;
		Invalidate();
	}
}

void CJournalButton::OnMouseLeave()
{
	m_Hover = m_Pressed = -1;
	Invalidate();
}

void CJournalButton::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	INT ID = GetHover;
	if (m_CmdEnabled[ID])
	{
		m_Pressed = GetHover;
		Invalidate();

		SetCapture();
	}
}

void CJournalButton::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	INT ID = GetHover;
	if ((m_CmdEnabled[ID]) && ((m_Pressed==-1) || (m_Pressed==ID)))
		GetOwner()->PostMessage(WM_COMMAND, (ID==0) ? ID_NAV_BACK : ID_NAV_FORWARD);

	if (m_Pressed!=-1)
	{
		m_Pressed = -1;
		Invalidate();

		ReleaseCapture();
	}
}

void CJournalButton::OnRButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();

	CRect rect;
	GetWindowRect(rect);
	point += rect.TopLeft();
	GetParent()->ScreenToClient(&point);

	GetParent()->SendMessage(WM_RBUTTONUP, (WPARAM)nFlags, (LPARAM)((point.y<<16) | point.x));
}

void CJournalButton::OnIdleUpdateCmdUI()
{
	BOOL Update = FALSE;

	CJournalUI cmdUI;
	cmdUI.m_nID = ID_NAV_BACK;
	cmdUI.DoUpdate(GetOwner(), FALSE);
	Update |= (cmdUI.m_Enabled!=m_CmdEnabled[0]);
	m_CmdEnabled[0] = cmdUI.m_Enabled;

	cmdUI.m_nID = ID_NAV_FORWARD;
	cmdUI.DoUpdate(GetOwner(), FALSE);
	Update |= (cmdUI.m_Enabled!=m_CmdEnabled[1]);
	m_CmdEnabled[1] = cmdUI.m_Enabled;

	if (Update)
		Invalidate();
}
