
// Migrate.h: Hauptheaderdatei f�r die Migrations-Anwendung
//
#pragma once
#include "LFCommDlg.h"


// CMigrateApp:
// Siehe Migrate.cpp f�r die Implementierung dieser Klasse
//

class CMigrateApp : public LFApplication
{
public:
	CMigrateApp();
	virtual ~CMigrateApp();

	BOOL m_DeleteImported;
	BOOL m_Simulate;

	virtual BOOL InitInstance();
	virtual int ExitInstance();
};

extern CMigrateApp theApp;
