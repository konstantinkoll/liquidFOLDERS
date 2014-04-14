
// LFRunCmd.h: Hauptheaderdatei f�r die RunCmd-Anwendung
//

#pragma once
#include "resource.h"
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"


// CRunCmdApp:
// Siehe LFRunCmd.cpp f�r die Implementierung dieser Klasse
//

class CRunCmdApp : public LFApplication
{
public:
	CRunCmdApp();

	virtual BOOL InitInstance();

protected:
	void OnAppAbout();
	void OnStoreCreate(CHAR Drive='\0');
	void OnStoreDelete(CString ID);
	void OnStoreImportFolder(CString ID);
	void OnStoreProperties(CString ID);
};

extern CRunCmdApp theApp;
