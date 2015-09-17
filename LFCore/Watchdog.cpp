
#include "stdafx.h"
#include "Stores.h"
#include "Watchdog.h"


extern HMODULE LFCoreModuleHandle;

HWND hWndWatchdog = NULL;
ULONG ulSHChangeNotifyRegister;


LRESULT CALLBACK WndProc(HWND hWnd, UINT32 Message, WPARAM wParam, LPARAM lParam)
{
	if (Message==WM_USER)
	{
		PIDLIST_ABSOLUTE* pidls;
		LONG Event;
		HANDLE hNotifyLock = SHChangeNotification_Lock((HANDLE)wParam, (DWORD)lParam, &pidls, &Event);

		if (hNotifyLock)
		{
			WCHAR Path[MAX_PATH];
			if (SHGetPathFromIDList(pidls[0], Path))
				if ((Path[0]>=L'A') && (Path[0]<=L'Z'))
					switch(Event)
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

			SHChangeNotification_Unlock(hNotifyLock);
		}

		return NULL;
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
		SHChangeNotifyEntry shCNE = { NULL, TRUE };
		ulSHChangeNotifyRegister = SHChangeNotifyRegister(hWndWatchdog, SHCNRF_ShellLevel | SHCNRF_NewDelivery,
			SHCNE_DRIVEADD | SHCNE_DRIVEREMOVED | SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED,
			WM_USER, 1, &shCNE);
	}
}
