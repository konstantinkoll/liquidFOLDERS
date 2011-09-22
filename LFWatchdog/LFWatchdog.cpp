
// BFWatchdog.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "resource.h"
#include "LFWatchdog.h"
#include "MenuIcons.h"
#include <io.h>
#include <shlobj.h>
#include <shlwapi.h>

#define WM_USER_MEDIACHANGED                WM_USER+1
#define WMAPP_NOTIFYCALLBACK                WM_APP+1

WCHAR const szWindowClass[] = L"LFWatchdog";
ULONG ulSHChangeNotifyRegister;
BOOL AboutWindow = FALSE;
BOOL UpdateInProgress = FALSE;
BOOL ReceivedDoubleclk = FALSE;


// Use a guid to uniquely identify our icon
class __declspec(uuid("F144CA00-1A4F-11DF-8A39-0800200C9A66")) LFIcon;


void PrepareTrayTip(WCHAR* Buf, size_t cCount)
{
	UINT Stores = LFGetStoreCount();

	WCHAR Mask[256];
	ENSURE(LoadString(AfxGetResourceHandle(), (Stores==1) ? IDS_TOOLTIP_SINGULAR : IDS_TOOLTIP_PLURAL, Mask, 256));
	swprintf(Buf, cCount, Mask, Stores);
}

BOOL AddNotificationIcon(HWND hWnd)
{
	INT sz = GetSystemMetrics(SM_CXSMICON);

	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = hWnd;
	nid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE | NIF_GUID | NIF_SHOWTIP;
	nid.guidItem = __uuidof(LFIcon);
	nid.uVersion = NOTIFYICON_VERSION_4;
	nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
	nid.hIcon = (HICON)LoadImage(AfxGetResourceHandle(), MAKEINTRESOURCE(IDI_ICON), IMAGE_ICON, sz, sz, LR_DEFAULTCOLOR);
	PrepareTrayTip(nid.szTip, 128);
	Shell_NotifyIcon(NIM_ADD, &nid);

	return Shell_NotifyIcon(NIM_SETVERSION, &nid);
}

BOOL DeleteNotificationIcon(HWND hWnd)
{
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = hWnd;
	nid.uFlags = NIF_GUID;
	nid.guidItem = __uuidof(LFIcon);

	return Shell_NotifyIcon(NIM_DELETE, &nid);
}

BOOL OnStoresChanged(HWND hWnd)
{
	NOTIFYICONDATA nid;
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(nid);
	nid.hWnd = hWnd;
	nid.uFlags = NIF_TIP | NIF_GUID | NIF_MESSAGE | NIF_GUID | NIF_SHOWTIP;
	nid.guidItem = __uuidof(LFIcon);
	nid.uVersion = NOTIFYICON_VERSION_4;
	nid.uCallbackMessage = WMAPP_NOTIFYCALLBACK;
	PrepareTrayTip(nid.szTip, 128);
	Shell_NotifyIcon(NIM_ADD, &nid);

	return Shell_NotifyIcon(NIM_MODIFY, &nid);
}

void RegisterWindowClass(PCWSTR pszClassName, WNDPROC lpfnWndProc)
{
	LPTSTR MenuName = MAKEINTRESOURCE(IDM_TRAY);

	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(wcex));
	wcex.cbSize = sizeof(wcex);
	wcex.style = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = lpfnWndProc;
	wcex.hInstance = AfxGetInstanceHandle();
	wcex.hIcon = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName = MenuName;
	wcex.lpszClassName = pszClassName;
	RegisterClassEx(&wcex);
}

LRESULT OnMediaChanged(HWND /*hWnd*/, WPARAM wParam, LPARAM lParam)
{
	typedef struct
	{
		LPCITEMIDLIST dwItem1;
		DWORD dwItem2;
	} SHNOTIFYSTRUCT;

	SHNOTIFYSTRUCT* shns = (SHNOTIFYSTRUCT*)wParam;
	WCHAR sPath[MAX_PATH];

	if (SHGetPathFromIDList(shns->dwItem1, sPath))
		if ((sPath[0]>=L'A') && (sPath[0]<=L'Z'))
			switch(lParam)
			{
			case SHCNE_DRIVEADD:
			case SHCNE_MEDIAINSERTED:
				LFErrorBox(LFMountDrive((CHAR)sPath[0]));
				break;
			case SHCNE_MEDIAREMOVED:
			case SHCNE_DRIVEREMOVED:
				LFErrorBox(LFUnmountDrive((CHAR)sPath[0]));
				break;
			}

	return NULL;
}

void ShowAboutDlg()
{
	if (!AboutWindow)
	{
		AboutWindow = TRUE;
		TIMESTAMP;
		LFAbout(_T("Watchdog"), Timestamp, IDB_ABOUTICON);
		AboutWindow = FALSE;
	}
}

