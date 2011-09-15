
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
	ZeroMemory(&m_Thumbnails, sizeof(m_Thumbnails));
	ZeroMemory(&m_NoThumbnails, sizeof(m_NoThumbnails));
	m_NextPtr[0] = m_NextPtr[1] = 0;
}

CThumbnailCache::~CThumbnailCache()
{
	for (UINT a=0; a<ThumbnailCount; a++)
		FreeItem(a);
}

__forceinline void CThumbnailCache::FreeItem(UINT idx)
{
	if (m_Thumbnails[idx].hBmp)
		DeleteObject(m_Thumbnails[idx].hBmp);
}

HBITMAP CThumbnailCache::Lookup(LFItemDescriptor* i)
{
	// Assumption: we know the item has no thumbnail
	INT Ptr = m_NextPtr[1];

	for (UINT a=0; a<NoThumbnailCount; a++)
	{
		if (--Ptr<0)
			Ptr = NoThumbnailCount-1;

		if ((strcmp(m_NoThumbnails[Ptr].StoreID, i->StoreID)==0) && (strcmp(m_NoThumbnails[Ptr].FileID, i->CoreAttributes.FileID)==0))
		{
			if (Ptr!=m_NextPtr[1])
				std::swap(m_NoThumbnails[m_NextPtr[1]], m_NoThumbnails[Ptr]);

			return NULL;
		}
	}

	// See if we have a thumbnail...
	Ptr = m_NextPtr[0];

	for (UINT a=0; a<ThumbnailCount; a++)
	{
		if (--Ptr<0)
			Ptr = ThumbnailCount-1;

		if ((strcmp(m_Thumbnails[Ptr].StoreID, i->StoreID)==0) && (strcmp(m_Thumbnails[Ptr].FileID, i->CoreAttributes.FileID)==0))
		{
			HBITMAP hBmp = m_Thumbnails[Ptr].hBmp;

			if (Ptr!=m_NextPtr[0])
				std::swap(m_Thumbnails[m_NextPtr[0]], m_Thumbnails[Ptr]);

			if (++m_NextPtr[0]>=ThumbnailCount)
				m_NextPtr[0] = 0;

			return hBmp;
		}
	}

	// We don't know the file yet. Lets request a thumbnail!
	HBITMAP hBmp = LFGetThumbnail(i);

	if (hBmp)
	{
		FreeItem(m_NextPtr[0]);

		strcpy_s(m_Thumbnails[m_NextPtr[0]].StoreID, LFKeySize, i->StoreID);
		strcpy_s(m_Thumbnails[m_NextPtr[0]].FileID, LFKeySize, i->CoreAttributes.FileID);
		m_Thumbnails[m_NextPtr[0]].hBmp = hBmp;
		if (++m_NextPtr[0]>=ThumbnailCount)
			m_NextPtr[0] = 0;
	}
	else
	{
		strcpy_s(m_NoThumbnails[m_NextPtr[1]].StoreID, LFKeySize, i->StoreID);
		strcpy_s(m_NoThumbnails[m_NextPtr[1]].FileID, LFKeySize, i->CoreAttributes.FileID);
		if (++m_NextPtr[1]>=NoThumbnailCount)
			m_NextPtr[1] = 0;
	}

	return hBmp;
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
