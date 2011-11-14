
// Migrate.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "Migrate.h"
#include "MigrateWnd.h"
#include "resource.h"


BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM lParam)
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

	if (!LFApplication::InitInstance())
		return FALSE;

	// Registry auslesen
	SetRegistryBase(_T("Settings"));
	m_ExpandAll = GetInt(_T("ExpandAll"), FALSE);

	CMigrateWnd* pFrame = new CMigrateWnd();
	pFrame->Create();
	pFrame->ShowWindow(SW_SHOW);

	return TRUE;
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
