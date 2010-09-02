
// Migrate.h: Hauptheaderdatei für die Migrations-Anwendung
//
#pragma once
#include "LFCommDlg.h"


// CMigrateApp:
// Siehe Migrate.cpp für die Implementierung dieser Klasse
//

class CMigrateApp : public LFApplication
{
public:
	CMigrateApp();

	virtual BOOL InitInstance();
	virtual int ExitInstance();

	LPITEMIDLIST GetNextItem(LPITEMIDLIST pidl);
	UINT GetByteSize(LPITEMIDLIST pidl);
	LPITEMIDLIST Clone(LPITEMIDLIST pidl);

	LPMALLOC p_Malloc;
	BOOL m_DeleteImported;
	BOOL m_Simulate;

protected:
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
};

extern CMigrateApp theApp;
