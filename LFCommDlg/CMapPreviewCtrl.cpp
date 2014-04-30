
// CMapPreviewCtrl.cpp: Implementierung der Klasse CMapPreviewCtrl
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "Resource.h"


// CMapPreviewCtrl
//

extern AFX_EXTENSION_MODULE LFCommDlgDLL;

CMapPreviewCtrl::CMapPreviewCtrl()
	: CWnd()
{
	WNDCLASS wndcls;
	ZeroMemory(&wndcls, sizeof(wndcls));
	wndcls.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc = ::DefWindowProc;
	wndcls.cbClsExtra = wndcls.cbWndExtra = 0;
	wndcls.hIcon = NULL;
	wndcls.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndcls.hbrBackground = NULL;
	wndcls.lpszMenuName = NULL;
	wndcls.lpszClassName = L"CMapPreviewCtrl";

	if (!(::GetClassInfo(AfxGetInstanceHandle(), L"CMapPreviewCtrl", &wndcls)))
	{
		wndcls.hInstance = AfxGetInstanceHandle();

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}
	if (!(::GetClassInfo(LFCommDlgDLL.hModule, L"CMapPreviewCtrl", &wndcls)))
	{
		wndcls.hInstance = LFCommDlgDLL.hModule;

		if (!AfxRegisterClass(&wndcls))
			AfxThrowResourceException();
	}

	p_Airport = NULL;
	m_Location.Latitude = m_Location.Longitude = 0;
}

void CMapPreviewCtrl::Update(LFAirport* pAirport)
{
	if (pAirport!=p_Airport)
	{
		p_Airport = pAirport;
		if (pAirport)
		{
			FontFamily fontFamily(LFGetApp()->GetDefaultFontFace());
			WCHAR pszBuf[4];
			MultiByteToWideChar(CP_ACP, 0, pAirport->Code, -1, pszBuf, 4);

			StringFormat strformat;
			m_TextPath.Reset();
			m_TextPath.AddString(pszBuf, (INT)wcslen(pszBuf), &fontFamily, FontStyleRegular, 21, Gdiplus::Point(0, 0), &strformat);

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
	DrawControlBorder(this);
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

	Graphics g(dc);
	g.SetCompositingMode(CompositingModeSourceOver);
	g.SetSmoothingMode(SmoothingModeAntiAlias);
	g.SetInterpolationMode(InterpolationModeHighQualityBicubic);

	CGdiPlusBitmap* pMap = LFGetApp()->GetCachedResourceImage(IDB_EARTHMAP, _T("JPG"), LFCommDlgDLL.hResource);
	CGdiPlusBitmap* pIndicator = LFGetApp()->GetCachedResourceImage(IDB_LOCATIONINDICATOR_16, _T("PNG"), LFCommDlgDLL.hResource);

	// Karte
	if (p_Airport)
		m_Location = p_Airport->Location;

	INT L = pMap->m_pBitmap->GetWidth();
	INT H = pMap->m_pBitmap->GetHeight();
	INT LocX = (INT)(((m_Location.Longitude+180.0)*L)/360.0);
	INT LocY = (INT)(((m_Location.Latitude+90.0)*H)/180.0);
	INT PosX = -LocX+rect.Width()/2;
	INT PosY = -LocY+rect.Height()/2;
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
		g.DrawImage(pMap->m_pBitmap, PosX-L, PosY);
	g.DrawImage(pMap->m_pBitmap, PosX, PosY);
	if (PosX<rect.Width()-L)
		g.DrawImage(pMap->m_pBitmap, PosX+L, PosY);

	if (p_Airport)
	{
		// Punkt
		LocX += PosX-pIndicator->m_pBitmap->GetWidth()/2+1;
		LocY += PosY-pIndicator->m_pBitmap->GetHeight()/2+1;
		g.DrawImage(pIndicator->m_pBitmap, LocX, LocY);

		// Pfad verschieben
		if (m_FirstPathDraw)
		{
			Rect tr;
			m_TextPath.GetBounds(&tr);

			INT FntX = LocX+pIndicator->m_pBitmap->GetWidth();
			INT FntY = LocY-tr.Y;

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
