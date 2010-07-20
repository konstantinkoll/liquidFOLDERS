
// LFRunCmd.h: Hauptheaderdatei für die RunCmd-Anwendung
//

#pragma once
#include "resource.h"
#include "liquidFOLDERS.h"
#include "LFCommDlg.h"


// CRunCmdApp:
// Siehe RunCmd.cpp für die Implementierung dieser Klasse
//

class CRunCmdApp : public LFApplication
{
public:
	CRunCmdApp();
	virtual ~CRunCmdApp();

	virtual BOOL InitInstance();

protected:
	void OnAppAbout(UINT ResIDName, UINT ResIDPicture);
	void OnStoreCreate(UINT ResID, char Drive='\0');
	void OnStoreDelete(CString ID);
	void OnStoreProperties(CString ID);
};

extern CRunCmdApp theApp;
