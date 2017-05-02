
// CBackstageShadow.cpp: Implementierung der Klasse CBackstageShadow
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include <math.h>


// CBackstageShadow
//

#define SHADOWSIZE          19
#define SHADOWOFFSET        (SHADOWSIZE/2-1)
#define SHADOWSIDEWIDTH     (SHADOWSIZE+1)
#define SHADOWCORNER        (SHADOWSIDEWIDTH+BACKSTAGERADIUS)
#define TOPHEIGHT           (SHADOWCORNER-SHADOWOFFSET)
#define BOTTOMHEIGHT        SHADOWCORNER

LPBYTE CBackstageShadow::m_pShadowCorner = NULL;

CBackstageShadow::CBackstageShadow()
{
	m_Width = m_Height = 0;
	ZeroMemory(&m_wndTopLeft, sizeof(m_wndTopLeft));
}

CBackstageShadow::~CBackstageShadow()
{
	for (UINT a=0; a<4; a++)
		if (IsWindow(m_wndShadow[a]))
			m_wndShadow[a].DestroyWindow();
}

BOOL CBackstageShadow::Create()
{
	// Corner mask
	if (!m_pShadowCorner)
	{
		m_pShadowCorner = new BYTE[SHADOWCORNER*SHADOWCORNER];
		LPBYTE pByte = m_pShadowCorner;

		for (INT Row=SHADOWCORNER-1; Row>=0; Row--)
			for (INT Column=SHADOWCORNER-1; Column>=0; Column--)
				*(pByte++) = CalcOpacity((DOUBLE)SHADOWSIZE-sqrt((DOUBLE)(Row*Row+Column*Column))+(DOUBLE)BACKSTAGERADIUS);
	}

	// Create transparent windows
	CString className = AfxRegisterWndClass(0);

	BOOL Result = TRUE;

	for (UINT a=0; a<4; a++)
		if (!m_wndShadow[a].CreateEx(WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_TRANSPARENT, className, _T(""), WS_POPUP, 0, 0, 0, 0, NULL, NULL))
			Result = FALSE;

	return Result;
}

void CBackstageShadow::Update(UINT nID, CDC& dc, POINT ptSrc, SIZE szWindow, CWnd* pBackstageWnd, const CRect& rectWindow)
{
	ASSERT(nID<4);

	m_wndTopLeft[nID] = ptSrc;

	CPoint ptDst(ptSrc.x+rectWindow.left, ptSrc.y+rectWindow.top);
	m_wndShadow[nID].UpdateLayeredWindow(&dc, &ptDst, &szWindow, &dc, &ptSrc, 0x000000, &BF, ULW_ALPHA);

	m_wndShadow[nID].SetWindowPos(pBackstageWnd, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_SHOWWINDOW);
}

void CBackstageShadow::Update(CWnd* pBackstageWnd, CRect rectWindow)
{
	ASSERT(pBackstageWnd);

	rectWindow.InflateRect(SHADOWSIDEWIDTH, SHADOWSIDEWIDTH);
	rectWindow.top += SHADOWOFFSET;

	// Show window
	BOOL Visible = pBackstageWnd->IsWindowVisible() && !pBackstageWnd->IsZoomed() && !pBackstageWnd->IsIconic() && IsCtrlThemed();

	if (Visible && ((rectWindow.Width()!=m_Width) || (rectWindow.Height()!=m_Height)))
	{
		m_Width = rectWindow.Width();
		m_Height = rectWindow.Height();

		CDC dc;
		dc.CreateCompatibleDC(NULL);

		// Prepare paint
		HBITMAP hWindowBitmap = CreateTransparentBitmap(m_Width, m_Height);
		HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hWindowBitmap);

		BITMAP Bitmap;
		GetObject(hWindowBitmap, sizeof(Bitmap), &Bitmap);

		for (UINT a=0; a<SHADOWSIZE; a++)
		{
			BYTE Opacity = CalcOpacity(a);

			if (a<SHADOWSIZE-SHADOWOFFSET)
				HorizontalLine(Bitmap, a, m_Width-2*SHADOWCORNER, Opacity);

			HorizontalLine(Bitmap, m_Height-a-1, m_Width-2*SHADOWCORNER, Opacity);
			VerticalLine(Bitmap, a, m_Height-2*SHADOWCORNER+SHADOWOFFSET, Opacity);
			VerticalLine(Bitmap, m_Width-a-1, m_Height-2*SHADOWCORNER+SHADOWOFFSET, Opacity);
		}

		CornersTop(Bitmap, m_Width);
		CornersBottom(Bitmap, m_Width, m_Height);

		CRect rect(SHADOWSIZE, SHADOWSIZE-SHADOWOFFSET, m_Width-SHADOWSIZE, m_Height-SHADOWSIZE);

		Graphics g(dc);
		g.SetSmoothingMode(LFGetApp()->m_SmoothingModeAntiAlias8x8);

		GraphicsPath path;
		CreateRoundRectangle(rect, BACKSTAGERADIUS, path);

		Pen pen(Color(0xFF000000));
		g.DrawPath(&pen, &path);

		// Update system-managed bitmap of window
		Update(0, dc, CPoint(0, 0), CSize(m_Width, TOPHEIGHT), pBackstageWnd, rectWindow);
		Update(1, dc, CPoint(0, TOPHEIGHT), CSize(SHADOWSIDEWIDTH, m_Height-TOPHEIGHT-BOTTOMHEIGHT),pBackstageWnd, rectWindow);
		Update(2, dc, CPoint(m_Width-SHADOWSIDEWIDTH, TOPHEIGHT), CSize(SHADOWSIDEWIDTH, m_Height-TOPHEIGHT-BOTTOMHEIGHT), pBackstageWnd, rectWindow);
		Update(3, dc, CPoint(0, m_Height-BOTTOMHEIGHT), CSize(m_Width, BOTTOMHEIGHT), pBackstageWnd, rectWindow);

		// Clean up
		dc.SelectObject(hOldBitmap);

		DeleteObject(hWindowBitmap);
	}
	else
	{
		// Move behind window
		for (UINT a=0; a<4; a++)
			m_wndShadow[a].SetWindowPos(pBackstageWnd, rectWindow.left+m_wndTopLeft[a].x, rectWindow.top+m_wndTopLeft[a].y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE | (Visible ? SWP_SHOWWINDOW : SWP_HIDEWINDOW));
	}
}

