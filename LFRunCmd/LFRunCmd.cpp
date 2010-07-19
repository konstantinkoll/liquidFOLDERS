
// LFRunCmd.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "LFRunCmd.h"
#include "..\\LFCore\\resource.h"
#include "LFCore.h"
#include "LFCommDlg.h"


// CRunCmdApp-Erstellung

CRunCmdApp::CRunCmdApp()
	: LFApplication(HasGUI_None)
{
}

CRunCmdApp::~CRunCmdApp()
{
}


// Das einzige CRunCmdApp-Objekt

CRunCmdApp theApp;


// CRunCmdApp-Initialisierung

BOOL CRunCmdApp::InitInstance()
{
	LFApplication::InitInstance();

	if (__argc)
	{
		CString command(__targv[1]);
		command.MakeUpper();

		switch (__argc)
		{
		case 2:
			if (command==_T("ABOUT"))
				OnAppAbout(IDS_ABOUT, IDB_ABOUTICON);
			if (command==_T("ABOUTEXTENSION"))
				OnAppAbout(IDS_EXTENSIONABOUT, IDB_EXTENSIONABOUTICON);
			if (command==_T("NEWSTORE"))
				OnStoreCreate();
			break;
		case 3:
			if (command==_T("DELETESTORE"))
				OnStoreDelete(__targv[2]);
			if (command==_T("STOREPROPERTIES"))
				OnStoreProperties(__targv[2]);
		}
	}

	return TRUE;
}

void CRunCmdApp::OnAppAbout(UINT ResIDName, UINT ResIDPicture)
{
	LFAboutDlgParameters p;
	ENSURE(p.appname.LoadString(ResIDName));
	p.build = __TIMESTAMP__;
	p.icon = new CGdiPlusBitmapResource();
	p.icon->Load(ResIDPicture, _T("PNG"), AfxGetResourceHandle());
	p.TextureSize = -1;
	p.RibbonColor = ID_VIEW_APPLOOK_OFF_2007_NONE;
	p.HideEmptyDrives = -1;
	p.HideEmptyDomains = -1;

	LFAboutDlg dlg(&p, CWnd::GetForegroundWindow());
	dlg.DoModal();

	delete p.icon;
}

void CRunCmdApp::OnStoreCreate()
{
	LFStoreDescriptor* s = LFAllocStoreDescriptor();

	LFStoreNewDlg dlg(CWnd::GetForegroundWindow(), IDD_STORENEW, '\0', s);
	if (dlg.DoModal()==IDOK)
		LFErrorBox(LFCreateStore(s, dlg.makeDefault));

	LFFreeStoreDescriptor(s);
}

void CRunCmdApp::OnStoreDelete(CString ID)
{
	char StoreID[LFKeySize];
	wcstombs_s(NULL, StoreID, ID, LFKeySize);

	LFStoreDescriptor* store = LFAllocStoreDescriptor();
	UINT res = LFGetStoreSettings(StoreID, store);

	if (res==LFOk)
		res = theApp.DeleteStore(store, CWnd::GetForegroundWindow());

	LFFreeStoreDescriptor(store);
	LFErrorBox(res);
}

void CRunCmdApp::OnStoreProperties(CString ID)
{
	char StoreID[LFKeySize];
	wcstombs_s(NULL, StoreID, ID, LFKeySize);

	LFStorePropertiesDlg dlg(StoreID, CWnd::GetForegroundWindow());
	dlg.DoModal();
}
