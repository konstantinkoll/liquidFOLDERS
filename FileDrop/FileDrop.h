
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
	~CFileDropApp();

	virtual BOOL InitInstance();

	UINT WakeupMsg;
};

extern CFileDropApp theApp;
