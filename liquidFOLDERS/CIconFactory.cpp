
// CIconFactory.cpp: Implementierung der Klasse CIconFactory
//

#include "stdafx.h"
#include "CIconFactory.h"
#include "liquidFOLDERS.h"


// CIconFactory
//

#define THUMBCUTOFF     2

CIcons CIconFactory::m_ApplicationIcons;

CIconFactory::CIconFactory()
{
	m_FileFormats.InitHashTable(2111);			// Prime number with pz = 4n+3

	m_SmallIconSize = GetSystemMetrics(SM_CYSMICON);

	// Image list for scales down 128x128 icons
	m_SystemIcons128.Create(128, 128, ILC_COLOR32, 64, 8);

	// Retrieve index of generic file format icons
	SHFILEINFO sfi;
	m_GenericSystemIconIndex = SUCCEEDED(SHGetFileInfo(_T("*"), 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES)) ? sfi.iIcon : 3;
	m_GenericIconIndex128 = QuarterJumboSystemIcon(m_GenericSystemIconIndex);
}

void CIconFactory::DrawJumboFormatIcon(CDC& dc, const CPoint& pt, LPCSTR lpszFileFormat, BOOL Ghosted)
{
	ASSERT(lpszFileFormat);

	FileFormatData Data;
	LookupFileFormat(lpszFileFormat, Data);

	const UINT nStyle = Ghosted ? ILD_BLEND50 : ILD_TRANSPARENT;
	if (theApp.OSVersion<OS_Vista)
	{
		// Draw extra large shell icon, centered
		theApp.m_SystemImageListExtraLarge.DrawEx(&dc, Data.SystemIconIndex, CPoint(pt.x+(128-theApp.m_ExtraLargeIconSize)/2, pt.y+(128-theApp.m_ExtraLargeIconSize)/2), CSize(theApp.m_ExtraLargeIconSize, theApp.m_ExtraLargeIconSize), CLR_NONE, 0xFFFFFF, nStyle);
	}
	else
	{
		// Draw jumbo icon
		m_SystemIcons128.DrawEx(&dc, Data.IconIndex128, pt, CSize(128, 128), CLR_NONE, 0xFFFFFF, nStyle);
	}
}

void CIconFactory::DrawJumboIcon(CDC& dc, Graphics& g, CPoint pt, LFItemDescriptor* pItemDescriptor, LFSearchResult* pRawFiles, BOOL DrawOverlays, INT ThumbnailYOffset)
{
	ASSERT(pItemDescriptor);

	BOOL DrawAppBadge = DrawOverlays;
	BOOL DrawSash = DrawOverlays;

	// Draw icon
	if (pItemDescriptor->IconID)
	{
		// No app badge for folders
		DrawAppBadge = FALSE;

		// Do we have a folder?
		if (LFIsFolder(pItemDescriptor))
		{
			// Try to draw a representative thumbnail
			if (DrawRepresentativeThumbnail(dc, g, pt, pItemDescriptor, pRawFiles, ThumbnailYOffset))
			{
				// No sash for representative thumbnails
				DrawSash = FALSE;

				goto FinishIcon;
			}

			// Try to draw a map using the GPS coordinates
			if (DrawJumboMap(g, pt, pItemDescriptor, ThumbnailYOffset))
				goto FinishIcon;
		}

		// Generic core icon
		if (pItemDescriptor->IconID>=IDI_FIRSTPLACEHOLDERICON)
		{
			// Offset placeholder icon for visually pleasing result
			pt.y++;
			ThumbnailYOffset--;
		}
		else
		{
			// No sash for system icons other than placeholder icon
			DrawSash = FALSE;
		}

		// Draw core icon
		theApp.m_CoreImageListJumbo.DrawEx(&dc, pItemDescriptor->IconID-1, pt, CSize(128, 128), CLR_NONE, 0xFFFFFF, ((pItemDescriptor->Type & LFTypeGhosted) ? ILD_BLEND50 : ILD_TRANSPARENT) | (pItemDescriptor->Type & LFTypeBadgeMask));
		DrawStoreIconShadow(g, pt, pItemDescriptor->IconID);
	}
	else
	{
		ASSERT(LFIsFile(pItemDescriptor));

		// Draw thumbnail or file format icon
		if (!DrawJumboThumbnail(dc, g, pt, pItemDescriptor, DrawSash, ThumbnailYOffset))
		{
			DrawJumboFormatIcon(dc, pt, pItemDescriptor->CoreAttributes.FileFormat, pItemDescriptor->Type & LFTypeGhosted);

			// No overlays for file format icons
			DrawAppBadge = DrawSash = FALSE;
		}
	}

FinishIcon:
	// Draw overlays
	if (DrawAppBadge)
	{
		LFVariantData VData;
		LFGetAttributeVariantDataEx(pItemDescriptor, LFAttrApplication, VData);

		if (!LFIsNullVariantData(VData) && (VData.Application<LFApplicationCount))
			m_ApplicationIcons.Draw(dc, pt.x-2, pt.y+ThumbnailYOffset+98, VData.Application-1);
	}

	if (DrawSash)
	{
		const INT Overlay = (pItemDescriptor->CoreAttributes.Flags & LFFlagMissing) ? IDI_OVR_ERROR : (pItemDescriptor->CoreAttributes.Flags & LFFlagNew) ? IDI_OVR_NEW : 0;
		if (Overlay)
			theApp.m_CoreImageListJumbo.DrawEx(&dc, Overlay-1, CPoint(pt.x+1, pt.y+ThumbnailYOffset-2), CSize(128, 128), CLR_NONE, 0xFFFFFF, ((pItemDescriptor->Type & LFTypeGhosted) ? ILD_BLEND50 : ILD_TRANSPARENT));
	}
}

