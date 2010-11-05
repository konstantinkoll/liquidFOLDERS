
// LFWatchdog.h: Hauptheaderdatei für die LFWatchdog-Anwendung
//

#pragma once
#include "LFCore.h"
#include <Windows.h>


// CWatchdogApp:
// Siehe LFWatchdog.cpp für die Implementierung dieser Klasse
//

class CWatchdogApp : public LFApplication
{
public:
	CWatchdogApp();

	virtual BOOL InitInstance();
	virtual INT ExitInstance();

protected:
	HANDLE sessionMutex;
};

extern CWatchdogApp theApp;
