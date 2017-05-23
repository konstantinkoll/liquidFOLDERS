
// CTimelineView.cpp: Implementierung der Klasse CTimelineView
//

#include "stdafx.h"
#include "CTimelineView.h"
#include "liquidFOLDERS.h"


// CTimelineView
//

#define ARROWSIZE          (GUTTER-4)
#define GUTTER             BACKSTAGEBORDER
#define MIDDLE             (2*GUTTER+6)
#define BLENDHEIGHT        2*(GUTTER+CARDPADDING)
#define CATEGORYRADIUS     8
#define LARGEPADDING       (CARDPADDING-1)
#define SMALLPADDING       (CARDPADDING/2+1)
#define THUMBMARGINX       2
#define THUMBMARGINY       THUMBMARGINX
#define THUMBOFFSETY       -1
#define MAXFILELIST        10

#define GetItemData(Index)       ((TimelineItemData*)(m_pItemData+(Index)*m_DataSize))
#define DrawCollectionIcon()     theApp.m_CoreImageListSmall.DrawEx(&dc, pData->CollectionIconID-1, CPoint(rect.left, rectAttr.top-(m_DefaultFontHeight-16)/2), CSize(m_SmallIconSize, m_SmallIconSize), CLR_NONE, 0xFFFFFF, ILD_TRANSPARENT);

CIcons CTimelineView::m_SourceIcons;
const ARGB CTimelineView::m_BevelColors[8] = { 0x80FFFFFF, 0xFF7A7A7C, 0xFFA7A8AA, 0xFFBEBFC2, 0xFFCACBCD, 0xFFCACBCD, 0xFF7A7A7C, 0x80FFFFFF };

CTimelineView::CTimelineView()
	: CFileView(sizeof(TimelineItemData), FF_ENABLESCROLLING | FF_ENABLEHOVER | FF_ENABLETOOLTIPS | FF_ENABLESHIFTSELECTION | FF_ENABLELABELEDIT)
{
}

LPCWSTR CTimelineView::GetAttribute(TimelineItemData* pData, UINT Mask, LFItemDescriptor* pItemDescriptor, UINT Attr)
{
	ASSERT(pData);
	ASSERT(pItemDescriptor);
	ASSERT(Attr<LFAttributeCount);
	ASSERT(theApp.m_Attributes[Attr].AttrProperties.Type==LFTypeUnicodeString);

	LPCWSTR pStr = (LPCWSTR)pItemDescriptor->AttributeValues[Attr];

	// Property present in item?
	if (pStr)
		// Is the sting non-empty?
		if (*pStr)
		{
			// Yes, set mask and leave!
			pData->PreviewMask |= Mask;

			return pStr;
		}

	return NULL;
}

void CTimelineView::AggregateAttribute(UINT& PreviewMask, LPCWSTR& pStrAggregated, UINT Mask, LFItemDescriptor* pItemDescriptor, UINT Attr)
{
	ASSERT(pItemDescriptor);
	ASSERT(Attr<LFAttributeCount);
	ASSERT(theApp.m_Attributes[Attr].AttrProperties.Type==LFTypeUnicodeString);

	// Aggregation still possible?
	if (PreviewMask & Mask)
	{
		LPCWSTR pStr = (LPCWSTR)pItemDescriptor->AttributeValues[Attr];

		// Property present in item?
		if (pStr)
			// Is the string non-empty?
			if (*pStr)
				// Is there already an aggregated string?
				if (pStrAggregated)
				{
					// Yes, compare with the current string. When they are the same, leave!
					if (wcscmp(pStr, pStrAggregated)==0)
						return;
				}
				else
				{
					// No, set the current string and leave!
					pStrAggregated = pStr;

					return;
				}

		// Strings contradict and cannot not be aggregated anymore
		PreviewMask &= ~Mask;
	}
}

void CTimelineView::AggregateIcon(UINT& PreviewMask, INT& AggregatedIconID, UINT Mask, INT IconID)
{
	// Aggregation still possible?
	if (PreviewMask & Mask)
	{
		// Is there already an aggregated icon?
		if (AggregatedIconID)
		{
			// Yes, compare with the current icon. When they are the same, leave!
			if (AggregatedIconID==IconID)
				return;
		}
		else
		{
			// No, set the current icon and leave!
			AggregatedIconID = IconID;

			return;
		}

		// Icons contradict and cannot not be aggregated anymore
		PreviewMask &= ~Mask;
	}
}

