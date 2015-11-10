
// CFormatCache.cpp: Implementierung der Klasse CFormatCache
//

#include "stdafx.h"
#include "liquidFOLDERS.h"


void fmtcpy(CHAR* Dst, CHAR* Src)
{
	ASSERT(Src);
	ASSERT(Dst);

	do *(Dst++) = (CHAR)toupper(*Src); while (*(Src++));
}


// CFormatCache
//

CFormatCache::CFormatCache()
{
	m_Cache.InitHashTable(2111);		// Primzahl mit pz = 4n+3

	m_SystemIcons128.Create(128, 128, ILC_COLOR32, 64, 8);

	SHFILEINFO sfi;
	m_GenericSysIconIndex = SUCCEEDED(SHGetFileInfo(_T("*"), 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_USEFILEATTRIBUTES)) ? sfi.iIcon : 3;
	m_GenericIconIndex128 = ConvertIcon(m_GenericSysIconIndex);

	m_ExtraLargeCX = m_ExtraLargeCY = 48;
	ImageList_GetIconSize(theApp.m_SystemImageListExtraLarge, &m_ExtraLargeCX, &m_ExtraLargeCY);
}

INT CFormatCache::ConvertIcon(INT SysIconIndex)
{
	if (theApp.OSVersion<OS_Vista)
		return -1;

	CDC dc;
	dc.CreateCompatibleDC(NULL);

	// Icon in eine Bitmap konvertieren
	HBITMAP hBitmap = CreateTransparentBitmap(256, -256);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBitmap);

	theApp.m_SystemImageListJumbo.DrawEx(&dc, SysIconIndex, CPoint(0, 0), CSize(256, 256), CLR_NONE, 0xFFFFFF, ILD_TRANSPARENT);

	dc.SelectObject(hOldBitmap);

	// Ziel-Bitmap erstellen
	hBitmap = LFQuarter256Bitmap(hBitmap);

	// Hinzufügen
	INT Result = ImageList_Add(m_SystemIcons128, hBitmap, NULL);

	DeleteObject(hBitmap);

	return Result;
}

void CFormatCache::Lookup(CHAR* FileFormat, FormatData& fd)
{
	if (FileFormat)
		if (FileFormat[0]!='\0')
		{
			CHAR Key[LFExtSize];
			fmtcpy(Key, FileFormat);

			if (m_Cache.Lookup(Key, fd))
				return;

			CString Ext(Key);
			if ((Key[1]!=':') || (Key[2]!='\\'))
				Ext.Insert(0, _T("*."));

			SHFILEINFO sfi;
			if (SUCCEEDED(SHGetFileInfo(Ext, 0, &sfi, sizeof(sfi), SHGFI_SYSICONINDEX | SHGFI_SMALLICON | SHGFI_TYPENAME | SHGFI_USEFILEATTRIBUTES)))
			{
				wcscpy_s(fd.FormatName, 80, sfi.szTypeName);
				fd.SysIconIndex = sfi.iIcon;
				fd.IconIndex128 = ConvertIcon(sfi.iIcon);

				m_Cache.SetAt(Key, fd);
				return;
			}
		}

	fd.FormatName[0] = L'\0';
	fd.SysIconIndex = m_GenericSysIconIndex;
	fd.IconIndex128 = m_GenericIconIndex128;
}

CString CFormatCache::GetTypeName(CHAR* FileFormat)
{
	FormatData fd;
	Lookup(FileFormat, fd);

	return fd.FormatName;
}

INT CFormatCache::GetSysIconIndex(CHAR* FileFormat)
{
	FormatData fd;
	Lookup(FileFormat, fd);

	return fd.SysIconIndex;
}

void CFormatCache::DrawJumboIcon(CDC& dc, const CRect& rect, CHAR* FileFormat, BOOL Ghosted)
{
	FormatData fd;
	Lookup(FileFormat, fd);

	if (theApp.OSVersion<OS_Vista)
	{
		theApp.m_SystemImageListExtraLarge.DrawEx(&dc, fd.SysIconIndex, CPoint(rect.left+(rect.Width()-m_ExtraLargeCX)/2, rect.top+(rect.Height()-m_ExtraLargeCY)/2), CSize(m_ExtraLargeCX, m_ExtraLargeCY), CLR_NONE, 0xFFFFFF, Ghosted ? ILD_BLEND50 : ILD_TRANSPARENT);
	}
	else
	{
		m_SystemIcons128.DrawEx(&dc, fd.IconIndex128, CPoint(rect.left+(rect.Width()-128)/2, rect.top+(rect.Height()-128)/2), CSize(128, 128), CLR_NONE, 0xFFFFFF, Ghosted ? ILD_BLEND50 : ILD_TRANSPARENT);
	}
}
