
// CDetailsView.cpp: Implementierung der Klasse CDetailsView
//

#include "stdafx.h"
#include "CDetailsView.h"
#include "liquidFOLDERS.h"


// CDetailsView
//

#define GetItemData(Index)     ((GridItemData*)(m_pItemData+(Index)*m_DataSize))
#define GUTTER                 9
#define PADDING                6
#define MINWIDTH               400

CDetailsView::CDetailsView(UINT DataSize)
	: CGridView(DataSize, FF_ENABLELABELEDIT)
{
}

void CDetailsView::SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData)
{
	CGridView::SetSearchResult(pRawFiles, pCookedFiles, pPersistentData);

	ValidateAllItems();
}

void CDetailsView::AdjustLayout()
{
	CRect rect;
	GetWindowRect(rect);

	const INT MinWidth = max(1, 25*m_DefaultFontHeight);
	const BOOL FullWidth = rect.Width()<BACKSTAGEBORDER+2*(MinWidth+2*PADDING+GUTTER)+GetSystemMetrics(SM_CXVSCROLL);

	Arrange(CSize(MinWidth, 128+PADDING/2+RATINGBITMAPHEIGHT+2), PADDING, CSize(GUTTER, GUTTER), FullWidth);
}

RECT CDetailsView::GetLabelRect(INT Index) const
{
	RECT rect = GetItemRect(Index);
	rect.left += 128+2*PADDING;

	return rect;
}

void CDetailsView::DrawItem(CDC& dc, LPCRECT rectItem, INT Index, BOOL Themed)
{
	LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];
	GridItemData* pData = GetItemData(Index);

	CRect rect(rectItem);
	rect.DeflateRect(PADDING, PADDING);

	// Properties
	if (!IsEditing() || (Index!=m_EditLabel))
	{
		CRect rectLabel(rect);
		rectLabel.left += 128+PADDING+1;

		CString Name;
		CString Value;

		for (UINT a=0; a<LFAttributeCount; a++)
		{
			const UINT Attr = theApp.m_SortedAttributeList[a];

			if (theApp.m_Attributes[Attr].AttrProperties.DefaultPriority==LFMaxAttributePriority)
				break;

			if (Attr==LFAttrFileName)
			{
				ASSERT(rect.Height()>=m_LargeFontHeight+PADDING/2+2*m_DefaultFontHeight);

				// Filename
				CFont* pOldFont = dc.SelectObject(&theApp.m_LargeFont);
				dc.DrawText(GetLabel(pItemDescriptor), rectLabel, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_SINGLELINE);
				dc.SelectObject(pOldFont);

				rectLabel.top += m_LargeFontHeight+PADDING/2;

				// Comments
				if (pItemDescriptor->CoreAttributes.Comments[0])
				{
					dc.DrawText(pItemDescriptor->CoreAttributes.Comments, -1, rectLabel, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_SINGLELINE);

					rectLabel.top += m_DefaultFontHeight;
				}

				// Description
				if (pItemDescriptor->Description[0])
				{
					dc.DrawText(pItemDescriptor->Description, -1, rectLabel, DT_END_ELLIPSIS | DT_NOPREFIX | DT_LEFT | DT_SINGLELINE);

					rectLabel.top += m_DefaultFontHeight;
				}
			}
			else
				if (theApp.IsAttributeAvailable(m_Context, Attr))
				{
					// Other properties
					theApp.AttributeToString(Name, Value, pItemDescriptor, Attr);

					if (!Value.IsEmpty())
					{
						CRect rectText(rectLabel);

						// Name
						if (!Name.IsEmpty())
						{
							Name.Append(_T(": "));
							dc.DrawText(Name, rectText, DT_LEFT | DT_SINGLELINE | DT_NOPREFIX);

							rectText.left += dc.GetTextExtent(Name).cx;
						}

						// Value
						COLORREF oldColor = dc.GetTextColor();
						if ((Themed) && (!(pItemDescriptor->CoreAttributes.Flags & LFFlagMissing)) && !pData->Hdr.Selected)
							dc.SetTextColor(0x808080);

						dc.DrawText(Value, rectText, DT_LEFT | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);
						dc.SetTextColor(oldColor);

						if ((rectLabel.top+=m_DefaultFontHeight)>rect.bottom-m_DefaultFontHeight)
							break;
					}
				}
		}
	}

	// Icon
	DrawJumboIcon(dc, rect.TopLeft(), pItemDescriptor);

	// Rating
	if (pItemDescriptor->AttributeValues[LFAttrRating])
	{
		const UCHAR Rating = *((UCHAR*)pItemDescriptor->AttributeValues[LFAttrRating]);
		ASSERT(Rating<=LFMaxRating);

		if (((pItemDescriptor->Type & LFTypeMask)==LFTypeFile) || (Rating))
		{
			CDC dcMem;
			dcMem.CreateCompatibleDC(&dc);

			HBITMAP hOldBitmap = (HBITMAP)dcMem.SelectObject(theApp.hRatingBitmaps[Rating]);
			dc.AlphaBlend(rect.left+(128-RATINGBITMAPWIDTH)/2, rect.top+128+PADDING/2, RATINGBITMAPWIDTH, RATINGBITMAPHEIGHT, &dcMem, 0, 0, RATINGBITMAPWIDTH, RATINGBITMAPHEIGHT, BF);
			SelectObject(dcMem, hOldBitmap);
		}
	}
}
