
// LFCommDlg.cpp : Definiert die Initialisierungsroutinen für die DLL.
//

#include "stdafx.h"
#include "LFCore.h"
#include "LFCommDlg.h"


BOOL DuplicateGlobalMemory(const HGLOBAL hSrc, HGLOBAL& hDst)
{
	if (!hSrc)
	{
		hDst = NULL;
		return FALSE;
	}

	SIZE_T Size = GlobalSize(hSrc);

	if ((hDst=GlobalAlloc(GMEM_MOVEABLE, Size))==NULL)
		return FALSE;

	LPVOID pSrc = GlobalLock(hSrc);
	LPVOID pDst = GlobalLock(hDst);

	memcpy(pDst, pSrc, Size);

	GlobalUnlock(hSrc);
	GlobalUnlock(hDst);

	return TRUE;
}

void GetFileVersion(HMODULE hModule, CString& Version, CString* Copyright)
{
	Version.Empty();

	if (Copyright)
		Copyright->Empty();

	CString modFilename;
	if (GetModuleFileName(hModule, modFilename.GetBuffer(MAX_PATH), MAX_PATH)>0)
	{
		modFilename.ReleaseBuffer(MAX_PATH);
		DWORD dwHandle = 0;
		DWORD dwSize = GetFileVersionInfoSize(modFilename, &dwHandle);
		if (dwSize>0)
		{
			LPBYTE lpInfo = new BYTE[dwSize];
			ZeroMemory(lpInfo, dwSize);

			if (GetFileVersionInfo(modFilename, 0, dwSize, lpInfo))
			{
				UINT valLen = 0;
				LPVOID valPtr = NULL;
				LPCWSTR valData = NULL;

				if (VerQueryValue(lpInfo, _T("\\"), &valPtr, &valLen))
				{
					VS_FIXEDFILEINFO* pFinfo = (VS_FIXEDFILEINFO*)valPtr;
					Version.Format(_T("%u.%u.%u"), 
						(UINT)((pFinfo->dwProductVersionMS >> 16) & 0xFF),
						(UINT)((pFinfo->dwProductVersionMS) & 0xFF),
						(UINT)((pFinfo->dwProductVersionLS >> 16) & 0xFF));
				}

				if (Copyright)
					*Copyright = VerQueryValue(lpInfo, _T("StringFileInfo\\000004E4\\LegalCopyright"), (void**)&valData, &valLen) ? valData : _T("© liquidFOLDERS");
			}

			delete[] lpInfo;
		}
	}
}


// Draw

BLENDFUNCTION BF = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };

struct BITMAPINFO16
{
	BITMAPINFOHEADER bmiHeader;
	RGBQUAD bmiColors[16];
};

static const BYTE DefaultPalette[64] = {
	255, 255, 255, 0,
	0, 255, 255, 0,
	0, 102, 255, 0,
	0, 0, 221, 0,
	153, 0, 255, 0,
	153, 0, 51, 0,
	204, 0, 0, 0,
	255, 153, 0, 0,
	0, 170, 0, 0,
	0, 102, 0, 0,
	102, 51, 0, 0,
	51, 102, 153, 0,
	187, 187, 187, 0,
	136, 136, 136, 0,
	68, 68, 68, 0,
	0, 0, 0, 0
};

void CreateRoundRectangle(LPCRECT lpRect, INT Radius, GraphicsPath& Path)
{
	ASSERT(lpRect);

	const INT Diameter = Radius*2+1;
	const INT Right = lpRect->right-Diameter-1;
	const INT Bottom = lpRect->bottom-Diameter-1;

	Path.Reset();
	Path.AddArc(lpRect->left, lpRect->top, Diameter, Diameter, 180, 90);
	Path.AddArc(Right, lpRect->top, Diameter, Diameter, 270, 90);
	Path.AddArc(Right-1, Bottom-1, Diameter+1, Diameter+1, 0, 90);
	Path.AddArc(lpRect->left, Bottom-1, Diameter+1, Diameter+1, 90, 90);
	Path.CloseFigure();
}

void CreateRoundTop(LPCRECT lpRect, INT Radius, GraphicsPath& Path)
{
	ASSERT(lpRect);

	const INT Diameter = Radius*2+1;
	const INT Right = lpRect->right-Diameter-1;

	// 10° less to both sides
	Path.Reset();
	Path.AddArc(lpRect->left, lpRect->top, Diameter, Diameter, 190, 80);
	Path.AddArc(Right, lpRect->top, Diameter, Diameter, 270, 80);
}

void CreateReflectionRectangle(LPCRECT lpRect, INT Radius, GraphicsPath& Path)
{
	ASSERT(lpRect);

	Path.Reset();

	const INT Diameter = Radius*2+1;
	const INT Width = lpRect->right-lpRect->left;

	Path.AddLine(Point(lpRect->left+Width*2/3, lpRect->top), Point(lpRect->left+Diameter, lpRect->top));
	Path.AddArc(lpRect->left, lpRect->top, Diameter, Diameter, 180, 90);
	Path.AddLine(Point(lpRect->left, lpRect->top+Diameter), Point(lpRect->left, lpRect->bottom-Diameter));
	Path.AddArc(lpRect->left, lpRect->bottom, Diameter, Diameter, 180, -90);
	Path.AddLine(Point(lpRect->left+Diameter, lpRect->bottom), Point(lpRect->left+Width/3, lpRect->bottom));

	Path.CloseFigure();
}

