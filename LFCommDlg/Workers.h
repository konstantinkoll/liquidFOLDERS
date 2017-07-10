
// Workers.h: Schnittstelle für Methoden zum Threading
//

struct WorkerParameters
{
	LFWorkerParameters Hdr;
	CHAR StoreID[LFKeySize];
	WCHAR StoreName[256];
	BOOL DeleteSource;
	LFItemDescriptor* pItemTemplate;
	WCHAR Path[MAX_PATH];
	union
	{
		LFFileImportList* pFileImportList;
		LFMaintenanceList* pMaintenanceList;
		LFTransactionList* pTransactionList;
		UINT Result;
	};
};

DWORD WINAPI WorkerCreateStoreWindows(void* lParam);
DWORD WINAPI WorkerSendTo(void* lParam);
DWORD WINAPI WorkerImport(void* lParam);
DWORD WINAPI WorkerDelete(void* lParam);

void LFDoWithProgress(LPTHREAD_START_ROUTINE pThreadProc, LFWorkerParameters* pParameters, CWnd* pParentWnd=NULL);
void LFRunSynchronize(const LPCSTR pStoreID, CWnd* pParentWnd=NULL);
void LFRunSynchronizeAll(CWnd* pParentWnd=NULL);
void LFRunMaintenance(CWnd* pParentWnd=NULL);
void LFDeleteStore(const LPCSTR pStoreID, CWnd* pParentWnd=NULL);
