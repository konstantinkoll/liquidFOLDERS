
// RunCmd.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "LFRunCmd.h"
#include "..\\LFCore\\resource.h"
#include "LFCore.h"
#include "LFCommDlg.h"


// CRunCmdApp-Erstellung

CRunCmdApp::CRunCmdApp()
{
}

CRunCmdApp::~CRunCmdApp()
{
}


// Das einzige CRunCmdApp-Objekt

CRunCmdApp theApp;


// CRunCmdApp-Initialisierung

BOOL CRunCmdApp::InitInstance()
{
	LFApplication::InitInstance();

	OnAppAbout();

	// Rufen Sie DragAcceptFiles nur auf, wenn ein Suffix vorhanden ist.
	//  In einer SDI-Anwendung ist dies nach ProcessShellCommand erforderlich
	return TRUE;
}

int CRunCmdApp::ExitInstance()
{
	CWinApp::ExitInstance();
	GdiplusShutdown(m_gdiplusToken);
	return 0;
}

void CRunCmdApp::OnAppAbout()
{
	LFAboutDlgParameters p;
	p.appname = "RunCmd";
	p.build = __TIMESTAMP__;
	ENSURE(p.caption.LoadString(IDS_ABOUT));
	p.icon = new CGdiPlusBitmapResource();
	p.icon->Load(IDB_ABOUTICON, _T("PNG"), AfxGetInstanceHandle());
	p.TextureSize = -1;
	p.RibbonColor = ID_VIEW_APPLOOK_OFF_2007_NONE;
	p.HideEmptyDrives = -1;
	p.HideEmptyDomains = -1;

	LFAboutDlg dlg(&p, m_pActiveWnd);
	dlg.DoModal();

	delete p.icon;
}
