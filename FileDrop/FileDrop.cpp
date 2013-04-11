
// FileDrop.cpp: Definiert das Klassenverhalten f�r die Anwendung.
//

#include "stdafx.h"
#include "FileDrop.h"
#include "FileDropWnd.h"
#include "resource.h"


BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	if (GetWindow(hWnd, GW_OWNER))
		return TRUE;

	DWORD_PTR Result;
	return SendMessageTimeout(hWnd, theApp.m_WakeupMsg, NULL, lParam, SMTO_NORMAL, 500, &Result) ? Result!=24878 : TRUE;
}


// CFileDropApp

BEGIN_MESSAGE_MAP(CFileDropApp, LFApplication)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()


// CFileDropApp-Erstellung

CFileDropApp::CFileDropApp()
	: LFApplication(TRUE)
{
	m_WakeupMsg = RegisterWindowMessage(_T("liquidFOLDERS.FileDrop.Wakeup"));
}


// Das einzige CFileDropApp-Objekt

CFileDropApp theApp;


// CFileDropApp-Initialisierung

BOOL CFileDropApp::InitInstance()
{
	if (!EnumWindows((WNDENUMPROC)EnumWindowsProc, NULL))
		return FALSE;

	if (!LFApplication::InitInstance())
		return FALSE;

	// Registry auslesen
	SetRegistryBase(_T("Settings"));

	m_pMainWnd = new CFileDropWnd();
	((CFileDropWnd*)m_pMainWnd)->Create();

	if (!LFIsLicensed())
		ShowNagScreen(NAG_NOTLICENSED | NAG_FORCE, m_pMainWnd);

	m_pMainWnd->ShowWindow(SW_SHOW);

	return TRUE;
}

void CFileDropApp::OnAppAbout()
{
	TIMESTAMP;
	LFAbout(_T("FileDrop"), Timestamp, IDB_ABOUTICON, m_pMainWnd);
}
