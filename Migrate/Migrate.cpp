
// Migrate.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "Migrate.h"
#include "MigrateWnd.h"
#include "resource.h"


// CMigrateApp

BEGIN_MESSAGE_MAP(CMigrateApp, LFApplication)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()


// CMigrateApp-Erstellung

CMigrateApp::CMigrateApp()
	: LFApplication(HasGUI_Standard)
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

void CMigrateApp::OnAppAbout()
{
	LFAboutDlgParameters p;
	ENSURE(p.AppName.LoadString(IDR_APPLICATION));
	p.Build = __TIMESTAMP__;
	p.Icon = new CGdiPlusBitmapResource();
	p.Icon->Load(IDB_ABOUTICON, _T("PNG"), AfxGetResourceHandle());

	LFAboutDlg dlg(&p, m_pMainWnd);
	dlg.DoModal();

	delete p.Icon;
}