void CTimelineView::SetSearchResult(LFFilter* pFilter, LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData)
{
	CFileView::SetSearchResult(pFilter, pRawFiles, pCookedFiles, pPersistentData);

	if (p_CookedFiles)
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			TimelineItemData* pData = GetItemData(a);
			LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[a];

			LFVariantData Property;
			LFGetAttributeVariantDataEx(pItemDescriptor, m_ContextViewSettings.SortBy, Property);

			if (!LFIsNullVariantData(Property))
			{
				// Year
				SYSTEMTIME stUTC;
				SYSTEMTIME stLocal;
				FileTimeToSystemTime(&Property.Time, &stUTC);
				SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

				pData->Year = stLocal.wYear;

				// Source
				if ((pItemDescriptor->Type & LFTypeSourceMask)>LFTypeSourceInternal)
					pData->PreviewMask |= PRV_SOURCE;

				UINT ContextID;
				UINT CreatorAttr;
				UINT CollectionAttr;

				// Map properties to Creator, Title, Collection and Comments
				switch (pItemDescriptor->Type & LFTypeMask)
				{
				case LFTypeFile:
					ContextID = pItemDescriptor->CoreAttributes.ContextID;
					CreatorAttr = (ContextID==LFContextAudio) || (ContextID==LFContextVideos) ? LFAttrArtist : LFAttrAuthor;
					CollectionAttr = (ContextID==LFContextAudio) ? LFAttrAlbum : LFAttrRoll;

					pData->pStrCreator = GetAttribute(pData, PRV_CREATOR, pItemDescriptor, CreatorAttr);
					pData->pStrTitle = GetAttribute(pData, PRV_TITLE, pItemDescriptor, LFAttrTitle);
					pData->pStrCollection = GetAttribute(pData, PRV_COLLECTION, pItemDescriptor, CollectionAttr);
					pData->pStrComments = GetAttribute(pData, PRV_COMMENTS, pItemDescriptor, LFAttrComments);

					if (pData->pStrCollection)
					{
						pData->PreviewMask |= PRV_COLLECTIONICON;
						VERIFY(pData->CollectionIconID=theApp.m_Attributes[CollectionAttr].AttrProperties.IconID);
					}

					break;

				case LFTypeFolder:
					// Aggregate properties
					pData->PreviewMask |= PRV_CREATOR | PRV_TITLE | PRV_COMMENTS | PRV_COLLECTIONICON | PRV_REPRESENTATIVE;
					pData->pStrCreator = pData->pStrTitle = pData->pStrComments = NULL;
					pData->CollectionIconID = 0;

					ASSERT(pItemDescriptor->FirstAggregate>=0);
					ASSERT(pItemDescriptor->LastAggregate>=0);

					for (INT b=pItemDescriptor->FirstAggregate; b<=pItemDescriptor->LastAggregate; b++)
					{
						LFItemDescriptor* pItemDescriptor = (*p_RawFiles)[b];

						if ((ContextID=pItemDescriptor->CoreAttributes.ContextID)!=LFContextAudio)
							pData->PreviewMask &= ~PRV_REPRESENTATIVE;

						CreatorAttr = (ContextID==LFContextAudio) || (ContextID==LFContextVideos) ? LFAttrArtist : LFAttrAuthor;
						CollectionAttr = (ContextID==LFContextAudio) ? LFAttrAlbum : LFAttrRoll;

						AggregateAttribute(pData->PreviewMask, pData->pStrCreator, PRV_CREATOR, pItemDescriptor, CreatorAttr);
						AggregateAttribute(pData->PreviewMask, pData->pStrTitle, PRV_TITLE, pItemDescriptor, CollectionAttr);
						AggregateAttribute(pData->PreviewMask, pData->pStrComments, PRV_COMMENTS, pItemDescriptor, LFAttrComments);

						AggregateIcon(pData->PreviewMask, pData->CollectionIconID, PRV_COLLECTIONICON, theApp.m_Attributes[CollectionAttr].AttrProperties.IconID);
					}

					// Only show icon and indent text when the collection is valid
					if ((pData->PreviewMask & PRV_TITLE)==0)
						pData->PreviewMask &= ~PRV_COLLECTIONICON;

					break;
				}

				pData->Hdr.Valid = TRUE;
			}
		}
}

BOOL CTimelineView::UsePreview(LFItemDescriptor* pItemDescriptor)
{
	if (pItemDescriptor->Type & LFTypeMounted)
		switch (pItemDescriptor->CoreAttributes.ContextID)
		{
		case LFContextAudio:
			return (LFGetApp()->OSVersion>OS_XP);

		case LFContextPictures:
			return ((_stricmp(pItemDescriptor->CoreAttributes.FileFormat, "BMP")==0) ||
					(_stricmp(pItemDescriptor->CoreAttributes.FileFormat, "DIB")==0) ||
					(_stricmp(pItemDescriptor->CoreAttributes.FileFormat, "GIF")==0) ||
					(_stricmp(pItemDescriptor->CoreAttributes.FileFormat, "JPG")==0) ||
					(_stricmp(pItemDescriptor->CoreAttributes.FileFormat, "JPEG")==0) ||
					(_stricmp(pItemDescriptor->CoreAttributes.FileFormat, "PNG")==0) ||
					(_stricmp(pItemDescriptor->CoreAttributes.FileFormat, "TIF")==0) ||
					(_stricmp(pItemDescriptor->CoreAttributes.FileFormat, "TIFF")==0));

		case LFContextVideos:
			return TRUE;

		case LFContextDocuments:
			return ((_stricmp(pItemDescriptor->CoreAttributes.FileFormat, "PPT")==0) ||
					(_stricmp(pItemDescriptor->CoreAttributes.FileFormat, "PPTX")==0));
		}

	return FALSE;
}

