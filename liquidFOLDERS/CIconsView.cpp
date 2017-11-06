
// CIconsView.cpp: Implementierung der Klasse CIconsView
//

#include "stdafx.h"
#include "CIconsView.h"
#include "liquidFOLDERS.h"


// CIconsView
//

#define GUTTER      9
#define PADDING     6

CIconsView::CIconsView(UINT DataSize)
	: CGridView(DataSize, FF_ENABLELABELEDIT)
{
}

void CIconsView::SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData)
{
	CGridView::SetSearchResult(pFilter, pRawFiles, pCookedFiles, pPersistentData);

	ValidateAllItems();
}

void CIconsView::AdjustLayout()
{
	// Item width must be odd for proper alignment of jumbo core icons
	Arrange(CSize(max(127, m_DefaultFontHeight*8+1), 128+m_DefaultFontHeight*2+PADDING/2), PADDING, CSize(GUTTER, GUTTER));
}

RECT CIconsView::GetLabelRect(INT Index) const
{
	RECT rect = GetItemRect(Index);

	rect.top += 128+PADDING+PADDING/2-2;
	rect.bottom = rect.top+m_DefaultFontHeight+4;
	rect.left += PADDING/2;
	rect.right -= PADDING/2;

	return rect;
}

void CIconsView::DrawWrapLabel(CDC& dc, const CRect& rectLabel, LFItemDescriptor* pItemDescriptor, BOOL Themed, UINT MaxLineCount) const
{
	// Make width one pixel larger due to ClearType
	const INT MaxLineWidth = rectLabel.Width()+1;

	// Label and with of color dots
	CString strLabel = GetLabel(pItemDescriptor);
	INT ColorDotWidth = GetColorDotWidth(pItemDescriptor);

	// We only care for the height of rectLine here
	CRect rectLine(rectLabel);
	rectLine.bottom = rectLine.top+m_DefaultFontHeight;

	// Iterate the lines
	UINT Line = 0;

	while (Line<MaxLineCount)
	{
		CString strLine;
		INT LineWidth = 0;
		BOOL Break;

		do
		{
			// Find delimiter
			INT Pos = strLabel.Find(L' ');

			// If none is found, use end of string
			if (Pos==-1)
				Pos = strLabel.GetLength();

			// Extract next token
			const CString strToken = strLabel.Left(Pos);

			// Calculate new line width, assuming the token would be concatenated
			const INT NewLineWidth = ColorDotWidth+dc.GetTextExtent(strLine+strToken).cx;

			// Do we add the token?
			if (((Break=(NewLineWidth>MaxLineWidth))==FALSE) || strLine.IsEmpty())
			{
				// Yes, so we have a new line width...
				LineWidth = NewLineWidth;
				strLine += strToken+_T(" ");

				// ...and consume the token! Also remove all extra spaces on the left
				strLabel.Delete(0, Pos+1);
				strLabel = strLabel.TrimLeft();
			}
		}
		while (!Break && !strLabel.IsEmpty());

		// There might me trailing spaces
		strLine.TrimRight();

		// Calculate left and right bounds of line
		rectLine.left = (rectLabel.left+rectLabel.right-min(MaxLineWidth, LineWidth))/2;
		rectLine.right = min(rectLabel.right, rectLine.left+LineWidth);

		// Draw color dots on first line
		if (ColorDotWidth)
		{
			DrawColorDots(dc, rectLine, pItemDescriptor, m_DefaultFontHeight);
			ColorDotWidth = 0;
		}

		// Draw text
		dc.DrawText(strLine, rectLine, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_TOP | DT_SINGLELINE);

		// Move rectLine one line down
		rectLine.OffsetRect(0, m_DefaultFontHeight);

		Line++;

		if (strLabel.IsEmpty())
			break;
	}

	// Hint
	if ((Line<MaxLineCount) && ((pItemDescriptor->Type & LFTypeMask)==LFTypeFolder))
	{
		ASSERT(pItemDescriptor->Description[0]);

		CFont* pOldFont = dc.SelectObject(&theApp.m_SmallFont);
		COLORREF oldColor = SetGrayText(dc, pItemDescriptor, Themed);

		dc.DrawText(pItemDescriptor->Description, -1, CRect(rectLabel.left, rectLabel.bottom-m_SmallFontHeight, rectLabel.right, rectLabel.bottom), DT_CENTER | DT_BOTTOM | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

		dc.SelectObject(pOldFont);
		dc.SetTextColor(oldColor);
	}
}

void CIconsView::DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed)
{
	LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

	// Label
	if (!IsEditing() || (Index!=m_EditLabel))
	{
		CRect rectLabel(rectItem);
		rectLabel.DeflateRect(Themed ? PADDING+1 : PADDING, PADDING);
		rectLabel.top += 128+PADDING/2;

		DrawWrapLabel(dc, rectLabel, pItemDescriptor, Themed);
	}

	// Icon
	DrawJumboIcon(dc, g, CPoint((rectItem->left+rectItem->right-128)/2, rectItem->top+PADDING), pItemDescriptor);
}
