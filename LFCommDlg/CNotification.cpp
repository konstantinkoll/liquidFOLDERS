
// CNotification.cpp: Implementierung der Klasse CNotification
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CNotification
//

#define BORDER     BACKSTAGEBORDER

CNotification::CNotification()
	: CFrontstageWnd()
{
	m_Dismissed = TRUE;
	m_hIcon = NULL;

	m_IconSize = GetSystemMetrics(SM_CYICON);
	m_GradientHeight = max(m_IconSize/16, 2);
	m_CloseHover = m_ClosePressed = FALSE;
}

BOOL CNotification::Create(CWnd* pParentWnd, UINT nID)
{
	const CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return CWnd::Create(className, _T(""), WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_BORDER, CRect(0, 0, 0, 0), pParentWnd, nID);
}

UINT CNotification::GetPreferredHeight() const
{
	return m_GradientHeight+2*BORDER+m_IconSize+2;
}

void CNotification::SetNotification(UINT Type, const CString& Text, UINT Command)
{
	// Kill timer
	KillTimer(1);

	// Set color and message text
	LPCWSTR nIconID = NULL;

	switch (Type)
	{
	case ENT_READY:
		m_FirstCol = 0x00E600;
		m_SecondCol = 0x00AF00;

		nIconID = MAKEINTRESOURCE(IDI_READY);

		break;

	case ENT_INFO:
		m_FirstCol = 0xFF8E6F;
		m_SecondCol = 0xF26120;

		nIconID = IDI_INFORMATION;

		break;

	case ENT_WARNING:
		m_FirstCol = 0x49CEFF;
		m_SecondCol = 0x00B1F2;

		nIconID = IDI_WARNING;

		break;

	case ENT_SHIELD:
		m_FirstCol = 0x49CEFF;
		m_SecondCol = 0x00B1F2;

		nIconID = (LFGetApp()->OSVersion==OS_Vista) ? MAKEINTRESOURCE(IDI_SHIELD_VISTA) : IDI_SHIELD;

		break;

	default:
		m_FirstCol = 0x0000E6;
		m_SecondCol = 0x0000AF;

		nIconID = IDI_ERROR;
	}

	m_Text = Text;

	// Load icon
	if (nIconID)
		m_hIcon = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(nIconID), IMAGE_ICON, m_IconSize, m_IconSize, LR_SHARED);

	// Set command button
	if (Command)
	{
		ENSURE(m_CommandText.LoadString(Command));

		const INT Pos = m_CommandText.Find(L'\n');
		if (Pos!=-1)
			m_CommandText.Delete(0, Pos+1);

		m_wndCommandButton.SetWindowText(m_CommandText);
		m_wndCommandButton.EnableWindow(TRUE);
		m_wndCommandButton.ShowWindow(SW_SHOW);
	}
	else
	{
		m_wndCommandButton.ShowWindow(SW_HIDE);
		m_wndCommandButton.EnableWindow(FALSE);
	}
	m_Command = Command;

	// Arrange and show window
	AdjustLayout();
	ShowWindow(SW_SHOW);

	// Finalize
	if (Type==ENT_READY)
	{
		// Start timer to auto-hide notification when there is no button to click
		if (!Command)
			SetTimer(1, GetTimerLength(), NULL);
	}
	else
	{
		// Notification sound
		LFGetApp()->PlayNotificationSound();
	}

	m_Dismissed = FALSE;
}

void CNotification::DismissNotification()
{
	if (!m_Dismissed)
	{
		ReleaseCapture();
		ShowWindow(SW_HIDE);

		m_wndCommandButton.EnableWindow(FALSE);
		m_Command = 0;

		if (m_hIcon)
		{
			DestroyIcon(m_hIcon);
			m_hIcon = NULL;
		}

		m_Dismissed = TRUE;
	}
}