void CIconFactory::DrawSmallIcon(CDC& dc, const CPoint& pt, const LFItemDescriptor* pItemDescriptor)
{
	ASSERT(pItemDescriptor);

	const UINT nStyle = (pItemDescriptor->Type & LFTypeGhosted) ? ILD_BLEND50 : ILD_TRANSPARENT;
	if (pItemDescriptor->IconID)
	{
		// Draw core icon
		theApp.m_CoreImageListSmall.DrawEx(&dc, pItemDescriptor->IconID-1, pt, CSize(m_SmallIconSize, m_SmallIconSize), CLR_NONE, 0xFFFFFF, nStyle);
	}
	else
	{
		ASSERT(LFIsFile(pItemDescriptor));

		// Draw file format icon
		theApp.m_SystemImageListSmall.DrawEx(&dc, GetSystemIconIndex(pItemDescriptor->CoreAttributes.FileFormat), pt, CSize(m_SmallIconSize, m_SmallIconSize), CLR_NONE, 0xFFFFFF, nStyle);
	}
}

HBITMAP CIconFactory::GetRepresentativeThumbnailBitmap(LFSearchResult* pSearchResult)
{
	ASSERT(pSearchResult);

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	HBITMAP hBitmap = CreateTransparentBitmap(128, 128);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	Graphics g(dc);

	// Try to draw a thumbnail
	BOOL DrawSash;

	for (UINT a=0; a<pSearchResult->m_ItemCount; a++)
		if (DrawJumboThumbnail(dc, g, CPoint(0, 0), (*pSearchResult)[a], DrawSash, 0))
			return (HBITMAP)dc.SelectObject(hOldBitmap);

	dc.SelectObject(hOldBitmap);
	DeleteObject(hBitmap);

	return NULL;
}

HBITMAP CIconFactory::GetMapBitmap(LPCSTR lpszIATACode)
{
	ASSERT(lpszIATACode);

	LPCAIRPORT lpcAirport;
	if (!LFIATAGetAirportByCode(lpszIATACode, lpcAirport))
		return NULL;

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	HBITMAP hBitmap = CreateTransparentBitmap(128, 128);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	Graphics g(dc);

	DrawJumboMap(g, CPoint(0, 0), lpcAirport->Location, 0);

	return (HBITMAP)dc.SelectObject(hOldBitmap);
}

HBITMAP CIconFactory::GetJumboIconBitmap(LFItemDescriptor* pItemDescriptor, LFSearchResult* pRawFiles)
{
	ASSERT(pItemDescriptor);

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	HBITMAP hBitmap = CreateTransparentBitmap(128, 128);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	Graphics g(dc);

	DrawJumboIcon(dc, g, CPoint(0, 0), pItemDescriptor, pRawFiles, FALSE);

	return (HBITMAP)dc.SelectObject(hOldBitmap);
}


// Thumbnail handling

void CIconFactory::MakeBitmapSolid(HBITMAP hBitmap, INT x, INT y, INT cx, INT cy)
{
	ASSERT(hBitmap);

	BITMAP Bitmap;
	GetObject(hBitmap, sizeof(Bitmap), &Bitmap);

	ASSERT(Bitmap.bmBitsPixel==32);
	ASSERT(Bitmap.bmBits);

	// Set alpha channel to 0xFF
	for (INT Row=y; Row<y+cy; Row++)
	{
		LPBYTE pByte = (LPBYTE)Bitmap.bmBits+Bitmap.bmWidthBytes*Row+x*4+3;

		for (INT Column=cx; Column>0; Column--)
		{
			*pByte = 0xFF;

			pByte += 4;
		}
	}
}

