
// Workers.cpp: Implementierung von Methoden zum Threading
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFDeleteStoreDlg.h"


// Workers

#define LF_WORKERTHREAD_START(pParam) LF_WORKERTHREAD_START_EX(pParam, 0);
#define LF_WORKERTHREAD_START_EX(pParam, MajorCount) CoInitialize(NULL); WorkerParameters* wp = (WorkerParameters*)pParam; LFProgress p; LFInitProgress(&p, wp->Hdr.hWnd);
#define LF_WORKERTHREAD_FINISH() LF_WORKERTHREAD_FINISH_EX(LFOk);
#define LF_WORKERTHREAD_FINISH_EX(Result) CoUninitialize(); PostMessage(wp->Hdr.hWnd, WM_COMMAND, (WPARAM)IDOK, NULL); return Result;

DWORD WINAPI WorkerStoreMaintenance(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	wp->MaintenanceList = LFStoreMaintenance(&p);

	LF_WORKERTHREAD_FINISH();
}

DWORD WINAPI WorkerStoreDelete(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	wp->Result = LFDeleteStore(wp->StoreID, &p);

	LF_WORKERTHREAD_FINISH();
}

DWORD WINAPI WorkerImportFromStore(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	LFTransactionImport(wp->StoreID, wp->TransactionList, wp->DeleteSource==TRUE, &p);

	LF_WORKERTHREAD_FINISH();
}

DWORD WINAPI WorkerImportFromWindows(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	LFTransactionImport(wp->StoreID, wp->FileImportList, wp->Template, TRUE, wp->DeleteSource==TRUE, &p);

	LF_WORKERTHREAD_FINISH();
}

DWORD WINAPI WorkerDelete(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	LFTransactionDelete(wp->TransactionList, FALSE, &p);

	LF_WORKERTHREAD_FINISH();
}


// Worker API

void LFDoWithProgress(LPTHREAD_START_ROUTINE pThreadProc, LFWorkerParameters* pParameters, CWnd* pParentWnd)
{
	LFProgressDlg dlg(pThreadProc, pParameters, pParentWnd);
	dlg.DoModal();
}

void LFImportFolder(CHAR* StoreID, CWnd* pParentWnd)
{
	// Allowed?
	if (LFGetApp()->ShowNagScreen(NAG_EXPIRED | NAG_FORCE, pParentWnd, TRUE))
		return;

	CString Caption((LPCSTR)IDS_IMPORTFOLDER_CAPTION);
	CString Hint((LPCSTR)IDS_IMPORTFOLDER_HINT);

	LFBrowseForFolderDlg dlg(pParentWnd, Caption, Hint, TRUE, TRUE, _T(""));
	if (dlg.DoModal()==IDOK)
	{
		WorkerParameters wp;
		ZeroMemory(&wp, sizeof(wp));
		wp.FileImportList = LFAllocFileImportList();
		LFAddImportPath(wp.FileImportList, dlg.m_FolderPath);
		strcpy_s(wp.StoreID, LFKeySize, StoreID);
		wp.DeleteSource = dlg.m_DeleteSource;

		// Template füllen
		wp.Template = LFAllocItemDescriptor();
		LFItemTemplateDlg tdlg(wp.Template, StoreID, pParentWnd, TRUE);
		if (tdlg.DoModal()!=IDCANCEL)
		{
			LFDoWithProgress(WorkerImportFromWindows, (LFWorkerParameters*)&wp, pParentWnd);
			LFErrorBox(pParentWnd, wp.FileImportList->m_LastError);
		}

		LFFreeItemDescriptor(wp.Template);
		LFFreeFileImportList(wp.FileImportList);
	}
}

void LFRunMaintenance(CWnd* pParentWnd)
{
	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));

	LFDoWithProgress(WorkerStoreMaintenance, &wp.Hdr, pParentWnd);
	LFErrorBox(pParentWnd, wp.MaintenanceList->m_LastError);

	LFStoreMaintenanceDlg dlg(wp.MaintenanceList, pParentWnd);
	dlg.DoModal();
}

void LFDeleteStore(CHAR* StoreID, CWnd* pParentWnd)
{
	LFStoreDescriptor Store;
	UINT Result = LFGetStoreSettings(StoreID, &Store);
	if (Result!=LFOk)
	{
		LFErrorBox(pParentWnd, Result);
		return;
	}

	// LFMessageBox
	CString Caption;
	Caption.Format(IDS_DELETESTORE_CAPTION, Store.StoreName);
	CString Msg((LPCSTR)IDS_DELETESTORE_MSG);

	if (LFMessageBox(pParentWnd, Msg, Caption, MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING)!=IDYES)
		return;

	// Dialogbox nur zeigen, wenn der Store gemountet ist
	if (LFIsStoreMounted(&Store))
	{
		LFDeleteStoreDlg dlg(Store.StoreID, pParentWnd);
		if (dlg.DoModal()!=IDOK)
			return;
	}

	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));
	strcpy_s(wp.StoreID, LFKeySize, Store.StoreID);

	LFDoWithProgress(WorkerStoreDelete, &wp.Hdr, pParentWnd);
	LFErrorBox(pParentWnd, wp.Result);
}
