
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

class CIconFactory
{
public:
	CIconFactory();

	void DrawJumboFormatIcon(CDC& dc, const CPoint& pt, LPCSTR lpszFileFormat, BOOL Ghosted=FALSE);
	void DrawJumboIcon(CDC& dc, Graphics& g, CPoint pt, LFItemDescriptor* pItemDescriptor, LFSearchResult* pRawFiles, BOOL DrawOverlay=TRUE, INT ThumbnailYOffset=1);
	void DrawSmallIcon(CDC& dc, const CPoint& pt, LFItemDescriptor* pItemDescriptor);
	CString GetTypeName(LPCSTR lpszFileFormat);

	HBITMAP GetRepresentativeThumbnailBitmap(LFSearchResult* pSearchResult);
	HBITMAP GetJumboIconBitmap(LFItemDescriptor* pItemDescriptor, LFSearchResult* pRawFiles);

protected:
	HBITMAP LookupThumbnail(LFItemDescriptor* pItemDescriptor);
	BOOL DrawJumboThumbnail(CDC& dc, Graphics& g, const CPoint& pt, LFItemDescriptor* pItemDescriptor, INT ThumbnailYOffset=1);
	BOOL DrawRepresentativeThumbnail(CDC& dc, Graphics& g, const CPoint& pt, LFItemDescriptor* pItemDescriptor, LFSearchResult* pRawFiles, INT ThumbnailYOffset);
	BOOL DrawJumboMap(Graphics& g, const CPoint& pt, LFItemDescriptor* pItemDescriptor, INT ThumbnailYOffset=1);

	void LookupFileFormat(LPCSTR lpszFileFormat, FileFormatData& Data);
	INT GetSystemIconIndex(LPCSTR lpszFileFormat);

	ThumbnailList<2048> m_Thumbnails;
	ThumbnailList<8192> m_NoThumbnails;
	CMap<LPCSTR, LPCSTR, FileFormatData, FileFormatData> m_FileFormats;
	CImageList m_SystemIcons128;

private:
	static void MakeBitmapSolid(HBITMAP hBitmap, INT x, INT y, INT cx, INT cy);
	static HBITMAP GetThumbnailBitmap(LFItemDescriptor* pItemDescriptor);
	INT QuarterJumboSystemIcon(INT SystemIconIndex);

	INT m_SmallIconSize;
	INT m_GenericSystemIconIndex;
	INT m_GenericIconIndex128;
};