BOOL IsCtrlThemed()
{
	return LFGetApp()->m_ThemeLibLoaded ? LFGetApp()->zIsAppThemed() : FALSE;
}

HBITMAP CreateMaskBitmap(LONG Width, LONG Height)
{
	BITMAPINFO16 DIB;
	ZeroMemory(&DIB, sizeof(DIB));

	DIB.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	DIB.bmiHeader.biWidth = Width;
	DIB.bmiHeader.biHeight = -Height;
	DIB.bmiHeader.biPlanes = 1;
	DIB.bmiHeader.biBitCount = 4;
	DIB.bmiHeader.biCompression = BI_RGB;
	DIB.bmiHeader.biClrUsed = 16;

	memcpy(DIB.bmiColors, DefaultPalette, sizeof(DefaultPalette));

	return CreateDIBSection(NULL, (LPBITMAPINFO)&DIB, DIB_RGB_COLORS, NULL, NULL, 0);
}

HBITMAP CreateTransparentBitmap(LONG Width, LONG Height)
{
	BITMAPINFO DIB;
	ZeroMemory(&DIB, sizeof(DIB));

	DIB.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	DIB.bmiHeader.biWidth = Width;
	DIB.bmiHeader.biHeight = -Height;
	DIB.bmiHeader.biPlanes = 1;
	DIB.bmiHeader.biBitCount = 32;
	DIB.bmiHeader.biCompression = BI_RGB;

	return CreateDIBSection(NULL, &DIB, DIB_RGB_COLORS, NULL, NULL, 0);
}

HBITMAP CreateTruecolorBitmap(LONG Width, LONG Height)
{
	BITMAPINFO DIB;
	ZeroMemory(&DIB, sizeof(DIB));

	DIB.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	DIB.bmiHeader.biWidth = Width;
	DIB.bmiHeader.biHeight = -Height;
	DIB.bmiHeader.biPlanes = 1;
	DIB.bmiHeader.biBitCount = 24;
	DIB.bmiHeader.biCompression = BI_RGB;

	return CreateDIBSection(NULL, &DIB, DIB_RGB_COLORS, NULL, NULL, 0);
}

CBitmap* CreateTruecolorBitmapObject(LONG Width, LONG Height)
{
	CBitmap* pBitmap = new CBitmap();
	pBitmap->Attach(CreateTruecolorBitmap(Width, Height));
	pBitmap->SetBitmapDimension(Width, Height);

	return pBitmap;
}

void DrawLocationIndicator(Graphics& g, INT x, INT y, INT Size)
{
	const REAL Diameter = (REAL)Size/8.0f;
	Rect rect(x+(INT)(Diameter/2.0f), y+(INT)(Diameter/2.0f), Size-(INT)Diameter-1, Size-(INT)Diameter-1);

	SolidBrush brush(Color(0xFFFF0000));
	g.FillEllipse(&brush, rect);

	Pen pen(Color(0xFFFFFFFF), Diameter);
	g.DrawEllipse(&pen, rect);
}

void DrawLocationIndicator(CDC& dc, INT x, INT y, INT Size)
{
	Graphics g(dc);
	g.SetPixelOffsetMode(PixelOffsetModeHalf);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	DrawLocationIndicator(g, x, y, Size);
}

