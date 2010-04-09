
// FileDrop.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "FileDrop.h"
#include "FileDropDlg.h"
#include "BFCore.h"


// CFileDropApp

BEGIN_MESSAGE_MAP(CFileDropApp, CWinAppEx)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CFileDropApp-Erstellung

CFileDropApp::CFileDropApp()
{
	// Anwendungspfad
	TCHAR szPathName[MAX_PATH];
	::GetModuleFileName(NULL, szPathName, MAX_PATH);
	LPTSTR pszFileName = _tcsrchr(szPathName, '\\')+1;
	*pszFileName = '\0';
	path = szPathName;
}


// Das einzige CFileDropApp-Objekt

CFileDropApp theApp;


// CFileDropApp-Initialisierung

BOOL CFileDropApp::InitInstance()
{
	// GDI+ initalisieren
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	// InitCommonControlsEx() ist für Windows XP erforderlich, wenn ein Anwendungsmanifest
	// die Verwendung von ComCtl32.dll Version 6 oder höher zum Aktivieren
	// von visuellen Stilen angibt. Ansonsten treten beim Erstellen von Fenstern Fehler auf.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Legen Sie dies fest, um alle allgemeinen Steuerelementklassen einzubeziehen,
	// die Sie in Ihrer Anwendung verwenden möchten.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinAppEx::InitInstance();

	SetRegistryKey(_T("BLUEfolders"));
	
	CFileDropDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	// Da das Dialogfeld geschlossen wurde, FALSE zurückliefern, sodass wir die
	//  Anwendung verlassen, anstatt das Nachrichtensystem der Anwendung zu starten.
	return FALSE;
}

int CFileDropApp::ExitInstance()
{
	CWinApp::ExitInstance();
	GdiplusShutdown(m_gdiplusToken);
	return 0;
}

void CFileDropApp::ErrorBox(UINT res)
{
	wchar_t* tmpStr = BFGetErrorText(res);
	MessageBox(NULL, tmpStr, L"Error", MB_OK | MB_ICONERROR);
	free(tmpStr);
}
