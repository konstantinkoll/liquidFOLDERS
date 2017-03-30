
// CTimelineView.cpp: Implementierung der Klasse CTimelineView
//

#include "stdafx.h"
#include "CTimelineView.h"
#include "liquidFOLDERS.h"


// CTimelineView
//

#define ARROWSIZE        (GUTTER-4)
#define GUTTER           BACKSTAGEBORDER
#define MIDDLE           (2*GUTTER+2)
#define THUMBMARGINX     2
#define THUMBMARGINY     (THUMBMARGINX+1)
#define THUMBOFFSETY     1
#define MAXFILELIST      10

#define GetItemData(Index)              ((TimelineItemData*)(m_ItemData+(Index)*m_DataSize))
#define UsePreview(pItemDescriptor)     ((pItemDescriptor->Type & LFTypeMounted) && (pItemDescriptor->CoreAttributes.ContextID>=LFContextPictures) && (pItemDescriptor->CoreAttributes.ContextID<=LFContextVideos))

CString CTimelineView::m_FilesSingular;
CString CTimelineView::m_FilesPlural;
CIcons CTimelineView::m_SourceIcons;

CTimelineView::CTimelineView()
	: CFileView(sizeof(TimelineItemData), TRUE, TRUE, TRUE, TRUE, TRUE, FALSE)
{
	if (m_FilesSingular.IsEmpty())
		ENSURE(m_FilesSingular.LoadString(IDS_FILES_SINGULAR));

	if (m_FilesPlural.IsEmpty())
		ENSURE(m_FilesPlural.LoadString(IDS_FILES_PLURAL));
}

WCHAR* CTimelineView::GetAttribute(TimelineItemData* pData, LFItemDescriptor* pItemDesciptor, UINT Attr, UINT Mask)
{
	ASSERT(theApp.m_Attributes[Attr].Type==LFTypeUnicodeString);

	if (pItemDesciptor->AttributeValues[Attr])
		if (*((WCHAR*)pItemDesciptor->AttributeValues[Attr]))
		{
			pData->Preview |= Mask;

			return (WCHAR*)pItemDesciptor->AttributeValues[Attr];
		}

	return NULL;
}