void DrawCategory(CDC& dc, CRect rect, LPCWSTR Caption, LPCWSTR Hint, BOOL Themed)
{
	ASSERT(Caption);

	rect.DeflateRect(0, LFCATEGORYPADDING);

	CFont* pOldFont = dc.SelectObject(&LFGetApp()->m_LargeFont);
	dc.SetTextColor(Themed ? 0xCC3300 : GetSysColor(COLOR_WINDOWTEXT));
	dc.DrawText(Caption, rect, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

	CRect rectLine(rect);
	dc.DrawText(Caption, rectLine, DT_LEFT | DT_TOP | DT_SINGLELINE | DT_END_ELLIPSIS | DT_CALCRECT | DT_NOPREFIX);
	rectLine.right += 2*LFCATEGORYPADDING;

	if (rectLine.right<=rect.right)
		if (Themed)
		{
			Graphics g(dc);
			g.SetPixelOffsetMode(PixelOffsetModeHalf);
	
			LinearGradientBrush brush(Point(rectLine.right, rectLine.top), Point(rect.right, rectLine.top), Color(0xFFE2E2E2), Color(0x00E2E2E2));
			g.FillRectangle(&brush, rectLine.right, rectLine.top+(rectLine.Height()+1)/2, rect.right-rectLine.right, 1);
		}
		else
		{
			CPen pen(PS_SOLID, 1, GetSysColor(COLOR_3DSHADOW));

			CPen* pOldPen = dc.SelectObject(&pen);
			dc.MoveTo(rectLine.right, rect.top+(rectLine.Height()+1)/2);
			dc.LineTo(rect.right, rect.top+(rectLine.Height()+1)/2);
			dc.SelectObject(pOldPen);
		}

	if (Hint)
		if (Hint[0]!=L'\0')
		{
			dc.SetTextColor(Themed ? 0xA39791 : GetSysColor(COLOR_3DSHADOW));

			rect.top += rectLine.Height();

			dc.SelectObject(&LFGetApp()->m_DefaultFont);
			dc.DrawText(Hint, rect, DT_LEFT | DT_TOP | DT_NOPREFIX | (rect.Height()>=2*LFGetApp()->m_DefaultFont.GetFontHeight() ? DT_WORDBREAK : DT_END_ELLIPSIS));
		}

	dc.SelectObject(pOldFont);
}

void DrawReflection(Graphics& g, LPCRECT lpRect)
{
	GraphicsPath pathReflection;
	CreateReflectionRectangle(lpRect, 1, pathReflection);

	const INT Height = lpRect->bottom-lpRect->top;
	const INT Width = lpRect->right-lpRect->left;

	LinearGradientBrush brush(Rect(lpRect->left, lpRect->top, lpRect->left+Width, Height), Color(0x30FFFFFF), Color(0x00FFFFFF), LinearGradientModeForwardDiagonal);
	g.FillPath(&brush, &pathReflection);
}

void DrawListItemBackground(CDC& dc, LPCRECT rectItem, BOOL Themed, BOOL WinFocused, BOOL Hover, BOOL Focused, BOOL Selected, COLORREF TextColor, BOOL ShowFocusRect)
{
	if (Themed)
	{
		if (Hover || Focused || Selected)
		{
			Graphics g(dc);
			g.SetPixelOffsetMode(PixelOffsetModeHalf);

			GraphicsPath pathOuter;
			CreateRoundRectangle(rectItem, 3, pathOuter);

			CRect rect(rectItem);
			rect.DeflateRect(1, 1);

			GraphicsPath pathInner;
			CreateRoundRectangle(rect, 2, pathInner);

			if (Selected)
			{
				LinearGradientBrush brush1(Point(0, rectItem->top), Point(0, rectItem->bottom), Color(0xFF20A0FF), Color(0xFF1080E0));
				g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), rect.Height());
			}
			else
				if (Hover)
				{
					LinearGradientBrush brush1(Point(0, rectItem->top), Point(0, rectItem->bottom), Color(0xFFF9FCFF), Color(0xFFE0EBFA));
					g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), rect.Height());
				}

			g.SetPixelOffsetMode(PixelOffsetModeNone);
			g.SetSmoothingMode(SmoothingModeAntiAlias);

			if ((ShowFocusRect && WinFocused) || Hover || Selected)
				if ((Focused && ShowFocusRect && WinFocused) || Selected)
				{
					Pen pen1(Color(0xFF1080E0));
					g.DrawPath(&pen1, &pathOuter);
				}
				else
				{
					Pen pen1(Color(0xFF8AC0F0));
					g.DrawPath(&pen1, &pathOuter);
				}

			if (Hover || Selected)
			{
				Pen pen2(Color(((Hover && !Selected) ? 0x60000000 : 0x48000000) | 0xFFFFFF));
				g.DrawPath(&pen2, &pathInner);
			}
		}

		dc.SetTextColor(Selected ? 0xFFFFFF : TextColor!=(COLORREF)-1 ? TextColor : 0x000000);
	}
	else
	{
		if (Selected)
		{
			dc.FillSolidRect(rectItem, GetSysColor(COLOR_HIGHLIGHT));
			dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
			dc.SetBkColor(0x000000);
		}
		else
		{
			dc.SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
			dc.SetBkColor(GetSysColor(COLOR_WINDOW));
		}

		if (WinFocused && Focused)
			dc.DrawFocusRect(rectItem);

		if (TextColor!=(COLORREF)-1 && !Selected)
			dc.SetTextColor(TextColor);
	}
}

void DrawListItemForeground(CDC& dc, LPCRECT rectItem, BOOL Themed, BOOL /*WinFocused*/, BOOL Hover, BOOL /*Focused*/, BOOL Selected)
{
	if (Themed && (Hover || Selected))
	{
		Graphics g(dc);
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		CRect rect(rectItem);
		rect.DeflateRect(1, 1);

		DrawReflection(g, rect);
	}
}

