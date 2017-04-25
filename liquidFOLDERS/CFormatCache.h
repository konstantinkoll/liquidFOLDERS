
// CFormatCache.h: Schnittstelle der Klasse CFormatCache
//

#pragma once
#include "LF.h"


// CFormatCache
//

struct FormatData
{
	WCHAR FormatName[80];
	INT SysIconIndex;
	INT IconIndex128;
};

class CFormatCache
{
public:
	CFormatCache();

	void Lookup(LPCSTR FileFormat, FormatData& fd);
	void Remove(LPCSTR FileFormat);
	void Remove(LPCWSTR Path);
	CString GetTypeName(LPCSTR FileFormat);
	INT GetSysIconIndex(LPCSTR FileFormat);
	void DrawJumboIcon(CDC& dc, const CPoint& pt, LPCSTR FileFormat, BOOL Ghosted=FALSE);
	void DrawJumboIcon(CDC& dc, const CRect& rect, LPCSTR FileFormat, BOOL Ghosted=FALSE);

protected:
	INT ConvertIcon(INT SysIconIndex);

	CMap<LPCSTR, LPCSTR, FormatData, FormatData> m_Cache;
	CImageList m_SystemIcons128;
	INT m_GenericSysIconIndex;
	INT m_GenericIconIndex128;
};

inline void CFormatCache::DrawJumboIcon(CDC& dc, const CRect& rect, LPCSTR FileFormat, BOOL Ghosted)
{
	DrawJumboIcon(dc, CPoint((rect.right+rect.left-128)/2, (rect.top+rect.bottom-128)/2), FileFormat, Ghosted);
}
