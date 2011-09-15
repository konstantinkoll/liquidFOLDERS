
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
	rect.OffsetRect((rect.Width()-bm.bmWidth)/2, (rect.Height()-bm.bmHeight)/2);

	HDC hdcMem = CreateCompatibleDC(dc);
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hBmp);

	BitBlt(dc, rect.left, rect.top+1, 128, 128, hdcMem, 0, 0, SRCCOPY);

	SelectObject(hdcMem, hbmOld);
	DeleteDC(hdcMem);

	return TRUE;
}