void DrawSubitemBackground(CDC& dc, Graphics& g, CRect rect, BOOL Themed, BOOL Selected, BOOL Hover, BOOL ClipHorizontal)
{
	if (Hover || Selected)
		if (Themed)
		{
			COLORREF clr = Hover ? 0xB17F3C : 0x8B622C;

			if (ClipHorizontal)
			{
				dc.FillSolidRect(rect.left, rect.top, 1, rect.Height(), clr);
				dc.FillSolidRect(rect.right-1, rect.top, 1, rect.Height(), clr);
				rect.DeflateRect(1, 0);
			}
			else
			{
				dc.Draw3dRect(rect, clr, clr);
				rect.DeflateRect(1, 1);
			}

			g.SetPixelOffsetMode(PixelOffsetModeHalf);

			if (Hover)
			{
				LinearGradientBrush brush1(Point(rect.left, rect.top), Point(rect.left, rect.bottom), Color(0xFFFAFDFE), Color(0xFFE8F5FC));
				g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), rect.Height());

				rect.DeflateRect(1, 1);
				INT y = (rect.top+rect.bottom)/2;

				LinearGradientBrush brush2(Point(rect.left, rect.top), Point(rect.left, y), Color(0xFFEAF6FD), Color(0xFFD7EFFC));
				g.FillRectangle(&brush2, rect.left, rect.top, rect.Width(), y-rect.top);

				LinearGradientBrush brush3(Point(rect.left, y), Point(rect.left, rect.bottom), Color(0xFFBDE6FD), Color(0xFFA6D9F4));
				g.FillRectangle(&brush3, rect.left, y, rect.Width(), rect.bottom-y);
			}
			else
			{
				dc.FillSolidRect(rect, 0xF6E4C2);

				INT y = (rect.top+rect.bottom)/2;

				LinearGradientBrush brush2(Point(rect.left, y), Point(rect.left, rect.bottom), Color(0xFFA9D9F2), Color(0xFF90CBEB));
				g.FillRectangle(&brush2, rect.left, y, rect.Width(), rect.bottom-y);

				LinearGradientBrush brush3(Point(rect.left, rect.top), Point(rect.left, rect.top+2), Color(0x20163145), Color(0x00163145));
				g.FillRectangle(&brush3, rect.left, rect.top, rect.Width(), 2);

				LinearGradientBrush brush4(Point(rect.left, rect.top), Point(rect.left+2, rect.top), Color(0x20163145), Color(0x00163145));
				g.FillRectangle(&brush4, rect.left, rect.top, 2, rect.Height());
			}
		}
		else
		{
			dc.DrawEdge(rect, Selected ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT | BF_SOFT);
		}

	dc.SetTextColor(Themed ? Selected || Hover ? 0x000000 : 0x404040 : GetSysColor(COLOR_WINDOWTEXT));
}

void DrawMilledRectangle(Graphics& g, CRect rect, BOOL Backstage, INT Radius)
{
	rect.left++;
	rect.top++;

	g.SetPixelOffsetMode(PixelOffsetModeHalf);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	GraphicsPath path;
	CreateRoundRectangle(rect, Radius-1, path);

	SolidBrush brush1(Color(Backstage ? 0x80000000 : 0x18000000));
	g.FillPath(&brush1, &path);

	rect.left--;
	rect.top--;

	g.SetPixelOffsetMode(PixelOffsetModeNone);
	CreateRoundTop(rect, Radius, path);

	LinearGradientBrush brush2(Point(0, rect.top-1), Point(0, rect.top+5), Color(Backstage ? 0xF0000000 : 0x40000000), Color(0x00000000));

	Pen pen(&brush2);
	g.DrawPath(&pen, &path);

	CreateRoundRectangle(rect, 4, path);

	LinearGradientBrush brush3(Point(0, rect.top-1), Point(0, rect.bottom), Color(Backstage ? 0x00FFFFFF : 0x00E0E0E0), Color(Backstage ? 0x38FFFFFF : 0x60E0E0E0));

	pen.SetBrush(&brush3);
	g.DrawPath(&pen, &path);
}

void DrawBackstageSelection(CDC& dc, Graphics& g, const CRect& rect, BOOL Selected, BOOL Enabled, BOOL Themed)
{
	if (Selected && Enabled)
	{
		if (Themed)
		{
			LinearGradientBrush brush1(Point(rect.left, rect.top), Point(rect.left, rect.bottom-1), Color(0xC020A0FF), Color(0xC01078FF));
			g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), rect.Height()-1);

			SolidBrush brush2(Color(0x40FFFFFF));
			g.FillRectangle(&brush2, rect.left, rect.top, rect.Width(), 1);

			SolidBrush brush3(Color(0x18FFFFFF));
			g.FillRectangle(&brush3, rect.left, rect.top+1, 1, rect.Height()-3);
			g.FillRectangle(&brush3, rect.right-1, rect.top+1, 1, rect.Height()-3);
			g.FillRectangle(&brush3, rect.left, rect.bottom-2, rect.Width(), 1);

			dc.SetTextColor(0xFFFFFF);
		}
		else
		{
			dc.FillSolidRect(rect, GetSysColor(COLOR_HIGHLIGHT));
			dc.SetTextColor(GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
	}
	else
	{
		dc.SetTextColor(Enabled ? Themed ? 0xDACCC4 : GetSysColor(COLOR_3DFACE) : Themed ? 0x998981 : GetSysColor(COLOR_3DSHADOW));
	}
}

