
// FileDrop.h: Hauptheaderdatei für die FileDrop-Anwendung
//
#pragma once
#include "LFCommDlg.h"


// CFileDropApp:
// Siehe FileDrop.cpp für die Implementierung dieser Klasse
//

class CFileDropApp : public LFApplication
{
public:
	CFileDropApp();

	virtual BOOL InitInstance();
	virtual CWnd* OpenCommandLine(WCHAR* CmdLine=NULL);

protected:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CFileDropApp theApp;
