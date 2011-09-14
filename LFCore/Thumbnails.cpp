
#include "stdafx.h"
#include "LFCore.h"
#include "Thumbnails.h"


extern HMODULE LFCoreModuleHandle;
extern HANDLE Mutex_Stores;


LFCore_API HBITMAP LFGetThumbnail(LFItemDescriptor* i)
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
				IExtractImage* pExtractImage = NULL;
				if (SUCCEEDED(pParentFolder->GetUIObjectOf(NULL,1, &pidlRel, IID_IExtractImage, NULL, (void**)&pExtractImage)))
				{
					SIZE sz = { 128, 128 };
					DWORD dwPriority = 0;
					DWORD dwFlags = IEIFLAG_SCREEN;
					HRESULT hResult = pExtractImage->GetLocation(Path, MAX_PATH, &dwPriority, &sz, 32, &dwFlags);
					if (SUCCEEDED(hResult) || (hResult==E_PENDING))
						pExtractImage->Extract(&hBmp);

					pExtractImage->Release();
				}

				pParentFolder->Release();
			}
		}
	}

	return hBmp;
}