BOOL CIconFactory::GetThumbnailBitmap(LFItemDescriptor* pItemDescriptor, ThumbnailData& Thumbnail)
{
	ASSERT(LFIsFile(pItemDescriptor));

	// Calculate thumbnail size and other parameters
	const BOOL BlackFrame = LFIsAudioFile(pItemDescriptor);
	const BOOL IsMediaFile = LFIsMediaFile(pItemDescriptor);
	const BOOL IsDocumentFile = LFIsDocumentFile(pItemDescriptor);
	const INT CutOff = IsMediaFile || IsDocumentFile ? THUMBCUTOFF : 0;
	const INT ThumbSize = (BlackFrame ? 124 : 118)+2*CutOff;

	HBITMAP hBitmapImage = LFGetThumbnail(pItemDescriptor, CSize(ThumbSize, ThumbSize));
	if (!hBitmapImage)
		return FALSE;

	// Size allowed?
	BITMAP BitmapImage;
	GetObject(hBitmapImage, sizeof(BitmapImage), &BitmapImage);

	if ((BitmapImage.bmWidth>128) || (BitmapImage.bmHeight>128))
	{
		DeleteObject(hBitmapImage);

		return FALSE;
	}

	// Convert thumbnail to 128x128x32bpp
	CRect rectSrc(CutOff, CutOff, BitmapImage.bmWidth-CutOff, BitmapImage.bmHeight-CutOff);

	CRect rectDst;
	rectDst.right = (rectDst.left=(128-BitmapImage.bmWidth)/2+CutOff)+BitmapImage.bmWidth-2*CutOff;
	rectDst.bottom = (rectDst.top=(128-BitmapImage.bmHeight)/2+CutOff-1)+BitmapImage.bmHeight-2*CutOff;

	Thumbnail.HasFrame = IsMediaFile || IsDocumentFile || ((rectSrc.Width()<=118+2*THUMBCUTOFF) && (rectSrc.Height()<=118+2*THUMBCUTOFF));
	Thumbnail.HasBackground = !Thumbnail.HasFrame || BlackFrame;

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	Thumbnail.hBitmap = CreateTransparentBitmap(128, 128);
	HBITMAP hOldBitmap1 = (HBITMAP)dc.SelectObject(Thumbnail.hBitmap);

	Graphics g(dc);
	g.SetSmoothingMode(SmoothingModeNone);

	if (Thumbnail.HasFrame)
	{
		// Draw background
		if (BlackFrame)
		{
			SolidBrush brush(Color(0xFF000000));
			g.FillRectangle(&brush, 2, 1, 124, 124);
		}

		// Inflate sizes which are just 1 or 2 pixels short
		if (CutOff)
		{
			if (rectSrc.Width()==ThumbSize-2*CutOff-2)
			{
				rectSrc.InflateRect(1, 0);
				rectDst.InflateRect(1, 0);
			}
			else
				if (rectSrc.Width()==ThumbSize-2*CutOff-1)
				{
					rectSrc.right++;
					rectDst.right++;
				}

			if (rectSrc.Height()==ThumbSize-2*CutOff-2)
			{
				rectSrc.InflateRect(0, 1);
				rectDst.InflateRect(0, 1);
			}
			else
				if (rectSrc.Height()==ThumbSize-2*CutOff-1)
				{
					rectSrc.bottom++;
					rectDst.bottom++;
				}
		}
	}

	// Draw image on thumbnail
	HDC hdcMem = CreateCompatibleDC(dc);
	HBITMAP hOldBitmap2 = (HBITMAP)SelectObject(hdcMem, hBitmapImage);

	if (BitmapImage.bmBitsPixel!=32)
	{
		BitBlt(dc, rectDst.left, rectDst.top, rectDst.Width(), rectDst.Height(), hdcMem, rectSrc.left, rectSrc.top, SRCCOPY);
		MakeBitmapSolid(Thumbnail.hBitmap, rectDst.left, rectDst.top, rectDst.Width(), rectDst.Height());
	}
	else
	{
		AlphaBlend(dc, rectDst.left, rectDst.top, rectDst.Width(), rectDst.Height(), hdcMem, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(), BF);
	}

	// Decorate with frame
	if (Thumbnail.HasFrame)
	{
		g.DrawImage(theApp.GetCachedResourceImage(IDB_THUMBNAIL), 0, 0);

		if (!BlackFrame && !IsDocumentFile)
		{
			Pen pen1(Color(0x30000000));
			g.DrawRectangle(&pen1, rectDst.left, rectDst.top, rectDst.Width()-1, rectDst.Height()-1);

			Pen pen2(Color(0x10000000));
			g.DrawRectangle(&pen2, rectDst.left+1, rectDst.top+1, rectDst.Width()-2, rectDst.Height()-2);
		}
	}

	SelectObject(hdcMem, hOldBitmap2);
	DeleteDC(hdcMem);

	dc.SelectObject(hOldBitmap1);

	DeleteObject(hBitmapImage);

	return TRUE;
}

