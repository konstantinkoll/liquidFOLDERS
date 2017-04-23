
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
	void DrawJumboIcon(CDC& dc, const CRect& rect, LPCSTR FileFormat, BOOL Ghosted=FALSE);

protected:
	INT ConvertIcon(INT SysIconIndex);

	CMap<LPCSTR, LPCSTR, FormatData, FormatData> m_Cache;
	CImageList m_SystemIcons128;
	INT m_GenericSysIconIndex;
	INT m_GenericIconIndex128;
	INT m_ExtraLargeCX;
	INT m_ExtraLargeCY;
};
