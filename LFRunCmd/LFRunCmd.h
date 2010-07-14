
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
	virtual int ExitInstance();

protected:
	void OnAppAbout();

	ULONG_PTR m_gdiplusToken;
};

extern CRunCmdApp theApp;
