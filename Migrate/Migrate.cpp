
// Migrate.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "Migrate.h"
#include "MigrateWnd.h"
#include "resource.h"


// CMigrateApp-Erstellung

CMigrateApp::CMigrateApp()
	: LFApplication(HasGUI_Standard)
{
}

CMigrateApp::~CMigrateApp()
{
}


// Das einzige CMigrateApp-Objekt

CMigrateApp theApp;


// CMigrateApp-Initialisierung

BOOL CMigrateApp::InitInstance()
{
	LFApplication::InitInstance();

	// Registry auslesen
	SetRegistryBase(_T("Settings"));
	m_DeleteImported = GetInt(_T("DeleteImported"), FALSE);
	m_Simulate = GetInt(_T("Simulate"), FALSE);

	m_pMainWnd = new CMigrateWnd();
	((CMigrateWnd*)m_pMainWnd)->Create();
	m_pMainWnd->ShowWindow(SW_SHOW);

	return TRUE;
}

int CMigrateApp::ExitInstance()
{
	WriteInt(_T("DeleteImported"), m_DeleteImported);
	WriteInt(_T("Simulate"), m_Simulate);

	return LFApplication::ExitInstance();
}
