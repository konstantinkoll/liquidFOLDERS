
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

	void Lookup(CHAR* FileFormat, FormatData& fd);
	void Remove(CHAR* FileFormat);
	void Remove(WCHAR* Path);
	CString GetTypeName(CHAR* FileFormat);
	INT GetSysIconIndex(CHAR* FileFormat);
	void DrawJumboIcon(CDC& dc, CRect& rect, CHAR* FileFormat, BOOL Ghosted=FALSE);

protected:
	INT ConvertIcon(INT SysIconIndex);

	CMap<LPCSTR, LPCSTR, FormatData, FormatData> m_Cache;
	CImageList m_SystemIcons128;
	INT m_GenericSysIconIndex;
	INT m_GenericIconIndex128;
	INT m_ExtraLargeCX;
	INT m_ExtraLargeCY;
};