void CTimelineView::AdjustLayout()
{
	if (!p_CookedFiles)
	{
		m_ScrollWidth = m_ScrollHeight = 0;
		return;
	}

	CRect rect;
	GetWindowRect(rect);

	BOOL HasScrollbars = FALSE;

Restart:
	SYSTEMTIME st;
	GetLocalTime(&st);

	WORD Year = st.wYear;

	m_Categories.m_ItemCount = 0;
	m_TwoColumns = rect.Width()-2*GUTTER-MIDDLE-7*CARDPADDING>=512;
	m_ItemWidth = m_TwoColumns ? (rect.Width()-MIDDLE)/2-GUTTER : rect.Width()-2*GUTTER;
	m_PreviewColumns = (m_ItemWidth-2*CARDPADDING+THUMBMARGINX+4)/(128+THUMBMARGINX);

	INT CurRow[2] = { GUTTER+1, GUTTER+1 };
	INT LastRow = -10;

	for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
	{
		TimelineItemData* pData = GetItemData(a);

		if (pData->Hdr.Valid)
		{
			LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[a];

			// Use thumbnails or representative image?
			if (m_ItemWidth<2*CARDPADDING+124)
			{
				// No, column is too small
				pData->PreviewMask &= ~PRV_PREVIEW;
			}
			else
			{
				INT ThumbnailCount = 0;
				pData->ListCount = 0;

				switch (pItemDescriptor->Type & LFTypeMask)
				{
				case LFTypeFile:
					if (UsePreview(pItemDescriptor))
					{
						pData->PreviewMask |= PRV_THUMBNAILS;
						ThumbnailCount = 1;
					}

					break;

				case LFTypeFolder:
					// Is it a music album? It must have a title and a valid collection icon, which has to match LFAttrAlbum
					if (((pData->PreviewMask & (PRV_TITLE | PRV_COLLECTIONICON))==(PRV_TITLE | PRV_COLLECTIONICON)) &&
						(pData->CollectionIconID==theApp.m_Attributes[LFAttrAlbum].AttrProperties.IconID))
					{
						// Remove thumbnails and folder contents
						pData->PreviewMask = (pData->PreviewMask & ~(PRV_THUMBNAILS | PRV_CONTENTS)) | PRV_REPRESENTATIVE;
						ThumbnailCount = 1;
					}
					else
					{
						// Remove representative image
						pData->PreviewMask &= ~PRV_REPRESENTATIVE;

						// Decide for thumbnails or folder contents
						for (INT b=pItemDescriptor->FirstAggregate; b<=pItemDescriptor->LastAggregate; b++)
							if (UsePreview((*p_RawFiles)[b]))
							{
								pData->PreviewMask |= PRV_THUMBNAILS;
								ThumbnailCount++;
							}
							else
							{
								pData->PreviewMask |= PRV_CONTENTS;
								pData->ListCount++;
							}
					}

					break;
				}

				// Calc number of thumb rows, but cap at square ratio for visually pleasing card
				pData->ThumbnailRows = m_PreviewColumns ? max(1, min(ThumbnailCount/m_PreviewColumns, m_PreviewColumns)) : 1;
			}

			// Card padding and caption
			INT Height = 2*CARDPADDING+m_CaptionHeight;

			// Is there something to show? If yes, add spacing and separator line
			if (pData->PreviewMask)
			{
				Height += SMALLPADDING;

				// Attributes
				if (pData->PreviewMask & PRV_ATTRIBUTES)
				{
					Height += SMALLPADDING;

					// Creator and/or title
					if (pData->PreviewMask & (PRV_CREATOR | PRV_TITLE))
						Height += m_DefaultFontHeight;

					// Collection (album/roll)
					if (pData->PreviewMask & PRV_COLLECTION)
						Height += m_DefaultFontHeight;

					// Comments
					if (pData->PreviewMask & PRV_COMMENTS)
						Height += m_DefaultFontHeight;
				}

				// Thumbnails or representative image
				if (pData->PreviewMask & PRV_PREVIEW)
					// Height of margin and thumbnails
					Height += LARGEPADDING+(128+THUMBMARGINY)*pData->ThumbnailRows-THUMBMARGINY-4;

				// Folder contents
				if (pData->PreviewMask & PRV_CONTENTS)
					Height += LARGEPADDING+min(MAXFILELIST, pData->ListCount)*m_SmallFontHeight;

				// Source
				if (pData->PreviewMask & PRV_SOURCE)
				{
					switch (pData->PreviewMask & (PRV_CONTENTS | PRV_PREVIEW | PRV_ATTRIBUTES))
					{
					case PRV_ATTRIBUTES:
						Height += SMALLPADDING;

					case 0:
						break;

					default:
						Height += LARGEPADDING;
					}

					// Source height
					Height += 1+LARGEPADDING+m_SourceHeight;
				}
			}

			// New section?
			if (pData->Year!=Year)
			{
				Year = pData->Year;

				ItemCategory ic;
				ZeroMemory(&ic, sizeof(ic));

				swprintf_s(ic.Caption, 5, L"%u", (UINT)Year);
				ic.Rect.left = rect.Width()/2-m_LabelWidth/2;
				ic.Rect.right = ic.Rect.left+m_LabelWidth;
				ic.Rect.top = max(CurRow[0], CurRow[1]);
				ic.Rect.bottom = ic.Rect.top+2*CARDPADDING+theApp.m_SmallBoldFont.GetFontHeight()-2;

				m_Categories.AddItem(ic);

				CurRow[0] = CurRow[1] = ic.Rect.bottom+GUTTER;
			}

			// Arrow
			const INT Column = m_TwoColumns ? CurRow[0]<=CurRow[1] ? 0 : 1 : 0;
			pData->Arrow = m_TwoColumns ? 1-(BYTE)Column*2 : 0;
			pData->ArrowOffs = 0;
			pData->Hdr.RectInflate = pData->Arrow ? ARROWSIZE+1 : 0;

			if (abs(CurRow[Column]-LastRow)<2*ARROWSIZE)
				if (Height>m_CaptionHeight+CARDPADDING-2*ARROWSIZE+2*GUTTER)
				{
					pData->ArrowOffs = 2*GUTTER;
				}
				else
				{
					CurRow[Column] += 2*GUTTER;
				}

			// Place card
			pData->Hdr.Rect.left = (Column==0) ? GUTTER : GUTTER+m_ItemWidth+MIDDLE;
			pData->Hdr.Rect.top = CurRow[Column];
			pData->Hdr.Rect.right = pData->Hdr.Rect.left+m_ItemWidth;
			pData->Hdr.Rect.bottom = pData->Hdr.Rect.top+Height;

			LastRow = CurRow[Column];
			CurRow[Column] += Height+GUTTER+1;

			if ((CurRow[Column]>rect.Height()) && (!HasScrollbars))
			{
				HasScrollbars = TRUE;
				rect.right -= GetSystemMetrics(SM_CXVSCROLL);

				goto Restart;
			}
		}
	}

	m_ScrollHeight = max(CurRow[0], CurRow[1]);
	m_ScrollWidth = rect.Width();

	CFileView::AdjustLayout();
}