void DrawBackstageButtonBackground(CDC& dc, Graphics& g, CRect rect, BOOL Hover, BOOL Pressed, BOOL Enabled, BOOL Themed, BOOL Red)
{
	if (Pressed && Enabled)
	{
		if (Themed)
		{
			g.SetPixelOffsetMode(PixelOffsetModeNone);

			GraphicsPath path;
			CreateRoundRectangle(rect, 3, path);

			LinearGradientBrush brush1(Point(rect.left, rect.top), Point(rect.left, rect.bottom), Red ? Color(0xB0FF0000) : Color(0xE020A0FF), Red ? Color(0xA0FF0000) : Color(0xE01078FF));
			g.FillPath(&brush1, &path);

			Pen pen(Red ? Color(0x80600000) : Color(0x80062D60));
			g.DrawPath(&pen, &path);

			g.SetPixelOffsetMode(PixelOffsetModeHalf);

			rect.left++;
			rect.bottom = (++rect.top)+rect.Height()/2;

			if (Red)
			{
				CreateRoundRectangle(rect, 2, path);

				LinearGradientBrush brush2(Point(rect.left, rect.top), Point(rect.left, rect.bottom), Color(0x60FFFFFF), Color(0x20FFFFFF));
				g.FillPath(&brush2, &path);
			}
			else
			{
				g.SetPixelOffsetMode(PixelOffsetModeNone);

				rect.right--;
				CreateRoundTop(rect, 2, path);

				Pen pen(Color(0x40FFFFFF));
				g.DrawPath(&pen, &path);
			}

			dc.SetTextColor(0xFFFFFF);
		}
		else
		{
			rect.top++;
			dc.FillSolidRect(rect, Red ? 0x0000FF : GetSysColor(COLOR_HIGHLIGHT));

			dc.SetTextColor(Red ? 0xFFFFFF : GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
	}
	else
	{
		dc.SetTextColor(Enabled ? Hover ? 0xFFFFFF : Themed ? 0xDACCC4 : GetSysColor(COLOR_3DFACE) : Themed ? 0x998981 : GetSysColor(COLOR_3DSHADOW));
	}
}

void DrawLightButtonBackground(CDC& dc, CRect rect, BOOL Themed, BOOL Focused, BOOL Selected, BOOL Hover)
{
	if (Themed)
	{
		if (Focused || Selected || Hover)
		{
			Graphics g(dc);

			// Inner Border
			rect.DeflateRect(1, 1);

			if (Selected)
			{
				LinearGradientBrush brush1(Point(0, rect.top), Point(0, rect.bottom+1), Color(0x0CA0AEC4), Color(0x20505762));
				g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), rect.Height());

				LinearGradientBrush brush2(Point(rect.left, rect.top), Point(rect.left, rect.top+2), Color(0x20000000), Color(0x00000000));
				g.FillRectangle(&brush2, rect.left, rect.top, rect.Width(), 2);

				SolidBrush brush3(Color(0x18000000));
				g.FillRectangle(&brush3, rect.left, rect.top, 1, rect.Height());
			}
			else
				if (Hover)
				{
					SolidBrush brush1(Color(0x40FFFFFF));
					g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), rect.Height()/2);

					SolidBrush brush2(Color(0x28A0AFC3));
					g.FillRectangle(&brush2, rect.left, rect.top+rect.Height()/2+1, rect.Width(), rect.Height()-rect.Height()/2);
				}

			g.SetSmoothingMode(SmoothingModeAntiAlias);
			GraphicsPath path;

			if (!Selected)
			{
				CreateRoundRectangle(rect, 1, path);

				Pen pen(Color(0x80FFFFFF));
				g.DrawPath(&pen, &path);
			}

			// Outer Border
			rect.InflateRect(1, 1);
			CreateRoundRectangle(rect, 2, path);

			Pen pen(Color(Selected || Hover ? 0x80505762 : 0x40505762));
			g.DrawPath(&pen, &path);
		}
	}
	else
	{
		if (Selected || Hover)
		{
			dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
			dc.DrawEdge(rect, Selected ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT | BF_SOFT);
		}

		if (Focused)
		{
			rect.DeflateRect(2, 2);

			dc.SetTextColor(0x000000);
			dc.DrawFocusRect(rect);
		}
	}
}

void DrawWhiteButtonBorder(Graphics& g, LPCRECT lpRect, BOOL IncludeBottom)
{
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	GraphicsPath path;
	CreateRoundRectangle(lpRect, 3, path);

	LinearGradientBrush brush1(Point(0, lpRect->top-1), Point(0, lpRect->bottom), Color(0x0C000000), Color(0x00000000));

	Pen pen(&brush1);
	g.DrawPath(&pen, &path);

	if (IncludeBottom)
	{
		LinearGradientBrush brush2(Point(0, lpRect->top-1), Point(0, lpRect->bottom), Color(0x00FFFFFF), Color(0xFFFFFFFF));

		pen.SetBrush(&brush2);
		g.DrawPath(&pen, &path);
	}
}

