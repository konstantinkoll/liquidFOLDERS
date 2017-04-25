
// CThumbnailCache.cpp: Implementierung der Klasse CThumbnailCache
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


// CThumbnailCache
//

#define THUMBCUTOFF     2

void CThumbnailCache::MakeBitmapSolid(HBITMAP hBitmap, INT x, INT y, INT cx, INT cy)
{
	ASSERT(hBitmap);

	BITMAP Bitmap;
	GetObject(hBitmap, sizeof(Bitmap), &Bitmap);

	ASSERT(Bitmap.bmBitsPixel==32);
	ASSERT(Bitmap.bmBits);

	// Set alpha channel to 0xFF
	for (INT Row=y; Row<y+cy; Row++)
	{
		LPBYTE pChar = (LPBYTE)Bitmap.bmBits+Bitmap.bmWidthBytes*Row+x*4+3;

		for (INT Column=cx; Column>0; Column--)
		{
			*pChar = 0xFF;

			pChar += 4;
		}
	}
}

HBITMAP CThumbnailCache::Lookup(LFItemDescriptor* pItemDescriptor)
{
	ASSERT(pItemDescriptor);

	ThumbnailData td;

	if (m_Thumbnails.Lookup(pItemDescriptor, td))
		return td.hBitmap;

	if (m_NoThumbnails.Lookup(pItemDescriptor, td))
		return td.hBitmap;

	const BOOL BlackFrame = (pItemDescriptor->CoreAttributes.ContextID==LFContextAudio);
	const BOOL Media = (pItemDescriptor->CoreAttributes.ContextID>=LFContextAudio) && (pItemDescriptor->CoreAttributes.ContextID<=LFContextVideos);
	const BOOL Document = (pItemDescriptor->CoreAttributes.ContextID==LFContextDocuments);
	const INT CutOff = Media || Document ? THUMBCUTOFF : 0;
	const INT ThumbSize = (BlackFrame ? 124 : 118)+2*CutOff;

	strcpy_s(td.StoreID, LFKeySize, pItemDescriptor->StoreID);
	strcpy_s(td.FileID, LFKeySize, pItemDescriptor->CoreAttributes.FileID);
	td.hBitmap = LFGetThumbnail(pItemDescriptor, CSize(ThumbSize, ThumbSize));

	if (td.hBitmap)
	{
		// Quarter bitmap if neccessary
		td.hBitmap = LFQuarter256Bitmap(td.hBitmap);

		// Size allowed?
		BITMAP Bitmap;
		GetObject(td.hBitmap, sizeof(Bitmap), &Bitmap);

		if ((Bitmap.bmWidth>128) || (Bitmap.bmHeight>128))
		{
			td.hBitmap = NULL;
			m_NoThumbnails.AddItem(td);

			return NULL;
		}

		// Convert thumbnail to 128x128x32bpp
		CRect rectSrc(CutOff, CutOff, Bitmap.bmWidth-CutOff, Bitmap.bmHeight-CutOff);

		CRect rectDst;
		rectDst.left = (128-Bitmap.bmWidth)/2+CutOff;
		rectDst.top = (128-Bitmap.bmHeight)/2+CutOff-1;
		rectDst.right = rectDst.left+Bitmap.bmWidth-2*CutOff;
		rectDst.bottom = rectDst.top+Bitmap.bmHeight-2*CutOff;

		const BOOL DrawFrame = Media || Document || ((rectSrc.Width()<=118+2*THUMBCUTOFF) && (rectSrc.Height()<=118+2*THUMBCUTOFF));

		CDC dc;
		dc.CreateCompatibleDC(NULL);

		HBITMAP hBitmap = CreateTransparentBitmap(128, 128);
		HBITMAP hOldBitmap1 = (HBITMAP)dc.SelectObject(hBitmap);

		Graphics g(dc);
		g.SetSmoothingMode(SmoothingModeNone);

		if (DrawFrame)
		{
			const INT FrameSize = BlackFrame ? 124 : 122;

			SolidBrush brush(Color(BlackFrame ? 0xFF000000 : 0xFFFFFFFF));
			g.FillRectangle(&brush, (128-FrameSize)/2, (128-FrameSize)/2-1, FrameSize, FrameSize);

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

		// Copy thumbnail
		HDC hdcMem = CreateCompatibleDC(dc);
		HBITMAP hOldBitmap2 = (HBITMAP)SelectObject(hdcMem, td.hBitmap);

		if (Bitmap.bmBitsPixel!=32)
		{
			BitBlt(dc, rectDst.left, rectDst.top, rectDst.Width(), rectDst.Height(), hdcMem, rectSrc.left, rectSrc.top, SRCCOPY);
			MakeBitmapSolid(hBitmap, rectDst.left, rectDst.top, rectDst.Width(), rectDst.Height());
		}
		else
		{
			AlphaBlend(dc, rectDst.left, rectDst.top, rectDst.Width(), rectDst.Height(), hdcMem, rectSrc.left, rectSrc.top, rectSrc.Width(), rectSrc.Height(), BF);
		}

		// Decorate with frame
		if (DrawFrame)
		{
			g.DrawImage(LFGetApp()->GetCachedResourceImage(IDB_THUMBNAIL), 0, 0);

			if (!BlackFrame && !Document)
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

		DeleteObject(td.hBitmap);
		td.hBitmap = hBitmap;

		m_Thumbnails.AddItem(td);
	}
	else
	{
		m_NoThumbnails.AddItem(td);
	}

	return td.hBitmap;
}

BOOL CThumbnailCache::DrawJumboThumbnail(CDC& dc, const CPoint& pt, LFItemDescriptor* pItemDescriptor, INT YOffset)
{
	ASSERT(pItemDescriptor);
	ASSERT((pItemDescriptor->Type & LFTypeMask)==LFTypeFile);

	HBITMAP hBitmap = Lookup(pItemDescriptor);
	if (!hBitmap)
		return FALSE;

	HDC hdcMem = CreateCompatibleDC(dc);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

	AlphaBlend(dc, pt.x, pt.y+YOffset, 128, 128, hdcMem, 0, 0, 128, 128, BF);

	SelectObject(hdcMem, hOldBitmap);
	DeleteDC(hdcMem);

	return TRUE;
}

BOOL CThumbnailCache::DrawRepresentativeThumbnail(CDC& dc, const CPoint& pt, LFSearchResult* pSearchResult, INT First, INT Last, INT YOffset)
{
	ASSERT(pSearchResult);

	if (First>=0)
	{
		if (Last==-1)
			Last = (INT)pSearchResult->m_ItemCount-1;

		for (INT a=First; a<=Last; a++)
			if (DrawJumboThumbnail(dc, pt, (*pSearchResult)[a], YOffset))
				return TRUE;
	}

	return FALSE;
}

HBITMAP CThumbnailCache::GetJumboThumbnailBitmap(LFItemDescriptor* pItemDescriptor)
{
	ASSERT(pItemDescriptor);

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	HBITMAP hBitmap = CreateTransparentBitmap(128, 128);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	if (DrawJumboThumbnail(dc, CPoint(0, 0), pItemDescriptor, 0))
		return (HBITMAP)dc.SelectObject(hOldBitmap);

	dc.SelectObject(hOldBitmap);
	DeleteObject(hBitmap);

	return NULL;
}

HBITMAP CThumbnailCache::GetRepresentativeThumbnailBitmap(LFSearchResult* pSearchResult, INT First, INT Last)
{
	ASSERT(pSearchResult);

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	HBITMAP hBitmap = CreateTransparentBitmap(128, 128);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	if (DrawRepresentativeThumbnail(dc, CPoint(0, 0), pSearchResult, First, Last, 0))
		return (HBITMAP)dc.SelectObject(hOldBitmap);

	dc.SelectObject(hOldBitmap);
	DeleteObject(hBitmap);

	return NULL;
}
