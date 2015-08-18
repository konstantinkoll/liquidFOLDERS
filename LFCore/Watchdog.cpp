
#include "stdafx.h"
#include "StoreCache.h"
#include "Watchdog.h"


extern HMODULE LFCoreModuleHandle;

HWND hWndWatchdog = NULL;
ULONG ulSHChangeNotifyRegister;


LRESULT CALLBACK WndProc(HWND hWnd, UINT32 Message, WPARAM wParam, LPARAM lParam)
{
	if (Message==WM_USER)
	{
		WCHAR Path[MAX_PATH];

		if (SHGetPathFromIDList(*((LPCITEMIDLIST*)wParam), Path))
			if ((Path[0]>=L'A') && (Path[0]<=L'Z'))
				switch(lParam)
				{
				case SHCNE_DRIVEADD:
				case SHCNE_MEDIAINSERTED:
					LFCoreErrorBox(MountVolume((CHAR)Path[0]));
					break;

				case SHCNE_MEDIAREMOVED:
				case SHCNE_DRIVEREMOVED:
					LFCoreErrorBox(UnmountVolume((CHAR)Path[0]));
					break;
				}

		return 0;
	}

	return DefWindowProc(hWnd, Message, wParam, lParam);
}

void InitWatchdog()
{
	if (hWndWatchdog)
		return;

	static const WCHAR szWindowClass[] = L"LFWatchdog";

	// Fenster-Klasse registrieren
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));

	wcex.cbSize = sizeof(wcex);
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = LFCoreModuleHandle;
	wcex.lpszClassName = szWindowClass;

	RegisterClassEx(&wcex);

	// Fenster erzeugen
	hWndWatchdog = CreateWindow(szWindowClass, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);

	if (hWndWatchdog)
	{
		// Benachrichtigung, wenn sich Laufwerke ändern
		LPITEMIDLIST pidl;
		if (SHGetSpecialFolderLocation(hWndWatchdog, CSIDL_DESKTOP, &pidl)==NOERROR)
		{
			SHChangeNotifyEntry shCNE;
			shCNE.pidl = pidl;
			shCNE.fRecursive = TRUE;

			ulSHChangeNotifyRegister = SHChangeNotifyRegister(hWndWatchdog, SHCNRF_ShellLevel,
				SHCNE_DRIVEADD | SHCNE_DRIVEREMOVED | SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED,
				WM_USER, 1, &shCNE);
		}
	}
}