BOOL CIconFactory::LookupThumbnail(LFItemDescriptor* pItemDescriptor, ThumbnailData& Thumbnail)
{
	ASSERT(LFIsFile(pItemDescriptor));

	// Is there a thumbnail?
	if (m_Thumbnails.Lookup(pItemDescriptor, Thumbnail))
		return TRUE;

	// Do we know that this file has no thumbnail?
	if (m_NoThumbnails.Lookup(pItemDescriptor, Thumbnail))
		return FALSE;

	// Add file to either m_Thumbnails or m_NoThumbnails
	ZeroMemory(&Thumbnail, sizeof(Thumbnail));
	Thumbnail.StoreID = pItemDescriptor->StoreID;
	Thumbnail.FileID = pItemDescriptor->CoreAttributes.FileID;

	if (GetThumbnailBitmap(pItemDescriptor, Thumbnail))
	{
		// Thumbnail available
		m_Thumbnails.AddItem(Thumbnail);

		return TRUE;
	}
	else
	{
		// No thumbnail available
		m_NoThumbnails.AddItem(Thumbnail);

		return FALSE;
	}
}

BOOL CIconFactory::DrawJumboThumbnail(CDC& dc, Graphics& g, const CPoint& pt, LFItemDescriptor* pItemDescriptor, BOOL& DrawSash, INT ThumbnailYOffset)
{
	ASSERT(LFIsFile(pItemDescriptor));

	ThumbnailData Thumbnail;
	if (!LookupThumbnail(pItemDescriptor, Thumbnail))
		return FALSE;

	// Draw color background
	if (!Thumbnail.HasBackground)
	{
		ASSERT(Thumbnail.HasFrame);

		g.SetSmoothingMode(SmoothingModeNone);

		SolidBrush brush(Color(COLORREF2RGB(LFGetItemColor(pItemDescriptor->CoreAttributes.Color, LFItemColorFadeLight))));
		g.FillRectangle(&brush, pt.x+3, pt.y+2+ThumbnailYOffset, 122, 122);
	}

	// Handle sash
	DrawSash &= Thumbnail.HasFrame;

	// Draw thumbnail
	HDC hdcMem = CreateCompatibleDC(dc);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, Thumbnail.hBitmap);

	AlphaBlend(dc, pt.x, pt.y+ThumbnailYOffset, 128, 128, hdcMem, 0, 0, 128, 128, BF);

	SelectObject(hdcMem, hOldBitmap);
	DeleteDC(hdcMem);

	return TRUE;
}

BOOL CIconFactory::DrawRepresentativeThumbnail(CDC& dc, Graphics& g, const CPoint& pt, LFItemDescriptor* pItemDescriptor, LFSearchResult* pRawFiles, INT ThumbnailYOffset)
{
	ASSERT(LFIsFolder(pItemDescriptor));

	// No placeholder icon
	if (!theApp.IsPlaceholderIcon(pItemDescriptor->IconID))
		return FALSE;

	// Do we have a raw search result?
	if (!pRawFiles)
		return FALSE;

	// Is the folder an aggregated folder?
	if (!LFIsAggregated(pItemDescriptor))
		return FALSE;

	// Try to draw a thumbnail
	for (INT a=pItemDescriptor->AggregateFirst; a<=pItemDescriptor->AggregateLast; a++)
		if (DrawJumboThumbnail(dc, g, pt, (*pRawFiles)[a], ThumbnailYOffset))
			return TRUE;

	return FALSE;
}

