
// CDetailsView.cpp: Implementierung der Klasse CDetailsView
//

#include "stdafx.h"
#include "CDetailsView.h"
#include "liquidFOLDERS.h"


// CDetailsView
//

#define GUTTER                 9
#define PADDING                6
#define MINWIDTH               400

CDetailsView::CDetailsView()
	: CFileView(FRONTSTAGE_ENABLESCROLLING | FRONTSTAGE_ENABLESELECTION | FRONTSTAGE_ENABLESHIFTSELECTION | FRONTSTAGE_ENABLELABELEDIT | FF_ENABLEFOLDERTOOLTIPS)
{
}

void CDetailsView::SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData)
{
	CFileView::SetSearchResult(pFilter, pRawFiles, pCookedFiles, pPersistentData);

	ValidateAllItems();
}

void CDetailsView::AdjustLayout()
{
	CRect rect;
	GetWindowRect(rect);

	const INT MinWidth = max(1, 25*m_DefaultFontHeight);
	const BOOL FullWidth = rect.Width()<BACKSTAGEBORDER+2*(MinWidth+2*PADDING+GUTTER)+GetSystemMetrics(SM_CXVSCROLL);

	AdjustLayoutGrid(CSize(MinWidth+2*PADDING, 128+3*PADDING+RATINGBITMAPHEIGHT+3), CSize(GUTTER, GUTTER), FullWidth, BACKSTAGEBORDER);
}

LFFont* CDetailsView::GetLabelFont() const
{
	return &theApp.m_LargeFont;
}

RECT CDetailsView::GetLabelRect(INT Index) const
{
	RECT rect = GetItemRect(Index);

	rect.top += PADDING-2;
	rect.bottom = rect.top+m_LargeFontHeight+4;
	rect.left += 128+2*PADDING+GetColorDotWidth(Index, m_LargeColorDots)-5;
	rect.right -= PADDING-2;

	return rect;
}

void CDetailsView::DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed)
{
	ASSERT(LFAttrFileName==0);

	LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

	CRect rect(rectItem);
	rect.DeflateRect(PADDING, PADDING);

	// Properties
	CRect rectText(rect);
	rectText.left += 128+PADDING+1;

	CString Name;
	CString Value;

	for (UINT a=0; a<LFAttributeCount; a++)
	{
		const UINT Attr = theApp.m_SortedAttributeList[a];

		if (theApp.m_Attributes[Attr].AttrProperties.DefaultPriority==LFMinAttributePriority)
			break;

		if (Attr==LFAttrFileName)
		{
			ASSERT(rect.Height()>=m_LargeFontHeight+PADDING/2+2*m_DefaultFontHeight);

			CRect rectLabel(rectText);
			rectLabel.bottom = rectLabel.top+m_LargeFontHeight;

			const CString strLabel = GetItemLabel(pItemDescriptor);
			if (!strLabel.IsEmpty())
			{
				// Color
				DrawColorDots(dc, rectLabel, pItemDescriptor, m_LargeFontHeight, m_LargeColorDots);

				if (!IsEditing() || (Index!=m_EditItem))
				{
					// Filename
					CFont* pOldFont = dc.SelectObject(&theApp.m_LargeFont);
					dc.DrawText(strLabel, rectLabel, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_SINGLELINE);
					dc.SelectObject(pOldFont);
				}

				rectText.top += m_LargeFontHeight+PADDING/2;
			}

			SetDarkTextColor(dc, pItemDescriptor, Themed);

			// Comments
			if (pItemDescriptor->CoreAttributes.Comments[0])
			{
				dc.DrawText(pItemDescriptor->CoreAttributes.Comments, -1, rectText, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_SINGLELINE);

				rectText.top += m_DefaultFontHeight;
			}

			// Description
			if (pItemDescriptor->Description[0])
			{
				dc.DrawText(pItemDescriptor->Description, -1, rectText, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_SINGLELINE);

				rectText.top += m_DefaultFontHeight;
			}
		}
		else
			if (theApp.IsAttributeAvailable(m_Context, Attr))
			{
				// Other properties
				theApp.AttributeToString(Name, Value, pItemDescriptor, Attr);

				if (!Value.IsEmpty())
				{
					CRect rectLabel(rectText);

					// Name
					if (!Name.IsEmpty())
					{
						Name.Append(_T(": "));
						dc.DrawText(Name, rectLabel, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

						rectLabel.left += dc.GetTextExtent(Name).cx;
					}

					// Value
					COLORREF oldColor = SetLightTextColor(dc, pItemDescriptor, Themed);
					dc.DrawText(Value, rectLabel, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
					dc.SetTextColor(oldColor);

					if ((rectText.top+=m_DefaultFontHeight)>rect.bottom-m_DefaultFontHeight)
						break;
				}
			}
	}

	// Icon
	DrawJumboIcon(dc, g, rect.TopLeft(), pItemDescriptor);

	// Rating
	if (pItemDescriptor->AttributeValues[LFAttrRating])
	{
		const UCHAR Rating = *((UCHAR*)pItemDescriptor->AttributeValues[LFAttrRating]);
		ASSERT(Rating<=LFMaxRating);

		if (((pItemDescriptor->Type & LFTypeMask)==LFTypeFile) || Rating)
		{
			CDC dcMem;
			dcMem.CreateCompatibleDC(&dc);

			HBITMAP hOldBitmap = (HBITMAP)dcMem.SelectObject(theApp.hRatingBitmaps[Rating]);

			dc.AlphaBlend(rect.left+(128-RATINGBITMAPWIDTH)/2, rect.top+128+PADDING, RATINGBITMAPWIDTH, RATINGBITMAPHEIGHT, &dcMem, 0, 0, RATINGBITMAPWIDTH, RATINGBITMAPHEIGHT, BF);

			SelectObject(dcMem, hOldBitmap);
		}
	}
}