RECT CTimelineView::GetLabelRect(INT Index) const
{
	RECT rect = GetItemRect(Index);

	rect.left += CARDPADDING+m_SmallIconSize+SMALLPADDING-5;
	rect.top += CARDPADDING-2;
	rect.right -= CARDPADDING-2;
	rect.bottom = rect.top+m_DefaultFontHeight+4;

	return rect;
}

void CTimelineView::DrawCategory(CDC& dc, Graphics& g, LPCRECT rectCategory, ItemCategory* pItemCategory, BOOL Themed)
{
	CRect rectText(rectCategory);

	if (Themed)
	{
		// Background
		g.SetPixelOffsetMode(PixelOffsetModeHalf);

		GraphicsPath path;
		CreateRoundRectangle(rectCategory, CATEGORYRADIUS, path);

		LinearGradientBrush brush1(Point(0, rectCategory->top), Point(0, rectCategory->bottom), Color((ARGB)m_BevelColors[2]), Color((ARGB)m_BevelColors[4]));
		g.FillPath(&brush1, &path);

		// Light border
		g.SetPixelOffsetMode(PixelOffsetModeNone);

		Region OldClip;
		g.GetClip(&OldClip);

		const INT Left = (rectCategory->left+rectCategory->right)/2-4;
		g.SetClip(Rect(Left, rectCategory->top-1, 8, rectCategory->bottom-rectCategory->top+1), CombineModeExclude);

		CRect rectBorder(rectCategory);
		rectBorder.left--;
		rectBorder.top--;

		CreateRoundRectangle(rectBorder, CATEGORYRADIUS+1, path);

		Pen pen(Color((ARGB)m_BevelColors[0]));
		g.DrawPath(&pen, &path);

		// Dark border
		g.SetClip(&OldClip);
		g.SetClip(Rect(Left+2, rectCategory->top, 4, rectCategory->bottom-rectCategory->top), CombineModeExclude);

		rectBorder.DeflateRect(1, 1);
		CreateRoundRectangle(rectBorder, CATEGORYRADIUS, path);

		pen.SetColor(Color((ARGB)m_BevelColors[1]));
		g.DrawPath(&pen, &path);

		g.SetClip(&OldClip);

		// Text
		rectText.OffsetRect(1, 0);

		COLORREF OldColor = dc.SetTextColor(0x7C7A7A);
		dc.DrawText(pItemCategory->Caption, -1, rectText, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);

		rectText.OffsetRect(-1, -1);

		dc.SetTextColor(OldColor);

		// Finishing touches
		g.SetPixelOffsetMode(PixelOffsetModeHalf);

		LinearGradientBrush brush2(Point(0, rectCategory->top-BLENDHEIGHT), Point(0, rectCategory->top), Color(0x00A7A8AA), Color(0xFFA7A8AA));
		g.FillRectangle(&brush2, Left+3, rectCategory->top-BLENDHEIGHT, 3, BLENDHEIGHT);

		dc.SetPixel((rectCategory->left+rectCategory->right)/2-1, rectCategory->bottom-1, 0xCDCBCA);
	}
	else
	{
		dc.FillSolidRect(rectCategory, GetSysColor(COLOR_3DSHADOW));

		rectText.OffsetRect(0, -1);
	}

	// Text
	dc.DrawText(pItemCategory->Caption, -1, rectText, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
}

void CTimelineView::DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed)
{
	// Card
	LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];
	TimelineItemData* pData = GetItemData(Index);

	DrawCardForeground(dc, g, rectItem, Themed, m_HotItem==Index, m_FocusItem==Index, pData->Hdr.Selected,
		(pItemDescriptor->CoreAttributes.Flags & LFFlagMissing) ? 0x0000FF : (COLORREF)-1,
		m_ShowFocusRect);

	// Arrows
	if (pData->Arrow)
	{
		INT Base = (pData->Arrow==1) ? rectItem->right-1 : rectItem->left;
		INT Start = rectItem->top+(m_CaptionHeight+CARDPADDING)/2-ARROWSIZE+pData->ArrowOffs;
		INT y = Start;

		COLORREF ptCol = dc.GetPixel(Base, max(y, 0));

		for (INT a=-ARROWSIZE; a<=ARROWSIZE; a++)
		{
			CPen pen(PS_SOLID, 1, dc.GetPixel(Base-2*pData->Arrow, y));
			CPen* pPen = dc.SelectObject(&pen);

			const INT x = Base+(ARROWSIZE-abs(a)+1)*pData->Arrow;
			dc.MoveTo(Base-pData->Arrow, y);
			dc.LineTo(x, y);
			dc.SetPixel(x, y++, ptCol);

			dc.SelectObject(pPen);
		}

		if (Themed)
		{
			const INT Offs = pData->Arrow>0 ? 0 : 1;

			Pen pen(Color(0x0C000000));
			g.DrawLine(&pen, Base+(ARROWSIZE+1)*pData->Arrow, Start+ARROWSIZE+1, Base+pData->Arrow+1-Offs, Start+2*ARROWSIZE+Offs);
		}
	}

	// Icon
	CRect rect(rectItem);
	rect.DeflateRect(CARDPADDING, CARDPADDING);

	theApp.m_IconFactory.DrawSmallIcon(dc, rect.TopLeft(), pItemDescriptor);

	// Caption
	CRect rectCaption(rect);
	rectCaption.left += m_SmallIconSize+SMALLPADDING;

	dc.SetTextColor(pData->Hdr.Selected ? Themed ? 0xFFFFFF : GetSysColor(COLOR_HIGHLIGHTTEXT) : (pItemDescriptor->CoreAttributes.Flags & LFFlagMissing) ? 0x0000FF : Themed ? pData->Hdr.Selected ? 0xFFFFFF : pItemDescriptor->AggregateCount ? 0xCC3300 : 0x000000 : GetSysColor(COLOR_WINDOWTEXT));

	if ((pItemDescriptor->Type & LFTypeMask)!=LFTypeFolder)
	{
		dc.DrawText(GetLabel(pItemDescriptor), rectCaption, DT_LEFT | DT_TOP | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);
	}
	else
	{
		LPCWSTR pSubstring = wcsstr(pItemDescriptor->Description, L" (");

		dc.DrawText(pItemDescriptor->Description, pSubstring ? (INT)(pSubstring-pItemDescriptor->Description) : -1, rectCaption, DT_LEFT | DT_TOP | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);
	}

	rectCaption.top += m_DefaultFontHeight+CARDPADDING/3;

	// Subtext
	WCHAR tmpBuf[256];
	LFAttributeToString(pItemDescriptor, ((pItemDescriptor->Type & LFTypeMask)==LFTypeFile) ? m_ContextViewSettings.SortBy : LFAttrFileName, tmpBuf, 256);

	// Light text color
	if (!pData->Hdr.Selected)
		dc.SetTextColor(Themed ? 0xA39791 : GetSysColor(COLOR_3DSHADOW));

	CFont* pOldFont = dc.SelectObject(&theApp.m_SmallFont);
	dc.DrawText(tmpBuf, -1, rectCaption, DT_LEFT | DT_TOP | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);
	dc.SelectObject(pOldFont);

	rect.top += m_CaptionHeight+SMALLPADDING;

	// Separator line
	if (pData->PreviewMask && Themed)
		dc.FillSolidRect(rectItem->left+CARDPADDING, rect.top-1, m_ItemWidth-2*CARDPADDING, 1, pData->Hdr.Selected ? 0xFFFFFF : 0xE5E5E5);

	// Attributes
	if (pData->PreviewMask & PRV_ATTRIBUTES)
	{
		// Slightly lighter text color
		if (!pData->Hdr.Selected)
			dc.SetTextColor(Themed ? 0x404040 : GetSysColor(COLOR_WINDOWTEXT));

		CRect rectAttr(rect);
		rectAttr.top += SMALLPADDING;

		// Inset text when attribute icons are present
		if (pData->PreviewMask & PRV_COLLECTIONICON)
			rectAttr.left += m_SmallIconSize+SMALLPADDING;

		// Concatenate creator and title
		if (pData->PreviewMask & (PRV_CREATOR | PRV_TITLE))
		{
			WCHAR tmpStr[514] = L"";

			// Creator
			if (pData->PreviewMask & PRV_CREATOR)
			{
				ASSERT(pData->pStrCreator);

				wcscpy_s(tmpStr, 514, pData->pStrCreator);

				if (pData->PreviewMask & PRV_TITLE)
					wcscat_s(tmpStr, 514, (GetThreadLocale() & 0x1FF)==LANG_ENGLISH ? L"—" : L" – ");
			}

			// Title
			if (pData->PreviewMask & PRV_TITLE)
			{
				ASSERT(pData->pStrTitle);

				wcscat_s(tmpStr, 514, pData->pStrTitle);
			}

			dc.DrawText(tmpStr, -1, rectAttr, DT_LEFT | DT_TOP | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);

			// Attribute icon?
			if ((pData->PreviewMask & (PRV_TITLE | PRV_COLLECTION | PRV_COLLECTIONICON))==(PRV_TITLE | PRV_COLLECTIONICON))
				DrawCollectionIcon();

			rectAttr.top += m_DefaultFontHeight;
		}

		// Collection
		if (pData->PreviewMask & PRV_COLLECTION)
		{
			ASSERT(pData->pStrCollection);

			dc.DrawText(pData->pStrCollection, -1, rectAttr, DT_LEFT | DT_TOP | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);

			// Attribute icon?
			if (pData->PreviewMask & PRV_COLLECTIONICON)
				DrawCollectionIcon();

			rectAttr.top += m_DefaultFontHeight;
		}

		// Comments
		if (pData->PreviewMask & PRV_COMMENTS)
		{
			ASSERT(pData->pStrComments);

			dc.DrawText(pData->pStrComments, -1, rectAttr, DT_LEFT | DT_TOP | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);
			rectAttr.top += m_DefaultFontHeight;
		}

		rect.top = rectAttr.top;
	}

	// Thumbnails
	if (pData->PreviewMask & PRV_PREVIEW)
	{
		rect.top += LARGEPADDING;

		CPoint ptPreview(rect.left-2, rect.top+THUMBOFFSETY);

		if (pData->PreviewMask & PRV_REPRESENTATIVE)
		{
			ASSERT((pItemDescriptor->Type & LFTypeMask)==LFTypeFolder);

			theApp.m_IconFactory.DrawJumboIcon(dc, g, ptPreview, pItemDescriptor, p_RawFiles, 0);
		}
		else
			if ((pItemDescriptor->Type & LFTypeMask)==LFTypeFolder)
			{
				INT Row = 0;
				INT Col = 0;

				for (INT a=LFMaxRating; a>=0; a--)
					for (INT b=pItemDescriptor->FirstAggregate; b<=pItemDescriptor->LastAggregate; b++)
					{
						LFItemDescriptor* pItemDescriptor = (*p_RawFiles)[b];
						if (UsePreview(pItemDescriptor) && (pItemDescriptor->CoreAttributes.Rating==a))
						{
							DrawJumboIcon(dc, g, ptPreview, pItemDescriptor, 0);
							ptPreview.x += 128+THUMBMARGINX;

							if (++Col==m_PreviewColumns)
							{
								ptPreview.Offset(-(128+THUMBMARGINX)*m_PreviewColumns, 128+THUMBMARGINY);
								Col = 0;

								if (++Row==pData->ThumbnailRows)
									break;
							}
						}
					}
			}
			else
			{
				DrawJumboIcon(dc, g, ptPreview, pItemDescriptor, 0);
			}

		rect.top += (128+THUMBMARGINY)*pData->ThumbnailRows-THUMBMARGINY-4;
	}

	// Folder contents and source
	if (pData->PreviewMask & (PRV_SOURCE | PRV_CONTENTS))
	{
		// Light text color
		if (!pData->Hdr.Selected)
			dc.SetTextColor(Themed ? 0xA39791 : GetSysColor(COLOR_3DSHADOW));

		pOldFont = dc.SelectObject(&theApp.m_SmallFont);

		// Folder contents
		if (pData->PreviewMask & PRV_CONTENTS)
		{
			rect.top += LARGEPADDING;

			INT ListCount = min(MAXFILELIST, pData->ListCount);

			for (INT a=pItemDescriptor->FirstAggregate; a<=pItemDescriptor->LastAggregate; a++)
				if (!UsePreview((*p_RawFiles)[a]))
				{
					dc.DrawText(GetLabel((*p_RawFiles)[a]), rect, DT_LEFT | DT_TOP | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);

					rect.top += m_SmallFontHeight;

					if (!--ListCount)
						break;
				}
		}

		// Source
		if (pData->PreviewMask & PRV_SOURCE)
		{
			const UINT Content = (pData->PreviewMask & (PRV_CONTENTS | PRV_PREVIEW | PRV_ATTRIBUTES));
			if (Content)
			{
				rect.top += (Content==PRV_ATTRIBUTES) ? SMALLPADDING : LARGEPADDING;

				if (Themed)
					dc.FillSolidRect(rect.left, rect.top, m_ItemWidth-2*CARDPADDING, 1, pData->Hdr.Selected ? 0xFFFFFF : 0xE5E5E5);

				rect.top++;
			}
			
			rect.top += LARGEPADDING;

			m_SourceIcons.Draw(dc, rect.left, rect.top+(m_SourceHeight-m_SourceIconSize)/2, (pItemDescriptor->Type & LFTypeSourceMask)-2);
			rect.left += m_SourceIconSize+SMALLPADDING;

			dc.DrawText(theApp.m_SourceNames[pItemDescriptor->Type & LFTypeSourceMask][0], -1, rect, DT_LEFT | DT_VCENTER | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);
		}

		dc.SelectObject(pOldFont);
	}
}

