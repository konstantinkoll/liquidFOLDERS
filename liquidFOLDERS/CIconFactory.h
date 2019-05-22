
// CIconFactory.h: Schnittstelle der Klasse CIconFactory
//

#pragma once
#include "LF.h"
#include "LFCommDlg.h"
#include "ThumbnailList.h"


// CIconFactory
//

struct FileFormatData
{
	WCHAR FileFormatName[80];
	INT SystemIconIndex;
	INT IconIndex128;
};

class CIconFactory sealed
{
public:
	CIconFactory();

	void DrawJumboFormatIcon(CDC& dc, const CPoint& pt, LPCSTR lpszFileFormat, BOOL Ghosted=FALSE);
	void DrawJumboIcon(CDC& dc, Graphics& g, CPoint pt, LFItemDescriptor* pItemDescriptor, LFSearchResult* pRawFiles, BOOL DrawOverlays=TRUE, INT ThumbnailYOffset=1);
	void DrawSmallIcon(CDC& dc, const CPoint& pt, const LFItemDescriptor* pItemDescriptor);

	HBITMAP GetHeaderBitmap(LFSearchResult* pSearchResult, LFFilter* pFilter, INT ViewID, CPoint& BitmapOffset);
	HBITMAP GetTooltipBitmap(LFItemDescriptor* pItemDescriptor, LFSearchResult* pRawFiles);

	CString GetTypeName(LPCSTR lpszFileFormat);

	static CIcons m_ApplicationIcons;

protected:
	BOOL DrawJumboThumbnail(CDC& dc, Graphics& g, const CPoint& pt, LFItemDescriptor* pItemDescriptor, BOOL& DrawSash, INT ThumbnailYOffset=1);
	BOOL DrawRepresentativeThumbnail(CDC& dc, Graphics& g, const CPoint& pt, LFItemDescriptor* pItemDescriptor, LFSearchResult* pRawFiles, INT ThumbnailYOffset);
	BOOL DrawJumboMap(Graphics& g, const CPoint& pt, const LFGeoCoordinates& GeoCoordinates, INT ThumbnailYOffset=1);
	BOOL DrawJumboMap(Graphics& g, const CPoint& pt, const LFItemDescriptor* pItemDescriptor, INT ThumbnailYOffset=1);
	void DrawRollCount(CDC&dc, const CPoint& pt, UINT IconID, UINT Count);

	BOOL LookupThumbnail(LFItemDescriptor* pItemDescriptor, ThumbnailData& Thumbnail);

	void LookupFileFormat(LPCSTR lpszFileFormat, FileFormatData& Data);
	INT GetSystemIconIndex(LPCSTR lpszFileFormat);

	ThumbnailList<2048> m_Thumbnails;
	ThumbnailList<8192> m_NoThumbnails;
	CMap<LPCSTR, LPCSTR, FileFormatData, FileFormatData> m_FileFormats;
	CImageList m_SystemIcons128;
	LFFont m_RollFont;

private:
	static void MakeBitmapSolid(HBITMAP hBitmap, INT x, INT y, INT cx, INT cy);
	static BOOL GetThumbnailBitmap(LFItemDescriptor* pItemDescriptor, ThumbnailData& Thumbnail);
	INT QuarterJumboSystemIcon(INT SystemIconIndex);

	INT m_SmallIconSize;
	INT m_GenericSystemIconIndex;
	INT m_GenericIconIndex128;
};
