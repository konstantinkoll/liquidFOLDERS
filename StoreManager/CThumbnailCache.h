
// CThumbnailCache.h: Schnittstelle der Klasse CThumbnailCache
//

#pragma once
#include "liquidFOLDERS.h"
#include "ThumbnailList.h"


// CThumbnailCache
//

class CThumbnailCache
{
public:
	CThumbnailCache();

	BOOL DrawJumboThumbnail(CDC& dc, CRect& rect, LFItemDescriptor* i);

protected:
	HBITMAP Lookup(LFItemDescriptor* i);

	ThumbnailList<2048> m_Thumbnails;
	ThumbnailList<8192> m_NoThumbnails;
};
