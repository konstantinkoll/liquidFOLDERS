
// Migrate.h: Hauptheaderdatei für die Migrations-Anwendung
//
#pragma once
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#include "LFCommDlg.h"

#define SaveMode_SettingsChanged  0
#define SaveMode_Force            1
#define SaveMode_FlagOnly         2


// CMigrateApp:
// Siehe Migrate.cpp für die Implementierung dieser Klasse
//

class CMigrateApp : public LFApplication
{
public:
	CMigrateApp();
	virtual ~CMigrateApp();

	BOOL m_GrannyMode;
	BOOL m_DeleteImported;
	BOOL m_Simulate;

	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual void SetApplicationLook(UINT nID);

private:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

	void GetMaxTextureSize();
};

extern CMigrateApp theApp;