void ShowMenu(HWND hTargetWnd)
{
	HMENU hMenu = LoadMenu(AfxGetResourceHandle(), MAKEINTRESOURCE(IDM_TRAY));
	if (!hMenu)
		return;

	HMENU hSubMenu = GetSubMenu(hMenu, 0);
	if (!hSubMenu)
	{
		DestroyMenu(hMenu);
		return;
	}

	HBITMAP bmp1 = SetMenuItemIcon(hSubMenu, 2, ID_APP_NEWSTOREMANAGER);
	HBITMAP bmp2 = SetMenuItemIcon(hSubMenu, 3, ID_APP_NEWFILEDROP);
	HBITMAP bmp3 = SetMenuItemIcon(hSubMenu, 4, ID_APP_NEWMIGRATE);
	SetMenuItemBitmap(hSubMenu, 6, HBMMENU_POPUP_CLOSE);

	SetMenuDefaultItem(hSubMenu, ID_APP_ABOUT, 0);
	if (AboutWindow)
		EnableMenuItem(hSubMenu, ID_APP_ABOUT, MF_BYCOMMAND | MF_GRAYED);
	if (_waccess(theApp.m_Path+_T("StoreManager.exe"), 0)!=0)
		EnableMenuItem(hSubMenu, ID_APP_NEWSTOREMANAGER, MF_BYCOMMAND | MF_GRAYED);
	if (_waccess(theApp.m_Path+_T("Migrate.exe"), 0)!=0)
		EnableMenuItem(hSubMenu, ID_APP_NEWMIGRATE, MF_BYCOMMAND | MF_GRAYED);
	if (_waccess(theApp.m_Path+_T("FileDrop.exe"), 0)!=0)
		EnableMenuItem(hSubMenu, ID_APP_NEWFILEDROP, MF_BYCOMMAND | MF_GRAYED);

	POINT pos;
	GetCursorPos(&pos);

	SetForegroundWindow(hTargetWnd);
	TrackPopupMenu(hSubMenu, 0, pos.x, pos.y, 0, hTargetWnd, NULL);
	PostMessage(hTargetWnd, WM_NULL, 0, 0);

	DestroyMenu(hMenu);
	DeleteObject(bmp1);
	DeleteObject(bmp2);
	DeleteObject(bmp3);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message==theApp.p_MessageIDs->StoresChanged)
	{
		OnStoresChanged(hWnd);
	}
	else
		switch (message)
		{
		case WM_CREATE:
			AddNotificationIcon(hWnd);
			break;
		case WM_DESTROY:
			KillTimer(hWnd, 1);
			DeleteNotificationIcon(hWnd);
			PostQuitMessage(0);
			break;
		case WMAPP_NOTIFYCALLBACK:
			switch (LOWORD(lParam))
			{
			case WM_LBUTTONUP:
				if (ReceivedDoubleclk)
				{
					ReceivedDoubleclk = FALSE;
					break;
				}
			case WM_RBUTTONUP:
				ShowMenu(hWnd);
				break;
			case WM_LBUTTONDBLCLK:
				ReceivedDoubleclk = TRUE;
				ShowAboutDlg();
				break;
			}
			break;
		case WM_USER_MEDIACHANGED:
			OnMediaChanged(hWnd, wParam, lParam);
			break;
		case WM_TIMER:
			if (!UpdateInProgress)
			{
				UpdateInProgress = TRUE;
				LFCheckForUpdate();
				UpdateInProgress = FALSE;
			}
			break;
		case WM_COMMAND:
			switch (wParam)
			{
			case ID_APP_ABOUT:
				ShowAboutDlg();
				break;
			case ID_APP_NEWSTOREMANAGER:
				theApp.OnAppNewStoreManager();
				break;
			case ID_APP_NEWMIGRATE:
				theApp.OnAppNewMigrate();
				break;
			case ID_APP_NEWFILEDROP:
				theApp.OnAppNewFileDrop();
				break;
			case ID_APP_EXIT:
				DeleteNotificationIcon(hWnd);
				PostQuitMessage(0);
				break;
			}
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

	return 0;
}

HWND CreateHostWindow()
{
	RegisterWindowClass(szWindowClass, WndProc);

	HWND hWnd = CreateWindow(szWindowClass, _T("LFWatchdog"), WS_DISABLED, CW_USEDEFAULT, 0, 250, 200, NULL, NULL, AfxGetInstanceHandle(), NULL);
	if (hWnd)
	{
		// Update-Timer
		SetTimer(hWnd, 1, 1000/3600, NULL);

		// Benachrichtigung, wenn sich Laufwerke ändern
		LPITEMIDLIST ppidl;
		if (SHGetSpecialFolderLocation(hWnd, CSIDL_DESKTOP, &ppidl)==NOERROR)
		{
			SHChangeNotifyEntry shCNE;
			shCNE.pidl = ppidl;
			shCNE.fRecursive = TRUE;

			ulSHChangeNotifyRegister = SHChangeNotifyRegister(hWnd, SHCNRF_ShellLevel,
				SHCNE_DRIVEADD | SHCNE_DRIVEREMOVED | SHCNE_MEDIAINSERTED | SHCNE_MEDIAREMOVED,
				WM_USER_MEDIACHANGED, 1, &shCNE);
			ASSERT(ulSHChangeNotifyRegister);
			return hWnd;
		}
	}

	return NULL;
}


// CWatchdogApp-Erstellung

CWatchdogApp::CWatchdogApp()
	: LFApplication(FALSE)
{
}


// Das einzige CWatchdogApp-Objekt

CWatchdogApp theApp;


// CWatchdogApp-Initialisierung

BOOL CWatchdogApp::InitInstance()
{
	hSessionMutex = CreateMutex(NULL, TRUE, _T(LFCM_Watchdog));
	if (hSessionMutex && (GetLastError()==ERROR_ALREADY_EXISTS))
	{
		ReleaseMutex(hSessionMutex);
		return FALSE;
	}

	LFApplication::InitInstance();
	CreateHostWindow();

	LFCheckForUpdate();

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return FALSE;
}

INT CWatchdogApp::ExitInstance()
{
	LFApplication::ExitInstance();
	CloseHandle(hSessionMutex);
	hSessionMutex = NULL;
	return 0;
}