void CBackstageShadow::Update(CWnd* pBackstageWnd)
{
	ASSERT(pBackstageWnd);

	CRect rectWindow;
	pBackstageWnd->GetWindowRect(rectWindow);

	Update(pBackstageWnd, rectWindow);
}

void CBackstageShadow::HorizontalLine(const BITMAP& Bitmap, UINT Row, UINT Width, BYTE Opacity)
{
	RGBQUAD* pRGBQUAD = (RGBQUAD*)((LPBYTE)Bitmap.bmBits+Row*Bitmap.bmWidthBytes+SHADOWCORNER*4);

	for (UINT Column=0; Column<Width; Column++)
		(pRGBQUAD++)->rgbReserved = Opacity;
}

void CBackstageShadow::VerticalLine(const BITMAP& Bitmap, UINT Column, UINT Height, BYTE Opacity)
{
	LPBYTE pByte = (LPBYTE)Bitmap.bmBits+(SHADOWCORNER-SHADOWOFFSET)*Bitmap.bmWidthBytes+Column*4+3;

	for (UINT Row=0; Row<Height; Row++)
	{
		*pByte = Opacity;

		pByte += Bitmap.bmWidthBytes;
	}
}

inline void CBackstageShadow::CornersTop(const BITMAP& Bitmap, UINT Width)
{
	ASSERT(m_pShadowCorner);

	LPBYTE pByteSrc = m_pShadowCorner;
	LPBYTE pByteLeft = (LPBYTE)Bitmap.bmBits+3;
	LPBYTE pByteRight = pByteLeft+(Width-1)*4;

	for (UINT Row=0; Row<SHADOWCORNER; Row++)
	{
		for (UINT Column=0; Column<SHADOWCORNER; Column++)
		{
			*pByteLeft = *pByteRight = *(pByteSrc++);

			pByteLeft += 4;
			pByteRight -= 4;
		}

		pByteLeft += Bitmap.bmWidthBytes-(SHADOWCORNER*4);
		pByteRight += Bitmap.bmWidthBytes+(SHADOWCORNER*4);
	}
}

inline void CBackstageShadow::CornersBottom(const BITMAP& Bitmap, UINT Width, UINT Height)
{
	ASSERT(m_pShadowCorner);

	LPBYTE pByteSrc = m_pShadowCorner;
	LPBYTE pByteLeft = (LPBYTE)Bitmap.bmBits+(Height-1)*Bitmap.bmWidthBytes+3;
	LPBYTE pByteRight = pByteLeft+(Width-1)*4;

	for (UINT Row=0; Row<SHADOWCORNER; Row++)
	{
		for (UINT Column=0; Column<SHADOWCORNER; Column++)
		{
			*pByteLeft = *pByteRight = *(pByteSrc++);

			pByteLeft += 4;
			pByteRight -= 4;
		}

		pByteLeft -= Bitmap.bmWidthBytes+(SHADOWCORNER*4);
		pByteRight -= Bitmap.bmWidthBytes-(SHADOWCORNER*4);
	}
}
