
// FooterGraph.cpp: Implementierung von Methoden zur Darstellung von Diagrammen
//

#include "stdafx.h"
#include "FooterGraph.h"


void Finish(CDC& dc, CRect rect, BOOL Themed)
{
	dc.Draw3dRect(rect, 0x000000, 0x000000);
	if (Themed)
	{
		const COLORREF clrBack = Themed ? 0xFFFFFF : GetSysColor(COLOR_WINDOW);
		dc.SetPixel(rect.left, rect.top, clrBack);
		dc.SetPixel(rect.right-1, rect.top, clrBack);
		dc.SetPixel(rect.left, rect.bottom-1, clrBack);
		dc.SetPixel(rect.right-1, rect.bottom-1, clrBack);

		const COLORREF clrBlend50 = (clrBack>>1) & 0x7F7F7F;
		dc.SetPixel(rect.left+1, rect.top, clrBlend50);
		dc.SetPixel(rect.left, rect.top+1, clrBlend50);
		dc.SetPixel(rect.right-2, rect.top, clrBlend50);
		dc.SetPixel(rect.right-1, rect.top+1, clrBlend50);
		dc.SetPixel(rect.left+1, rect.bottom-1, clrBlend50);
		dc.SetPixel(rect.left, rect.bottom-2, clrBlend50);
		dc.SetPixel(rect.right-2, rect.bottom-1, clrBlend50);
		dc.SetPixel(rect.right-1, rect.bottom-2, clrBlend50);

		const COLORREF clrBlend75 = (clrBlend50>>1) & 0x7F7F7F;
		dc.SetPixel(rect.left+2, rect.top, clrBlend75);
		dc.SetPixel(rect.left, rect.top+2, clrBlend75);
		dc.SetPixel(rect.right-3, rect.top, clrBlend75);
		dc.SetPixel(rect.right-1, rect.top+2, clrBlend75);
		dc.SetPixel(rect.left+2, rect.bottom-1, clrBlend75);
		dc.SetPixel(rect.left, rect.bottom-3, clrBlend75);
		dc.SetPixel(rect.right-3, rect.bottom-1, clrBlend75);
		dc.SetPixel(rect.right-1, rect.bottom-3, clrBlend75);

#define Blend(x, y) dc.SetPixel(x, y, (dc.GetPixel(x, y)>>1) & 0x7F7F7F);
		Blend(rect.left+1, rect.top+1);
		Blend(rect.right-2, rect.top+1);
		Blend(rect.left+1, rect.bottom-2);
		Blend(rect.right-2, rect.bottom-2);
	}
}

void DrawSolidColor(CDC& dc, CRect rect, COLORREF clr, BOOL Themed)
{
	dc.FillSolidRect(rect, clr);
	Finish(dc, rect, Themed);
}

void DrawLegend(CDC& dc, CRect& rect, COLORREF clr, CString Text, BOOL Themed)
{
	INT Height = dc.GetTextExtent(_T("Wy"), 2).cy;

	CRect rectColor(rect);
	rectColor.bottom = rectColor.top+Height;
	rectColor.right = rectColor.left+Height;
	DrawSolidColor(dc, rectColor, clr, Themed);

	CRect rectText(rect);
	rectText.left = rectColor.right+GraphSpacer-1;
	rectText.bottom = rectColor.bottom;
	dc.DrawText(Text, rectText, DT_LEFT | DT_END_ELLIPSIS | DT_TOP);

	rect.top += Height+GraphSpacer;
}
