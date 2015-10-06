
#include "stdafx.h"
#include "LFCore.h"
#include "Thumbnails.h"
#include <shellapi.h>
#include <Thumbcache.h>


extern OSVERSIONINFO osInfo;


LFCORE_API HBITMAP LFGetThumbnail(LFItemDescriptor* i, SIZE sz)
{
	WCHAR Path[MAX_PATH];
	if (LFGetFileLocation(i, Path, MAX_PATH, FALSE)!=LFOk)
		return NULL;

	HBITMAP hBitmap = NULL;

	LPITEMIDLIST pidlFQ;
	if (SUCCEEDED(SHParseDisplayName(Path, NULL, &pidlFQ, 0, NULL)))
	{
		IShellFolder* pParentFolder;
		LPCITEMIDLIST pidlRel;
		if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder, (void**)&pParentFolder, &pidlRel)))
		{
			// IThumbnailProvider, verfügbar seit Windows Vista
			// Liefert auch rechteckige Vorschaubilder
			if (osInfo.dwMajorVersion>=6)
			{
				IThumbnailProvider* pThumbnailProvider;
				if (SUCCEEDED(pParentFolder->GetUIObjectOf(NULL, 1, &pidlRel, IID_IThumbnailProvider, NULL, (void**)&pThumbnailProvider)))
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
			if (SUCCEEDED(pParentFolder->GetUIObjectOf(NULL, 1, &pidlRel, IID_IExtractImage, NULL, (void**)&pExtractImage)))
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

	if ((BitmapSrc.bmWidth!=256) || (BitmapSrc.bmHeight!=256) || (BitmapSrc.bmBitsPixel!=32))
		return hBitmap;

	BYTE* pBitsSrc = (BYTE*)BitmapSrc.bmBits;
	BYTE* pBitsDst;

	// Neue Bitmap erzeugen
	BITMAPINFO DIB;
	ZeroMemory(&DIB, sizeof(DIB));

	DIB.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	DIB.bmiHeader.biWidth = 128;
	DIB.bmiHeader.biHeight = 128;
	DIB.bmiHeader.biPlanes = 1;
	DIB.bmiHeader.biBitCount = 32;
	DIB.bmiHeader.biCompression = BI_RGB;

	HDC hDC = GetDC(NULL);
	HBITMAP hBitmapNew = CreateDIBSection(hDC, &DIB, DIB_RGB_COLORS, (void**)&pBitsDst, NULL, 0);
	ReleaseDC(NULL, hDC);

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

	DeleteObject(hBitmap);

	return hBitmapNew;
}
