
// LFRunCmd.h: Hauptheaderdatei f�r die RunCmd-Anwendung
//

#pragma once
#include "resource.h"
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"


// CRunCmdApp:
// Siehe RunCmd.cpp f�r die Implementierung dieser Klasse
//

class CRunCmdApp : public LFApplication
{
public:
	CRunCmdApp();
	virtual ~CRunCmdApp();

	virtual BOOL InitInstance();

protected:
	void OnAppAbout(UINT ResIDName, UINT ResIDPicture);
	void OnStoreCreate();
	void OnStoreCreateDrive(CHAR Drive);
	void OnStoreDelete(CString ID);
	void OnImportFolder(CString ID);
	void OnStoreProperties(CString ID);
};

extern CRunCmdApp theApp;
