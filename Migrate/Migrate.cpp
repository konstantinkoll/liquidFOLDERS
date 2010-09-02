
// Migrate.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "Migrate.h"
#include "MigrateWnd.h"
#include "resource.h"


// CMigrateApp

BEGIN_MESSAGE_MAP(CMigrateApp, LFApplication)
	ON_COMMAND(ID_APP_ABOUT, OnAppAbout)
END_MESSAGE_MAP()


// CMigrateApp-Erstellung

CMigrateApp::CMigrateApp()
	: LFApplication(HasGUI_Standard)
{
	SHGetMalloc(&p_Malloc);
}

// Das einzige CMigrateApp-Objekt

CMigrateApp theApp;


// CMigrateApp-Initialisierung

BOOL CMigrateApp::InitInstance()
{
	LFApplication::InitInstance();

	// Registry auslesen
	SetRegistryBase(_T("Settings"));
	m_DeleteImported = GetInt(_T("DeleteImported"), FALSE);
	m_Simulate = GetInt(_T("Simulate"), FALSE);

	m_pMainWnd = new CMigrateWnd();
	((CMigrateWnd*)m_pMainWnd)->Create();
	m_pMainWnd->ShowWindow(SW_SHOW);

	return TRUE;
}

int CMigrateApp::ExitInstance()
{
	WriteInt(_T("DeleteImported"), m_DeleteImported);
	WriteInt(_T("Simulate"), m_Simulate);

	return LFApplication::ExitInstance();
}

void CMigrateApp::OnAppAbout()
{
	LFAboutDlgParameters p;
	ENSURE(p.appname.LoadString(IDR_APPLICATION));
	p.build = __TIMESTAMP__;
	p.icon = new CGdiPlusBitmapResource();
	p.icon->Load(IDB_ABOUTICON, _T("PNG"), AfxGetResourceHandle());
	p.TextureSize = -1;
	p.RibbonColor = ID_VIEW_APPLOOK_OFF_2007_NONE;
	p.HideEmptyDrives = -1;
	p.HideEmptyDomains = -1;

	LFAboutDlg dlg(&p, m_pMainWnd);
	dlg.DoModal();

	delete p.icon;
}

LPITEMIDLIST CMigrateApp::GetNextItem(LPITEMIDLIST pidl)
{
	if (!pidl)
		return NULL;

	return (LPITEMIDLIST)(LPBYTE)(((LPBYTE)pidl)+pidl->mkid.cb);
}

UINT CMigrateApp::GetByteSize(LPITEMIDLIST pidl)
{
	if (!pidl)
		return 0;

	UINT Size = 0;
	while (pidl->mkid.cb)
	{
		Size += pidl->mkid.cb;
		pidl = GetNextItem(pidl);
	}

	return Size+sizeof(ITEMIDLIST);
}

LPITEMIDLIST CMigrateApp::Clone(LPITEMIDLIST pidl)
{
	UINT cb = GetByteSize(pidl);

	LPITEMIDLIST pidlcopy = (LPITEMIDLIST)p_Malloc->Alloc(cb);
	if (pidlcopy)
		CopyMemory(pidlcopy, pidl, cb);

	return pidlcopy;
}
