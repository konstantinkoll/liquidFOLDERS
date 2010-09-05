
// CMapSelectionCtrl.cpp: Implementierung der Klasse CMapSelectionCtrl
//

#include "stdafx.h"
#include "CMapSelectionCtrl.h"
#include "Resource.h"


// CMapSelectionCtrl
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

CMapSelectionCtrl::CMapSelectionCtrl()
	: CWnd()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = LFCommDlgDLL.hModule;

	if (!(::GetClassInfo(hInst, L"CMapSelectionCtrl", &wndcls)))
	{
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = hInst;
		wndcls.hIcon = NULL;
		wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndcls.hbrBackground = NULL;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = L"CMapSelectionCtrl";

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	// Karte und Icon laden
	if (!Map1)
	{
		Map1 = new CGdiPlusBitmapResource();
		Map1->Load(IDB_EARTHMAP_1024, _T("PNG"), LFCommDlgDLL.hResource);
	}
	m_Indicator = new CGdiPlusBitmapResource();
	m_Indicator->Load(IDB_LOCATIONINDICATOR_8, _T("PNG"), LFCommDlgDLL.hResource);
	m_Coord.Latitude = 0;
	m_Coord.Longitude = 0;

	blink = TRUE;
	remainVisible = 0;
}

CMapSelectionCtrl::~CMapSelectionCtrl()
{
	if (m_Indicator)
		delete m_Indicator;
}

void CMapSelectionCtrl::OnBlink()
{
	if (remainVisible)
	{
		remainVisible--;
	}
	else
	{
		blink = !blink;
		Invalidate();
	}
}

void CMapSelectionCtrl::SetGeoCoordinates(const LFGeoCoordinates _Coord)
{
	m_Coord = _Coord;
	SendUpdateMsg();
}

void CMapSelectionCtrl::UpdateLocation(CPoint point)
{
	CRect rect;
	GetClientRect(rect);

	double latitude = (((point.y-1)*180.0)/rect.Height())-90.0;
	if (latitude<-90.0)
		latitude = -90.0;
	if (latitude>90.0)
		latitude = 90.0;
	double longitude = (((point.x-1)*360.0)/rect.Width())-180.0;
	if (longitude<-180.0)
		longitude = -180.0;
	if (longitude>180.0)
		longitude = 180.0;

	m_Coord.Latitude = latitude;
	m_Coord.Longitude = longitude;
	SendUpdateMsg();
}

void CMapSelectionCtrl::SendUpdateMsg()
{
	blink = TRUE;
	remainVisible = 1;
	Invalidate();

	tag.hdr.code = MAP_UPDATE_LOCATION;
	tag.hdr.hwndFrom = m_hWnd;
	tag.hdr.idFrom = GetDlgCtrlID();
	tag.pCoord = &m_Coord;
	::PostMessage(GetParent()->m_hWnd, WM_NOTIFY, tag.hdr.idFrom, LPARAM(&tag));
}


BEGIN_MESSAGE_MAP(CMapSelectionCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

BOOL CMapSelectionCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMapSelectionCtrl::OnNcPaint()
{
	if (GetStyle() & WS_BORDER)
		CMFCVisualManager::GetInstance()->OnDrawControlBorder(this);
}

void CMapSelectionCtrl::OnPaint()
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

	Graphics g(dc.m_hDC);
	g.SetCompositingMode(CompositingModeSourceOver);
	g.DrawImage(Map1->m_pBitmap, 0, 0, rect.Width(), rect.Height());

	if (blink)
	{
		int cx = (int)((m_Coord.Longitude+180)*rect.Width()/360)+1;
		int cy = (int)((m_Coord.Latitude+90)*rect.Height()/180)+1;
		int h = m_Indicator->m_pBitmap->GetHeight();
		int l = m_Indicator->m_pBitmap->GetWidth();
		g.DrawImage(m_Indicator->m_pBitmap, cx-l/2, cy-h/2);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}

void CMapSelectionCtrl::OnLButtonDown(UINT /*nFlags*/, CPoint point)
{
	UpdateLocation(point);
}

void CMapSelectionCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	if (nFlags & MK_LBUTTON)
		UpdateLocation(point);
}
