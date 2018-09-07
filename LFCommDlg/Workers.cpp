
// Workers.cpp: Implementierung von Methoden zum Threading
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFDeleteStoreDlg.h"


// Workers

#define LF_WORKERTHREAD_START(pParam) CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE); LFProgress Progress; LFInitProgress(Progress, ((LFWorkerParameters*)pParam)->hWnd);
#define LF_WORKERTHREAD_FINISH(pParam) CoUninitialize(); PostMessage(((LFWorkerParameters*)pParam)->hWnd, WM_COMMAND, (WPARAM)IDOK, NULL); return ((LFWorkerParameters*)pParam)->Result;

DWORD WINAPI WorkerCreateStoreWindows(LPVOID lpParam)
{
	LF_WORKERTHREAD_START(lpParam);

	WorkerCreateStoreWindowsParameters* pParameters = (WorkerCreateStoreWindowsParameters*)lpParam;
	pParameters->Hdr.Result = LFCreateStoreWindows(pParameters->Path, pParameters->StoreName, &Progress);

	LF_WORKERTHREAD_FINISH(lpParam);
}

DWORD WINAPI WorkerSynchronizeStores(LPVOID lpParam)
{
	LF_WORKERTHREAD_START(lpParam);

	WorkerSynchronizeStoresParameters* pParameters = (WorkerSynchronizeStoresParameters*)lpParam;
	pParameters->Hdr.Result = LFSynchronizeStores(pParameters->StoreID, &Progress);

	LF_WORKERTHREAD_FINISH(lpParam);
}

DWORD WINAPI WorkerStoreMaintenance(LPVOID lpParam)
{
	LF_WORKERTHREAD_START(lpParam);

	WorkerStoreMaintenanceParameters* pParameters = (WorkerStoreMaintenanceParameters*)lpParam;
	pParameters->Hdr.Result = (pParameters->pMaintenanceList=LFScheduledMaintenance(&Progress))->m_LastError;

	LF_WORKERTHREAD_FINISH(lpParam);
}

DWORD WINAPI WorkerStoreDelete(LPVOID lpParam)
{
	LF_WORKERTHREAD_START(lpParam);

	WorkerStoreDeleteParameters* pParameters = (WorkerStoreDeleteParameters*)lpParam;
	pParameters->Hdr.Result = LFDeleteStore(pParameters->StoreID, &Progress);

	LF_WORKERTHREAD_FINISH(lpParam);
}

DWORD WINAPI WorkerSendTo(LPVOID lpParam)
{
	LF_WORKERTHREAD_START(lpParam);

	WorkerSendToParameters* pParameters = (WorkerSendToParameters*)lpParam;
	pParameters->Hdr.Result = LFDoTransaction(pParameters->pTransactionList, LFTransactionSendTo, &Progress, (UINT_PTR)&pParameters->StoreID);

	LF_WORKERTHREAD_FINISH(lpParam);
}

DWORD WINAPI WorkerImport(LPVOID lpParam)
{
	LF_WORKERTHREAD_START(lpParam);

	WorkerImportParameters* pParameters = (WorkerImportParameters*)lpParam;
	pParameters->Hdr.Result = LFDoFileImport(pParameters->pFileImportList, TRUE, pParameters->StoreID, pParameters->pItemTemplate, pParameters->DeleteSource, &Progress);

	LF_WORKERTHREAD_FINISH(lpParam);
}

DWORD WINAPI WorkerDelete(LPVOID lpParam)
{
	LF_WORKERTHREAD_START(lpParam);

	WorkerDeleteParameters* pParameters = (WorkerDeleteParameters*)lpParam;
	pParameters->Hdr.Result = LFDoTransaction(pParameters->pTransactionList, LFTransactionDelete, &Progress);

	LF_WORKERTHREAD_FINISH(lpParam);
}


// Worker API

void LFDoWithProgress(LPTHREAD_START_ROUTINE pThreadProc, LFWorkerParameters* pParameters, CWnd* pParentWnd)
{
	LFProgressDlg(pThreadProc, pParameters, pParentWnd).DoModal();
}

void LFRunSynchronizeStores(const STOREID& StoreID, CWnd* pParentWnd)
{
	// Allowed?
	if (!LFNagScreen(pParentWnd))
		return;

	// Run
	WorkerSynchronizeStoresParameters Parameters;
	ZeroMemory(&Parameters, sizeof(Parameters));
	Parameters.StoreID = StoreID;

	LFDoWithProgress(WorkerSynchronizeStores, &Parameters.Hdr, pParentWnd);
	LFErrorBox(pParentWnd, Parameters.Hdr.Result);
}

void LFRunStoreMaintenance(CWnd* pParentWnd)
{
	// Run
	WorkerStoreMaintenanceParameters Parameters;
	ZeroMemory(&Parameters, sizeof(Parameters));

	LFDoWithProgress(WorkerStoreMaintenance, &Parameters.Hdr, pParentWnd);
	LFErrorBox(pParentWnd, Parameters.Hdr.Result);

	LFStoreMaintenanceDlg(Parameters.pMaintenanceList, pParentWnd).DoModal();
}

void LFDeleteStore(const ABSOLUTESTOREID& StoreID, CWnd* pParentWnd)
{
	LFStoreDescriptor Victim;
	UINT Result;
	if ((Result=LFGetStoreSettings(StoreID, Victim))!=LFOk)
	{
		LFErrorBox(pParentWnd, Result);
		return;
	}

	// LFMessageBox
	CString Caption;
	Caption.Format(IDS_DELETESTORE_CAPTION, Victim.StoreName);

	if (LFMessageBox(pParentWnd, CString((LPCSTR)IDS_DELETESTORE_MSG), Caption, MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING)!=IDYES)
		return;
	// Only show dialog when store is mounted
	if (LFIsStoreMounted(&Victim) && (LFDeleteStoreDlg(Victim.StoreID, pParentWnd).DoModal()!=IDOK))
		return;

	// Run
	WorkerStoreDeleteParameters Parameters;
	ZeroMemory(&Parameters, sizeof(Parameters));
	Parameters.StoreID = Victim.StoreID;

	LFDoWithProgress(WorkerStoreDelete, &Parameters.Hdr, pParentWnd);
	LFErrorBox(pParentWnd, Parameters.Hdr.Result);
}