BOOL CIconFactory::DrawJumboMap(Graphics& g, const CPoint& pt, const LFGeoCoordinates& GeoCoordinates, INT ThumbnailYOffset)
{
	// Draw map
	g.SetSmoothingMode(SmoothingModeAntiAlias);

	Bitmap* pMap = theApp.GetCachedResourceImage(IDB_BLUEMARBLE_512);
	const CSize Size(pMap->GetWidth(), pMap->GetHeight());

	// Map location
	INT X = (INT)((GeoCoordinates.Longitude+180.0)*(DOUBLE)Size.cx/360.0+0.5f);
	INT Y = (INT)((GeoCoordinates.Latitude+90.0)*(DOUBLE)Size.cy/180.0+0.5f);

	// Map offset
	INT OffsX = -X+124/2;
	INT OffsY = -Y+124/2;

	if (OffsY>0)
	{
		OffsY = 0;
	}
	else
		if (OffsY<124-Size.cy)
		{
			OffsY = 124-Size.cy;
		}

	ImageAttributes ImgAttr;
	ImgAttr.SetWrapMode(WrapModeTile);
	g.DrawImage(pMap, Rect(pt.x+2, pt.y+ThumbnailYOffset+1, 124, 124), -OffsX, -OffsY, 124, 124, UnitPixel, &ImgAttr);

	// Location indicator
	X += OffsX-4;
	Y += OffsY-4;
	DrawLocationIndicator(g, pt.x+2+X, pt.y+ThumbnailYOffset+1+Y, 8);

	// Decorate map with thumbnail frame
	g.DrawImage(theApp.GetCachedResourceImage(IDB_THUMBNAIL), pt.x, pt.y+ThumbnailYOffset);

	return TRUE;
}

BOOL CIconFactory::DrawJumboMap(Graphics& g, const CPoint& pt, const LFItemDescriptor* pItemDescriptor, INT ThumbnailYOffset)
{
	ASSERT(LFIsFolder(pItemDescriptor));

	// GPS coordinates
	LFVariantData VData;
	LFGetAttributeVariantDataEx(pItemDescriptor, LFAttrLocationGPS, VData);

	return LFIsNullVariantData(VData) ? FALSE : DrawJumboMap(g, pt, VData.GeoCoordinates, ThumbnailYOffset);
}


// File format handling

INT CIconFactory::QuarterJumboSystemIcon(INT SystemIconIndex)
{
	if (theApp.OSVersion<OS_Vista)
		return -1;

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	// Icon in eine Bitmap konvertieren
	HBITMAP hBitmap = CreateTransparentBitmap(256, -256);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	theApp.m_SystemImageListJumbo.DrawEx(&dc, SystemIconIndex, CPoint(0, 0), CSize(256, 256), CLR_NONE, 0xFFFFFF, ILD_TRANSPARENT);

	dc.SelectObject(hOldBitmap);

	// Hinzufügen
	const INT Result = ImageList_Add(m_SystemIcons128, hBitmap=LFSanitizeThumbnail(hBitmap), NULL);

	DeleteObject(hBitmap);

	return Result;
}

void CIconFactory::LookupFileFormat(LPCSTR lpszFileFormat, FileFormatData& Data)
{
	ASSERT(lpszFileFormat);

	if (*lpszFileFormat)
	{
		// Convert extension to upper case
		CHAR Key[LFExtSize];

		CHAR* pChar = Key;
		do *(pChar++) = (CHAR)toupper(*lpszFileFormat); while (*(lpszFileFormat++));

		// Look up in hashmap
		if (m_FileFormats.Lookup(Key, Data))
			return;

		// Gather file format data from Windows
		CString Ext(Key);
		if ((Key[1]!=':') || (Key[2]!='\\'))
			Ext.Insert(0, _T("*."));

		SHFILEINFO sfi;
		if (SUCCEEDED(SHGetFileInfo(Ext, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES)))
		{
			// Copy data into hashmap
			wcscpy_s(Data.FileFormatName, 80, sfi.szTypeName);
			Data.SystemIconIndex = sfi.iIcon;
			Data.IconIndex128 = QuarterJumboSystemIcon(sfi.iIcon);

			m_FileFormats.SetAt(Key, Data);

			return;
		}
	}

	// Return generic format name and icon for empty extension
	Data.FileFormatName[0] = L'\0';
	Data.SystemIconIndex = m_GenericSystemIconIndex;
	Data.IconIndex128 = m_GenericIconIndex128;
}

INT CIconFactory::GetSystemIconIndex(LPCSTR lpszFileFormat)
{
	FileFormatData Data;
	LookupFileFormat(lpszFileFormat, Data);

	return Data.SystemIconIndex;
}

CString CIconFactory::GetTypeName(LPCSTR lpszFileFormat)
{
	FileFormatData Data;
	LookupFileFormat(lpszFileFormat, Data);

	return Data.FileFormatName;
}
