
// Workers.cpp: Implementierung von Methoden zum Threading
//

#include "stdafx.h"
#include "LFCommDlg.h"
#include "LFDeleteStoreDlg.h"


// Workers

#define LF_WORKERTHREAD_START(pParam) LF_WORKERTHREAD_START_EX(pParam, 0);
#define LF_WORKERTHREAD_START_EX(pParam, MajorCount) CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE); WorkerParameters* wp = (WorkerParameters*)pParam; LFProgress p; LFInitProgress(&p, wp->Hdr.hWnd);
#define LF_WORKERTHREAD_FINISH() LF_WORKERTHREAD_FINISH_EX(LFOk);
#define LF_WORKERTHREAD_FINISH_EX(Result) CoUninitialize(); PostMessage(wp->Hdr.hWnd, WM_COMMAND, (WPARAM)IDOK, NULL); return Result;

DWORD WINAPI WorkerCreateStoreWindows(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	wp->Result = LFCreateStoreWindows(wp->Path, wp->StoreName, &p);

	LF_WORKERTHREAD_FINISH();
}

DWORD WINAPI WorkerStoreSynchronize(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	wp->Result = LFSynchronizeStore(wp->StoreID, &p);

	LF_WORKERTHREAD_FINISH();
}

DWORD WINAPI WorkerStoreSynchronizeAll(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	wp->Result = LFSynchronizeStores(&p);

	LF_WORKERTHREAD_FINISH();
}

DWORD WINAPI WorkerStoreMaintenance(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	wp->pMaintenanceList = LFScheduledMaintenance(&p);

	LF_WORKERTHREAD_FINISH();
}

DWORD WINAPI WorkerStoreDelete(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	wp->Result = LFDeleteStore(wp->StoreID, &p);

	LF_WORKERTHREAD_FINISH();
}

DWORD WINAPI WorkerSendTo(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	LFDoTransaction(wp->pTransactionList, LFTransactionTypeSendTo, &p, (UINT_PTR)wp->StoreID);

	LF_WORKERTHREAD_FINISH();
}

DWORD WINAPI WorkerImport(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	LFDoFileImport(wp->pFileImportList, TRUE, wp->StoreID, wp->pItemTemplate, wp->DeleteSource, &p);

	LF_WORKERTHREAD_FINISH();
}

DWORD WINAPI WorkerDelete(void* lParam)
{
	LF_WORKERTHREAD_START(lParam);

	LFDoTransaction(wp->pTransactionList, LFTransactionTypeDelete, &p);

	LF_WORKERTHREAD_FINISH();
}


// Worker API

void LFDoWithProgress(LPTHREAD_START_ROUTINE pThreadProc, LFWorkerParameters* pParameters, CWnd* pParentWnd)
{
	LFProgressDlg(pThreadProc, pParameters, pParentWnd).DoModal();
}

void LFRunSynchronize(const LPCSTR pStoreID, CWnd* pParentWnd)
{
	// Allowed?
	if (!LFNagScreen(pParentWnd))
		return;

	// Run
	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));
	strcpy_s(wp.StoreID, LFKeySize, pStoreID);

	LFDoWithProgress(WorkerStoreSynchronize, &wp.Hdr, pParentWnd);
	LFErrorBox(pParentWnd, wp.Result);
}

void LFRunSynchronizeAll(CWnd* pParentWnd)
{
	// Allowed?
	if (!LFNagScreen(pParentWnd))
		return;

	// Run
	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));

	LFDoWithProgress(WorkerStoreSynchronizeAll, &wp.Hdr, pParentWnd);
	LFErrorBox(pParentWnd, wp.Result);
}

void LFRunMaintenance(CWnd* pParentWnd)
{
	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));

	LFDoWithProgress(WorkerStoreMaintenance, &wp.Hdr, pParentWnd);
	LFErrorBox(pParentWnd, wp.pMaintenanceList->m_LastError);

	LFStoreMaintenanceDlg(wp.pMaintenanceList, pParentWnd).DoModal();
}

void LFDeleteStore(const LPCSTR pStoreID, CWnd* pParentWnd)
{
	LFStoreDescriptor Store;
	UINT Result = LFGetStoreSettings(pStoreID, Store);
	if (Result!=LFOk)
	{
		LFErrorBox(pParentWnd, Result);
		return;
	}

	// LFMessageBox
	CString Caption;
	Caption.Format(IDS_DELETESTORE_CAPTION, Store.StoreName);

	if (LFMessageBox(pParentWnd, CString((LPCSTR)IDS_DELETESTORE_MSG), Caption, MB_YESNO | MB_DEFBUTTON2 | MB_ICONWARNING)!=IDYES)
		return;

	// Dialogbox nur zeigen, wenn der Store gemountet ist
	if (LFIsStoreMounted(&Store) && (LFDeleteStoreDlg(Store.StoreID, pParentWnd).DoModal()!=IDOK))
		return;

	WorkerParameters wp;
	ZeroMemory(&wp, sizeof(wp));
	strcpy_s(wp.StoreID, LFKeySize, Store.StoreID);

	LFDoWithProgress(WorkerStoreDelete, &wp.Hdr, pParentWnd);
	LFErrorBox(pParentWnd, wp.Result);
}
