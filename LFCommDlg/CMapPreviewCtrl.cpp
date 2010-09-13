
// CMapPreviewCtrl.cpp: Implementierung der Klasse CMapPreviewCtrl
//

#include "stdafx.h"
#include "CMapPreviewCtrl.h"
#include "LFApplication.h"
#include "Resource.h"


// CMapPreviewCtrl
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

CMapPreviewCtrl::CMapPreviewCtrl()
	: CWnd()
{
	WNDCLASS wndcls;
	HINSTANCE hInst = LFCommDlgDLL.hModule;

	if (!(::GetClassInfo(hInst, L"CMapPreviewCtrl", &wndcls)))
	{
		wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
		wndcls.lpfnWndProc = ::DefWindowProc;
		wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
		wndcls.hInstance = hInst;
		wndcls.hIcon = NULL;
		wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
		wndcls.hbrBackground = NULL;
		wndcls.lpszMenuName = NULL;
		wndcls.lpszClassName = L"CMapPreviewCtrl";

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	// Karte und Icon laden
	if (!Map2)
	{
		Map2 = new CGdiPlusBitmapResource();
		Map2->Load(IDB_EARTHMAP_2048, _T("PNG"), LFCommDlgDLL.hResource);
	}
	m_Indicator = new CGdiPlusBitmapResource();
	m_Indicator->Load(IDB_LOCATIONINDICATOR_16, _T("PNG"), LFCommDlgDLL.hResource);
	m_Airport = NULL;
	m_Location.Latitude = 0;
	m_Location.Longitude = 0;
}

CMapPreviewCtrl::~CMapPreviewCtrl()
{
	if (m_Indicator)
		delete m_Indicator;
}

void CMapPreviewCtrl::Update(LFAirport* _Airport)
{
	if (_Airport!=m_Airport)
	{
		m_Airport = _Airport;
		if (_Airport)
		{
			FontFamily fontFamily(((LFApplication*)AfxGetApp())->GetDefaultFontFace());
			wchar_t pszBuf[4];
			MultiByteToWideChar(CP_ACP, 0, m_Airport->Code, 4, pszBuf, 4);

			StringFormat strformat;
			m_TextPath.Reset();
			m_TextPath.AddString(pszBuf, (int)wcslen(pszBuf), &fontFamily, FontStyleRegular, 21, Gdiplus::Point(0, 0), &strformat);

			m_FirstPathDraw = TRUE;
		}

		Invalidate();
	}
}


BEGIN_MESSAGE_MAP(CMapPreviewCtrl, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_NCPAINT()
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CMapPreviewCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return TRUE;
}

void CMapPreviewCtrl::OnNcPaint()
{
	if (GetStyle() & WS_BORDER)
		CMFCVisualManager::GetInstance()->OnDrawControlBorder(this);
}

void CMapPreviewCtrl::OnPaint()
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
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

	// Karte
	if (m_Airport)
		m_Location = m_Airport->Location;

	int L = Map2->m_pBitmap->GetWidth();
	int H = Map2->m_pBitmap->GetHeight();
	int LocX = (int)(((m_Location.Longitude+180.0)*L)/360.0);
	int LocY = (int)(((m_Location.Latitude+90.0)*H)/180.0);
	int PosX = -LocX+rect.Width()/2;
	int PosY = -LocY+rect.Height()/2;
	if (PosY>1)
	{
		PosY = 1;
	}
	else
		if (PosY<rect.Height()-H)
		{
			PosY = rect.Height()-H;
		}
	if (PosX>1)
		g.DrawImage(Map2->m_pBitmap, PosX-L, PosY, L, H);
	g.DrawImage(Map2->m_pBitmap, PosX, PosY, L, H);
	if (PosX<rect.Width()-L)
		g.DrawImage(Map2->m_pBitmap, PosX+L, PosY, L, H);

	if (m_Airport)
	{
		// Punkt
		LocX += PosX-m_Indicator->m_pBitmap->GetWidth()/2+1;
		LocY += PosY-m_Indicator->m_pBitmap->GetHeight()/2+1;
		g.DrawImage(m_Indicator->m_pBitmap, LocX, LocY);

		// Pfad verschieben
		if (m_FirstPathDraw)
		{
			Rect tr;
			m_TextPath.GetBounds(&tr);

			int FntX = LocX+m_Indicator->m_pBitmap->GetWidth();
			int FntY = LocY-tr.Y;

			if (FntY<10)
			{
				FntY = 10;
			}
			else
				if (FntY+tr.Height+10>rect.Height())
				{
					FntY = rect.Height()-tr.Height-10;
				}
			Matrix m;
			m.Translate((REAL)FntX, (REAL)FntY-1.25f);
			m_TextPath.Transform(&m);
			m_FirstPathDraw = FALSE;
		}

		// Text
		Pen pen(Color(0, 0, 0), 3.0);
		pen.SetLineJoin(LineJoinRound);
		g.DrawPath(&pen, &m_TextPath);
		SolidBrush brush(Color(255, 255, 255));
		g.FillPath(&brush, &m_TextPath);
	}

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);
	dc.SelectObject(pOldBitmap);
}
