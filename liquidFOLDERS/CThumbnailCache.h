
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
	~CThumbnailCache();

	BOOL DrawJumboThumbnail(CDC& dc, CRect& rect, LFItemDescriptor* pItemDescriptor);
	HBITMAP GetThumbnailBitmap(LFItemDescriptor* pItemDescriptor, CDC* pDC);
	HICON GetThumbnailIcon(LFItemDescriptor* pItemDescriptor, CDC* pDC);

protected:
	HBITMAP Lookup(LFItemDescriptor* pItemDescriptor);

	ThumbnailList<2048> m_Thumbnails;
	ThumbnailList<8192> m_NoThumbnails;
	HBITMAP hBitmapFrame;
	HBITMAP hBitmapShadow;
};
