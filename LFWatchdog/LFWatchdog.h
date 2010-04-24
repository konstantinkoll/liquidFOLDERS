
// LFWatchdog.h: Hauptheaderdatei für die LFWatchdog-Anwendung
//
#include <Windows.h>

#pragma once
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// CWatchdogApp:
// Siehe LFWatchdog.cpp für die Implementierung dieser Klasse
//


class CWatchdogApp : public LFApplication
{
public:
	CWatchdogApp();

	CString path;

	virtual BOOL InitInstance();
	virtual int ExitInstance();

	void ErrorBox(UINT res);

protected:
	ULONG_PTR m_gdiplusToken;
	HANDLE sessionMutex;
};

extern CWatchdogApp theApp;
