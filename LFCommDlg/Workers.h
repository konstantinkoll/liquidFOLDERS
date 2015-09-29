
// Workers.h: Schnittstelle f�r Methoden zum Threading
//

struct WorkerParameters
{
	LFWorkerParameters Hdr;
	CHAR StoreID[LFKeySize];
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
DWORD WINAPI WorkerStoreMaintenance(void* lParam);
DWORD WINAPI WorkerStoreDelete(void* lParam);
DWORD WINAPI WorkerSendTo(void* lParam);
DWORD WINAPI WorkerImport(void* lParam);
DWORD WINAPI WorkerDelete(void* lParam);

void LFDoWithProgress(LPTHREAD_START_ROUTINE pThreadProc, LFWorkerParameters* pParameters, CWnd* pParentWnd=NULL);
void LFImportFolder(CHAR* pStoreID, CWnd* pParentWnd=NULL);
void LFRunSynchronization(CHAR* pStoreID, CWnd* pParentWnd=NULL);
void LFRunMaintenance(CWnd* pParentWnd=NULL);
void LFDeleteStore(CHAR* pStoreID, CWnd* pParentWnd=NULL);
