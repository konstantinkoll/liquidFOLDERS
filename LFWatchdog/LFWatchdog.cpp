
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

wchar_t const szWindowClass[] = L"LFWatchdog";
ULONG ulSHChangeNotifyRegister;
BOOL AboutWindow = FALSE;
BOOL ReceivedDoubleclk = FALSE;


// Use a guid to uniquely identify our icon
class __declspec(uuid("f144ca00-1a4f-11df-8a39-0800200c9a66")) LFIcon;


void PrepareTrayTip(wchar_t* Buf, size_t cCount)
{
	UINT Stores = LFGetStoreCount();

	wchar_t Mask[256];
	ENSURE(LoadString(AfxGetResourceHandle(), (Stores==1) ? IDS_TOOLTIP_SINGULAR : IDS_TOOLTIP_PLURAL, Mask, 256));
	swprintf(Buf, cCount, Mask, Stores);
}

BOOL AddNotificationIcon(HWND hWnd)
{
	int sz = GetSystemMetrics(SM_CXSMICON);

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
	nid.uFlags = NIF_TIP;
	nid.uVersion = NOTIFYICON_VERSION_4;
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
	wcex.style = CS_HREDRAW | CS_VREDRAW;
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

	SHNOTIFYSTRUCT *shns = (SHNOTIFYSTRUCT*)wParam;
	char sPath[MAX_PATH];

	switch(lParam)
	{
	case SHCNE_DRIVEADD:
	case SHCNE_MEDIAINSERTED:
		if (SHGetPathFromIDListA(shns->dwItem1, sPath))
			LFErrorBox(LFMountDrive(sPath[0]));
		break;
	case SHCNE_MEDIAREMOVED:
	case SHCNE_DRIVEREMOVED:
		if (SHGetPathFromIDListA(shns->dwItem1, sPath))
			LFErrorBox(LFUnmountDrive(sPath[0]));
		break;
	}

	return NULL;
}

void ShowAboutDlg()
{
	if (!AboutWindow)
	{
		AboutWindow = TRUE;

		LFAboutDlgParameters p;
		p.appname = "Watchdog";
		p.build = __TIMESTAMP__;
		p.icon = new CGdiPlusBitmapResource();
		p.icon->Load(IDB_ABOUTICON, _T("PNG"), AfxGetResourceHandle());
		p.TextureSize = -1;
		p.RibbonColor = ID_VIEW_APPLOOK_OFF_2007_NONE;
		p.HideEmptyDrives = -1;
		p.HideEmptyDomains = -1;

		LFAboutDlg dlg(&p, NULL);
		dlg.DoModal();

		delete p.icon;
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
	if (_access(theApp.path+"StoreManager.exe", 0)!=0)
		EnableMenuItem(hSubMenu, ID_APP_NEWSTOREMANAGER, MF_BYCOMMAND | MF_GRAYED);
	if (_access(theApp.path+"Migrate.exe", 0)!=0)
		EnableMenuItem(hSubMenu, ID_APP_NEWMIGRATE, MF_BYCOMMAND | MF_GRAYED);
	if (_access(theApp.path+"FileDrop.exe", 0)!=0)
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
	: LFApplication(HasGUI_None)
{
}


// Das einzige CWatchdogApp-Objekt

CWatchdogApp theApp;


// CWatchdogApp-Initialisierung

BOOL CWatchdogApp::InitInstance()
{
	sessionMutex = CreateMutex(NULL, TRUE, _T(LFCM_Watchdog));
	if (sessionMutex && GetLastError()==ERROR_ALREADY_EXISTS)
	{
		ReleaseMutex(sessionMutex);
		return FALSE;
	}

	LFApplication::InitInstance();
	CreateHostWindow();

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return FALSE;
}

int CWatchdogApp::ExitInstance()
{
	LFApplication::ExitInstance();
	CloseHandle(sessionMutex);
	sessionMutex = NULL;
	return 0;
}
