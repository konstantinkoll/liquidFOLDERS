
// Migrate.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#pragma once
#include "stdafx.h"
#include "afxwinappex.h"
#include "Migrate.h"
#include "MainFrm.h"
#include "..\\LFCore\\resource.h"
#include "LFCore.h"
#include "LFCommDlg.h"
#include <list>


// CMigrateApp

BEGIN_MESSAGE_MAP(CMigrateApp, LFApplication)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()


// CMigrateApp-Erstellung

CMigrateApp::CMigrateApp()
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
	CString oldBase = GetRegistryBase();
	SetRegistryBase(_T("Settings"));
	m_GrannyMode = GetInt(_T("GrannyMode"), FALSE);
	m_DeleteImported = GetInt(_T("DeleteImported"), FALSE);
	m_Simulate = GetInt(_T("Simulate"), FALSE);
	SetRegistryBase(oldBase);

	m_pMainWnd = new CMainFrame();
	((CMainFrame*)m_pMainWnd)->LoadFrame(IDR_MAINFRAME, WS_OVERLAPPEDWINDOW);
	m_pMainWnd->ShowWindow(SW_SHOW);

	// Rufen Sie DragAcceptFiles nur auf, wenn ein Suffix vorhanden ist.
	//  In einer SDI-Anwendung ist dies nach ProcessShellCommand erforderlich
	return TRUE;
}

int CMigrateApp::ExitInstance()
{
	CString oldBase = GetRegistryBase();
	SetRegistryBase(_T("Settings"));
	WriteInt(_T("GrannyMode"), m_GrannyMode);
	WriteInt(_T("DeleteImported"), m_DeleteImported);
	WriteInt(_T("Simulate"), m_Simulate);
	SetRegistryBase(oldBase);

	return LFApplication::ExitInstance();
}

void CMigrateApp::SetApplicationLook(UINT nID)
{
	LFApplication::SetApplicationLook(nID);

	if (m_pMainWnd)
		((CMainFrame*)m_pMainWnd)->RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);
}

void CMigrateApp::OnAppAbout()
{
	LFAboutDlgParameters p;
	p.appname = "Migration Wizard";
	p.build = __TIMESTAMP__;
	p.icon = new CGdiPlusBitmapResource();
	p.icon->Load(IDB_ABOUTICON, _T("PNG"), AfxGetInstanceHandle());
	p.TextureSize = -1;
	p.RibbonColor = m_nAppLook;
	p.HideEmptyDrives = -1;
	p.HideEmptyDomains = -1;

	LFAboutDlg dlg(&p, m_pActiveWnd);
	dlg.DoModal();

	delete p.icon;
}