void CTimelineView::ScrollWindow(INT dx, INT dy, LPCRECT lpRect, LPCRECT lpClipRect)
{
	if (IsCtrlThemed())
	{
		Invalidate();
	}
	else
	{
		CFileView::ScrollWindow(dx, dy, lpRect, lpClipRect);
	}
}


BEGIN_MESSAGE_MAP(CTimelineView, CFileView)
	ON_WM_CREATE()
	ON_WM_PAINT()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

INT CTimelineView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFileView::OnCreate(lpCreateStruct)==-1)
		return -1;

	// Icons
	m_SmallIconSize = GetSystemMetrics(SM_CYSMICON);
	m_SourceIconSize = m_SourceIcons.LoadSmall(IDB_SOURCES_16);

	// Heights
	m_CaptionHeight = max(m_SmallIconSize, m_DefaultFontHeight+CARDPADDING/3+m_SmallFontHeight);
	m_SourceHeight = max(m_SourceIconSize, m_SmallFontHeight);
	m_LabelWidth = (theApp.m_SmallBoldFont.GetTextExtent(_T("8888")).cx+2*CARDPADDING) | 1;

	return 0;
}

void CTimelineView::OnPaint()
{
	CRect rectUpdate;
	GetUpdateRect(rectUpdate);

	CPaintDC pDC(this);

	CRect rect;
	GetClientRect(rect);

	CDC dc;
	dc.CreateCompatibleDC(&pDC);
	dc.SetBkMode(TRANSPARENT);

	CBitmap MemBitmap;
	MemBitmap.CreateCompatibleBitmap(&pDC, rect.Width(), rect.Height());
	CBitmap* pOldBitmap = dc.SelectObject(&MemBitmap);

	Graphics g(dc);

	// Background
	const BOOL Themed = IsCtrlThemed();

	DrawCardBackground(dc, g, rect, Themed);

	// Items
	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	if (!DrawNothing(dc, rect, Themed))
	{
		RECT rectIntersect;

		// Timeline
		if (m_TwoColumns)
			if (Themed)
			{
				const INT x = rect.Width()/2-4;

				for (UINT a=0; a<8; a++)
				{
					SolidBrush brush(Color((ARGB)m_BevelColors[a]));
					g.FillRectangle(&brush, x+a, 0, 1, rect.Height());
				}
			}
			else
			{
				dc.FillSolidRect(rect.Width()/2-3, 0, 6, rect.Height(), GetSysColor(COLOR_3DSHADOW));
			}

		// Categories
		CFont* pOldFont = dc.SelectObject(&theApp.m_SmallBoldFont);

		dc.SetTextColor(Themed ? 0xFFFFFF : GetSysColor(COLOR_HIGHLIGHTTEXT));

		for (UINT a=0; a<m_Categories.m_ItemCount; a++)
		{
			CRect rect(m_Categories[a].Rect);
			rect.OffsetRect(0, -m_VScrollPos);
			rect.top -= BLENDHEIGHT;

			if (IntersectRect(&rectIntersect, rect, rectUpdate))
			{
				rect.top += BLENDHEIGHT;

				DrawCategory(dc, g, rect, &m_Categories[a], Themed);
			}
		}

		dc.SelectObject(pOldFont);

		g.SetPixelOffsetMode(PixelOffsetModeNone);

		// Items
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			TimelineItemData* pData = GetItemData(a);

			if (pData->Hdr.Valid)
			{
				CRect rect(pData->Hdr.Rect);
				rect.OffsetRect(0, -m_VScrollPos);

				if (IntersectRect(&rectIntersect, rect, rectUpdate))
					DrawItem(dc, g, rect, a, Themed);
			}
		}
	}

	DrawWindowEdge(g, Themed);

	pDC.BitBlt(0, 0, rect.Width(), rect.Height(), &dc, 0, 0, SRCCOPY);

	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldBitmap);
}

