
// FooterGraph.cpp: Implementierung von Methoden zur Darstellung von Diagrammen
//

#include "stdafx.h"
#include "FooterGraph.h"
#include "StoreManager.h"


void FinishGraph(CDC& dc, CRect rect, BOOL Themed)
{
	if (Themed)
	{
		Graphics g(dc);

		LinearGradientBrush brush1(Point(rect.left, rect.top), Point(rect.left, rect.top+rect.Height()/4+1), Color(0xA0, 0xFF, 0xFF, 0xFF), Color(0x00, 0xFF, 0xFF, 0xFF));
		g.FillRectangle(&brush1, Rect(rect.left, rect.top, rect.Width(), rect.Height()/4));

		LinearGradientBrush brush2(Point(rect.left, rect.top), Point(rect.left, rect.top+rect.Height()/2+1), Color(0x40, 0xFF, 0xFF, 0xFF), Color(0x50, 0xFF, 0xFF, 0xFF));
		g.FillRectangle(&brush2, Rect(rect.left, rect.top, rect.Width(), rect.Height()/2));

		INT Line = rect.top+rect.Height()/2;
		LinearGradientBrush brush3(Point(rect.left, Line-1), Point(rect.left, rect.bottom+1), Color(0x40, 0xFF, 0xFF, 0xFF), Color(0x00, 0xFF, 0xFF, 0xFF));
		g.FillRectangle(&brush3, Rect(rect.left, Line, rect.Width(), rect.bottom-Line));

		Line = rect.top+rect.Height()*3/4-2;
		LinearGradientBrush brush4(Point(rect.left, Line-1), Point(rect.left, rect.bottom+1), Color(0x00, 0x00, 0x00, 0x00), Color(0xC0, 0x00, 0x00, 0x00));
		g.FillRectangle(&brush4, Rect(rect.left, Line, rect.Width(), rect.bottom-Line));
	}

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
	FinishGraph(dc, rect, Themed);
}

void DrawLegend(CDC& dc, CRect& rect, COLORREF clr, CString Text, BOOL Themed)
{
	INT Height = dc.GetTextExtent(_T("Wy")).cy;

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

void DrawGraphCaption(CDC& dc, CRect& rect, UINT nID)
{
	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	INT Height = dc.GetTextExtent(_T("Wy")).cy;

	CString tmpStr;
	ENSURE(tmpStr.LoadString(nID));

	dc.DrawText(tmpStr+_T(":"), rect, DT_LEFT | DT_END_ELLIPSIS | DT_TOP);
	dc.SelectObject(pOldFont);

	rect.top += Height+GraphSpacer;
}

void DrawBarChart(CDC& dc, CRect& rect, INT64* Values, COLORREF* Colors, UINT Count, UINT Height, BOOL Themed)
{
	ASSERT(Count>0);

	INT64 Sum = 0;
	for (UINT a=0; a<Count; a++)
	{
		ASSERT(Values[a]>=0);
		Sum += Values[a];
	}

	Graphics g(dc);
	g.SetSmoothingMode(SmoothingModeAntiAlias);

#define Scale(x) (REAL)(x*((INT64)rect.Width()-1)/((REAL)Sum)-0.5)

	INT64 Left = 0;
	for (UINT a=0; a<Count; a++)
	{
		SolidBrush brush(Color(Colors[a] & 0xFF, (Colors[a]>>8) & 0xFF, (Colors[a]>>16) & 0xFF));
		g.FillRectangle(&brush, RectF(Scale(Left), (REAL)rect.top, Scale(Values[a])+(REAL)1.0, (REAL)Height-1));

		Left += Values[a];
	}

	FinishGraph(dc, CRect(rect.left, rect.top, rect.right, rect.top+Height), Themed);

	rect.top += Height+2*GraphSpacer;
}

void DrawChartLegend(CDC& dc, CRect& rect, INT64 Count, INT64 Size, COLORREF clr, CString Legend, BOOL Themed)
{
	CString Hint;
	CString Mask;
	WCHAR tmpBuf[256];

	ENSURE(Mask.LoadString(Count==1 ? IDS_FILES_SINGULAR : IDS_FILES_PLURAL));
	LFINT64ToString(Size, tmpBuf, 256);
	Hint.Format(Mask, Count);
	Hint.Append(_T(" ("));
	Hint.Append(tmpBuf);
	Hint.Append(_T(")"));

	Hint.Insert(0, (GetThreadLocale() & 0x1FF)==LANG_ENGLISH ? _T("—") : _T(" – "));
	Hint.Insert(0, Legend);

	DrawLegend(dc, rect, clr, Hint, Themed);
}
