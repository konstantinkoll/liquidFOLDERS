
#include "stdafx.h"
#include "LFCore.h"
#include "Thumbnails.h"
#include "Thumbcache.h"


extern HMODULE LFCoreModuleHandle;
extern HANDLE Mutex_Stores;


LFCore_API HBITMAP LFGetThumbnail(LFItemDescriptor* i)
{
	SIZE sz = { 118, 118 };
	return LFGetThumbnail(i, sz);
}

LFCore_API HBITMAP LFGetThumbnail(LFItemDescriptor* i, SIZE sz)
{
	if ((i->Type & LFTypeMask)!=LFTypeFile)
		return NULL;

	wchar_t Path[MAX_PATH];

	if (LFGetFileLocation(i, Path, MAX_PATH, true)!=LFOk)
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
				OSVERSIONINFO osInfo;
				ZeroMemory(&osInfo, sizeof(OSVERSIONINFO));
				osInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
				GetVersionEx(&osInfo);

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

	return hBmp;
}