void DrawWhiteButtonBackground(CDC& dc, Graphics& g, CRect rect, BOOL Themed, BOOL Focused, BOOL Selected, BOOL Hover, BOOL Disabled, BOOL DrawBorder)
{
	if (Themed)
	{
		if (DrawBorder)
			DrawWhiteButtonBorder(g, rect, FALSE);

		g.SetPixelOffsetMode(PixelOffsetModeHalf);
		g.SetSmoothingMode(SmoothingModeNone);

		// Inner border
		rect.DeflateRect(2, 2);

		if (Selected)
		{
			LinearGradientBrush brush1(Point(0, rect.top), Point(0, rect.bottom+1), Color(0xFFFAFAFC), Color(0xFFDDDDDF));
			g.FillRectangle(&brush1, rect.left, rect.top, rect.Width(), rect.Height());

			LinearGradientBrush brush2(Point(rect.left, rect.top), Point(rect.left, rect.top+3), Color(0x20000000), Color(0x00000000));
			g.FillRectangle(&brush2, rect.left, rect.top, rect.Width(), 3);

			LinearGradientBrush brush3(Point(rect.left, rect.top), Point(rect.left+3, rect.top), Color(0x18000000), Color(0x00000000));
			g.FillRectangle(&brush3, rect.left, rect.top, 2, rect.Height());
		}
		else
		{
			dc.FillSolidRect(rect, Hover ? 0xEFECEC : 0xF7F4F4);

			LinearGradientBrush brush1(Point(0, rect.top+1), Point(0, (rect.top+rect.bottom)/2+1), Color(0xFFFFFFFF), Color(Disabled ? 0x00FFFFFF : 0x40FFFFFF));
			g.FillRectangle(&brush1, rect.left, rect.top+1, rect.Width(), rect.Height()/2);

			if (!Disabled)
			{
				LinearGradientBrush brush2(Point(0, rect.bottom-3), Point(0, rect.bottom), Color(0x00000000), Color(Hover ? 0x20000000 : 0x10000000));
				g.FillRectangle(&brush2, rect.left, rect.bottom-3, rect.Width(), 3);
			}
		}

		g.SetPixelOffsetMode(PixelOffsetModeNone);
		g.SetSmoothingMode(SmoothingModeAntiAlias);

		rect.InflateRect(1, 1);

		GraphicsPath pathOuter;
		CreateRoundRectangle(rect, 2, pathOuter);

		if (Focused || Selected)
		{
			if (!Selected && !Hover)
			{
				rect.DeflateRect(1, 1);

				GraphicsPath pathInner;
				CreateRoundRectangle(rect, 1, pathInner);

				Pen pen(Color(0xC0FFFFFF));
				g.DrawPath(&pen, &pathInner);
			}

			Pen pen(Color(0xFF808397));
			g.DrawPath(&pen, &pathOuter);
		}
		else
		{
			Pen pen(Color(Hover ? 0xFFA6ABB2 : 0xFFBCBDBE));
			g.DrawPath(&pen, &pathOuter);
		}
	}
	else
	{
		dc.FillSolidRect(rect, GetSysColor(COLOR_3DFACE));
		dc.DrawEdge(rect, Selected ? EDGE_SUNKEN : EDGE_RAISED, BF_RECT | BF_SOFT);

		if (Focused)
		{
			rect.DeflateRect(2, 2);

			dc.SetTextColor(0x000000);
			dc.DrawFocusRect(rect);
		}
	}

	dc.SetTextColor(Themed ? Disabled ? 0xA0A0A0 : Focused || Selected || Hover ? 0x000000 : 0x404040 : GetSysColor(Disabled ? COLOR_GRAYTEXT : COLOR_WINDOWTEXT));
}

void DrawWhiteButtonForeground(CDC& dc, LPDRAWITEMSTRUCT lpDrawItemStruct, BOOL ShowKeyboardCues)
{
	CRect rect(lpDrawItemStruct->rcItem);
	rect.DeflateRect(6, 4);

	WCHAR Caption[256];
	::GetWindowText(lpDrawItemStruct->hwndItem, Caption, 256);

	UINT nFormat = DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_END_ELLIPSIS;
	if (!ShowKeyboardCues)
		nFormat |= DT_HIDEPREFIX;

	HFONT hOldFont = (HFONT)dc.SelectObject((HFONT)::SendMessage(lpDrawItemStruct->hwndItem, WM_GETFONT, NULL, NULL));
	dc.DrawText(Caption, rect, nFormat);
	dc.SelectObject(hOldFont);
}

void DrawColor(CDC& dc, CRect rect, BOOL Themed, COLORREF clr, BOOL Enabled, BOOL Focused, BOOL Hover)
{
	if (Themed)
	{
		Graphics g(dc);

		DrawWhiteButtonBorder(g, rect, FALSE);

		rect.top++;
		rect.left++;

		if (clr!=(COLORREF)-1)
		{
			g.SetPixelOffsetMode(PixelOffsetModeHalf);

			GraphicsPath PathInner;
			CreateRoundRectangle(rect, 2, PathInner);

			SolidBrush brush(Color(COLORREF2RGB(clr) & (Enabled ? 0xFFFFFFFF : 0x3FFFFFFF)));
			g.FillPath(&brush, &PathInner);

			if (Enabled)
			{
				// Boost reflection for light colors
				UINT Alpha = (((clr>>16)*(clr>>16)+6*((clr & 0x00FF00)>>8)*((clr & 0x00FF00)>>8)+5*(clr & 0xFF)*(clr & 0xFF))<<10) & 0xFF000000;

				LinearGradientBrush brush1(Point(0, rect.top), Point(0, (rect.top+rect.bottom)/2+1), Color(Alpha+0x30FFFFFF), Color(Alpha+0x20FFFFFF));
				g.FillRectangle(&brush1, rect.left, rect.top, rect.Width()-1, rect.Height()/2);

				LinearGradientBrush brush2(Point(0, rect.bottom-6), Point(0, rect.bottom), Color(0x00000000), Color(Hover ? 0x40000000 : 0x20000000));
				g.FillRectangle(&brush2, rect.left+1, rect.bottom-6, rect.Width()-3, 5);
			}

			g.SetPixelOffsetMode(PixelOffsetModeNone);
		}

		rect.right--;
		rect.bottom--;

		GraphicsPath PathOuter;
		CreateRoundRectangle(rect, 2, PathOuter);

		Pen pen(Color(Enabled ? Focused || Hover ? 0x40000000 : (clr!=(COLORREF)-1) ? 0x20000000 : 0x08000000 : 0x10000000));
		g.DrawPath(&pen, &PathOuter);
	}
	else
	{
		if (clr!=(COLORREF)-1)
			dc.FillSolidRect(rect, clr);

		dc.DrawEdge(rect, EDGE_SUNKEN, BF_RECT);

		if (Focused)
		{
			rect.DeflateRect(2, 2);

			dc.SetTextColor(0x000000);
			dc.DrawFocusRect(rect);
		}
	}
}

