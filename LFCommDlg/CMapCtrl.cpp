
// CMapCtrl.cpp: Implementierung der Klasse CMapCtrl
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CMapCtrl
//

CMapCtrl::CMapCtrl()
	: CFrontstageWnd()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.hCursor = LFGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.lpszClassName = L"CMapCtrl";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CMapCtrl", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	m_Location.Latitude = m_Location.Longitude = 0;

	m_BackBufferL = m_BackBufferH = 0;
	hBackgroundBrush = NULL;
	m_Blink = TRUE;
	m_RemainVisible = m_BackgroundMenuID = 0;
}

void CMapCtrl::PreSubclassWindow()
{
	CFrontstageWnd::PreSubclassWindow();

	SetTimer(100, 500, NULL);
}

void CMapCtrl::ShowTooltip(const CPoint& point)
{
	LFGeoCoordinates Location;
	LocationFromPoint(point, Location.Latitude, Location.Longitude);

	WCHAR tmpStr[256];
	LFGeoCoordinatesToString(Location, tmpStr, 256, FALSE);

	if (tmpStr[0]!=L'\0')
	{
		LFGetApp()->ShowTooltip(this, point, _T(""), tmpStr);
	}
	else
	{
		HideTooltip();
	}
}

BOOL CMapCtrl::GetContextMenu(CMenu& Menu, INT /*Index*/)
{
	Menu.LoadMenu(IDM_MAPCTRL);

	return FALSE;
}

void CMapCtrl::LocationFromPoint(const CPoint& point, DOUBLE& Latitude, DOUBLE& Longitude) const
{
	CRect rect;
	GetClientRect(rect);

	Latitude = point.y*180.0/(rect.Height()-1)-90.0;
	if (Latitude<-90.0)
		Latitude = -90.0;
	if (Latitude>90.0)
		Latitude = 90.0;

	Longitude = point.x*360.0/(rect.Width()-1)-180.0;
	if (Longitude<-180.0)
		Longitude = -180.0;
	if (Longitude>180.0)
		Longitude = 180.0;
}

void CMapCtrl::PointFromLocation(INT& PosX, INT& PosY) const
{
	CRect rect;
	GetClientRect(rect);

	PosX = (INT)(m_Location.Longitude+180)*rect.Width()/360;
	PosY = (INT)(m_Location.Latitude+90)*rect.Height()/180;
}

void CMapCtrl::SendNotifyMessage()
{
	// Reset blink
	m_Blink = TRUE;
	m_RemainVisible = 1;
	Invalidate();

	// Send message
	const NM_GPSDATA nmHdr = { { m_hWnd, GetDlgCtrlID(), MAP_UPDATE_LOCATION }, &m_Location };

	GetOwner()->SendMessage(WM_NOTIFY, nmHdr.hdr.idFrom, (LPARAM)&nmHdr);
}

void CMapCtrl::SetLocation(const CPoint& point)
{
	LocationFromPoint(point, m_Location.Latitude, m_Location.Longitude);

	SendNotifyMessage();
}

void CMapCtrl::SetLocation(const LFGeoCoordinates& Location)
{
	m_Location = Location;

	SendNotifyMessage();
}

void CMapCtrl::PrepareBitmap(const CRect& rect)
{
	if ((m_BackBufferL!=rect.Width()) || (m_BackBufferH!=rect.Height()))
	{
		m_BackBufferL = rect.Width();
		m_BackBufferH = rect.Height();

		CDC dc;
		dc.CreateCompatibleDC(NULL);

		dc.SetBkMode(TRANSPARENT);

		CBitmap MemBitmap;
		MemBitmap.CreateBitmap(rect.Width(), rect.Height(), 1, 32, NULL);
		CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

		Graphics g(dc);

		Bitmap* pMap = LFGetApp()->GetCachedResourceImage(IDB_BLUEMARBLE_2048);
		g.DrawImage(pMap, 0, 0, rect.Width(), rect.Height());

		dc.SelectObject(pOldBitmap);

		DeleteObject(hBackgroundBrush);
		hBackgroundBrush = CreatePatternBrush(MemBitmap);
	}
}


BEGIN_MESSAGE_MAP(CMapCtrl, CFrontstageWnd)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
END_MESSAGE_MAP()

void CMapCtrl::OnDestroy()
{
	KillTimer(100);

	DeleteObject(hBackgroundBrush);

	CFrontstageWnd::OnDestroy();
}

void CMapCtrl::OnPaint()
{
	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	PrepareBitmap(rect);
	FillRect(dc, rect, hBackgroundBrush);

	if (m_Blink && (m_Location.Latitude || m_Location.Longitude))
	{
		INT PosX;
		INT PosY;
		PointFromLocation(PosX, PosY);

		DrawLocationIndicator(dc, PosX-4, PosY-4, 8);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldBitmap);
}

void CMapCtrl::OnMouseMove(UINT /*nFlags*/, CPoint point)
{
	ShowTooltip(point);

	TRACKMOUSE(TME_LEAVE);
}

void CMapCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_DELETE:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
		{
			m_Location.Latitude = m_Location.Longitude = 0;

			SendNotifyMessage();
		}

		break;

	default:
		CFrontstageWnd::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void CMapCtrl::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	SetLocation(point);

	if (GetFocus()!=this)
		SetFocus();
}

void CMapCtrl::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent==100)
		if (m_RemainVisible)
		{
			m_RemainVisible--;
		}
		else
		{
			m_Blink = !m_Blink;

			Invalidate();
		}

	// Eat bogus WM_TIMER messages
	MSG msg;
	while (PeekMessage(&msg, m_hWnd, WM_TIMER, WM_TIMER, PM_REMOVE));
}
