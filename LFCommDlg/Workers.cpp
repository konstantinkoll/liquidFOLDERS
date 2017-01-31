
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
	LFProgressDlg dlg(pThreadProc, pParameters, pParentWnd);
	dlg.DoModal();
}

BOOL LFImportFolder(const CHAR* pStoreID, CWnd* pParentWnd)
{
	BOOL Result = FALSE;

	if (LFNagScreen(pParentWnd))
	{
		CString Caption((LPCSTR)IDS_IMPORTFOLDER_CAPTION);
		CString Hint((LPCSTR)IDS_IMPORTFOLDER_HINT);

		LFBrowseForFolderDlg dlg(pParentWnd, Caption, Hint, TRUE, TRUE, _T(""));
		if (dlg.DoModal()==IDOK)
		{
			WorkerParameters wp;
			ZeroMemory(&wp, sizeof(wp));

			wp.pFileImportList = LFAllocFileImportList();
			LFAddImportPath(wp.pFileImportList, dlg.m_FolderPath);

			strcpy_s(wp.StoreID, LFKeySize, pStoreID);
			wp.DeleteSource = dlg.m_DeleteSource;

			// Template füllen
			wp.pItemTemplate = LFAllocItemDescriptor();
			LFItemTemplateDlg tdlg(wp.pItemTemplate, pStoreID, pParentWnd, TRUE);
			if (tdlg.DoModal()!=IDCANCEL)
			{
				LFDoWithProgress(WorkerImport, (LFWorkerParameters*)&wp, pParentWnd);
				LFErrorBox(pParentWnd, wp.pFileImportList->m_LastError);

				Result = TRUE;
			}

			LFFreeItemDescriptor(wp.pItemTemplate);
			LFFreeFileImportList(wp.pFileImportList);
		}
	}

	return Result;
}

void LFRunSynchronize(const CHAR* pStoreID, CWnd* pParentWnd)
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

	LFStoreMaintenanceDlg dlg(wp.pMaintenanceList, pParentWnd);
	dlg.DoModal();
}

void LFDeleteStore(const CHAR* pStoreID, CWnd* pParentWnd)
{
	LFStoreDescriptor Store;
	UINT Result = LFGetStoreSettings(pStoreID, &Store);
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
