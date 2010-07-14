
// RunCmd.h: Hauptheaderdatei f�r die RunCmd-Anwendung
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
	void OnAppAbout();
};

extern CRunCmdApp theApp;
