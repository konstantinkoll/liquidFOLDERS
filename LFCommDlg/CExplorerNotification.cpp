
// CExplorerNotification.cpp: Implementierung der Klasse CExplorerNotification
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CExplorerNotification
//

#define BORDERX     8
#define BORDERY     10

CExplorerNotification::CExplorerNotification()
	: CWnd()
{
	m_Dismissed = TRUE;
	hIcon = NULL;

	m_IconCX = GetSystemMetrics(SM_CXICON);
	m_IconCY = GetSystemMetrics(SM_CYICON);
	ImageList_GetIconSize(LFGetApp()->m_SystemImageListLarge, &m_IconCX, &m_IconCY);

	m_GradientCY = max(m_IconCY/16, 2);
	m_CloseHover = m_ClosePressed = FALSE;
}

BOOL CExplorerNotification::Create(CWnd* pParentWnd, UINT nID)
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	CRect rect;
	rect.SetRectEmpty();
	return CWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER, rect, pParentWnd, nID);
}

UINT CExplorerNotification::GetPreferredHeight()
{
	return m_GradientCY+2*BORDERY+m_IconCY+2;
}

void CExplorerNotification::SetNotification(UINT Type, CString Text, UINT Command)
{
	switch (Type)
	{
	case ENT_READY:
		m_FirstCol = 0x00E600;
		m_SecondCol = 0x00AF00;

		break;

	case ENT_INFO:
		m_FirstCol = 0xFF8E6F;
		m_SecondCol = 0xF26120;

		hIcon = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_INFORMATION), IMAGE_ICON, m_IconCX, m_IconCY, LR_SHARED);

		break;

	case ENT_WARNING:
		m_FirstCol = 0x49CEFF;
		m_SecondCol = 0x00B1F2;

		hIcon = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_WARNING), IMAGE_ICON, m_IconCX, m_IconCY, LR_SHARED);

		break;

	case ENT_SHIELD:
		m_FirstCol = 0x49CEFF;
		m_SecondCol = 0x00B1F2;

		hIcon = (HICON)LoadImage(AfxGetResourceHandle(), (LFGetApp()->OSVersion==OS_Vista) ? MAKEINTRESOURCE(IDI_SHIELD_VISTA) : IDI_SHIELD, IMAGE_ICON, m_IconCX, m_IconCY, LR_SHARED);

		break;

	default:
		m_FirstCol = 0x0000E6;
		m_SecondCol = 0x0000AF;

		hIcon = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ERROR), IMAGE_ICON, m_IconCX, m_IconCY, LR_SHARED);
	}

	m_Text = Text;

	if (Command)
	{
		ENSURE(m_CommandText.LoadString(Command));

		INT pos = m_CommandText.Find(L'\n');
		if (pos!=-1)
			m_CommandText.Delete(0, pos+1);

		m_CommandButton.SetWindowText(m_CommandText);
		m_CommandButton.EnableWindow(TRUE);
		m_CommandButton.ShowWindow(SW_SHOW);
	}
	else
	{
		m_CommandButton.ShowWindow(SW_HIDE);
		m_CommandButton.EnableWindow(FALSE);
	}
	m_Command = Command;

	AdjustLayout();
	ShowWindow(SW_SHOW);
	Invalidate();

	if (Type!=ENT_READY)
		LFGetApp()->PlayWarningSound();

	m_Dismissed = FALSE;
}

void CExplorerNotification::DismissNotification()
{
	if (!m_Dismissed)
	{
		ReleaseCapture();
		ShowWindow(SW_HIDE);
		m_CommandButton.EnableWindow(FALSE);
		m_Command = 0;

		if (hIcon)
		{
			DestroyIcon(hIcon);
			hIcon = NULL;
		}

		m_Dismissed = TRUE;
	}
}

void CExplorerNotification::AdjustLayout()
{
	// Close button
	CSize sz = CMenuImages::Size();
	GetClientRect(m_RectClose);
	CRect rectClient(m_RectClose);

	m_RectClose.top += m_GradientCY+BORDERY+1;
	m_RectClose.bottom = m_RectClose.top+sz.cy;
	m_RectClose.right -= BORDERY-1;
	m_RectClose.left = m_RectClose.right-sz.cx;

	// Command button
	if (m_Command)
	{
		CSize sz;

		CDC* pDC = GetDC();
		HGDIOBJ hOldFont = pDC->SelectStockObject(DEFAULT_GUI_FONT);
		sz = pDC->GetTextExtent(m_CommandText);
		pDC->SelectObject(hOldFont);
		ReleaseDC(pDC);

		const UINT Height = MulDiv(11, HIWORD(GetDialogBaseUnits()), 8)+1;
		const UINT Width = sz.cx+Height+BORDERX;
		m_RightMargin = m_RectClose.left-BORDERX-Width;

		m_CommandButton.SetWindowPos(NULL, m_RightMargin, m_GradientCY+1+(rectClient.Height()-m_GradientCY-2-Height)/2, Width, Height, SWP_NOACTIVATE | SWP_NOZORDER);
	}
	else
	{
		m_RightMargin = m_RectClose.left;
	}
}


