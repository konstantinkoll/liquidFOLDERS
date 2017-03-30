
// CThumbnailCache.cpp: Implementierung der Klasse CThumbnailCache
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


// CThumbnailCache
//

void CThumbnailCache::MakeBitmapSolid(HBITMAP hBitmap, INT x, INT y, INT cx, INT cy)
{
	BITMAP Bitmap;
	GetObject(hBitmap, sizeof(Bitmap), &Bitmap);

	ASSERT(Bitmap.bmBitsPixel==32);
	ASSERT(Bitmap.bmBits);

	// Alpha-Kanal auf 0xFF setzen
	for (INT Row=y; Row<y+cy; Row++)
	{
		BYTE* Ptr = (BYTE*)Bitmap.bmBits+Bitmap.bmWidthBytes*Row+x*4+3;

		for (INT Column=cx; Column>0; Column--)
		{
			*Ptr = 0xFF;

			Ptr += 4;
		}
	}
}

HBITMAP CThumbnailCache::Lookup(LFItemDescriptor* pItemDescriptor)
{
	ThumbnailData td;

	if (m_Thumbnails.Lookup(pItemDescriptor, td))
		return td.hBitmap;

	if (m_NoThumbnails.Lookup(pItemDescriptor, td))
		return td.hBitmap;

	strcpy_s(td.StoreID, LFKeySize, pItemDescriptor->StoreID);
	strcpy_s(td.FileID, LFKeySize, pItemDescriptor->CoreAttributes.FileID);
	td.hBitmap = LFGetThumbnail(pItemDescriptor, CSize(118, 118));

	if (td.hBitmap)
	{
		// Ggf. Bitmap vierteln
		td.hBitmap = LFQuarter256Bitmap(td.hBitmap);

		// Zulässige Größe?
		BITMAP Bitmap;
		GetObject(td.hBitmap, sizeof(Bitmap), &Bitmap);

		if ((Bitmap.bmWidth>128) || (Bitmap.bmHeight>128))
		{
			td.hBitmap = NULL;
			m_NoThumbnails.AddItem(td);

			return NULL;
		}

		// Thumbnail dekorieren
		CDC dc;
		dc.CreateCompatibleDC(NULL);

		HBITMAP hBitmap = CreateTransparentBitmap(128, 128);
		HBITMAP hOldBitmap1 = (HBITMAP)dc.SelectObject(hBitmap);

		CRect rect(0, 0, 128, 128);

		BOOL DrawFrame = ((pItemDescriptor->CoreAttributes.ContextID>=LFContextPictures) && (pItemDescriptor->CoreAttributes.ContextID<=LFContextVideos)) || ((Bitmap.bmWidth<=118) && (Bitmap.bmHeight<=118));
		BOOL DrawShadow = !DrawFrame && (Bitmap.bmWidth>=4) && (Bitmap.bmWidth<=118) && (Bitmap.bmHeight>=4) && (Bitmap.bmHeight<=118);

		Graphics g(dc);

		// Frame
		if (DrawFrame)
			g.DrawImage(LFGetApp()->GetCachedResourceImage(IDB_THUMBNAIL_FRAME), 0, 0);

		rect.left += max((128-Bitmap.bmWidth)/2-1, 0);
		rect.right = rect.left+Bitmap.bmWidth;
		rect.top += max((128-Bitmap.bmHeight)/2-1, 0);
		rect.bottom = rect.top+Bitmap.bmHeight;

		// Thumbnail
		HDC hdcMem = CreateCompatibleDC(dc);
		HBITMAP hOldBitmap2 = (HBITMAP)SelectObject(hdcMem, td.hBitmap);

		if (DrawFrame || DrawShadow || (Bitmap.bmBitsPixel!=32))
		{
			BitBlt(dc, rect.left, rect.top, Bitmap.bmWidth, Bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
			MakeBitmapSolid(hBitmap, rect.left, rect.top, Bitmap.bmWidth, Bitmap.bmHeight);
		}
		else
		{
			AlphaBlend(dc, rect.left, rect.top, Bitmap.bmWidth, Bitmap.bmHeight, hdcMem, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, BF);
		}

		SelectObject(hdcMem, hOldBitmap2);
		DeleteDC(hdcMem);

		// Shadow
		if (DrawShadow)
		{
			Gdiplus::Bitmap* pShadow = LFGetApp()->GetCachedResourceImage(IDB_THUMBNAIL_SHADOW);

			g.DrawImage(pShadow, rect.left-2, rect.top-2, 0, 0, 2+Bitmap.bmWidth, 2, UnitPixel);
			g.DrawImage(pShadow, rect.left-2, rect.top+Bitmap.bmHeight, 0, 124, 2+Bitmap.bmWidth, 4, UnitPixel);
			g.DrawImage(pShadow, rect.left-2, rect.top, 0, 4, 2, Bitmap.bmHeight, UnitPixel);
			g.DrawImage(pShadow, rect.left+Bitmap.bmWidth, rect.top-2, 124, 0, 4, 2+Bitmap.bmHeight, UnitPixel);
			g.DrawImage(pShadow, rect.left+Bitmap.bmWidth, rect.top+Bitmap.bmHeight, 124, 124, 4, 4, UnitPixel);
		}

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

BOOL CThumbnailCache::DrawJumboThumbnail(CDC& dc, const CRect& rect, LFItemDescriptor* pItemDescriptor)
{
	ASSERT((pItemDescriptor->Type & LFTypeMask)==LFTypeFile);

	HBITMAP hBitmap = Lookup(pItemDescriptor);
	if (!hBitmap)
		return FALSE;

	HDC hdcMem = CreateCompatibleDC(dc);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

	AlphaBlend(dc, rect.left+(rect.Width()-128)/2, rect.top+(rect.Height()-128)/2, 128, 128, hdcMem, 0, 0, 128, 128, BF);

	SelectObject(hdcMem, hOldBitmap);
	DeleteDC(hdcMem);

	return TRUE;
}

HBITMAP CThumbnailCache::GetThumbnailBitmap(LFItemDescriptor* pItemDescriptor, CDC* pDC)
{
	CDC dc;
	dc.CreateCompatibleDC(pDC);

	HBITMAP hBitmap = CreateTransparentBitmap(128, 128);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	CRect rect(0, 0, 128, 128);
	if (DrawJumboThumbnail(dc, rect, pItemDescriptor))
		return (HBITMAP)dc.SelectObject(hOldBitmap);

	dc.SelectObject(hOldBitmap);
	DeleteObject(hBitmap);

	return NULL;
}

HICON CThumbnailCache::GetThumbnailIcon(LFItemDescriptor* pItemDescriptor, CDC* pDC)
{
	HICON hIcon = NULL;

	HBITMAP hBitmap = GetThumbnailBitmap(pItemDescriptor, pDC);
	if (hBitmap)
	{
		ICONINFO ii;
		ZeroMemory(&ii, sizeof(ii));
		ii.fIcon = TRUE;
		ii.hbmColor = hBitmap;
		ii.hbmMask = hBitmap;

		hIcon = CreateIconIndirect(&ii);
		DeleteObject(hBitmap);
	}

	return hIcon;
}
