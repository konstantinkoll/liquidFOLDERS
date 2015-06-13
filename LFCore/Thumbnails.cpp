
#include "stdafx.h"
#include "LFCore.h"
#include "Thumbnails.h"
#include "Thumbcache.h"


extern HMODULE LFCoreModuleHandle;
extern HANDLE Mutex_Stores;
extern OSVERSIONINFO osInfo;


LFCORE_API HBITMAP LFGetThumbnail(LFItemDescriptor* i)
{
	static SIZE sz = { 118, 118 };
	return LFGetThumbnail(i, sz);
}

LFCORE_API HBITMAP LFGetThumbnail(LFItemDescriptor* i, SIZE sz)
{
	if ((i->Type & LFTypeMask)!=LFTypeFile)
		return NULL;

	wchar_t Path[MAX_PATH];

	if (LFGetFileLocation(i, Path, MAX_PATH, true, false)!=LFOk)
		return NULL;

	HBITMAP hBmp = NULL;

	IShellFolder* pDesktop = NULL;
	if (SUCCEEDED(SHGetDesktopFolder(&pDesktop)))
	{
		LPITEMIDLIST pidlFQ = NULL;
		if (SUCCEEDED(pDesktop->ParseDisplayName(NULL, NULL, Path, NULL, &pidlFQ, NULL)))
		{
			IShellFolder* pParentFolder = NULL;
			LPCITEMIDLIST pidlRel = NULL;
			if (SUCCEEDED(SHBindToParent(pidlFQ, IID_IShellFolder, (void**)&pParentFolder, &pidlRel)))
			{
				// IThumbnailProvider
				if (osInfo.dwMajorVersion>=6)
				{
					IThumbnailProvider* pThumbnailProvider = NULL;
					if (SUCCEEDED(pParentFolder->GetUIObjectOf(NULL, 1, &pidlRel, IID_IThumbnailProvider, NULL, (void**)&pThumbnailProvider)))
					{
						DWORD dwAlpha = WTSAT_UNKNOWN;
						pThumbnailProvider->GetThumbnail(min(sz.cx, sz.cy), &hBmp, &dwAlpha);

						pThumbnailProvider->Release();
						goto Finish;
					}
				}

				// IExtractImage
				IExtractImage* pExtractImage = NULL;
				if (SUCCEEDED(pParentFolder->GetUIObjectOf(NULL, 1, &pidlRel, IID_IExtractImage, NULL, (void**)&pExtractImage)))
				{
					DWORD dwPriority = 0;
					DWORD dwFlags = IEIFLAG_SCREEN | IEIFLAG_NOBORDER | IEIFLAG_NOSTAMP | IEIFLAG_OFFLINE;
					HRESULT hResult = pExtractImage->GetLocation(Path, MAX_PATH, &dwPriority, &sz, 32, &dwFlags);
					if (SUCCEEDED(hResult) || (hResult==E_PENDING))
						pExtractImage->Extract(&hBmp);

					pExtractImage->Release();
					goto Finish;
				}

Finish:
				pParentFolder->Release();
			}
		}

		pDesktop->Release();
	}

	if (hBmp)
	{
		BITMAP bm;
		GetObject(hBmp, sizeof(bm), &bm);

		if ((bm.bmWidth>128) || (bm.bmHeight>128))
			if ((bm.bmWidth==256) && (bm.bmHeight==256) && (bm.bmBitsPixel==32))
			{
				// Scale down to 128x128
				BITMAPINFO dib = { 0 };
				dib.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
				dib.bmiHeader.biWidth = 128;
				dib.bmiHeader.biHeight = 128;
				dib.bmiHeader.biPlanes = 1;
				dib.bmiHeader.biBitCount = 32;
				dib.bmiHeader.biCompression = BI_RGB;

				HBITMAP hBmpNew = CreateDIBSection(GetDC(NULL), &dib, DIB_RGB_COLORS, NULL, NULL, 0);

				BITMAP bmNew;
				GetObject(hBmpNew, sizeof(BITMAP), &bmNew);
				BYTE* pBitsSrc = ((BYTE*)bm.bmBits);
				BYTE* pBitsDst = ((BYTE*)bmNew.bmBits);

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

				DeleteObject(hBmp);
				hBmp = hBmpNew;
			}
			else
			{
				DeleteObject(hBmp);
				return NULL;
			}
	}

	return hBmp;
}
