
// CThumbnailCache.cpp: Implementierung der Klasse CThumbnailCache
//

#pragma once
#include "stdafx.h"
#include "CThumbnailCache.h"
#include "StoreManager.h"


// CThumbnailCache
//

CThumbnailCache::CThumbnailCache()
{
}

void CThumbnailCache::LoadFrames()
{
	if (!m_pFrame)
		m_pFrame = new CGdiPlusBitmapResource(IDB_THUMBNAIL_FRAME, _T("PNG"));
	if (!m_pShadow)
		m_pShadow = new CGdiPlusBitmapResource(IDB_THUMBNAIL_SHADOW, _T("PNG"));
}

HBITMAP CThumbnailCache::Lookup(LFItemDescriptor* i)
{
	ThumbnailData td;

	if (m_NoThumbnails.Lookup(i, td))
		return td.hBmp;
	if (m_Thumbnails.Lookup(i, td))
		return td.hBmp;

	strcpy_s(td.StoreID, LFKeySize, i->StoreID);
	strcpy_s(td.FileID, LFKeySize, i->CoreAttributes.FileID);
	td.hBmp = LFGetThumbnail(i);

	if (td.hBmp)
	{
		m_Thumbnails.AddItem(td);
	}
	else
	{
		m_NoThumbnails.AddItem(td);
	}

	return td.hBmp;
}

BOOL CThumbnailCache::DrawJumboThumbnail(CDC& dc, CRect& rect, LFItemDescriptor* i)
{
	ASSERT((i->Type & LFTypeMask)==LFTypeFile);

	HBITMAP hBmp = Lookup(i);
	if (!hBmp)
		return FALSE;

	BITMAP bm;
	GetObject(hBmp, sizeof(bm), &bm);

	static const BLENDFUNCTION BF = { AC_SRC_OVER, 0, 0xFF, AC_SRC_ALPHA };

	BOOL DrawFrame = ((i->CoreAttributes.DomainID>=LFDomainPhotos) && (i->CoreAttributes.DomainID<=LFDomainVideos)) || ((bm.bmWidth==118) && (bm.bmHeight==118));
	BOOL DrawShadow = !DrawFrame;
	if (theApp.m_ThemeLibLoaded)
		if (!theApp.zIsAppThemed())
			DrawFrame = DrawShadow = FALSE;

	rect.OffsetRect((rect.Width()-128)/2, (rect.Height()-128)/2);

	if (DrawFrame)
	{
		HDC hdcMem = CreateCompatibleDC(dc);
		HBITMAP hBmp;
		m_pFrame->m_pBitmap->GetHBITMAP(Color(0, 0, 0, 0), &hBmp);
		HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hBmp);

		AlphaBlend(dc, rect.left, rect.top, 128, 128, hdcMem, 0, 0, 128, 128, BF);

		SelectObject(hdcMem, hbmOld);
		DeleteDC(hdcMem);
	}

	rect.OffsetRect((128-bm.bmWidth)/2-1, (128-bm.bmHeight)/2-1);

	HDC hdcMem = CreateCompatibleDC(dc);
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hBmp);

	BitBlt(dc, rect.left, rect.top, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

	SelectObject(hdcMem, hbmOld);
	DeleteDC(hdcMem);

	if ((DrawShadow) && (bm.bmWidth>=4) && (bm.bmHeight>=4))
	{
		HDC hdcMem = CreateCompatibleDC(dc);
		HBITMAP hBmp;
		m_pShadow->m_pBitmap->GetHBITMAP(Color(0, 0, 0, 0), &hBmp);
		HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hBmp);

		AlphaBlend(dc, rect.left-2, rect.top-2, 2+bm.bmWidth, 2, hdcMem, 0, 0, 2+bm.bmWidth, 2, BF);
		AlphaBlend(dc, rect.left-2, rect.top+bm.bmHeight, 2+bm.bmWidth, 4, hdcMem, 0, 124, 2+bm.bmWidth, 4, BF);
		AlphaBlend(dc, rect.left-2, rect.top, 2, bm.bmHeight, hdcMem, 0, 4, 2, bm.bmHeight, BF);
		AlphaBlend(dc, rect.left+bm.bmWidth, rect.top-2, 4, 2+bm.bmHeight, hdcMem, 124, 0, 4, 2+bm.bmHeight, BF);
		AlphaBlend(dc, rect.left+bm.bmWidth, rect.top+bm.bmHeight, 4, 4, hdcMem, 124, 124, 4, 4, BF);

		SelectObject(hdcMem, hbmOld);
		DeleteDC(hdcMem);
	}

	return TRUE;
}
