
// LFTooltip.cpp: Implementierung der Klasse LFTooltip
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFTooltip
//

#define MARGIN           4
#define BORDER           8
#define SHADOWSIZE       8
#define SHADOWOFFSET     -SHADOWSIZE/2

BOOL LFTooltip::Create()
{
	CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return CWnd::CreateEx(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_LAYERED | WS_EX_TRANSPARENT, className, _T(""), WS_POPUP, 0, 0, 0, 0, NULL, NULL);
}

void LFTooltip::ShowTooltip(CPoint point, const CString& strCaption, const CString& strText, HICON hIcon, HBITMAP hBitmap)
{
	ASSERT(IsWindow(m_hWnd));

	// Get screen size
	MONITORINFO mi;
	mi.cbSize = sizeof(MONITORINFO);

	CRect rectScreen;
	if (GetMonitorInfo(MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST), &mi))
	{
		rectScreen = mi.rcWork;
	}
	else
	{
		SystemParametersInfo(SPI_GETWORKAREA, 0, &rectScreen, 0);
	}

	rectScreen.DeflateRect(2, 2);

	// Calculate size of tooltip
	CSize Size(0, 0);
	BITMAP Bitmap = { 0 };

	CDC dc;
	dc.CreateCompatibleDC(NULL);
	dc.SetBkMode(TRANSPARENT);

	INT TextHeight = 0;
	if (!strCaption.IsEmpty())
	{
		CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontBold);
		CSize sz = dc.GetTextExtent(strCaption);
		dc.SelectObject(pOldFont);

		Size.cx = max(Size.cx, sz.cx);
		Size.cy += sz.cy;
		TextHeight = max(TextHeight, sz.cy);

		if (!strText.IsEmpty())
			Size.cy += MARGIN;
	}

	if (!strText.IsEmpty())
	{
		CRect rectText(rectScreen);

		CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontTooltip);
		dc.DrawText(strText, rectText, DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX | DT_CALCRECT);
		dc.SelectObject(pOldFont);

		Size.cx = max(Size.cx, rectText.Width());
		Size.cy += rectText.Height();
		TextHeight = max(TextHeight, rectText.Height());
	}

	if (hIcon)
	{
		ICONINFO IconInfo;
		if (GetIconInfo(hIcon, &IconInfo))
			if (GetObject(IconInfo.hbmColor, sizeof(Bitmap), &Bitmap))
			{
				Size.cx += Bitmap.bmWidth+2*MARGIN;
				Size.cy = max(Size.cy, Bitmap.bmHeight);
			}
	}
	else
		if (hBitmap)
		{
			if (GetObject(hBitmap, sizeof(Bitmap), &Bitmap))
			{
				Size.cx += Bitmap.bmWidth+2*MARGIN;
				Size.cy = max(Size.cy, Bitmap.bmHeight);
			}
		}

	Size.cx += 2*BORDER;
	Size.cy += 2*BORDER;

	if (Size.cx>TextHeight*40)
		Size.cx = TextHeight*40;

	// Position
	CRect rectWindow(point.x, point.y+18-SHADOWOFFSET, point.x+Size.cx, point.y+18+Size.cy-SHADOWOFFSET);

	if (rectWindow.Width()>rectScreen.Width())
	{
		rectWindow.left = rectScreen.left;
		rectWindow.right = rectScreen.right;
	}
	else 
		if (rectWindow.right>rectScreen.right)
		{
			rectWindow.right = rectScreen.right;
			rectWindow.left = rectWindow.right-Size.cx;
		}
		else
			if (rectWindow.left<rectScreen.left)
			{
				rectWindow.left = rectScreen.left;
				rectWindow.right = rectWindow.left+Size.cx;
			}

	if (rectWindow.Height()>rectScreen.Height())
	{
		rectWindow.top = rectScreen.top;
		rectWindow.bottom = rectScreen.bottom;
	}
	else
		if (rectWindow.bottom>rectScreen.bottom)
		{
			rectWindow.bottom = point.y-1;
			rectWindow.top = rectWindow.bottom-Size.cy;
		}
		else
			if (rectWindow.top<rectScreen.top)
			{
				rectWindow.top = rectScreen.top;
				rectWindow.bottom = rectWindow.top+Size.cy;
			}

	rectWindow.InflateRect(SHADOWSIZE, SHADOWSIZE);
	rectWindow.bottom += SHADOWOFFSET;

	// Prepare paint
	HBITMAP hWindowBitmap = CreateTransparentBitmap(rectWindow.Width(), rectWindow.Height());
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hWindowBitmap);

	CRect rectBitmap(0, 0, rectWindow.Width(), rectWindow.Height());
	CRect rect(rectBitmap);

	Graphics g(dc);

	// Draw background
	CRect rectAlpha;

	BOOL Themed = IsCtrlThemed();
	if (Themed)
	{
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		for (UINT a=0; a<SHADOWSIZE; a++)
		{
			GraphicsPath path;
			CreateRoundRectangle(rect, SHADOWSIZE+5-a, path);

			Pen pen(Color((BYTE)(((a+1)*(a+1)*(a+1)>>2)), 0x00, 0x00, 0x00));
			g.DrawPath(&pen, &path);

			rect.DeflateRect(1, 1);
		}

		rect.top += SHADOWOFFSET;
		rect.DeflateRect(1, 1);

		g.SetPixelOffsetMode(PixelOffsetModeHalf);

		const INT y1 = 6;
		const INT y2 = (rect.Height()-y1)*2/5;

		LinearGradientBrush brush1(Point(0, rect.top), Point(0, rect.bottom-y2), Color(0x32, 0x32, 0x32), Color(0x2C, 0x2C, 0x2C));
		g.FillRectangle(&brush1, rect.left+2, rect.top, rect.Width()-4, 1);
		g.FillRectangle(&brush1, rect.left+1, rect.top+1, rect.Width()-2, 1);
		g.FillRectangle(&brush1, rect.left, rect.top+2, rect.Width(), rect.Height()-y2-2);

		LinearGradientBrush brush2(Point(0, rect.bottom-y2), Point(0, rect.bottom), Color(0x2C, 0x2C, 0x2C), Color(0x0C, 0x0C, 0x0C));
		g.FillRectangle(&brush2, rect.left, rect.bottom-y2, rect.Width(), y2-2);
		g.FillRectangle(&brush2, rect.left+1, rect.bottom-2, rect.Width()-2, 1);
		g.FillRectangle(&brush2, rect.left+2, rect.bottom-1, rect.Width()-4, 1);

		LinearGradientBrush brush3(Point(0, rect.top), Point(0, rect.top+y1), Color(0x30, 0xFF, 0xFF, 0xFF), Color(0x00, 0xFF, 0xFF, 0xFF));
		g.FillRectangle(&brush3, rect.left+2, rect.top, rect.Width()-4, 1);
		g.FillRectangle(&brush3, rect.left+1, rect.top+1, rect.Width()-2, 1);
		g.FillRectangle(&brush3, rect.left, rect.top+2, rect.Width(), y1-2);

		GraphicsPath pathInner;
		CreateRoundRectangle(rect, 4, pathInner);

		rect.InflateRect(1, 1);
		GraphicsPath pathOuter;
		CreateRoundRectangle(rect, 5, pathOuter);

		g.SetPixelOffsetMode(PixelOffsetModeNone);

		Pen pen(Color(0x00, 0x00, 0x00));
		g.DrawPath(&pen, &pathOuter);

		LinearGradientBrush brush4(Point(0, rect.top), Point(0, rect.bottom), Color(0x28, 0xFF, 0xFF, 0xFF), Color(0x18, 0xFF, 0xFF, 0xFF));

		pen.SetBrush(&brush4);
		g.DrawPath(&pen, &pathInner);

		rect.DeflateRect(BORDER, BORDER);
		rectAlpha = rect;
	}
	else
	{
		rect.DeflateRect(SHADOWSIZE+1, SHADOWSIZE+1);
		rect.top += SHADOWOFFSET;

		dc.FillSolidRect(rect, GetSysColor(COLOR_INFOBK));

		rect.InflateRect(1, 1);
		dc.Draw3dRect(rect, GetSysColor(COLOR_INFOTEXT), GetSysColor(COLOR_INFOTEXT));

		rectAlpha = rect;
		rect.DeflateRect(BORDER, BORDER);
	}

	// Draw interior
	dc.SetTextColor(Themed ? 0xFFFFFF : GetSysColor(COLOR_INFOTEXT));

	if (hIcon)
	{
		DrawIconEx(dc, rect.left, rect.top, hIcon, Bitmap.bmWidth, Bitmap.bmHeight, 0, NULL, DI_NORMAL);
		rect.left += Bitmap.bmWidth+2*MARGIN;
	}
	else
		if (hBitmap)
		{
			CDC dcBitmap;
			dcBitmap.CreateCompatibleDC(&dc);
			HBITMAP hOldBitmap = (HBITMAP)SelectObject(dcBitmap, hBitmap);

			if (Bitmap.bmBitsPixel==32)
			{
				dc.AlphaBlend(rect.left, rect.top, Bitmap.bmWidth, Bitmap.bmHeight, &dcBitmap, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, BF);
			}
			else
			{
				dc.BitBlt(rect.left, rect.top, Bitmap.bmWidth, Bitmap.bmHeight, &dcBitmap, 0, 0, SRCCOPY);

				Pen pen(Color(0x40, 0xFF, 0xFF, 0xFF));
				g.DrawRectangle(&pen, rect.left, rect.top, Bitmap.bmWidth, Bitmap.bmHeight);
			}

			SelectObject(dcBitmap, hOldBitmap);

			rect.left += Bitmap.bmWidth+2*MARGIN;
		}

	if (!strCaption.IsEmpty())
	{
		CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontBold);
		dc.DrawText(strCaption, rect, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
		rect.top += dc.GetTextExtent(strCaption).cy+MARGIN;
		dc.SelectObject(pOldFont);
	}

	if (!strText.IsEmpty())
	{
		if (Themed)
			dc.SetTextColor(0xF0F0F0);

		CFont* pOldFont = dc.SelectObject(&afxGlobalData.fontTooltip);
		dc.DrawText(strText, rect, DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
		dc.SelectObject(pOldFont);
	}

	// Fix alpha channel messed up by GDI
	GetObject(hWindowBitmap, sizeof(Bitmap), &Bitmap);

	for (LONG Row=rectAlpha.top; Row<rectAlpha.bottom; Row++)
	{
		BYTE* Ptr = (BYTE*)Bitmap.bmBits+Bitmap.bmWidthBytes*Row+rectAlpha.left*4+3;

		for (LONG Column=rectAlpha.left; Column<rectAlpha.right; Column++)
		{
			*Ptr = 0xFF;

			Ptr += 4;
		}
	}

	// Update system-managed bitmap of window
	POINT ptDst = { rectWindow.left, rectWindow.top };
	SIZE sz = { rectWindow.Width(), rectWindow.Height() };
	POINT ptSrc = { 0, 0 };
	UpdateLayeredWindow(&dc, &ptDst, &sz, &dc, &ptSrc, 0x000000, &BF, ULW_ALPHA);

	// Clean up
	dc.SelectObject(hOldBitmap);
	dc.DeleteDC();

	DestroyIcon(hIcon);
	DeleteObject(hBitmap);
	DeleteObject(hWindowBitmap);

	// Show window
	if (!IsWindowVisible())
		ShowWindow(SW_SHOWNOACTIVATE);
}

void LFTooltip::HideTooltip()
{
	if (IsWindow(m_hWnd))
		ShowWindow(SW_HIDE);
}
