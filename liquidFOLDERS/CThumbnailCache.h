
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
	BOOL DrawJumboThumbnail(CDC& dc, const CRect& rect, LFItemDescriptor* pItemDescriptor);
	HBITMAP GetThumbnailBitmap(LFItemDescriptor* pItemDescriptor, CDC* pDC);
	HICON GetThumbnailIcon(LFItemDescriptor* pItemDescriptor, CDC* pDC);

protected:
	void MakeBitmapSolid(HBITMAP hBitmap, INT x, INT y, INT cx, INT cy);
	HBITMAP Lookup(LFItemDescriptor* pItemDescriptor);

	ThumbnailList<2048> m_Thumbnails;
	ThumbnailList<8192> m_NoThumbnails;
};
