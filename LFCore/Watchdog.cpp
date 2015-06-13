
#include "stdafx.h"
#include "LFCore.h"
#include "StoreCache.h"


extern HMODULE LFCoreModuleHandle;

const wchar_t szWindowClass[] = L"LFWatchdog";
HWND hWndWatchdog = NULL;
ULONG ulSHChangeNotifyRegister;


LRESULT CALLBACK WndProc(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	struct SHNOTIFYSTRUCT
	{
		LPCITEMIDLIST dwItem1;
		DWORD dwItem2;
	};

	if (Message==WM_USER)
	{
		SHNOTIFYSTRUCT* shns = (SHNOTIFYSTRUCT*)wParam;
		WCHAR sPath[MAX_PATH];

		if (SHGetPathFromIDList(shns->dwItem1, sPath))
			if ((sPath[0]>=L'A') && (sPath[0]<=L'Z'))
				switch(lParam)
				{
				case SHCNE_DRIVEADD:
				case SHCNE_MEDIAINSERTED:
					LFErrorBox(MountVolume((char)sPath[0]));
					break;
				case SHCNE_MEDIAREMOVED:
				case SHCNE_DRIVEREMOVED:
					LFErrorBox(UnmountVolume((char)sPath[0]));
					break;
				}

		return 0;
	}

	return DefWindowProc(hWnd, Message, wParam, lParam);
}

void RegisterWindowClass(PCWSTR pszClassName, WNDPROC lpfnWndProc)
{
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.lpfnWndProc = lpfnWndProc;
	wcex.hInstance = LFCoreModuleHandle;
	wcex.lpszClassName = pszClassName;
	RegisterClassEx(&wcex);
}

void InitWatchdog()
{
	if (hWndWatchdog)
		return;

	RegisterWindowClass(szWindowClass, WndProc);

	hWndWatchdog = CreateWindowEx(0, szWindowClass, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
	if (hWndWatchdog)
	{
		// Benachrichtigung, wenn sich Laufwerke ändern
		LPITEMIDLIST ppidl;
		if (SHGetSpecialFolderLocation(hWndWatchdog, CSIDL_DESKTOP, &ppidl)==NOERROR)
		{
			SHChangeNotifyEntry shCNE;
			shCNE.pidl = ppidl;
			shCNE.fRecursive = TRUE;

			ulSHChangeNotifyRegister = SHChangeNotifyRegister(hWndWatchdog, SHCNRF_ShellLevel,
				SHCNE_DRIVEADD | SHCNE_DRIVEREMOVED | SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED,
				WM_USER, 1, &shCNE);
		}
	}
}
