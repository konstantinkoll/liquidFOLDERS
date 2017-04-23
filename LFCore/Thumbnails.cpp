
#include "stdafx.h"
#include "LFCore.h"
#include "Thumbnails.h"
#include <shellapi.h>
#include <Thumbcache.h>


extern OSVERSIONINFO osInfo;


LFCORE_API HBITMAP LFGetThumbnail(LFItemDescriptor* pItemDescriptor, SIZE sz)
{
	WCHAR Path[MAX_PATH];
	if (LFGetFileLocation(pItemDescriptor, Path, MAX_PATH, FALSE)!=LFOk)
		return NULL;

	HBITMAP hBitmap = NULL;

	LPITEMIDLIST pidlFQ;
	if (SUCCEEDED(SHParseDisplayName(Path, NULL, &pidlFQ, 0, NULL)))
	{
		IShellFolder* pParentFolder;
		LPCITEMIDLIST pidlRel;
		if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder, (LPVOID*)&pParentFolder, &pidlRel)))
		{
			// IThumbnailProvider, verfügbar seit Windows Vista
			// Liefert auch rechteckige Vorschaubilder
			if (osInfo.dwMajorVersion>=6)
			{
				IThumbnailProvider* pThumbnailProvider;
				if (SUCCEEDED(pParentFolder->GetUIObjectOf(NULL, 1, &pidlRel, IID_IThumbnailProvider, NULL, (LPVOID*)&pThumbnailProvider)))
				{
					DWORD dwAlpha = WTSAT_UNKNOWN;
					pThumbnailProvider->GetThumbnail(min(sz.cx, sz.cy), &hBitmap, &dwAlpha);

					pThumbnailProvider->Release();

					goto Finish;
				}
			}

			// IExtractImage, verfügbar seit Windows XP und Fallback seit Windows Vista
			// Liefert immer quadratische Vorschaubilder
			IExtractImage* pExtractImage;
			if (SUCCEEDED(pParentFolder->GetUIObjectOf(NULL, 1, &pidlRel, IID_IExtractImage, NULL, (LPVOID*)&pExtractImage)))
			{
				DWORD dwPriority = 0;
				DWORD dwFlags = IEIFLAG_SCREEN | IEIFLAG_NOBORDER | IEIFLAG_NOSTAMP | IEIFLAG_OFFLINE;
				HRESULT hResult = pExtractImage->GetLocation(Path, MAX_PATH, &dwPriority, &sz, 32, &dwFlags);
				if (SUCCEEDED(hResult) || (hResult==E_PENDING))
					pExtractImage->Extract(&hBitmap);

				pExtractImage->Release();

				goto Finish;
			}

Finish:
			pParentFolder->Release();
		}

		CoTaskMemFree(pidlFQ);
	}

	return hBitmap;
}

LFCORE_API HBITMAP LFQuarter256Bitmap(HBITMAP hBitmap)
{
	BITMAP BitmapSrc;
	GetObject(hBitmap, sizeof(BitmapSrc), &BitmapSrc);

	if ((BitmapSrc.bmBits==NULL) || (BitmapSrc.bmWidth!=256) || (BitmapSrc.bmHeight!=256) || ((BitmapSrc.bmBitsPixel!=24) && (BitmapSrc.bmBitsPixel!=32)))
		return hBitmap;

	BYTE* pBitsSrc = (BYTE*)BitmapSrc.bmBits;

	// Blt bitmaps without alpha channel
	if (BitmapSrc.bmBitsPixel==24)
	{
		// Create new bitmap
		BITMAPINFO DIB;
		ZeroMemory(&DIB, sizeof(DIB));

		DIB.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		DIB.bmiHeader.biWidth = 256;
		DIB.bmiHeader.biHeight = 256;
		DIB.bmiHeader.biPlanes = 1;
		DIB.bmiHeader.biBitCount = 24;
		DIB.bmiHeader.biCompression = BI_RGB;

		HBITMAP hBitmapNew = CreateDIBSection(NULL, &DIB, DIB_RGB_COLORS, (LPVOID*)&pBitsSrc, NULL, 0);

		// Blt bitmap to solve mirrored orientation by Foxit reader and others
		HDC hDCMem = CreateCompatibleDC(NULL);
		HDC hDC = CreateCompatibleDC(hDCMem);

		HBITMAP hBitmapOld1 = (HBITMAP)SelectObject(hDCMem, hBitmap);
		HBITMAP hBitmapOld2 = (HBITMAP)SelectObject(hDC, hBitmapNew);

		BitBlt(hDC, 0, 0, 256, 256, hDCMem, 0, 0, SRCCOPY);

		SelectObject(hDCMem, hBitmapOld1);
		SelectObject(hDC, hBitmapOld2);

		DeleteObject(hBitmap);
		hBitmap = hBitmapNew;

		ReleaseDC(NULL, hDCMem);
		ReleaseDC(NULL, hDC);
	}

	// Create new bitmap
	BITMAPINFO DIB;
	ZeroMemory(&DIB, sizeof(DIB));

	DIB.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	DIB.bmiHeader.biWidth = 128;
	DIB.bmiHeader.biHeight = 128;
	DIB.bmiHeader.biPlanes = 1;
	DIB.bmiHeader.biBitCount = 32;
	DIB.bmiHeader.biCompression = BI_RGB;

	BYTE* pBitsDst;
	HBITMAP hBitmapNew = CreateDIBSection(NULL, &DIB, DIB_RGB_COLORS, (LPVOID*)&pBitsDst, NULL, 0);

	switch (BitmapSrc.bmBitsPixel)
	{
	case 32:
		// True color with alpha channel
		for (UINT Row=0; Row<128; Row++)
		{
			for (UINT Column=0; Column<128; Column++)
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

		break;

	case 24:
		// True color without alpha channel
		for (UINT Row=0; Row<128; Row++)
		{
			for (UINT Column=0; Column<128; Column++)
			{
				*(pBitsDst+0) = (*(pBitsSrc+0)+*(pBitsSrc+3)+*(pBitsSrc+256*3)+*(pBitsSrc+256*3+3))>>2;
				*(pBitsDst+1) = (*(pBitsSrc+1)+*(pBitsSrc+4)+*(pBitsSrc+256*3+1)+*(pBitsSrc+256*3+4))>>2;
				*(pBitsDst+2) = (*(pBitsSrc+2)+*(pBitsSrc+5)+*(pBitsSrc+256*3+2)+*(pBitsSrc+256*3+5))>>2;
				*(pBitsDst+3) = 0xFF;

				pBitsSrc += 6;
				pBitsDst += 4;
			}

			pBitsSrc += 256*3;
		}

		break;
	}

	DeleteObject(hBitmap);

	return hBitmapNew;
}
