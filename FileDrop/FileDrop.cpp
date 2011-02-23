
// FileDrop.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "FileDrop.h"
#include "FileDropWnd.h"
#include "resource.h"


static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM /*lParam*/)
{
	BOOL res = TRUE;

	CWnd* wnd = CWnd::FromHandle(hWnd);

	CString caption;
	wnd->GetWindowText(caption);
	if (caption==_T("FileDrop"))
		res = (wnd->SendMessage(theApp.WakeupMsg, NULL, NULL)!=24878);

	return res;
}


// CFileDropApp-Erstellung

CFileDropApp::CFileDropApp()
	: LFApplication(TRUE)
{
	WakeupMsg = RegisterWindowMessageA("liquidFOLDERS.FileDrop.Wakeup");
}


// Das einzige CFileDropApp-Objekt

CFileDropApp theApp;


// CFileDropApp-Initialisierung

BOOL CFileDropApp::InitInstance()
{
	if (!EnumWindows((WNDENUMPROC)EnumWindowsProc, NULL))
		return FALSE;

	LFApplication::InitInstance();

	SetRegistryBase(_T("Settings"));

	m_pMainWnd = new CFileDropWnd();
	((CFileDropWnd*)m_pMainWnd)->Create();
	m_pMainWnd->ShowWindow(SW_SHOW);

	return TRUE;
}
