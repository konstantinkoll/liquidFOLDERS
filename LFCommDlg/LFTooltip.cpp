
// LFTooltip.cpp: Implementierung der Klasse LFTooltip
//

#include "stdafx.h"
#include "LFCommDlg.h"


// LFTooltip
//

#define MARGIN           4
#define BORDER           8
#define SHADOWSIZE       8
#define SHADOWOFFSET     -SHADOWSIZE/2+1

LFTooltip::LFTooltip()
	: CWnd()
{
	m_ContentRect.SetRectEmpty();
}

BOOL LFTooltip::Create()
{
	const CString className = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, LFGetApp()->LoadStandardCursor(IDC_ARROW));

	return CWnd::CreateEx(WS_EX_TOPMOST | WS_EX_NOACTIVATE | WS_EX_LAYERED, className, _T(""), WS_POPUP, 0, 0, 0, 0, NULL, NULL);
}

void LFTooltip::ShowTooltip(const CPoint& point, const CString& strCaption, const CString& strText, HICON hIcon, HBITMAP hBitmap)
{
	ASSERT(IsWindow(m_hWnd));

	if (strCaption.IsEmpty() && strText.IsEmpty())
		return;

	// Get screen size
	MONITORINFO MonitorInfo;
	MonitorInfo.cbSize = sizeof(MONITORINFO);

	CRect rectScreen;
	if (GetMonitorInfo(MonitorFromPoint(point, MONITOR_DEFAULTTONEAREST), &MonitorInfo))
	{
		rectScreen = MonitorInfo.rcWork;
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
		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_SmallBoldFont);
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

		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_SmallFont);
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
		{
			if (GetObject(IconInfo.hbmColor, sizeof(Bitmap), &Bitmap))
			{
				Size.cx += Bitmap.bmWidth+2*MARGIN;
				Size.cy = max(Size.cy, Bitmap.bmHeight);
			}

			DeleteObject(IconInfo.hbmColor);
			DeleteObject(IconInfo.hbmMask);
		}
	}
	else
		if (hBitmap)
			if (GetObject(hBitmap, sizeof(Bitmap), &Bitmap))
			{
				Size.cx += Bitmap.bmWidth+2*MARGIN;
				Size.cy = max(Size.cy, Bitmap.bmHeight);
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

	m_ContentRect = rectWindow;

	const BOOL Themed = IsCtrlThemed();
	if (Themed)
	{
		m_ContentRect.OffsetRect(0, SHADOWOFFSET);

		rectWindow.InflateRect(SHADOWSIZE, SHADOWSIZE);
		rectWindow.bottom += SHADOWOFFSET;
	}

	// Prepare paint
	HBITMAP hWindowBitmap = CreateTransparentBitmap(rectWindow.Width(), rectWindow.Height());
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hWindowBitmap);

	CRect rectBitmap(0, 0, rectWindow.Width(), rectWindow.Height());
	CRect rect(rectBitmap);

	Graphics g(dc);

	// Draw background
	CRect rectAlpha;

	if (Themed)
	{
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		for (UINT a=0; a<SHADOWSIZE; a++)
		{
			GraphicsPath path;
			CreateRoundRectangle(rect, SHADOWSIZE+5-a, path);

			Pen pen(Color(((a+2)*(a+2)*(a+2)/6)<<24));
			g.DrawPath(&pen, &path);

			rect.DeflateRect(1, 1);
		}

		rect.top += SHADOWOFFSET;
		rect.DeflateRect(1, 1);

		g.SetPixelOffsetMode(PixelOffsetModeHalf);

		const INT y1 = 6;
		const INT y2 = (rect.Height()-y1)*2/5;

		LinearGradientBrush brush1(Point(0, rect.top), Point(0, rect.bottom-y2), Color(0xFF323232), Color(0xFF2C2C2C));
		g.FillRectangle(&brush1, rect.left+2, rect.top, rect.Width()-4, 1);
		g.FillRectangle(&brush1, rect.left+1, rect.top+1, rect.Width()-2, 1);
		g.FillRectangle(&brush1, rect.left, rect.top+2, rect.Width(), rect.Height()-y2-2);

		LinearGradientBrush brush2(Point(0, rect.bottom-y2), Point(0, rect.bottom), Color(0xFF2C2C2C), Color(0xFF0C0C0C));
		g.FillRectangle(&brush2, rect.left, rect.bottom-y2, rect.Width(), y2-2);
		g.FillRectangle(&brush2, rect.left+1, rect.bottom-2, rect.Width()-2, 1);
		g.FillRectangle(&brush2, rect.left+2, rect.bottom-1, rect.Width()-4, 1);

		LinearGradientBrush brush3(Point(0, rect.top), Point(0, rect.top+y1), Color(0x30FFFFFF), Color(0x00FFFFFF));
		g.FillRectangle(&brush3, rect.left+2, rect.top, rect.Width()-4, 1);
		g.FillRectangle(&brush3, rect.left+1, rect.top+1, rect.Width()-2, 1);
		g.FillRectangle(&brush3, rect.left, rect.top+2, rect.Width(), y1-2);

		g.SetPixelOffsetMode(PixelOffsetModeNone);

		GraphicsPath pathInner;
		CreateRoundRectangle(rect, 4, pathInner);

		TextureBrush brush4(LFGetApp()->GetCachedResourceImage(IDB_BACKGROUND_TOOLTIP));
		g.FillPath(&brush4, &pathInner);

		rect.InflateRect(1, 1);
		GraphicsPath pathOuter;
		CreateRoundRectangle(rect, 5, pathOuter);

		Pen pen(Color(0xFF000000));
		g.DrawPath(&pen, &pathOuter);

		LinearGradientBrush brush5(Point(0, rect.top), Point(0, rect.bottom), Color(0x28FFFFFF), Color(0x18FFFFFF));

		pen.SetBrush(&brush5);
		g.DrawPath(&pen, &pathInner);

		rect.DeflateRect(BORDER, BORDER);
		rectAlpha = rect;
	}
	else
	{
		rect.DeflateRect(1, 1);
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
		rect.left += (Bitmap.bmWidth==34 ? 32 : Bitmap.bmWidth)+2*MARGIN;
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

				g.SetSmoothingMode(SmoothingModeNone);

				Pen pen(Color(0x40FFFFFF));
				g.DrawRectangle(&pen, rect.left, rect.top, Bitmap.bmWidth-1, Bitmap.bmHeight-1);
			}

			SelectObject(dcBitmap, hOldBitmap);

			rect.left += Bitmap.bmWidth+2*MARGIN;
		}

	if (!strCaption.IsEmpty())
	{
		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_SmallBoldFont);
		dc.DrawText(strCaption, rect, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

		rect.top += LFGetApp()->m_SmallBoldFont.GetFontHeight()+MARGIN;

		dc.SelectObject(pOldFont);
	}

	if (!strText.IsEmpty())
	{
		if (Themed)
			dc.SetTextColor(0xF0F0F0);

		CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_SmallFont);
		dc.DrawText(strText, rect, DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
		dc.SelectObject(pOldFont);
	}

	// Fix alpha channel messed up by GDI
	GetObject(hWindowBitmap, sizeof(Bitmap), &Bitmap);

	for (LONG Row=rectAlpha.top; Row<rectAlpha.bottom; Row++)
	{
		LPBYTE Ptr = (LPBYTE)Bitmap.bmBits+Bitmap.bmWidthBytes*Row+rectAlpha.left*4+3;

		for (LONG Column=rectAlpha.left; Column<rectAlpha.right; Column++)
		{
			*Ptr = 0xFF;

			Ptr += 4;
		}
	}

	// Update system-managed bitmap of window
	POINT ptDst = { rectWindow.left, rectWindow.top };
	SIZE szWindow = { rectWindow.Width(), rectWindow.Height() };
	POINT ptSrc = { 0, 0 };
	UpdateLayeredWindow(&dc, &ptDst, &szWindow, &dc, &ptSrc, 0x000000, &BF, ULW_ALPHA);

	// TopMost
	SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

	// Clean up
	dc.SelectObject(hOldBitmap);

	DestroyIcon(hIcon);
	DeleteObject(hBitmap);
	DeleteObject(hWindowBitmap);

	// Show window
	if (!IsWindowVisible())
		ShowWindow(SW_SHOWNOACTIVATE);
}

