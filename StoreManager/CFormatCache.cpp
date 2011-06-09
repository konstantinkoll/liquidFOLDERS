
// CFormatCache.cpp: Implementierung der Klasse CFormatCache
//

#pragma once
#include "stdafx.h"
#include "CFormatCache.h"
#include "StoreManager.h"


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
	BITMAPINFO dib = { 0 };
	dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	dib.bmiHeader.biWidth = 256;
	dib.bmiHeader.biHeight = -256;
	dib.bmiHeader.biPlanes = 1;
	dib.bmiHeader.biBitCount = 32;
	dib.bmiHeader.biCompression = BI_RGB;

	HBITMAP hBmpSrc = CreateDIBSection(dc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);
	HBITMAP hOldBitmap = (HBITMAP)dc.SelectObject(hBmpSrc);

	theApp.m_SystemImageListJumbo.DrawEx(&dc, SysIconIndex, CPoint(0, 0), CSize(256, 256), CLR_NONE, 0xFFFFFF, ILD_TRANSPARENT);

	dc.SelectObject(hOldBitmap);

	// Ziel-Bitmap erstellen
	dib.bmiHeader.biWidth = 128;
	dib.bmiHeader.biHeight = -128;

	HBITMAP hBmpDst = CreateDIBSection(dc, &dib, DIB_RGB_COLORS, NULL, NULL, 0);

	// Skalieren
	BITMAP bmpSrc;
	BITMAP bmpDst;
	GetObject(hBmpSrc, sizeof(BITMAP), &bmpSrc);
	GetObject(hBmpDst, sizeof(BITMAP), &bmpDst);
	BYTE* pBitsSrc = ((BYTE*)bmpSrc.bmBits);
	BYTE* pBitsDst = ((BYTE*)bmpDst.bmBits);

	for (UINT a=0; a<128; a++)
	{
		for (UINT b=0; b<128; b++)
		{
			*(pBitsDst+0) = (*(pBitsSrc+0)+*(pBitsSrc+4)+*(pBitsSrc+256*4)+*(pBitsSrc+256*4+4))>>2;
			*(pBitsDst+1) = (*(pBitsSrc+1)+*(pBitsSrc+5)+*(pBitsSrc+256*4+1)+*(pBitsSrc+256*4+5))>>2;
			*(pBitsDst+2) = (*(pBitsSrc+2)+*(pBitsSrc+6)+*(pBitsSrc+256*4+2)+*(pBitsSrc+256*4+6))>>2;
			*(pBitsDst+3) = (*(pBitsSrc+3)+*(pBitsSrc+7)+*(pBitsSrc+256*4+3)+*(pBitsSrc+256*4+7))>>2;
			pBitsSrc += 8;
			pBitsDst += 4;
		}
		pBitsSrc += 256*4;
	}

	// Hinzufügen
	INT res = ImageList_Add(m_SystemIcons128, hBmpDst, NULL);

	DeleteObject(hBmpSrc);
	DeleteObject(hBmpDst);

	return res;
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

void CFormatCache::DrawJumboIcon(CDC& dc, CRect& rect, CHAR* FileFormat, BOOL Ghosted)
{
	FormatData fd;
	Lookup(FileFormat, fd);

	if (theApp.OSVersion<OS_Vista)
	{
		rect.OffsetRect((rect.Width()-m_ExtraLargeCX)/2, (rect.Height()-m_ExtraLargeCY)/2);
		theApp.m_SystemImageListExtraLarge.DrawEx(&dc, fd.SysIconIndex, rect.TopLeft(), CSize(m_ExtraLargeCX, m_ExtraLargeCY), CLR_NONE, 0xFFFFFF, Ghosted ? ILD_BLEND50 : ILD_TRANSPARENT);
	}
	else
	{
		rect.OffsetRect((rect.Width()-128)/2, (rect.Height()-128)/2);
		m_SystemIcons128.DrawEx(&dc, fd.IconIndex128, rect.TopLeft(), CSize(128, 128), CLR_NONE, 0xFFFFFF, Ghosted ? ILD_BLEND50 : ILD_TRANSPARENT);
	}
}
