
#include "stdafx.h"
#include "LFCore.h"
#include "Thumbnails.h"
#include <shellapi.h>
#include <Thumbcache.h>

#include <gdiplus.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")


extern OSVERSIONINFO osInfo;


HMODULE hModShell;
PFNSHCREATEITEMFROMPARSINGNAME zSHCreateItemFromParsingName;


void InitThumbnails()
{
	if ((hModShell=LoadLibrary(L"SHELL32.DLL"))!=NULL)
	{
		zSHCreateItemFromParsingName = (PFNSHCREATEITEMFROMPARSINGNAME)GetProcAddress(hModShell, "SHCreateItemFromParsingName");
	}
	else
	{
		zSHCreateItemFromParsingName = NULL;
	}
}

LFCORE_API HBITMAP LFGetThumbnail(LFItemDescriptor* pItemDescriptor, SIZE Size)
{
	WCHAR Path[MAX_PATH];
	if (LFGetFileLocation(pItemDescriptor, Path, MAX_PATH, FALSE)!=LFOk)
		return NULL;

	HBITMAP hBitmap = NULL;

	// IShellItemImageFactory, available since Windows Vista
	// May deliver non-square thumbnails
	if ((osInfo.dwMajorVersion>=6) && zSHCreateItemFromParsingName)
	{
		IShellItemImageFactory* pShellItemImageFactory;
		if (SUCCEEDED(zSHCreateItemFromParsingName(Path, NULL, IID_IShellItemImageFactory, (LPVOID*)&pShellItemImageFactory)))
		{
			pShellItemImageFactory->GetImage(Size, SIIGBF_RESIZETOFIT | SIIGBF_THUMBNAILONLY, &hBitmap);
			pShellItemImageFactory->Release();

			goto Finish;
		}
	}

	// IExtractImage, available since Windows XP and fallback since Windows Vista
	// Always delivers square thumbnails
	LPITEMIDLIST pidlFQ;
	if (SUCCEEDED(SHParseDisplayName(Path, NULL, &pidlFQ, 0, NULL)))
	{
		IShellFolder* pParentFolder;
		LPCITEMIDLIST pidlRel;
		if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder, (LPVOID*)&pParentFolder, &pidlRel)))
		{
			IExtractImage* pExtractImage;
			if (SUCCEEDED(pParentFolder->GetUIObjectOf(NULL, 1, &pidlRel, IID_IExtractImage, NULL, (LPVOID*)&pExtractImage)))
			{
				DWORD dwPriority = 0;
				DWORD dwFlags = IEIFLAG_SCREEN | IEIFLAG_NOBORDER | IEIFLAG_NOSTAMP | IEIFLAG_OFFLINE;
				HRESULT hResult = pExtractImage->GetLocation(Path, MAX_PATH, &dwPriority, &Size, 32, &dwFlags);

				if (SUCCEEDED(hResult) || (hResult==E_PENDING))
					pExtractImage->Extract(&hBitmap);

				pExtractImage->Release();
			}

			pParentFolder->Release();
		}

		CoTaskMemFree(pidlFQ);
	}

Finish:
	return LFSanitizeThumbnail(hBitmap);
}

LFCORE_API HBITMAP LFSanitizeThumbnail(HBITMAP hBitmap)
{
	// Is hBitmap valid?
	if (!hBitmap)
		return NULL;

	// Get bitmap data
	BITMAP BitmapSrc;
	GetObject(hBitmap, sizeof(BitmapSrc), &BitmapSrc);

	// Are the pixels stored in RAM?
	if (!BitmapSrc.bmBits)
		return hBitmap;

	// Only try to fix thumbnails with a width of 256 or 512 pixels
	if (((BitmapSrc.bmWidth!=256) && (BitmapSrc.bmWidth!=512)))
		return hBitmap;

	// Only try to fix thumbnails with 24bpp or 32bpp color depth
	if ((BitmapSrc.bmBitsPixel!=24) && (BitmapSrc.bmBitsPixel!=32))
		return hBitmap;

	BYTE* pBitsSrc = (BYTE*)BitmapSrc.bmBits;

	// Blit 24bpp bitmaps to solve mirrored orientation by Foxit reader and others
	if (BitmapSrc.bmBitsPixel==24)
	{
		// Create new bitmap
		BITMAPINFO DIB;
		ZeroMemory(&DIB, sizeof(DIB));

		DIB.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		DIB.bmiHeader.biWidth = BitmapSrc.bmWidth;
		DIB.bmiHeader.biHeight = BitmapSrc.bmHeight;
		DIB.bmiHeader.biPlanes = 1;
		DIB.bmiHeader.biBitCount = 24;
		DIB.bmiHeader.biCompression = BI_RGB;

		HBITMAP hBitmapNew = CreateDIBSection(NULL, &DIB, DIB_RGB_COLORS, (LPVOID*)&pBitsSrc, NULL, 0);

		HDC hDCMem = CreateCompatibleDC(NULL);
		HDC hDC = CreateCompatibleDC(hDCMem);

		HBITMAP hBitmapOld1 = (HBITMAP)SelectObject(hDCMem, hBitmap);
		HBITMAP hBitmapOld2 = (HBITMAP)SelectObject(hDC, hBitmapNew);

		BitBlt(hDC, 0, 0, BitmapSrc.bmWidth, BitmapSrc.bmHeight, hDCMem, 0, 0, SRCCOPY);

		SelectObject(hDCMem, hBitmapOld1);
		SelectObject(hDC, hBitmapOld2);

		DeleteObject(hBitmap);
		hBitmap = hBitmapNew;

		ReleaseDC(NULL, hDCMem);
		ReleaseDC(NULL, hDC);
	}

	// Create new bitmap
	const LONG Scale = BitmapSrc.bmWidth/128;

	BITMAPINFO DIB;
	ZeroMemory(&DIB, sizeof(DIB));

	DIB.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	DIB.bmiHeader.biWidth = 128;
	DIB.bmiHeader.biHeight = BitmapSrc.bmHeight/Scale;
	DIB.bmiHeader.biPlanes = 1;
	DIB.bmiHeader.biBitCount = BitmapSrc.bmBitsPixel;
	DIB.bmiHeader.biCompression = BI_RGB;

	LPBYTE pBitsDst;
	HBITMAP hBitmapNew = CreateDIBSection(NULL, &DIB, DIB_RGB_COLORS, (LPVOID*)&pBitsDst, NULL, 0);

	// Invoke high-performance, high-quality scaler
	switch (BitmapSrc.bmBitsPixel)
	{
	case 24:
		// True color without alpha channel
		switch (BitmapSrc.bmWidth)
		{
		case 256:
			HQScale24<2>(DIB, pBitsSrc, pBitsDst);
			break;

		case 512:
			HQScale24<4>(DIB, pBitsSrc, pBitsDst);
			break;
		}

		break;

	case 32:
		// True color with alpha channel
		switch (BitmapSrc.bmWidth)
		{
		case 256:
			HQScale32<2>(DIB, pBitsSrc, pBitsDst);
			break;

		case 512:
			HQScale32<4>(DIB, pBitsSrc, pBitsDst);
			break;
		}

		break;
	}

	DeleteObject(hBitmap);

	return hBitmapNew;
}
