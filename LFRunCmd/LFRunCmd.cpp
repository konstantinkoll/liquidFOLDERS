
// LFRunCmd.cpp: Definiert das Klassenverhalten für die Anwendung.
//

#include "stdafx.h"
#include "afxwinappex.h"
#include "LFRunCmd.h"
#include "..\\LFCore\\resource.h"
#include "LFCore.h"
#include "LFCommDlg.h"


// Thread workers
//

struct WorkerParameters
{
	LFWorkerParameters Hdr;
	BOOL AllStores;
	CHAR StoreID[LFKeySize];
	HWND hWndSource;
	LFMaintenanceList* MaintenanceList;
};

DWORD WINAPI WorkerMaintenance(void* lParam)
{
	CoInitialize(NULL);
	WorkerParameters* wp = (WorkerParameters*)lParam;

	LFProgress p;
	LFInitProgress(&p, wp->Hdr.hWnd);

	wp->MaintenanceList = wp->AllStores ? LFStoreMaintenance(wp->hWndSource, &p) : LFStoreMaintenance(wp->StoreID, wp->hWndSource, &p);

	CoUninitialize();
	PostMessage(wp->Hdr.hWnd, WM_COMMAND, (WPARAM)IDOK, NULL);
	return 0;
}


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
	LFApplication::InitInstance();

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
			if (command==_T("BACKUP"))
				LFBackupStores(CWnd::GetForegroundWindow());
			if (command==_T("NEWSTORE"))
				OnStoresCreate();
			if (command==_T("MAINTAINALL"))
				OnStoresMaintainAll();
			if (command==_T("INSTALL"))
				LFCreateSendTo(true);
			break;
		case 3:
			if (command==_T("NEWSTOREVOLUME"))
				OnStoresCreateVolume(*__wargv[2] & 0xFF);
			if (command==_T("DELETESTORE"))
				OnStoreDelete(__wargv[2]);
			if (command==_T("IMPORTFOLDER"))
				OnStoreImportFolder(__wargv[2]);
			if (command==_T("MAINTAIN"))
				OnStoreMaintain(__wargv[2]);
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

void CRunCmdApp::OnStoresCreate()
{
	LFStoreDescriptor* s = LFAllocStoreDescriptor();

	LFStoreNewDlg dlg(CWnd::GetForegroundWindow(), s);
	if (dlg.DoModal()==IDOK)
		LFErrorBox(LFCreateStore(s, dlg.MakeDefault));

	LFFreeStoreDescriptor(s);
}

void CRunCmdApp::OnStoresCreateVolume(CHAR Drive)
{
	LFStoreDescriptor* s = LFAllocStoreDescriptor();

	LFStoreNewVolumeDlg dlg(CWnd::GetForegroundWindow(), Drive, s);
	if (dlg.DoModal()==IDOK)
		LFErrorBox(LFCreateStore(s, FALSE));

	LFFreeStoreDescriptor(s);
}

void CRunCmdApp::OnStoresMaintainAll()
{
	CWnd* pWndForeground = CWnd::GetForegroundWindow();

	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));
	wp.AllStores = TRUE;

	LFDoWithProgress(WorkerMaintenance, &wp.Hdr, pWndForeground);
	LFErrorBox(wp.MaintenanceList->m_LastError);

	LFStoreMaintenanceDlg dlg(wp.MaintenanceList, pWndForeground);
	dlg.DoModal();

	LFFreeMaintenanceList(wp.MaintenanceList);
}

void CRunCmdApp::OnStoreDelete(CString ID)
{
	CHAR StoreID[LFKeySize];
	WideCharToMultiByte(CP_ACP, 0, ID, -1, StoreID, LFKeySize, NULL, NULL);

	LFStoreDescriptor* store = LFAllocStoreDescriptor();
	UINT res = LFGetStoreSettings(StoreID, store);

	if (res==LFOk)
		res = theApp.DeleteStore(store, CWnd::GetForegroundWindow());

	LFFreeStoreDescriptor(store);
	LFErrorBox(res);
}

void CRunCmdApp::OnStoreImportFolder(CString ID)
{
	CHAR StoreID[LFKeySize];
	WideCharToMultiByte(CP_ACP, 0, ID, -1, StoreID, LFKeySize, NULL, NULL);

	LFImportFolder(StoreID, CWnd::GetForegroundWindow());
}

void CRunCmdApp::OnStoreMaintain(CString ID)
{
	CWnd* pWndForeground = CWnd::GetForegroundWindow();

	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));
	WideCharToMultiByte(CP_ACP, 0, ID, -1, wp.StoreID, LFKeySize, NULL, NULL);

	LFDoWithProgress(WorkerMaintenance, &wp.Hdr, pWndForeground);
	LFErrorBox(wp.MaintenanceList->m_LastError);

	LFStoreMaintenanceDlg dlg(wp.MaintenanceList, pWndForeground);
	dlg.DoModal();

	LFFreeMaintenanceList(wp.MaintenanceList);
}

void CRunCmdApp::OnStoreProperties(CString ID)
{
	CHAR StoreID[LFKeySize];
	WideCharToMultiByte(CP_ACP, 0, ID, -1, StoreID, LFKeySize, NULL, NULL);

	LFStorePropertiesDlg dlg(StoreID, CWnd::GetForegroundWindow());
	dlg.DoModal();
}
