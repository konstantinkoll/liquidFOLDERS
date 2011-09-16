
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

	BOOL DrawFrame = ((i->CoreAttributes.DomainID>=LFDomainPhotos) && (i->CoreAttributes.DomainID<=LFDomainVideos)) || ((bm.bmWidth==118) && (bm.bmHeight==118));
	Graphics g(dc);
	g.SetCompositingMode(CompositingModeSourceOver);

	rect.OffsetRect((rect.Width()-128)/2, (rect.Height()-128)/2);

	if (DrawFrame)
		g.DrawImage(m_pFrame->m_pBitmap, rect.left, rect.top, 128, 128);

	rect.OffsetRect((128-bm.bmWidth)/2-1, (128-bm.bmHeight)/2-2);

	HDC hdcMem = CreateCompatibleDC(dc);
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hBmp);

	BitBlt(dc, rect.left, rect.top+1, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

	if ((!DrawFrame) && (bm.bmWidth>=4) && (bm.bmHeight>=4))
	{
		g.DrawImage(m_pShadow->m_pBitmap, rect.left-2, rect.top-2, 0, 0, 2+bm.bmWidth, 2, UnitPixel);
		g.DrawImage(m_pShadow->m_pBitmap, rect.left-2, rect.top+bm.bmHeight, 0, 124, 2+bm.bmWidth, 4, UnitPixel);
		g.DrawImage(m_pShadow->m_pBitmap, rect.left-2, rect.top, 0, 2, 2, bm.bmHeight, UnitPixel);
		g.DrawImage(m_pShadow->m_pBitmap, rect.left+bm.bmWidth, rect.top-2, 124, 0, 4, 2+bm.bmHeight, UnitPixel);
		g.DrawImage(m_pShadow->m_pBitmap, rect.left+bm.bmWidth, rect.top+bm.bmHeight, 124, 124, 4, 4, UnitPixel);
	}

	SelectObject(hdcMem, hbmOld);
	DeleteDC(hdcMem);

	return TRUE;
}
