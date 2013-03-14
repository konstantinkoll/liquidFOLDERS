
// CThumbnailCache.h: Schnittstelle der Klasse CThumbnailCache
//

#pragma once
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"
#include "ThumbnailList.h"


// CThumbnailCache
//

class CThumbnailCache
{
public:
	void LoadFrames();
	void DeleteFrames();
	BOOL DrawJumboThumbnail(CDC& dc, CRect& rect, LFItemDescriptor* i, BOOL* HasSolidBackground=NULL);
	HICON GetThumbnailIcon(LFItemDescriptor* i, CDC* pDC);

protected:
	HBITMAP Lookup(LFItemDescriptor* i);

	ThumbnailList<2048> m_Thumbnails;
	ThumbnailList<8192> m_NoThumbnails;
	HBITMAP hBmpFrame;
	HBITMAP hBmpShadow;
};
