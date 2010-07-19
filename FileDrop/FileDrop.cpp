
// FileDrop.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "FileDrop.h"
#include "FileDropWnd.h"


// CFileDropApp-Erstellung

CFileDropApp::CFileDropApp()
	: LFApplication(HasGUI_Standard)
{
}

CFileDropApp::~CFileDropApp()
{
}


// Das einzige CFileDropApp-Objekt

CFileDropApp theApp;


// CFileDropApp-Initialisierung

BOOL CFileDropApp::InitInstance()
{
	LFApplication::InitInstance();

	SetRegistryBase(_T("Settings"));

	m_pMainWnd = new CFileDropWnd();
	((CFileDropWnd*)m_pMainWnd)->Create();
	m_pMainWnd->ShowWindow(SW_SHOW);

	return TRUE;
}
