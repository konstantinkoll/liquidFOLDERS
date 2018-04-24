
#include "stdafx.h"
#include "Stores.h"
#include "Watchdog.h"


extern HMODULE LFCoreModuleHandle;
extern LFMessageIDs LFMessages;

HWND hWndWatchdog = NULL;
ULONG ulSHChangeNotifyRegister;


LRESULT CALLBACK WndProc(HWND hWnd, UINT32 Message, WPARAM wParam, LPARAM lParam)
{
	// Volume
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
					switch (Event)
					{
					case SHCNE_DRIVEADD:
					case SHCNE_MEDIAINSERTED:
						LFCoreErrorBox(MountVolume((CHAR)Path[0]));
						break;

					case SHCNE_MEDIAREMOVED:
					case SHCNE_DRIVEREMOVED:
						LFCoreErrorBox(MountVolume((CHAR)Path[0], FALSE));
						break;
					}

			SHChangeNotification_Unlock(hNotifyLock);
		}

		return NULL;
	}

	// Timer
	if (Message==WM_TIMER)
	{
		// Remount network volumes
		if (wParam==1)
			MountVolumes(LFGLV_NETWORK);

		// Eat bogus WM_TIMER messages
		MSG msg;
		while (PeekMessage(&msg, hWndWatchdog, WM_TIMER, WM_TIMER, PM_REMOVE));
	}

	// Default
	return DefWindowProc(hWnd, Message, wParam, lParam);
}

void InitWatchdog()
{
	if (hWndWatchdog)
		return;

	static const WCHAR szWindowClass[] = L"LFWatchdog";

	// Register window class
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));

	wcex.cbSize = sizeof(wcex);
	wcex.lpfnWndProc = WndProc;
	wcex.hInstance = LFCoreModuleHandle;
	wcex.lpszClassName = szWindowClass;

	RegisterClassEx(&wcex);

	// Create Window
	if ((hWndWatchdog=CreateWindow(szWindowClass, L"", 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL))!=NULL)
	{
		// Shell notifications
		const SHChangeNotifyEntry shCNE = { NULL, TRUE };

		ulSHChangeNotifyRegister = SHChangeNotifyRegister(hWndWatchdog, SHCNRF_ShellLevel | SHCNRF_NewDelivery,
			SHCNE_DRIVEADD | SHCNE_DRIVEREMOVED | SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED,
			WM_USER, 1, &shCNE);

		// Timer
#ifndef _DEBUG
		SetTimer(hWndWatchdog, 1, 60000, NULL);		// 60s
#else
		SetTimer(hWndWatchdog, 1, 10000, NULL);		// 10s
#endif
	}
}
