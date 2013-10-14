
// FileDrop.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "FileDrop.h"
#include "FileDropWnd.h"
#include "resource.h"


GUID theAppID =	// {FA2D9FEE-05FA-4B0D-8702-88DB19A0F38F}
	{ 0xfa2d9fee, 0x5fa, 0x4b0d, { 0x87, 0x2, 0x88, 0xdb, 0x19, 0xa0, 0xf3, 0x8f } };

BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	if (GetWindow(hWnd, GW_OWNER))
		return TRUE;

	DWORD_PTR Result;
	if (SendMessageTimeout(hWnd, theApp.m_WakeupMsg, NULL, NULL, SMTO_NORMAL, 500, &Result))
		if (Result==24878)
		{
			CDS_Wakeup cdsw;
			ZeroMemory(&cdsw, sizeof(cdsw));
			cdsw.AppID = theAppID;
			if (lParam)
				wcscpy_s(cdsw.Command, MAX_PATH, (WCHAR*)lParam);

			COPYDATASTRUCT cds;
			cds.cbData = sizeof(cdsw);
			cds.lpData = &cdsw;
			if (SendMessage(hWnd, WM_COPYDATA, NULL, (LPARAM)&cds))
				return FALSE;
		}

	return TRUE;
}


// CFileDropApp

BEGIN_MESSAGE_MAP(CFileDropApp, LFApplication)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()


// CFileDropApp-Erstellung

CFileDropApp::CFileDropApp()
	: LFApplication(TRUE, theAppID)
{
	m_WakeupMsg = RegisterWindowMessage(_T("liquidFOLDERS.FileDrop.NewWindow"));
}


// Das einzige CFileDropApp-Objekt

CFileDropApp theApp;


// CFileDropApp-Initialisierung

BOOL CFileDropApp::InitInstance()
{
	// Parameter
	if (!EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)(__argc==2 ? __wargv[1] : NULL)))
		return FALSE;

	if (!LFApplication::InitInstance())
		return FALSE;

	// AppID
	if (m_ShellLibLoaded)
		zSetCurrentProcessExplicitAppUserModelID(L"liquidFOLDERS.FileDrop");

	// Registry auslesen
	SetRegistryBase(_T("Settings"));

	m_pMainWnd = OpenCommandLine();

	if (!LFIsLicensed())
		ShowNagScreen(NAG_NOTLICENSED | NAG_FORCE, m_pMainWnd);

	return TRUE;
}

CWnd* CFileDropApp::OpenCommandLine(WCHAR* /*CmdLine*/)
{
	if (m_pMainWnd)
	{
		if (m_pMainWnd->IsIconic())
			m_pMainWnd->ShowWindow(SW_RESTORE);

		m_pMainWnd->SetForegroundWindow();

		return m_pMainWnd;
	}

	CFileDropWnd* pFrame = new CFileDropWnd();
	pFrame->Create();
	pFrame->ShowWindow(SW_SHOW);

	return pFrame;
}

void CFileDropApp::OnAppAbout()
{
	TIMESTAMP;
	LFAbout(_T("FileDrop"), Timestamp, IDB_ABOUTICON, m_pMainWnd);
}
