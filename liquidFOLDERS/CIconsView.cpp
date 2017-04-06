
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
	: CGridView(DataSize)
{
}

void CIconsView::SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData)
{
	CGridView::SetSearchResult(pRawFiles, pCookedFiles, pPersistentData);

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
	rect.top += 128+PADDING;

	return rect;
}

void CIconsView::DrawItem(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed)
{
	LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

	// Label
	if (!IsEditing() || (Index!=m_EditLabel))
	{
		CRect rectLabel(rectItem);
		rectLabel.DeflateRect(Themed ? PADDING+1 : PADDING, PADDING);
		rectLabel.top += 128+PADDING/2;

		dc.DrawText(GetLabel(pItemDescriptor), rectLabel, DT_END_ELLIPSIS | DT_NOPREFIX | DT_CENTER | DT_WORDBREAK);
	}

	// Icon
	CRect rectIcon(rectItem);
	rectIcon.top += PADDING;
	rectIcon.bottom = rectIcon.top+128;

	DrawJumboIcon(dc, rectIcon, pItemDescriptor);
}
