
// CMapCtrl.cpp: Implementierung der Klasse CMapCtrl
//

#include "stdafx.h"
#include "LFCommDlg.h"


// CMapCtrl
//

CMapCtrl::CMapCtrl()
	: CWnd()
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

	hBackgroundBrush = NULL;
	m_BackBufferL = m_BackBufferH = 0;
	m_LastTrack.x = m_LastTrack.y = -1;
	m_Blink = TRUE;
	m_RemainVisible = m_BackgroundMenuID = 0;
}

void CMapCtrl::PreSubclassWindow()
{
	CWnd::PreSubclassWindow();

	_AFX_THREAD_STATE* pThreadState = AfxGetThreadState();
	if (!pThreadState->m_pWndInit)
		Init();
}

void CMapCtrl::Init()
{
	SetTimer(100, 500, NULL);
}

void CMapCtrl::SetMenu(UINT BackgroundMenuID, BOOL HighlightFirst)
{
	m_BackgroundMenuID = BackgroundMenuID;
	m_HighlightFirst = HighlightFirst;
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

void CMapCtrl::SetLocation(const CPoint& point)
{
	LocationFromPoint(point, m_Location.Latitude, m_Location.Longitude);

	SendUpdateMsg();
}

void CMapCtrl::SetLocation(const LFGeoCoordinates& Location)
{
	m_Location = Location;

	SendUpdateMsg();
}

void CMapCtrl::SendUpdateMsg()
{
	m_Blink = TRUE;
	m_RemainVisible = 1;
	Invalidate();

	NM_GPSDATA tag;
	tag.hdr.code = MAP_UPDATE_LOCATION;
	tag.hdr.hwndFrom = m_hWnd;
	tag.hdr.idFrom = GetDlgCtrlID();
	tag.pLocation = &m_Location;

	GetOwner()->SendMessage(WM_NOTIFY, tag.hdr.idFrom, LPARAM(&tag));
}


BEGIN_MESSAGE_MAP(CMapCtrl, CWnd)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_NCCALCSIZE()
	ON_WM_NCPAINT()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSELEAVE()
	ON_WM_KEYDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_TIMER()
	ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

INT CMapCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct)==-1)
		return -1;

	Init();

	return 0;
}

void CMapCtrl::OnDestroy()
{
	KillTimer(100);

	DeleteObject(hBackgroundBrush);

	CWnd::OnDestroy();
}

void CMapCtrl::OnNcCalcSize(BOOL /*bCalcValidRects*/, NCCALCSIZE_PARAMS* lpncsp)
{
	lpncsp->rgrc[0].top += 2;
	lpncsp->rgrc[0].left += 2;
	lpncsp->rgrc[0].bottom -= 2;
	lpncsp->rgrc[0].right -= 2;
}

void CMapCtrl::OnNcPaint()
{
	DrawControlBorder(this);
}

BOOL CMapCtrl::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	if ((m_BackBufferL!=rect.Width()) || (m_BackBufferH!=rect.Height()))
	{
		m_BackBufferL = rect.Width();
		m_BackBufferH = rect.Height();

		CDC dc;
		dc.CreateCompatibleDC(pDC);
		dc.SetBkMode(TRANSPARENT);

		CBitmap MemBitmap;
		MemBitmap.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
		CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

		Graphics g(dc);

		Bitmap* pMap = LFGetApp()->GetCachedResourceImage(IDB_BLUEMARBLE_2048);
		g.DrawImage(pMap, 0, 0, rect.Width(), rect.Height());

		dc.SelectObject(pOldBitmap);

		DeleteObject(hBackgroundBrush);
		hBackgroundBrush = CreatePatternBrush(MemBitmap);
	}

	return TRUE;
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

	FillRect(dc, rect, hBackgroundBrush);

	if (m_Blink && ((m_Location.Latitude!=0) || (m_Location.Longitude!=0)))
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
	if ((point.x!=m_LastTrack.x) || (point.y!=m_LastTrack.y))
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
			LFGetApp()->HideTooltip();
		}

		m_LastTrack = point;
	}

	TRACKMOUSEEVENT tme;
	tme.cbSize = sizeof(TRACKMOUSEEVENT);
	tme.dwFlags = TME_LEAVE;
	tme.hwndTrack = m_hWnd;
	TrackMouseEvent(&tme);
}

void CMapCtrl::OnMouseLeave()
{
	LFGetApp()->HideTooltip();

	m_LastTrack.x = m_LastTrack.y = -1;
}

void CMapCtrl::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case VK_DELETE:
		if ((GetKeyState(VK_CONTROL)>=0) && (GetKeyState(VK_SHIFT)>=0))
		{
			m_Location.Latitude = m_Location.Longitude = 0;

			SendUpdateMsg();
		}

		break;

	default:
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
	}
}

void CMapCtrl::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	SetLocation(point);
	m_LastTrack = point;

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

void CMapCtrl::OnContextMenu(CWnd* pWnd, CPoint pos)
{
	if (pWnd!=this)
		return;

	if ((pos.x<0) || (pos.y<0))
	{
		INT PosX;
		INT PosY;
		PointFromLocation(PosX, PosY);

		pos.x = PosX;
		pos.y = PosY;
	}
	else
	{
		ScreenToClient(&pos);
	}

	if (m_BackgroundMenuID)
	{
		ClientToScreen(&pos);

		CMenu Menu;
		Menu.LoadMenu(m_BackgroundMenuID);
		ASSERT_VALID(&Menu);

		CMenu* pPopup = Menu.GetSubMenu(0);
		ASSERT_VALID(pPopup);

		if (m_HighlightFirst)
			pPopup->SetDefaultItem(0, TRUE);

		pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, pos.x, pos.y, GetOwner());
	}
}
