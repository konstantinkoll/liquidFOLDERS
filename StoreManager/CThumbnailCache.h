
// CThumbnailCache.h: Schnittstelle der Klasse CThumbnailCache
//

#pragma once
#include "liquidFOLDERS.h"


// CThumbnailCache
//

#define ThumbnailCount       2048
#define NoThumbnailCount     32768

struct ThumbnailData
{
	CHAR StoreID[LFKeySize];
	CHAR FileID[LFKeySize];
	HBITMAP hBmp;
};

struct NoThumbnailData
{
	CHAR StoreID[LFKeySize];
	CHAR FileID[LFKeySize];
};

class CThumbnailCache
{
public:
	CThumbnailCache();
	~CThumbnailCache();

	BOOL DrawJumboThumbnail(CDC& dc, CRect& rect, LFItemDescriptor* i);

protected:
	void FreeItem(UINT idx);
	HBITMAP Lookup(LFItemDescriptor* i);

	ThumbnailData m_Thumbnails[ThumbnailCount];
	NoThumbnailData m_NoThumbnails[NoThumbnailCount];
	INT m_NextPtr[2];
};
