
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

	UINT m_WakeupMsg;

protected:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CFileDropApp theApp;
