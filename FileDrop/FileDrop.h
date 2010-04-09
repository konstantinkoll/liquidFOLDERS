
// FileDrop.h: Hauptheaderdatei f�r die FileDrop-Anwendung
//
#pragma once
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#include "LFCommDlg.h"


// CFileDropApp:
// Siehe FileDrop.cpp f�r die Implementierung dieser Klasse
//

class CFileDropApp : public LFApplication
{
public:
	CFileDropApp();
	~CFileDropApp();

	virtual BOOL InitInstance();
};

extern CFileDropApp theApp;