void DrawColorDot(CDC& dc, CRect& rect, BYTE nColor, BOOL& First, CIcons& Icons, INT FontHeight)
{
	ASSERT(nColor>0);

	const INT Size = Icons.GetIconSize();

	if (!First)
		rect.left -= 5*Size/6;

	Icons.Draw(dc, rect.left, (rect.top+rect.bottom-Size)/2+(FontHeight & 1), nColor-1);

	rect.left += 4*Size/3;
	First = FALSE;
}

void DrawStoreIconShadow(Graphics& g, const CPoint& pt, UINT IconID, INT IconSize)
{
	if (IconID<=IDI_LASTSTOREICON)
		switch (IconSize)
		{
		case 96:
			g.DrawImage(LFGetApp()->GetCachedResourceImage(IDB_ICONSHADOW_96), pt.x-6,pt.y+85);
			break;

		case 128:
			g.DrawImage(LFGetApp()->GetCachedResourceImage(IDB_ICONSHADOW_128), pt.x-5,pt.y+114);
			break;
		}
}

void DrawStoreIconShadow(CDC& dc, const CPoint& pt, UINT IconID, INT IconSize)
{
	Graphics g(dc);

	DrawStoreIconShadow(g, pt, IconID, IconSize);
}


// liquidFOLDERS

void AddCompare(CComboBox* pComboBox, UINT ResID, UINT CompareID)
{
	ASSERT(pComboBox);

	pComboBox->SetItemData(pComboBox->AddString(CString((LPCSTR)ResID)), CompareID);
}

void SetCompareComboBox(CComboBox* pComboBox, UINT Attr, INT Request)
{
	ASSERT(pComboBox);

	pComboBox->SetRedraw(FALSE);
	pComboBox->ResetContent();

	switch (LFGetApp()->m_Attributes[Attr].AttrProperties.Type)
	{
	case LFTypeUnicodeString:
	case LFTypeAnsiString:
		AddCompare(pComboBox, IDS_COMPARE_CONTAINS, LFFilterCompareContains);
		AddCompare(pComboBox, IDS_COMPARE_ISEQUAL, LFFilterCompareIsEqual);
		AddCompare(pComboBox, IDS_COMPARE_ISNOTEQUAL, LFFilterCompareIsNotEqual);
		AddCompare(pComboBox, IDS_COMPARE_BEGINSWITH, LFFilterCompareBeginsWith);
		AddCompare(pComboBox, IDS_COMPARE_ENDSWITH, LFFilterCompareEndsWith);

		break;

	case LFTypeUnicodeArray:
		AddCompare(pComboBox, IDS_COMPARE_CONTAINS, LFFilterCompareContains);

		break;

	case LFTypeIATACode:
	case LFTypeFourCC:
	case LFTypeFraction:
	case LFTypeColor:
	case LFTypeGeoCoordinates:
	case LFTypeGenre:
	case LFTypeApplication:
		AddCompare(pComboBox, IDS_COMPARE_ISEQUAL, LFFilterCompareIsEqual);
		AddCompare(pComboBox, IDS_COMPARE_ISNOTEQUAL, LFFilterCompareIsNotEqual);

		break;

	case LFTypeRating:
	case LFTypeUINT:
	case LFTypeSize:
	case LFTypeTime:
	case LFTypeDouble:
	case LFTypeDuration:
	case LFTypeBitrate:
	case LFTypeMegapixel:
	case LFTypeYear:
	case LFTypeFramerate:
		AddCompare(pComboBox, IDS_COMPARE_ISEQUAL, LFFilterCompareIsEqual);
		AddCompare(pComboBox, IDS_COMPARE_ISNOTEQUAL, LFFilterCompareIsNotEqual);
		AddCompare(pComboBox, IDS_COMPARE_ISABOVEEQUAL, LFFilterCompareIsAboveOrEqual);
		AddCompare(pComboBox, IDS_COMPARE_ISBELOWEQUAL, LFFilterCompareIsBelowOrEqual);

		break;
	}

	if (Request!=-1)
	{
		BOOL First = TRUE;

		for (INT a=0; a<pComboBox->GetCount(); a++)
		{
			const INT Data = (INT)pComboBox->GetItemData(a);
			if ((Data==Request) || (First && ((Data==LFFilterCompareIsEqual) || (Data==LFFilterCompareContains))))
			{
				pComboBox->SetCurSel(a);
				First = FALSE;
			}
		}
	}

	pComboBox->SetRedraw(TRUE);
	pComboBox->Invalidate();
}

