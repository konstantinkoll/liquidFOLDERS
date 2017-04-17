
// CThumbnailCache.h: Schnittstelle der Klasse CThumbnailCache
//

#pragma once
#include "LF.h"
#include "LFCommDlg.h"
#include "ThumbnailList.h"


// CThumbnailCache
//

class CThumbnailCache
{
public:
	BOOL DrawJumboThumbnail(CDC& dc, const CRect& rect, LFItemDescriptor* pItemDescriptor, INT YOffset=1);
	BOOL DrawRepresentativeThumbnail(CDC& dc, const CRect& rect, LFSearchResult* pSearchResult, INT First=0, INT Last=(UINT)-1, INT YOffset=1);
	HBITMAP GetJumboThumbnailBitmap(LFItemDescriptor* pItemDescriptor);
	HBITMAP GetRepresentativeThumbnailBitmap(LFSearchResult* pSearchResult, INT First=0, INT Last=-1);

protected:
	void MakeBitmapSolid(HBITMAP hBitmap, INT x, INT y, INT cx, INT cy);
	HBITMAP Lookup(LFItemDescriptor* pItemDescriptor);

	ThumbnailList<2048> m_Thumbnails;
	ThumbnailList<8192> m_NoThumbnails;
};
