
// Migrate.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "Migrate.h"
#include "MigrateWnd.h"
#include "resource.h"


static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
{
	return (SendMessage(hWnd, theApp.m_WakeupMsg, NULL, lParam)!=24878);
}


// CMigrateApp

BEGIN_MESSAGE_MAP(CMigrateApp, LFApplication)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()


// CMigrateApp-Erstellung

CMigrateApp::CMigrateApp()
	: LFApplication(TRUE)
{
	m_WakeupMsg = RegisterWindowMessage(_T("liquidFOLDERS.Migrate.NewWindow"));
}


// Das einzige CMigrateApp-Objekt

CMigrateApp theApp;


// CMigrateApp-Initialisierung

BOOL CMigrateApp::InitInstance()
{
	if (!EnumWindows((WNDENUMPROC)EnumWindowsProc, NULL))
		return FALSE;

	LFApplication::InitInstance();

	// Registry auslesen
	SetRegistryBase(_T("Settings"));
	m_ExpandAll = GetInt(_T("ExpandAll"), FALSE);
	m_DeleteImported = GetInt(_T("DeleteImported"), FALSE);

	m_pMainWnd = new CMigrateWnd();
	((CMigrateWnd*)m_pMainWnd)->Create();
	m_pMainWnd->ShowWindow(SW_SHOW);

	return TRUE;
}

INT CMigrateApp::ExitInstance()
{
	WriteInt(_T("ExpandAll"), m_ExpandAll);
	WriteInt(_T("DeleteImported"), m_DeleteImported);

	return LFApplication::ExitInstance();
}


void CMigrateApp::AddFrame(CMigrateWnd* pFrame)
{
	m_MainFrames.AddTail(pFrame);
	m_pMainWnd = pFrame;
	m_pActiveWnd = NULL;
}

void CMigrateApp::KillFrame(CMigrateWnd* pVictim)
{
	for (POSITION p=m_MainFrames.GetHeadPosition(); p; )
	{
		POSITION pl = p;
		CMigrateWnd* pFrame = m_MainFrames.GetNext(p);
		if (pFrame==pVictim)
		{
			m_MainFrames.RemoveAt(pl);
		}
		else
		{
			m_pMainWnd = pFrame;
		}
	}
}

void CMigrateApp::OnAppAbout()
{
	CString AppName;
	ENSURE(AppName.LoadString(IDR_APPLICATION));
	TIMESTAMP;
	LFAbout(AppName, Timestamp, IDB_ABOUTICON, m_pMainWnd);
}