void TooltipDataFromPIDL(LPITEMIDLIST pidlFQ, CImageList* pIcons, HICON& hIcon, CString& Caption, CString& Hint)
{
	SHFILEINFO sfi;
	if (SUCCEEDED(SHGetFileInfo((LPCWSTR)pidlFQ, 0, &sfi, sizeof(SHFILEINFO), SHGFI_PIDL | SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_SYSICONINDEX | SHGFI_LARGEICON)))
	{
		hIcon = pIcons->ExtractIcon(sfi.iIcon);
		Caption = sfi.szDisplayName;
		Hint = sfi.szTypeName;

		IShellFolder* pParentFolder;
		LPCITEMIDLIST pidlRel;
		if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder, (void**)&pParentFolder, &pidlRel)))
		{
			WIN32_FIND_DATA ffd;
			if (SUCCEEDED(SHGetDataFromIDList(pParentFolder, pidlRel, SHGDFIL_FINDDATA, &ffd, sizeof(WIN32_FIND_DATA))))
			{
				FILETIME lft;

				WCHAR tmpBuf1[256];
				FileTimeToLocalFileTime(&ffd.ftCreationTime, &lft);
				LFTimeToString(lft, tmpBuf1, 256);

				WCHAR tmpBuf2[256];
				FileTimeToLocalFileTime(&ffd.ftLastWriteTime, &lft);
				LFTimeToString(lft, tmpBuf2, 256);

				CString tmpStr;
				tmpStr.Format(_T("\n%s: %s\n%s: %s"),
					LFGetApp()->GetAttributeName(LFAttrCreationTime), tmpBuf1,
					LFGetApp()->GetAttributeName(LFAttrFileTime), tmpBuf2);

				Hint.Append(tmpStr);
			}

			pParentFolder->Release();
		}
	}
}


// IATA

HBITMAP LFIATACreateAirportMap(LFAirport* pAirport, LONG Width, LONG Height)
{
	ASSERT(pAirport);

	// Create bitmap
	CDC dc;
	dc.CreateCompatibleDC(NULL);

	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(CreateTruecolorBitmap(Width, Height));

	// Draw map
	Graphics g(dc);
	g.SetPixelOffsetMode(PixelOffsetModeHalf);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	Bitmap* pMap = LFGetApp()->GetCachedResourceImage(IDB_BLUEMARBLE_2048);
	const CSize Size(pMap->GetWidth(), pMap->GetHeight());

	// Map location
	INT X = (INT)((pAirport->Location.Longitude+180.0)*(DOUBLE)Size.cx/360.0+0.5f);
	INT Y = (INT)((pAirport->Location.Latitude+90.0)*(DOUBLE)Size.cy/180.0+0.5f);

	// Map offset
	INT OffsX = -X+Width/2;
	INT OffsY = -Y+Height/2;

	if (OffsY>0)
	{
		OffsY = 0;
	}
	else
		if (OffsY<Height-Size.cy)
		{
			OffsY = Height-Size.cy;
		}

	ImageAttributes ImgAttr;
	ImgAttr.SetWrapMode(WrapModeTile);

	g.DrawImage(pMap, Rect(0, 0, Width, Height), -OffsX, -OffsY, Width, Height, UnitPixel, &ImgAttr);

	// Location indicator
	X += OffsX-8;
	Y += OffsY-8;
	DrawLocationIndicator(g, X, Y);

	// Create font path
	FontFamily fontFamily(_T("Arial"));
	WCHAR pszBuf[4];
	MultiByteToWideChar(CP_ACP, 0, pAirport->Code, -1, pszBuf, 4);

	StringFormat StrFormat;
	GraphicsPath TextPath;
	TextPath.AddString(pszBuf, -1, &fontFamily, FontStyleRegular, 20.75f, Point(0, 0), &StrFormat);

	// Translate path
	Rect rectPath;
	TextPath.GetBounds(&rectPath);

	Matrix m;
	m.Translate((Gdiplus::REAL)X+(rectPath.X+12), (Gdiplus::REAL)Y-(rectPath.Y+1));
	TextPath.Transform(&m);

	// Draw label
	Pen pen(Color(0xCC000000), 3.0f);
	pen.SetLineJoin(LineJoinRound);
	g.DrawPath(&pen, &TextPath);

	SolidBrush brush(Color(0xFFFFFFFF));
	g.FillPath(&brush, &TextPath);

	return (HBITMAP)dc.SelectObject(hOldBitmap);
}


// MessageBox

INT LFMessageBox(CWnd* pParentWnd, const CString& Text, const CString& Caption, UINT Type)
{
	return (INT)LFMessageBoxDlg(pParentWnd, Text, Caption, Type).DoModal();
}

void LFErrorBox(CWnd* pParentWnd, UINT Result)
{
	if (Result>LFCancel)
	{
		WCHAR Message[256];
		LFGetErrorText(Message, 256, Result);

		// Type
		const UINT Type = (Result==LFOk) ? MB_ICONREADY : (Result>=LFFirstFatalError) ? MB_ICONERROR : MB_ICONWARNING;

		LFMessageBox(pParentWnd, Message, CString((LPCSTR)IDS_ERROR), Type);
	}
}

BOOL LFNagScreen(CWnd* pParentWnd)
{
	if (LFIsSharewareExpired())
	{
		if (LFMessageBox(pParentWnd, CString((LPCSTR)IDS_NOLICENSE), _T("liquidFOLDERS"), MB_OK | MB_ICONSTOP)==IDOK)
			LFGetApp()->OnBackstagePurchase();

		return FALSE;
	}

	return TRUE;
}