void CNotification::AdjustLayout()
{
	// Close button
	CSize Size = CMenuImages::Size();

	GetClientRect(m_RectClose);
	CRect rectClient(m_RectClose);

	m_RectClose.top += m_GradientHeight+BORDER+1;
	m_RectClose.bottom = m_RectClose.top+Size.cy;
	m_RectClose.right -= BORDER-1;
	m_RectClose.left = m_RectClose.right-Size.cx;

	// Command button
	if (m_Command)
	{
		Size = LFGetApp()->m_DialogFont.GetTextExtent(m_CommandText);

		const UINT Height = MulDiv(11, HIWORD(GetDialogBaseUnits()), 8)+1;
		const UINT Width = Size.cx+Height+BORDER;

		m_RightMargin = m_RectClose.left-BORDER-Width;

		m_wndCommandButton.SetWindowPos(NULL, m_RightMargin, m_GradientHeight+1+(rectClient.Height()-m_GradientHeight-2-Height)/2, Width, Height, SWP_NOACTIVATE | SWP_NOZORDER);
	}
	else
	{
		m_RightMargin = m_RectClose.left;
	}

	Invalidate();
}


BEGIN_MESSAGE_MAP(CNotification, CFrontstageWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_BN_CLICKED(1, OnButtonClicked)
END_MESSAGE_MAP()

INT CNotification::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrontstageWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Button
	if (!m_wndCommandButton.Create(_T(""), this, 1))
		return -1;

	m_wndCommandButton.SetFont(&LFGetApp()->m_DialogFont);

	return 0;
}

void CNotification::OnDestroy()
{
	if (m_hIcon)
		DestroyIcon(m_hIcon);

	CWnd::OnDestroy();
}

void CNotification::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==1)
	{
		KillTimer(1);

		DismissNotification();
	}

	CFrontstageWnd::OnTimer(nIDEvent);

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}

void CNotification::OnPaint()
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

	const BOOL Themed = IsCtrlThemed();

	FillRect(dc, rectClient, (HBRUSH)SendMessage(WM_CTLCOLORSTATIC, (WPARAM)dc.m_hDC, (LPARAM)m_hWnd));

	CRect rect(rectClient);
	rect.DeflateRect(1, 1);

	Graphics g(dc);
	g.SetPixelOffsetMode(PixelOffsetModeHalf);

	LinearGradientBrush brush(Point(0, 0), Point(rect.Width(), 0), Color(255, m_FirstCol & 0xFF, (m_FirstCol>>8) & 0xFF, (m_FirstCol>>16) & 0xFF), Color(255, m_SecondCol & 0xFF, (m_SecondCol>>8) & 0xFF, (m_SecondCol>>16) & 0xFF));
	g.FillRectangle(&brush, rect.top, rect.left, rect.Width(), m_GradientHeight);

	rect.top += m_GradientHeight;
	rect.left += BORDER;

	if (m_hIcon)
	{
		DrawIconEx(dc, rect.left, rect.top+(rect.Height()-m_IconSize)/2, m_hIcon, m_IconSize, m_IconSize, 0, NULL, DI_NORMAL);
		rect.left += BORDER+m_IconSize;
	}

	CMenuImages::Draw(&dc, CMenuImages::IdClose, m_RectClose, (!Themed || m_ClosePressed) ? CMenuImages::ImageDkGray : m_CloseHover ? CMenuImages::ImageGray : CMenuImages::ImageLtGray);

	rect.right = m_RightMargin-BORDER;
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

void CNotification::OnSize(UINT nType, INT cx, INT cy)
{
	CWnd::OnSize(nType, cx, cy);

	AdjustLayout();
}

void CNotification::OnMouseMove(UINT /*nFlags*/, CPoint point)
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

void CNotification::OnMouseLeave()
{
	if (m_CloseHover)
	{
		m_CloseHover = FALSE;
		InvalidateRect(m_RectClose);
	}
}

void CNotification::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	if (m_RectClose.PtInRect(point))
	{
		m_ClosePressed = TRUE;
		InvalidateRect(m_RectClose);

		SetCapture();
	}
}

void CNotification::OnLButtonUp(UINT /*nFlags*/, CPoint point)
{
	if (m_RectClose.PtInRect(point))
		DismissNotification();

	m_ClosePressed = FALSE;
	InvalidateRect(m_RectClose);

	ReleaseCapture();
}

void CNotification::OnButtonClicked()
{
	if (m_Command)
	{
		GetOwner()->PostMessage(WM_COMMAND, m_Command);
		DismissNotification();
	}
}