void CTimelineView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CFileView::OnKeyDown(nChar, nRepCnt, nFlags);

	if (p_CookedFiles)
	{
		CRect rect;
		GetClientRect(rect);

		INT Item = m_FocusItem;
		INT Left = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.left;
		INT Right = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.right;
		INT Top = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.top;
		INT Bottom = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.bottom;

		switch (nChar)
		{
		case VK_LEFT:
			for (INT a=Item-1; a>=0; a--)
			{
				TimelineItemData* pData = GetItemData(a);

				if ((pData->Hdr.Rect.right<Left) && pData->Hdr.Valid)
				{
					Item = a;

					break;
				}
			}

			break;

		case VK_RIGHT:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				TimelineItemData* pData = GetItemData(a);

				if ((pData->Hdr.Rect.left>Right) && pData->Hdr.Valid)
				{
					Item = a;

					break;
				}
			}

			for (INT a=Item; a>=0; a--)
			{
				TimelineItemData* pData = GetItemData(a);

				if ((pData->Hdr.Rect.left>Right) && pData->Hdr.Valid)
				{
					Item = a;

					break;
				}
			}

			break;

		case VK_UP:
			for (INT a=Item-1; a>=0; a--)
			{
				TimelineItemData* pData = GetItemData(a);

				if ((pData->Hdr.Rect.left==Left) && pData->Hdr.Valid)
				{
					Item = a;

					break;
				}
			}

			break;

		case VK_PRIOR:
			for (INT a=Item-1; a>=0; a--)
			{
				TimelineItemData* pData = GetItemData(a);

				if ((pData->Hdr.Rect.left<=Left) && pData->Hdr.Valid)
				{
					Item = a;

					if (pData->Hdr.Rect.top<=Bottom-rect.Height())
						break;
				}
			}

			break;

		case VK_DOWN:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				TimelineItemData* pData = GetItemData(a);

				if ((pData->Hdr.Rect.left==Left) && pData->Hdr.Valid)
				{
					Item = a;
					break;
				}
			}

			break;

		case VK_NEXT:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				TimelineItemData* pData = GetItemData(a);

				if ((pData->Hdr.Rect.right>=Right) && pData->Hdr.Valid)
				{
					Item = a;

					if (pData->Hdr.Rect.bottom>=Top+rect.Height())
						break;
				}
			}

			break;

		case VK_HOME:
			if (GetKeyState(VK_CONTROL)<0)
			{
				for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
					if (GetItemData(a)->Hdr.Valid)
					{
						Item = a;

						break;
					}
			}
			else
				for (INT a=Item-1; a>=0; a--)
				{
					TimelineItemData* pData = GetItemData(a);

					if (pData->Hdr.Valid)
						if (pData->Hdr.Rect.right<Left)
						{
							Item = a;

							break;
						}
				}

			break;

		case VK_END:
			if (GetKeyState(VK_CONTROL)<0)
			{
				for (INT a=(INT)p_CookedFiles->m_ItemCount-1; a>=0; a--)
					if (GetItemData(a)->Hdr.Valid)
					{
						Item = a;

						break;
					}
			}
			else
				for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
				{
					TimelineItemData* pData = GetItemData(a);

					if (pData->Hdr.Valid)
						if (pData->Hdr.Rect.left>Right)
						{
							Item = a;

							break;
						}
				}

			break;
		}

		if (Item!=m_FocusItem)
		{
			m_ShowFocusRect = TRUE;
			SetFocusItem(Item, GetKeyState(VK_SHIFT)<0);

			CPoint pt;
			GetCursorPos(&pt);
			ScreenToClient(&pt);

			OnMouseMove(0, pt);
		}
	}
}
