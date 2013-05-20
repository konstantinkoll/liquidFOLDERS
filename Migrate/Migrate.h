
// Migrate.h: Hauptheaderdatei für die Migrations-Anwendung
//

#pragma once
#include "LFCommDlg.h"
#include "MigrateWnd.h"


// CMigrateApp:
// Siehe Migrate.cpp für die Implementierung dieser Klasse
//

class CMigrateApp : public LFApplication
{
public:
	CMigrateApp();

	virtual BOOL InitInstance();
	virtual CWnd* OpenCommandLine(WCHAR* CmdLine=NULL);

	void AddFrame(CMigrateWnd* pFrame);
	void KillFrame(CMigrateWnd* pVictim);

	CList<CMigrateWnd*> m_MainFrames;
	BOOL m_ExpandAll;

protected:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMigrateApp theApp;
