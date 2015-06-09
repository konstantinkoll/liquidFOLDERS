
// Works.h: Schnittstelle für Methoden zum Threading
//

struct WorkerParameters
{
	LFWorkerParameters Hdr;
	CHAR StoreID[LFKeySize];
	BOOL DeleteSource;
	LFItemDescriptor* Template;
	union
	{
		LFFileIDList* FileIDList;
		LFFileImportList* FileImportList;
		LFMaintenanceList* MaintenanceList;
		LFTransactionList* TransactionList;
		UINT Result;
	};
};

DWORD WINAPI WorkerStoreMaintenance(void* lParam);
DWORD WINAPI WorkerStoreDelete(void* lParam);
DWORD WINAPI WorkerImportFromStore(void* lParam);
DWORD WINAPI WorkerImportFromWindows(void* lParam);
DWORD WINAPI WorkerDelete(void* lParam);

void LFDoWithProgress(LPTHREAD_START_ROUTINE pThreadProc, LFWorkerParameters* pParameters, CWnd* pParentWnd=NULL);
void LFImportFolder(CHAR* StoreID, CWnd* pParentWnd=NULL);
void LFRunMaintenance(CWnd* pParentWnd=NULL);
void LFDeleteStore(CHAR* StoreID, CWnd* pParentWnd=NULL);