void LFTooltip::HideTooltip()
{
	m_ContentRect.SetRectEmpty();

	if (IsWindow(m_hWnd))
		ShowWindow(SW_HIDE);
}

void LFTooltip::AppendAttribute(CString& Str, const CString& Name, const CString& Value)
{
	if (!Value.IsEmpty())
	{
		if (!Str.IsEmpty())
			Str.Append(_T("\n"));

		if (!Name.IsEmpty())
		{
			Str.Append(Name);
			Str.Append(_T(": "));
		}

		Str.Append(Value);
	}
}

void LFTooltip::AppendAttribute(CString& Str, UINT ResID, const CString& Value)
{
	AppendAttribute(Str, CString((LPCSTR)ResID), Value);
}

void LFTooltip::AppendAttribute(CString& Str, UINT ResID, LPCSTR pValue)
{
	ASSERT(pValue);

	AppendAttribute(Str, CString((LPCSTR)ResID), CString(pValue));
}

void LFTooltip::AppendAttribute(LPWSTR pStr, SIZE_T cCount, const CString& Name, const CString& Value)
{
	ASSERT(pStr);

	if (!Value.IsEmpty())
	{
		if (*pStr)
			wcscat_s(pStr, cCount, L"\n");

		if (!Name.IsEmpty())
		{
			wcscat_s(pStr, cCount, Name);
			wcscat_s(pStr, cCount, L": ");
		}

		wcscat_s(pStr, cCount, Value);
	}
}

void LFTooltip::AppendAttribute(LPWSTR pStr, SIZE_T cCount, UINT ResID, const CString& Value)
{
	ASSERT(pStr);

	AppendAttribute(pStr, cCount, CString((LPCSTR)ResID), Value);
}

void LFTooltip::AppendAttribute(LPWSTR pStr, SIZE_T cCount, UINT ResID, const LPCSTR pValue)
{
	ASSERT(pStr);
	ASSERT(pValue);

	AppendAttribute(pStr, cCount, CString((LPCSTR)ResID), CString(pValue));
}


BEGIN_MESSAGE_MAP(LFTooltip, CWnd)
	ON_WM_NCHITTEST()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

LRESULT LFTooltip::OnNcHitTest(CPoint point)
{
	LRESULT HitTest = CWnd::OnNcHitTest(point);

	if (HitTest==HTCLIENT)
		if (!m_ContentRect.PtInRect(point))
			HitTest = HTTRANSPARENT;

	return HitTest;
}

void LFTooltip::OnMouseMove(UINT /*nFlags*/, CPoint /*point*/)
{
	HideTooltip();
}
