
// FileDrop.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "FileDrop.h"
#include "FileDropDlg.h"


// CFileDropApp-Erstellung

CFileDropApp::CFileDropApp()
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

	CFileDropDlg dlg;
	dlg.DoModal();

	return FALSE;
}