void CTimelineView::SetSearchResult(LFSearchResult* pRawFiles, LFSearchResult* pCookedFiles, FVPersistentData* pPersistentData)
{
	CFileView::SetSearchResult(pRawFiles, pCookedFiles, pPersistentData);

	if (p_CookedFiles)
		for (UINT a=0; a<p_CookedFiles->m_ItemCount; a++)
		{
			TimelineItemData* pData = GetItemData(a);
			LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[a];

			LFVariantData v;
			LFGetAttributeVariantDataEx(pItemDescriptor, m_ViewParameters.SortBy, v);

			pData->Hdr.Valid = !LFIsNullVariantData(v);
			if (pData->Hdr.Valid)
			{
				SYSTEMTIME stUTC;
				SYSTEMTIME stLocal;
				FileTimeToSystemTime(&v.Time, &stUTC);
				SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);

				pData->Year = stLocal.wYear;

				if ((pItemDescriptor->Type & LFTypeSourceMask)>LFTypeSourceInternal)
					pData->Preview |= PRV_SOURCE;

				switch (pItemDescriptor->Type & LFTypeMask)
				{
				case LFTypeFile:
					if (pItemDescriptor->CoreAttributes.ContextID==LFContextAudio)
					{
						pData->pArtist = GetAttribute(pData, pItemDescriptor, LFAttrArtist, PRV_TITLE);
						pData->pAlbum = GetAttribute(pData, pItemDescriptor, LFAttrAlbum, PRV_ALBUM);
					}

					pData->pComments = GetAttribute(pData, pItemDescriptor, LFAttrComments, PRV_COMMENTS);
					pData->pTitle = GetAttribute(pData, pItemDescriptor, LFAttrTitle, PRV_TITLE);

					break;

				case LFTypeFolder:
					pData->Preview |= PRV_COMMENTS;
					pData->pComments = NULL;

					// Comments
					for (INT b=pItemDescriptor->FirstAggregate; b<=pItemDescriptor->LastAggregate; b++)
					{
						LFItemDescriptor* pItemDescriptor = (*p_RawFiles)[b];

						ASSERT(theApp.m_Attributes[LFAttrRoll].Type==LFTypeUnicodeString);

						if (pData->Preview & PRV_COMMENTS)
							if (!pItemDescriptor->AttributeValues[LFAttrRoll])
							{
								pData->Preview &= ~PRV_COMMENTS;
							}
							else
								if (pData->pComments)
								{
									if (wcscmp(pData->pComments, (WCHAR*)pItemDescriptor->AttributeValues[LFAttrRoll])!=0)
										pData->Preview &= ~PRV_COMMENTS;
								}
								else
								{
									pData->pComments = (WCHAR*)pItemDescriptor->AttributeValues[LFAttrRoll];
									if (*pData->pComments==L'\0')
										pData->Preview &= ~PRV_COMMENTS;
								}
					}
				}
			}
		}
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
	m_PreviewColumns = (m_ItemWidth-THUMBMARGINX)/(128+THUMBMARGINX);

	INT CurRow[2] = { GUTTER+1, GUTTER+1 };
	INT LastRow = -10;

	for (INT a=0; a<(INT)p_CookedFiles->m_ItemCount; a++)
	{
		TimelineItemData* pData = GetItemData(a);

		if (pData->Hdr.Valid)
		{
			LFItemDescriptor* i = (*p_CookedFiles)[a];

			if (m_ItemWidth<2*CARDPADDING+128)
			{
				pData->Preview &= ~PRV_THUMBS;
			}
			else
			{
				INT PreviewCount = 0;
				pData->ListCount = 0;

				switch (i->Type & LFTypeMask)
				{
				case LFTypeFile:
					if (UsePreview(i))
					{
						pData->Preview |= PRV_THUMBS;
						PreviewCount = 1;
					}

					break;

				case LFTypeFolder:
					for (INT b=i->FirstAggregate; b<=i->LastAggregate; b++)
					{
						LFItemDescriptor* i = (*p_RawFiles)[b];
						if (UsePreview(i))
						{
							pData->Preview |= PRV_THUMBS;
							PreviewCount++;
						}
						else
						{
							pData->Preview |= PRV_FOLDER;
							pData->ListCount++;
						}
					}

					break;
				}

				pData->PreviewRows = m_PreviewColumns ? max(1, min(PreviewCount/m_PreviewColumns, m_PreviewColumns)) : 1;
			}

			INT Height = 2*CARDPADDING+m_CaptionHeight;

			if (pData->Preview)
				Height += CARDPADDING/2+CARDPADDING;

			if (pData->Preview & PRV_TITLE)
				Height += m_DefaultFontHeight;

			if (pData->Preview & PRV_ALBUM)
				Height += m_DefaultFontHeight;

			if (pData->Preview & PRV_COMMENTS)
			{
				Height += m_DefaultFontHeight;

				if (pData->Preview & (PRV_THUMBS | PRV_FOLDER))
					Height += CARDPADDING/2;
			}

			if (pData->Preview & PRV_THUMBS)
				Height += (128+THUMBMARGINY)*pData->PreviewRows-THUMBMARGINY-THUMBOFFSETY;

			if (pData->Preview & PRV_FOLDER)
			{
				Height += min(MAXFILELIST, pData->ListCount)*m_SmallFontHeight-CARDPADDING/2;

				if (pData->Preview & PRV_THUMBS)
					Height += CARDPADDING;
			}

			if (pData->Preview & PRV_SOURCE)
			{
				Height += max(m_SmallFontHeight, 16);

				if ((pData->Preview & (PRV_FOLDER | PRV_THUMBS))==PRV_THUMBS)
				{
					Height += CARDPADDING/2;

					if (pData->Preview & (PRV_TITLE | PRV_ALBUM | PRV_COMMENTS))
						Height += CARDPADDING/2;
				}
				else
					if (pData->Preview & (PRV_TITLE | PRV_ALBUM | PRV_COMMENTS | PRV_FOLDER))
					{
						Height += 2*CARDPADDING;
					}
			}

			if (pData->Year!=Year)
			{
				Year = pData->Year;

				ItemCategory ic;
				ZeroMemory(&ic, sizeof(ic));

				swprintf_s(ic.Caption, 256, L"%u", (UINT)Year);

				ic.Rect.left = rect.Width()/2-m_LabelWidth/2;
				ic.Rect.right = ic.Rect.left+m_LabelWidth;
				ic.Rect.top = max(CurRow[0], CurRow[1]);
				ic.Rect.bottom = ic.Rect.top+2*CARDPADDING+m_DefaultFontHeight;
				m_Categories.AddItem(ic);

				CurRow[0] = CurRow[1] = ic.Rect.bottom+GUTTER;
			}

			INT Column = m_TwoColumns ? CurRow[0]<=CurRow[1] ? 0 : 1 : 0;
			pData->Arrow = m_TwoColumns ? 1-(BYTE)Column*2 : 0;
			pData->ArrowOffs = 0;
			pData->Hdr.RectInflate = pData->Arrow ? ARROWSIZE+1 : 0;

			if (abs(CurRow[Column]-LastRow)<2*ARROWSIZE)
				if (Height>(m_CaptionHeight+CARDPADDING)/2+ARROWSIZE+CARDPADDING+2*GUTTER)
				{
					pData->ArrowOffs = 2*GUTTER;
				}
				else
				{
					CurRow[Column] += GUTTER+1;
				}

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

	rect.left += 2*CARDPADDING+m_IconSize-5;
	rect.top += CARDPADDING-2;
	rect.right -= CARDPADDING-2;
	rect.bottom = rect.top+m_DefaultFontHeight+4;

	return rect;
}

void CTimelineView::DrawCategory(CDC& dc, Graphics& g, LPCRECT rectCategory, ItemCategory* ic, BOOL Themed)
{
	if (Themed)
	{
		GraphicsPath path;
		CreateRoundRectangle(rectCategory, 4, path);

		SolidBrush brush(Color(0xFFC1C1C1));
		g.FillPath(&brush, &path);
	}
	else
	{
		dc.FillSolidRect(rectCategory, GetSysColor(COLOR_3DSHADOW));
	}

	dc.DrawText(ic->Caption, -1, (LPRECT)rectCategory, DT_SINGLELINE | DT_CENTER | DT_VCENTER | DT_NOPREFIX);
}

void CTimelineView::DrawItem(CDC& dc, Graphics& g, LPCRECT rectItem, INT Index, BOOL Themed)
{
	const TimelineItemData* pData = GetItemData(Index);
	LFItemDescriptor* pItemDescriptor = (*p_CookedFiles)[Index];

	DrawCardForeground(dc, g, rectItem, Themed, m_HotItem==Index, m_FocusItem==Index, pData->Hdr.Selected,
		((*p_CookedFiles)[Index]->CoreAttributes.Flags & LFFlagMissing) ? 0x0000FF : (COLORREF)-1,
		m_ShowFocusRect);

	// Arrows
	if (pData->Arrow)
	{
		INT Base = (pData->Arrow==1) ? rectItem->right-1 : rectItem->left;
		INT Start = rectItem->top+(m_CaptionHeight+CARDPADDING)/2-ARROWSIZE+pData->ArrowOffs;
		INT y = Start;

		COLORREF ptCol = dc.GetPixel(Base, y);

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
			INT Offs = pData->Arrow>0 ? 0 : 1;

			Pen pen(Color(0x0C000000));
			g.DrawLine(&pen, Base+(ARROWSIZE+1)*pData->Arrow, Start+ARROWSIZE+1, Base+pData->Arrow+1-Offs, Start+2*ARROWSIZE+Offs);
		}
	}

	// Icon
	CRect rectText(rectItem);
	rectText.DeflateRect(CARDPADDING, CARDPADDING);

	const UINT nStyle = ((pItemDescriptor->Type & LFTypeGhosted) ? ILD_BLEND50 : ILD_TRANSPARENT) | (pItemDescriptor->Type & LFTypeDefault ? INDEXTOOVERLAYMASK(1) : 0);
	if ((pItemDescriptor->Type & LFTypeMask)==LFTypeFile)
	{
		theApp.m_SystemImageListSmall.DrawEx(&dc, theApp.m_FileFormats.GetSysIconIndex(pItemDescriptor->CoreAttributes.FileFormat), rectText.TopLeft(), CSize(m_IconSize, m_IconSize), CLR_NONE, 0xFFFFFF, nStyle);
	}
	else
	{
		theApp.m_CoreImageListSmall.DrawEx(&dc, pItemDescriptor->IconID-1, rectText.TopLeft(), CSize(m_IconSize, m_IconSize), CLR_NONE, 0xFFFFFF, nStyle);
	}

	// Caption
	rectText.left += m_IconSize+CARDPADDING;
	rectText.bottom = rectText.top+m_DefaultFontHeight;

	dc.SetTextColor(pData->Hdr.Selected ? Themed ? 0xFFFFFF : GetSysColor(COLOR_HIGHLIGHTTEXT) : (pItemDescriptor->CoreAttributes.Flags & LFFlagMissing) ? 0x0000FF : Themed ? pData->Hdr.Selected ? 0xFFFFFF : pItemDescriptor->AggregateCount ? 0xCC3300 : 0x000000 : GetSysColor(COLOR_WINDOWTEXT));
	if ((pItemDescriptor->Type & LFTypeMask)!=LFTypeFolder)
	{
		dc.DrawText(GetLabel(pItemDescriptor), rectText, DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);
	}
	else
	{
		CString tmpStr;
		tmpStr.Format(pItemDescriptor->AggregateCount==1 ? m_FilesSingular : m_FilesPlural, pItemDescriptor->AggregateCount);

		dc.DrawText(tmpStr, -1, rectText, DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);
	}

	rectText.top = rectText.bottom+CARDPADDING/2;
	rectText.bottom = rectText.top+m_SmallFontHeight;

	WCHAR tmpBuf[256];
	LFAttributeToString(pItemDescriptor, ((pItemDescriptor->Type & LFTypeMask)==LFTypeFile) ? m_ViewParameters.SortBy : LFAttrFileName, tmpBuf, 256);

	if (!pData->Hdr.Selected)
		dc.SetTextColor(Themed ? 0xA39791 : GetSysColor(COLOR_3DSHADOW));

	CFont* pOldFont = dc.SelectObject(&theApp.m_SmallFont);
	dc.DrawText(tmpBuf, -1, rectText, DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);

	// Preview
	if (pData->Preview)
	{
		if (Themed)
			dc.FillSolidRect(rectItem->left+CARDPADDING+1, rectText.bottom+CARDPADDING/2, m_ItemWidth-2*CARDPADDING-2, 1, pData->Hdr.Selected ? 0xFFFFFF : 0xE5E5E5);

		// Source
		INT BottomHeight = 0;
		if (pData->Preview & PRV_SOURCE)
		{
			BottomHeight = max(m_SmallFontHeight, 16);
			CRect rectSource(rectItem->left+CARDPADDING, rectItem->bottom-CARDPADDING-BottomHeight, rectItem->right-CARDPADDING, 0);
			rectSource.bottom = rectSource.top+BottomHeight;

			m_SourceIcons.Draw(dc, rectSource.left, rectSource.top, (pItemDescriptor->Type & LFTypeSourceMask)-2);

			rectSource.left += m_IconSize+CARDPADDING;
			dc.DrawText(theApp.m_SourceNames[pItemDescriptor->Type & LFTypeSourceMask][0], -1, rectSource, DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);

			if ((pData->Preview & (PRV_FOLDER | PRV_THUMBS))==PRV_THUMBS)
			{
				BottomHeight += CARDPADDING/2;
			}
			else
				if (pData->Preview & (PRV_TITLE | PRV_ALBUM | PRV_COMMENTS | PRV_FOLDER))
				{
					if (Themed)
						dc.FillSolidRect(rectItem->left+CARDPADDING+1, rectSource.top-CARDPADDING, m_ItemWidth-2*CARDPADDING-2, 1, pData->Hdr.Selected ? 0xFFFFFF : 0xE5E5E5);

					BottomHeight += 2*CARDPADDING;
				}
		}

		// Folder
		if (pData->Preview & PRV_FOLDER)
		{
			INT ListCount = min(MAXFILELIST, pData->ListCount);
			BottomHeight += ListCount*m_SmallFontHeight;

			CRect rectFilename(rectItem->left+CARDPADDING+1, rectItem->bottom-CARDPADDING-BottomHeight, rectItem->right-CARDPADDING, rectItem->bottom-CARDPADDING-BottomHeight+m_SmallFontHeight);

			for (INT a=pItemDescriptor->FirstAggregate; a<=pItemDescriptor->LastAggregate; a++)
				if (!UsePreview((*p_RawFiles)[a]))
				{
					dc.DrawText(GetLabel((*p_RawFiles)[a]), rectFilename, DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX | DT_SINGLELINE);

					if (!--ListCount)
						break;

					rectFilename.OffsetRect(0, m_SmallFontHeight);
				}

			BottomHeight += CARDPADDING/2;
		}

		dc.SelectObject(pOldFont);

		// Attributes
		if (pData->Preview & (PRV_TITLE | PRV_ALBUM | PRV_COMMENTS))
		{
			CRect rectAttr(rectItem->left+CARDPADDING+1, rectText.bottom+CARDPADDING+CARDPADDING/2, rectItem->right-CARDPADDING, 0);
			rectAttr.bottom = rectAttr.top+m_DefaultFontHeight;

			if (pData->Preview & PRV_ALBUM)
				rectAttr.left = rectItem->left+m_IconSize+2*CARDPADDING;

			if (!pData->Hdr.Selected)
				dc.SetTextColor(Themed ? 0x404040 : GetSysColor(COLOR_WINDOWTEXT));

			if (pData->Preview & PRV_TITLE)
			{
				WCHAR tmpStr[513] = L"";

				if (pData->pArtist)
				{
					wcscpy_s(tmpStr, 513, pData->pArtist);
					if (pData->pTitle)
						wcscat_s(tmpStr, 513, L": ");
				}

				if (pData->pTitle)
					wcscat_s(tmpStr, 513, pData->pTitle);

				dc.DrawText(tmpStr, -1, rectAttr, DT_SINGLELINE | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
				rectAttr.OffsetRect(0, m_DefaultFontHeight);
			}

			if (pData->Preview & PRV_ALBUM)
			{
				ASSERT(pData->pAlbum);

				dc.DrawText(pData->pAlbum, -1, rectAttr, DT_SINGLELINE | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);

				theApp.m_AttributeIconsSmall.Draw(dc, rectAttr.left-CARDPADDING-m_IconSize, rectAttr.top-(m_DefaultFontHeight-16)/2, GetAttributeIconIndex(LFAttrAlbum));

				rectAttr.OffsetRect(0, m_DefaultFontHeight);
			}

			if (pData->Preview & PRV_COMMENTS)
			{
				ASSERT(pData->pComments);

				dc.DrawText(pData->pComments, -1, rectAttr, DT_SINGLELINE | DT_LEFT | DT_END_ELLIPSIS | DT_NOPREFIX);
			}
		}

		// Thumbs
		if (pData->Preview & PRV_THUMBS)
		{
			CRect rectPreview(rectItem);
			rectPreview.DeflateRect(CARDPADDING, CARDPADDING);
			rectPreview.top = rectPreview.bottom-BottomHeight-pData->PreviewRows*(128+THUMBMARGINY)+THUMBMARGINY;
			rectPreview.bottom = rectPreview.top+128;
			rectPreview.right = rectPreview.left+128;

			if ((pItemDescriptor->Type & LFTypeMask)==LFTypeFolder)
			{
				INT Rows = 0;
				INT Cols = 0;

				for (INT a=LFMaxRating; a>=0; a--)
					for (INT b=pItemDescriptor->FirstAggregate; b<=pItemDescriptor->LastAggregate; b++)
					{
						LFItemDescriptor* ri = (*p_RawFiles)[b];
						if (UsePreview(ri) && (ri->CoreAttributes.Rating==a))
						{
							CRect rect(rectPreview);
							if (!theApp.m_ThumbnailCache.DrawJumboThumbnail(dc, rect, ri))
								theApp.m_FileFormats.DrawJumboIcon(dc, rect, ri->CoreAttributes.FileFormat, ri->Type & LFTypeGhosted);

							rectPreview.OffsetRect(128+THUMBMARGINX, 0);
							if (++Cols==m_PreviewColumns)
							{
								rectPreview.OffsetRect(-(128+THUMBMARGINX)*m_PreviewColumns, 128+THUMBMARGINY);
								Cols = 0;

								if (++Rows==pData->PreviewRows)
									return;
							}
						}
					}
			}
			else
			{
				if (!theApp.m_ThumbnailCache.DrawJumboThumbnail(dc, rectPreview, pItemDescriptor))
					theApp.m_FileFormats.DrawJumboIcon(dc, rectPreview, pItemDescriptor->CoreAttributes.FileFormat, pItemDescriptor->Type & LFTypeGhosted);
			}
		}
	}
	else
	{
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
	theApp.LoadAttributeIconsSmall();
	m_IconSize = m_SourceIcons.LoadSmall(IDB_SOURCES_16);

	// Heights
	m_CaptionHeight = max(m_IconSize, m_LargeFontHeight+m_SmallFontHeight+CARDPADDING/2);
	m_LabelWidth = theApp.m_SmallBoldFont.GetTextExtent(_T("8888")).cx+2*CARDPADDING;

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
	BOOL Themed = IsCtrlThemed();

	DrawCardBackground(dc, g, rect, Themed);

	// Items
	CFont* pOldFont = dc.SelectObject(&theApp.m_DefaultFont);

	if (m_Nothing)
	{
		CRect rectText(rect);
		rectText.top += m_HeaderHeight+BACKSTAGEBORDER;

		CString tmpStr((LPCSTR)IDS_NOTHINGTODISPLAY);

		dc.SetTextColor(Themed ? 0xBFB0A6 : GetSysColor(COLOR_3DSHADOW));
		dc.DrawText(tmpStr, rectText, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

		g.SetPixelOffsetMode(PixelOffsetModeNone);
	}
	else
	{
		RECT rectIntersect;

		// Timeline
		if (m_TwoColumns)
			dc.FillSolidRect(rect.Width()/2-1, -m_VScrollPos, 2, m_ScrollHeight, Themed ? 0xC1C1C1 : GetSysColor(COLOR_3DSHADOW));

		CFont* pOldFont = dc.SelectObject(&theApp.m_SmallBoldFont);

		dc.SetTextColor(Themed ? 0xFFFFFF : GetSysColor(COLOR_HIGHLIGHTTEXT));

		for (UINT a=0; a<m_Categories.m_ItemCount; a++)
		{
			CRect rect(m_Categories[a].Rect);
			rect.OffsetRect(0, -m_VScrollPos);

			if (IntersectRect(&rectIntersect, rect, rectUpdate))
				DrawCategory(dc, g, rect, &m_Categories[a], Themed);
		}

		dc.SelectObject(pOldFont);

		g.SetPixelOffsetMode(PixelOffsetModeNone);

		// Items
		if (p_CookedFiles)
			if (p_CookedFiles->m_ItemCount)
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
		INT left = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.left;
		INT right = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.right;
		INT top = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.top;
		INT bottom = (Item==-1) ? 0 : GetItemData(Item)->Hdr.Rect.bottom;

		switch (nChar)
		{
		case VK_LEFT:
			for (INT a=Item-1; a>=0; a--)
			{
				TimelineItemData* pData = GetItemData(a);
				if ((pData->Hdr.Rect.right<left) && pData->Hdr.Valid)
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
				if ((pData->Hdr.Rect.left>right) && pData->Hdr.Valid)
				{
					Item = a;
					break;
				}
			}
			for (INT a=Item; a>=0; a--)
			{
				TimelineItemData* pData = GetItemData(a);
				if ((pData->Hdr.Rect.left>right) && pData->Hdr.Valid)
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
				if ((pData->Hdr.Rect.left==left) && pData->Hdr.Valid)
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
				if ((pData->Hdr.Rect.left<=left) && pData->Hdr.Valid)
				{
					Item = a;
					if (pData->Hdr.Rect.top<=bottom-rect.Height())
						break;
				}
			}

			break;

		case VK_DOWN:
			for (INT a=Item+1; a<(INT)p_CookedFiles->m_ItemCount; a++)
			{
				TimelineItemData* pData = GetItemData(a);
				if ((pData->Hdr.Rect.left==left) && pData->Hdr.Valid)
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
				if ((pData->Hdr.Rect.right>=right) && pData->Hdr.Valid)
				{
					Item = a;
					if (pData->Hdr.Rect.bottom>=top+rect.Height())
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
						if (pData->Hdr.Rect.right<left)
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
						if (pData->Hdr.Rect.left>right)
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
