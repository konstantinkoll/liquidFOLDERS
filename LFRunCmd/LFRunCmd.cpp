
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
	: LFApplication(FALSE)
{
}


// Das einzige CRunCmdApp-Objekt

CRunCmdApp theApp;


// CRunCmdApp-Initialisierung

BOOL CRunCmdApp::InitInstance()
{
	if (!LFApplication::InitInstance())
		return FALSE;

	if (__argc)
	{
		CString command(__targv[1]);
		command.MakeUpper();

		switch (__argc)
		{
		case 1:
			OnAppAbout(IDS_ABOUT, IDB_ABOUTICON);
			break;
		case 2:
			if (command==_T("ABOUT"))
				OnAppAbout(IDS_ABOUT, IDB_ABOUTICON);
			if (command==_T("ABOUTEXTENSION"))
				OnAppAbout(IDS_EXTENSIONABOUT, IDB_EXTENSIONABOUTICON);
			if (command==_T("NEWSTORE"))
				OnStoreCreate();
			if (command==_T("INSTALL"))
				LFCreateSendTo(true);
			if (command==_T("CHECKUPDATE"))
				LFCheckForUpdate();
			break;
		case 3:
			if (command==_T("NEWSTOREVOLUME"))
				OnStoreCreate(*__wargv[2] & 0xFF);
			if (command==_T("DELETESTORE"))
				OnStoreDelete(__wargv[2]);
			if (command==_T("IMPORTFOLDER"))
				OnStoreImportFolder(__wargv[2]);
			if (command==_T("STOREPROPERTIES"))
				OnStoreProperties(__wargv[2]);
		}
	}

	return TRUE;
}

void CRunCmdApp::OnAppAbout(UINT ResIDName, UINT ResIDPicture)
{
	CString AppName;
	ENSURE(AppName.LoadString(ResIDName));
	TIMESTAMP;
	LFAbout(AppName, Timestamp, ResIDPicture, CWnd::GetForegroundWindow());
}

void CRunCmdApp::OnStoreCreate(CHAR Drive)
{
	LFCreateNewStore(CWnd::GetForegroundWindow(), Drive);
}

void CRunCmdApp::OnStoreDelete(CString ID)
{
	CHAR StoreID[LFKeySize];
	WideCharToMultiByte(CP_ACP, 0, ID, -1, StoreID, LFKeySize, NULL, NULL);

	LFStoreDescriptor store;
	UINT res = LFGetStoreSettings(StoreID, &store);
	if (res==LFOk)
		res = theApp.DeleteStore(&store, CWnd::GetForegroundWindow());

	LFErrorBox(res, GetForegroundWindow());
}

void CRunCmdApp::OnStoreImportFolder(CString ID)
{
	CHAR StoreID[LFKeySize];
	WideCharToMultiByte(CP_ACP, 0, ID, -1, StoreID, LFKeySize, NULL, NULL);

	LFImportFolder(StoreID, CWnd::GetForegroundWindow());
}

void CRunCmdApp::OnStoreProperties(CString ID)
{
	CHAR StoreID[LFKeySize];
	WideCharToMultiByte(CP_ACP, 0, ID, -1, StoreID, LFKeySize, NULL, NULL);

	LFStorePropertiesDlg dlg(StoreID, CWnd::GetForegroundWindow());
	dlg.DoModal();
}
