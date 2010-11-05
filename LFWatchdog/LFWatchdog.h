
// LFWatchdog.h: Hauptheaderdatei f�r die LFWatchdog-Anwendung
//

#pragma once
#include "LFCore.h"
#include <Windows.h>


// CWatchdogApp:
// Siehe LFWatchdog.cpp f�r die Implementierung dieser Klasse
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
