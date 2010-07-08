
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

	CString path;
	LFMessageIDs* p_MessageIDs;

	virtual BOOL InitInstance();
	virtual int ExitInstance();

	void ErrorBox(UINT res);

protected:
	ULONG_PTR m_gdiplusToken;
	HANDLE sessionMutex;
};

extern CWatchdogApp theApp;
