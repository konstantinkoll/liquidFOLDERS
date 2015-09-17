
// CThumbnailCache.cpp: Implementierung der Klasse CThumbnailCache
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


// CThumbnailCache
//

CThumbnailCache::~CThumbnailCache()
{
	DeleteObject(hBitmapFrame);
	DeleteObject(hBitmapShadow);
}

HBITMAP CThumbnailCache::Lookup(LFItemDescriptor* i)
{
	ThumbnailData td;

	if (m_Thumbnails.Lookup(i, td))
		return td.hBitmap;
	if (m_NoThumbnails.Lookup(i, td))
		return td.hBitmap;

	strcpy_s(td.StoreID, LFKeySize, i->StoreID);
	strcpy_s(td.FileID, LFKeySize, i->CoreAttributes.FileID);
	td.hBitmap = LFGetThumbnail(i, CSize(118, 118));

	if (td.hBitmap)
	{
		// Thumbnail dekorieren
		CDC dc;
		dc.CreateCompatibleDC(NULL);

		HBITMAP hBitmap = CreateTransparentBitmap(256, 256);
		HBITMAP hOldBitmap1 = (HBITMAP)dc.SelectObject(hBitmap);

		CRect rect(0, 0, 128, 128);

		BITMAP Bitmap;
		GetObject(td.hBitmap, sizeof(Bitmap), &Bitmap);

		BOOL DrawFrame = ((i->CoreAttributes.ContextID>=LFContextPictures) && (i->CoreAttributes.ContextID<=LFContextVideos)) || ((Bitmap.bmWidth==118) && (Bitmap.bmHeight==118));
		BOOL DrawShadow = !DrawFrame && (Bitmap.bmWidth>=4) && (Bitmap.bmWidth<=118) && (Bitmap.bmHeight>=4) && (Bitmap.bmHeight<=118);

		HDC hdcMem = CreateCompatibleDC(dc);
		HBITMAP hOldBitmap2 = (HBITMAP)SelectObject(hdcMem, td.hBitmap);

		// Frame
		if (DrawFrame)
		{
			if (!hBitmapFrame)
			{
				CGdiPlusBitmapResource Frame(IDB_THUMBNAIL_FRAME, _T("PNG"));
				Frame.m_pBitmap->GetHBITMAP(Color(0, 0, 0, 0), &hBitmapFrame);
			}

			HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmapFrame);

			AlphaBlend(dc, rect.left, rect.top, 128, 128, hdcMem, 0, 0, 128, 128, BF);

			SelectObject(hdcMem, hOldBitmap);
		}

		rect.left += max((128-Bitmap.bmWidth)/2-1, 0);
		rect.right = rect.left+Bitmap.bmWidth;
		rect.top += max((128-Bitmap.bmHeight)/2-1, 0);
		rect.bottom = rect.top+Bitmap.bmHeight;

		// Thumbnail
		if (DrawFrame || DrawShadow || (Bitmap.bmBitsPixel!=32))
		{
			BitBlt(dc, rect.left, rect.top, Bitmap.bmWidth, Bitmap.bmHeight, hdcMem, 0, 0, SRCCOPY);
		}
		else
		{
			AlphaBlend(dc, rect.left, rect.top, Bitmap.bmWidth, Bitmap.bmHeight, hdcMem, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, BF);
		}

		// Shadow
		if (DrawShadow)
		{
			if (!hBitmapShadow)
			{
				CGdiPlusBitmapResource Shadow(IDB_THUMBNAIL_SHADOW, _T("PNG"));
				Shadow.m_pBitmap->GetHBITMAP(Color(0, 0, 0, 0), &hBitmapShadow);
			}

			HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmapShadow);

			AlphaBlend(dc, rect.left-2, rect.top-2, 2+Bitmap.bmWidth, 2, hdcMem, 0, 0, 2+Bitmap.bmWidth, 2, BF);
			AlphaBlend(dc, rect.left-2, rect.top+Bitmap.bmHeight, 2+Bitmap.bmWidth, 4, hdcMem, 0, 124, 2+Bitmap.bmWidth, 4, BF);
			AlphaBlend(dc, rect.left-2, rect.top, 2, Bitmap.bmHeight, hdcMem, 0, 4, 2, Bitmap.bmHeight, BF);
			AlphaBlend(dc, rect.left+Bitmap.bmWidth, rect.top-2, 4, 2+Bitmap.bmHeight, hdcMem, 124, 0, 4, 2+Bitmap.bmHeight, BF);
			AlphaBlend(dc, rect.left+Bitmap.bmWidth, rect.top+Bitmap.bmHeight, 4, 4, hdcMem, 124, 124, 4, 4, BF);

			SelectObject(hdcMem, hOldBitmap);
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

BOOL CThumbnailCache::DrawJumboThumbnail(CDC& dc, CRect& rect, LFItemDescriptor* i)
{
	ASSERT((i->Type & LFTypeMask)==LFTypeFile);

	HBITMAP hBitmap = Lookup(i);
	if (!hBitmap)
		return FALSE;

	rect.OffsetRect((rect.Width()-128)/2, (rect.Height()-128)/2);

	HDC hdcMem = CreateCompatibleDC(dc);
	HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);

	AlphaBlend(dc, rect.left, rect.top, 128, 128, hdcMem, 0, 0, 128, 128, BF);

	SelectObject(hdcMem, hOldBitmap);
	DeleteDC(hdcMem);

	return TRUE;
}

HBITMAP CThumbnailCache::GetThumbnailBitmap(LFItemDescriptor* i, CDC* pDC)
{
	CDC dc;
	dc.CreateCompatibleDC(pDC);

	HBITMAP hBitmap = CreateTransparentBitmap(128, 128);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	CRect rect(0, 0, 128, 128);
	if (DrawJumboThumbnail(dc, rect, i))
		return (HBITMAP)dc.SelectObject(hOldBitmap);

	dc.SelectObject(hOldBitmap);
	DeleteObject(hBitmap);

	return NULL;
}

HICON CThumbnailCache::GetThumbnailIcon(LFItemDescriptor* i, CDC* pDC)
{
	HICON hIcon = NULL;

	HBITMAP hBitmap = GetThumbnailBitmap(i, pDC);
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
