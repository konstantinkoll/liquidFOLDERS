
// Migrate.cpp: Definiert das Klassenverhalten f�r die Anwendung.
//

#include "stdafx.h"
#include "Migrate.h"
#include "MigrateWnd.h"
#include "resource.h"


GUID theAppID =	// {711E9094-244A-4920-8B20-DBFB86C91B8B}
	{ 0x711E9094, 0x244A, 0x4920, { 0x8B, 0x20, 0xDB, 0xFB, 0x86, 0xC9, 0x1B, 0x8B } };

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


// CMigrateApp

BEGIN_MESSAGE_MAP(CMigrateApp, LFApplication)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()


// CMigrateApp-Erstellung

CMigrateApp::CMigrateApp()
	: LFApplication(theAppID)
{
	m_WakeupMsg = RegisterWindowMessage(_T("liquidFOLDERS.Migrate.NewWindow"));
}


// Das einzige CMigrateApp-Objekt

CMigrateApp theApp;


// CMigrateApp-Initialisierung

BOOL CMigrateApp::InitInstance()
{
	// Parameter
	if (!EnumWindows((WNDENUMPROC)EnumWindowsProc, (LPARAM)(__argc==2 ? __wargv[1] : NULL)))
		return FALSE;

	if (!LFApplication::InitInstance())
		return FALSE;

	// AppID
	if (m_ShellLibLoaded)
		zSetCurrentProcessExplicitAppUserModelID(L"liquidFOLDERS.Migrate");

	// Registry auslesen
	SetRegistryBase(_T("Settings"));
	m_ExpandAll = GetInt(_T("ExpandAll"), FALSE);

	CWnd* pFrame = OpenCommandLine();

	if (!LFIsLicensed())
		ShowNagScreen(NAG_NOTLICENSED | NAG_FORCE, pFrame);

	return TRUE;
}

CWnd* CMigrateApp::OpenCommandLine(WCHAR* /*CmdLine*/)
{
	CMigrateWnd* pFrame = new CMigrateWnd();
	pFrame->Create();
	pFrame->ShowWindow(SW_SHOW);

	return pFrame;
}


void CMigrateApp::OnAppAbout()
{
	CString AppName;
	ENSURE(AppName.LoadString(IDR_APPLICATION));
	TIMESTAMP;
	LFAbout(AppName, Timestamp, IDB_ABOUTICON, m_pMainWnd);
}