BEGIN_MESSAGE_MAP(CExplorerNotification, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_CTLCOLOR()
	ON_BN_CLICKED(1, OnButtonClicked)
END_MESSAGE_MAP()

INT CExplorerNotification::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	CRect rect;
	rect.SetRectEmpty();
	if (!m_CommandButton.Create(_T(""), WS_CHILD | WS_DISABLED | WS_GROUP | WS_TABSTOP, rect, this, 1))
		return -1;

	m_CommandButton.SendMessage(WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT));

	return 0;
}

void CExplorerNotification::OnDestroy()
{
	CWnd::OnDestroy();

	if (hIcon)
		DestroyIcon(hIcon);
}

BOOL CExplorerNotification::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CExplorerNotification::OnPaint()
{
	CPaintDC pDC(this);

	CRect rectClient;
	GetClientRect(rectClient);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rectClient.Width(), rectClient.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	BOOL Themed = IsCtrlThemed();

	dc.FillSolidRect(rectClient, Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));

	CRect rect(rectClient);
	rect.DeflateRect(1, 1);

	Graphics g(dc);
	g.SetPixelOffsetMode(PixelOffsetModeHalf);

	LinearGradientBrush brush(Point(0, 0), Point(rect.Width(), 0), Color(255, m_FirstCol & 0xFF, (m_FirstCol>>8) & 0xFF, (m_FirstCol>>16) & 0xFF), Color(255, m_SecondCol & 0xFF, (m_SecondCol>>8) & 0xFF, (m_SecondCol>>16) & 0xFF));
	g.FillRectangle(&brush, rect.top, rect.left, rect.Width(), m_GradientCY);

	rect.top += m_GradientCY;
	rect.left += BORDERX;

	if (hIcon)
	{
		DrawIconEx(dc, rect.left, rect.top+(rect.Height()-m_IconCY)/2, hIcon, m_IconCX, m_IconCY, 0, NULL, DI_NORMAL);
		rect.left += BORDERX+m_IconCX;
	}

	CMenuImages::Draw(&dc, CMenuImages::IdClose, m_RectClose, (!Themed || m_ClosePressed) ? CMenuImages::ImageDkGray : m_CloseHover ? CMenuImages::ImageGray : CMenuImages::ImageLtGray);

	rect.right = m_RightMargin-BORDERX;
	CRect rectText(rect);

	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_DefaultFont);

	dc.SetTextColor(Themed ? 0x000000 : GetSysColor(COLOR_WINDOWTEXT));
	dc.DrawText(m_Text, rectText, DT_WORDBREAK | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_CALCRECT);

	if (rect.Height()>rectText.Height())
		rect.top += (rect.Height()-rectText.Height())/2;

	dc.DrawText(m_Text, rect, DT_WORDBREAK | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX);

	dc.SelectObject(pOldFont);

	pDC.BitBlt(0, 0, rectClient.Width(), rectClient.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CExplorerNotification::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CExplorerNotification::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	if (m_CloseHover!=m_RectClose.PtInRect(point))
	{
		m_CloseHover = m_RectClose.PtInRect(point);
		InvalidateRect(m_RectClose);
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
}

void CExplorerNotification::OnMouseLeave()
{
	if (m_CloseHover)
	{
		m_CloseHover = FALSE;
		InvalidateRect(m_RectClose);
	}
}

void CExplorerNotification::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	if (m_RectClose.PtInRect(point))
	{
		m_ClosePressed = TRUE;
		InvalidateRect(m_RectClose);

		SetCapture();
	}
}

void CExplorerNotification::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	if (m_RectClose.PtInRect(point))
		DismissNotification();

	m_ClosePressed = FALSE;
	InvalidateRect(m_RectClose);

	ReleaseCapture();
}

HBRUSH CExplorerNotification::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	// Call base class version at first, else it will override changes
	HBRUSH hbr = CWnd::OnCtlColor(pDC, pWnd, nCtlColor);

	pDC->SetBkMode(TRANSPARENT);
	pDC->SetDCBrushColor(IsCtrlThemed() ? 0xFFFFFF : GetSysColor(COLOR_WINDOW));
	hbr = (HBRUSH)GetStockObject(DC_BRUSH);

	return hbr;
}

void CExplorerNotification::OnButtonClicked()
{
	if (m_Command)
	{
		GetOwner()->PostMessage(WM_COMMAND, m_Command);
		DismissNotification();
	}
}
